#pragma once
// 19.9 EXPLORATORY / OFFLINE: import native LightBurn LBRN2 cubic geometry.
// This file does not replace the active editor, issue machine commands, load
// laser settings, or flatten 1 Bezier to 24 points. No silent partial import.
#include "lightburn_geometry_import.hpp"
#include "bezier_native.hpp"
#include <QFile>
#include <QXmlStreamReader>
#include <QPointF>
#include <QTransform>
#include <QRegularExpression>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <map>
#include <stdexcept>
#include <vector>

namespace alaser::lightburn {
struct NativeImport199 {
 CurveDocument curves;
 std::size_t sourceShapes=0;
 std::size_t sourcePaths=0;
 std::size_t sourceEllipses=0;
 std::size_t cubicEdges=0;
 std::size_t linearEdges=0;
 std::size_t nativeAnchors=0;
 // Ellipses are analytic conics in LightBurn: Qt's native cubic support is
 // polynomial, so only these are represented by 16 near-exact cubic arcs.
 bool ellipseApproximated=false;
};
inline Pt transformNative199(Pt p,const QTransform& transform){
 const QPointF q=transform.map(QPointF(p.x,p.y));
 if(!std::isfinite(q.x())||!std::isfinite(q.y())||
    std::abs(q.x())>1e7||std::abs(q.y())>1e7)
  throw std::runtime_error("LBRN2 native: transformed coordinate outside safe range");
 return {q.x(),q.y()};
}
inline void appendNativeContour199(NativeImport199& result,CurveContour contour){
 if(contour.nodes.size()<2||(contour.closed&&contour.nodes.size()<3))
  throw std::runtime_error("LBRN2 native: unsupported degenerate curve topology");
 if(contour.edgeKind.size()!=edgeCount(contour))
  throw std::runtime_error("LBRN2 native: edge topology mismatch");
 result.nativeAnchors+=contour.nodes.size();
 if(result.nativeAnchors>250000||result.curves.contours.size()>=30000)
  throw std::runtime_error("LBRN2 native: node or contour safety budget exceeded");
 for(auto kind:contour.edgeKind)
  if(kind==1)++result.cubicEdges;else ++result.linearEdges;
 contour.key={int(result.curves.contours.size()+1),0};
 for(auto& node:contour.nodes)node.id=result.curves.nextId++;
 result.curves.contours.push_back(std::move(contour));
}
inline CurveNode nativeVertex199(const PackedVertex& v,const QTransform& t){
 const Pt a=transformNative199(v.p,t);
 // Only real B edges activate these handles: a missing handle is the anchor.
 return {0,a,a,a};
}
inline void appendNativePath199(NativeImport199& result,const ParseNode& n,
                                const QTransform& transform){
 const auto vertices=decodeVertices(n.vertList);
 const QString prims=n.primList.trimmed();
 if(prims=="LineOpen"||prims=="LineClosed"){
  CurveContour c;c.layer=n.cutIndex;c.closed=prims=="LineClosed";
  for(const auto& vertex:vertices)c.nodes.push_back(nativeVertex199(vertex,transform));
  if(c.closed&&c.nodes.size()>=2&&dist(c.nodes.front().anchor,c.nodes.back().anchor)<1e-7)
   c.nodes.pop_back();
  c.edgeKind.assign(edgeCount(c),uint8_t(0));
  appendNativeContour199(result,std::move(c));
  return;
 }
 static const QRegularExpression primitive(R"(([LB])(\d+)\s+(\d+))");
 auto it=primitive.globalMatch(prims);
 qsizetype consumed=0;
 CurveContour current;current.layer=n.cutIndex;
 int firstIndex=-1,lastIndex=-1,runCount=0,primitiveCount=0;
 auto flush=[&](){
  if(current.edgeKind.empty())return;
  current.closed=firstIndex==lastIndex;
  if(current.closed){
   if(current.nodes.size()<4)
    throw std::runtime_error("LBRN2 native: closed path has fewer than three anchors");
   current.nodes.front().handleIn=current.nodes.back().handleIn;
   current.nodes.pop_back();
  }
  appendNativeContour199(result,std::move(current));
  current=CurveContour{};current.layer=n.cutIndex;
  firstIndex=-1;lastIndex=-1;++runCount;
  if(runCount>30000)throw std::runtime_error("LBRN2 native: excess of disconnected runs");
 };
 while(it.hasNext()){
  const auto match=it.next();
  if(!prims.mid(consumed,match.capturedStart()-consumed).trimmed().isEmpty())
   throw std::runtime_error("LBRN2 native: unexpected primitive data");
  consumed=match.capturedEnd();
  bool okA=false,okB=false;
  const int a=match.captured(2).toInt(&okA),b=match.captured(3).toInt(&okB);
  if(!okA||!okB||a<0||b<0||
     a>=int(vertices.size())||b>=int(vertices.size()))
   throw std::runtime_error("LBRN2 native: primitive index outside VertList");
  if(++primitiveCount>50000)
   throw std::runtime_error("LBRN2 native: too many primitives in one Path");
  if(firstIndex>=0&&lastIndex!=a)flush(); // NEVER invent a connecting edge
  if(firstIndex<0){
   firstIndex=a;
   current.nodes.push_back(nativeVertex199(vertices[size_t(a)],transform));
  }
  const bool cubic=match.captured(1)=="B";
  if(cubic){
   const auto& left=vertices[size_t(a)],&right=vertices[size_t(b)];
   if(!(left.hasOutX&&left.hasOutY&&right.hasInX&&right.hasInY))
    throw std::runtime_error("LBRN2 native: cubic with missing handles");
   current.nodes.back().handleOut=transformNative199(left.out,transform);
  }else current.nodes.back().handleOut=current.nodes.back().anchor;
  CurveNode end=nativeVertex199(vertices[size_t(b)],transform);
  if(cubic)end.handleIn=transformNative199(vertices[size_t(b)].in,transform);
  current.nodes.push_back(end);
  current.edgeKind.push_back(cubic?uint8_t(1):uint8_t(0));
  lastIndex=b;
  if(result.nativeAnchors+current.nodes.size()>250001U)
   throw std::runtime_error("LBRN2 native: node budget exceeded during Path");
 }
 if(!prims.mid(consumed).trimmed().isEmpty()||primitiveCount==0)
  throw std::runtime_error("LBRN2 native: unknown or empty PrimList");
 flush();
}
inline void appendNativeEllipse199(NativeImport199& result,const ParseNode& n,
                                   const QTransform& total){
 if(!(n.w>0&&n.h>0))
  throw std::runtime_error("LBRN2 native: invalid ellipse dimensions");
 // B-spline/rational conics are not supported in the current native schema.
 // 16 cubic arcs (22.5 degrees each) limit the geometric deviation to tiny
 // fractions of a millimeter for regular engraving-sized ellipses.
 constexpr int arcCount=16;
 const double halfW=n.w/2.,halfH=n.h/2.;
 const double d=2.0*std::numbers::pi/arcCount;
 const double tangent=4.0/3.0*std::tan(d/4.0);
 CurveContour c;c.layer=n.cutIndex;c.closed=true;
 for(int i=0;i<arcCount;++i){
  const double angle=d*i;
  const Pt p{halfW*std::cos(angle),halfH*std::sin(angle)};
  const Pt velocity{-halfW*std::sin(angle),halfH*std::cos(angle)};
  CurveNode node;node.anchor=transformNative199(p,total);
  node.handleOut=transformNative199(p+velocity*tangent,total);
  node.handleIn=transformNative199(p-velocity*tangent,total);
  c.nodes.push_back(node);c.edgeKind.push_back(1);
 }
 appendNativeContour199(result,std::move(c));
 result.ellipseApproximated=true;
}
inline void appendNativeShape199(NativeImport199& result,const ParseNode& n,
                                 const QTransform& parent){
 const QTransform total=n.xform*parent;
 // CurveDocument lacks native group metadata: reject rather than silently
 // forgetting source grouping. A future schema can preserve this faithfully.
 if(n.type=="Group"||!n.children.empty())
  throw std::runtime_error("LBRN2 native: group hierarchy not yet preserved");
 if(n.type=="Path"){
  ++result.sourcePaths;
  appendNativePath199(result,n,total);
 }else if(n.type=="Ellipse"){
  ++result.sourceEllipses;
  appendNativeEllipse199(result,n,total);
 }else{
  throw std::runtime_error("LBRN2 native: unsupported source shape");
 }
 ++result.sourceShapes;
}
inline NativeImport199 readNativeGeometry199(const QString& path){
 QFile file(path);
 if(!file.open(QIODevice::ReadOnly))
  throw std::runtime_error("LBRN2 native: file not accessible");
 if(file.size()>32LL*1024*1024)
  throw std::runtime_error("LBRN2 native: 32 MB file size limit exceeded");
 QXmlStreamReader xml(&file);
 if(!xml.readNextStartElement()||xml.name()!=QLatin1String("LightBurnProject"))
  throw std::runtime_error("LBRN2 native: expected LightBurnProject XML");
 const auto attrs=xml.attributes();
 for(const char* axis:{"MirrorX","MirrorY"}){
  const auto v=attrs.value(QLatin1String(axis)).toString();
  if(v.compare("True",Qt::CaseInsensitive)==0||v=="1")
   throw std::runtime_error("LBRN2 native: mirrored machine origin is not certified");
 }
 std::vector<ParseNode> shapes;
 int seen=0;
 while(xml.readNextStartElement()){
  if(xml.name()==QLatin1String("Shape"))
   shapes.push_back(readShape(xml,0,seen));
  else xml.skipCurrentElement(); // laser settings intentionally ignored
 }
 if(xml.hasError())
  throw std::runtime_error(("LBRN2 native: malformed XML: "+xml.errorString()).toStdString());
 std::map<int,QString> verts,prims;
 for(const auto& s:shapes)gatherSharedGeometry(s,verts,prims);
 for(auto& s:shapes)resolveSharedGeometry(s,verts,prims);
 NativeImport199 output;
 for(const auto& s:shapes)appendNativeShape199(output,s,QTransform());
 if(output.curves.contours.empty())
  throw std::runtime_error("LBRN2 native: no compatible geometry");
 validateCurve(output.curves);
 return output;
}
} // namespace alaser::lightburn
