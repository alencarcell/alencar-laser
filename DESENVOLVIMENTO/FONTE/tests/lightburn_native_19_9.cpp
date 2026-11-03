#include "../src/lightburn_native_import_19_9.hpp"
#include "../src/integrated_project_edit.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QTemporaryDir>
#include <cmath>
#include <bit>
#include <cstdint>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace alaser;
static void need(bool condition,const char* error){
 if(!condition)throw std::runtime_error(error);
}
static QString fileWithShapes(const QString& path,const QString& shapes){
 QFile f(path);
 if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate))
  throw std::runtime_error("Could not create local fixture");
 const QByteArray xml=(QStringLiteral("<LightBurnProject MirrorX=\"False\" MirrorY=\"False\">")+
                       shapes+QStringLiteral("</LightBurnProject>")).toUtf8();
 need(f.write(xml)==xml.size(),"Failed to write local fixture");
 return path;
}
static void near(double a,double b,const char* reason){
 need(std::abs(a-b)<1e-8,reason);
}
static bool exactPoint(Pt a,Pt b){
 return std::bit_cast<std::uint64_t>(a.x)==std::bit_cast<std::uint64_t>(b.x)&&
        std::bit_cast<std::uint64_t>(a.y)==std::bit_cast<std::uint64_t>(b.y);
}
static void verifyNativeCodecRoundTrip(const CurveDocument& source){
 // The deployed .alaser v5 uses encodeCurves()/decodeCurves(): exercise the
 // actual serialized schema, not only a shallow copy of C++ objects.
 const auto wire=serialize(encodeCurves(source));
 const auto restored=decodeCurves(parse(wire).as_object());
 need(restored.nextId==source.nextId,"Bezier v5 next-id lost in codec");
 need(restored.contours.size()==source.contours.size(),
      "Bezier v5 contour count lost in codec");
 for(size_t i=0;i<source.contours.size();++i){
  const auto& a=source.contours[i];const auto& b=restored.contours[i];
  need(a.key==b.key&&a.closed==b.closed&&a.layer==b.layer,
       "Bezier v5 contour identity, closure or layer lost");
  need(a.edgeKind==b.edgeKind&&a.nodes.size()==b.nodes.size(),
       "Bezier v5 edge types or node count changed");
  for(size_t j=0;j<a.nodes.size();++j){
   const auto& x=a.nodes[j];const auto& y=b.nodes[j];
   need(x.id==y.id&&exactPoint(x.anchor,y.anchor)&&
        exactPoint(x.handleIn,y.handleIn)&&exactPoint(x.handleOut,y.handleOut),
        "Bezier v5 floating point geometry modified by codec");
  }
 }
}
static void verifyNativeAlaserZipRoundTrip(const CurveDocument& original,
                                          const QString& outputFile){
 // End-to-end package format (.alaser v5): encode into an independent,
 // temporary, non-fabrication project. Never writes over the user's .lbrn2.
 const EditorState empty=fromDocument(Document{});
 const auto project=buildEditedIntegratedProject(empty,original,nullptr);
 need(project.version==5,"Native package schema incorrect");
 writeIntegratedProject(outputFile.toStdString(),project);
 const auto reopened=readIntegratedProject(outputFile.toStdString());
 need(reopened.version==5,"Native ZIP version was lost");
 need(reopened.legacy.doc.contours.empty(),
      "Native ZIP unexpectedly added polygon contours");
 need(reopened.legacy.operations.is_object()&&
      reopened.legacy.operations.as_object().at("fabricationEnabled").as_bool()==false,
      "Machine fabrication was enabled by native ZIP codec");
 verifyNativeCodecRoundTrip(reopened.curves);
 need(serialize(encodeCurves(original))==serialize(encodeCurves(reopened.curves)),
      "Native ZIP exact geometry differs from initial source");
}
int main(int argc,char** argv){
 QCoreApplication app(argc,argv);
 try{
  QTemporaryDir dir;need(dir.isValid(),"No temporary test directory");
  const QString cubic=QStringLiteral(
    "<Shape Type=\"Path\" CutIndex=\"2\" VertID=\"1\" PrimID=\"1\">"
    "<XForm>2 0 0 2 10 20</XForm>"
    "<VertList>"
    "V0 0c0x1c0y0c1x-1c1y0"
    "V3 0c0x3c0y1c1x2c1y0"
    "V3 3c0x2c0y3c1x4c1y3"
    "</VertList><PrimList>B0 1B1 2B2 0</PrimList></Shape>");
  const QString lines=QStringLiteral(
    "<Shape Type=\"Path\" CutIndex=\"0\" VertID=\"2\" PrimID=\"2\">"
    "<XForm>1 0 0 1 0 0</XForm>"
    "<VertList>V0 0V1 0V1 1V0 1</VertList>"
    "<PrimList>LineClosed</PrimList></Shape>");
  const QString ellipse=QStringLiteral(
    "<Shape Type=\"Ellipse\" CutIndex=\"1\" Rx=\"1\" Ry=\"2\">"
    "<XForm>1 0 0 1 30 40</XForm></Shape>");
  const auto synthetic=fileWithShapes(dir.filePath("native.lbrn2"),cubic+lines+ellipse);
  auto imported=lightburn::readNativeGeometry199(synthetic);
  need(imported.sourceShapes==3&&imported.sourcePaths==2&&imported.sourceEllipses==1,
       "Source shape count changed");
  need(imported.curves.contours.size()==3,"Native curve contours missing");
  need(imported.nativeAnchors==23,"Expected 3+4+16 native nodes");
  need(imported.cubicEdges==19&&imported.linearEdges==4,
       "Native edge counts changed");
  validateCurve(imported.curves);
  verifyNativeCodecRoundTrip(imported.curves);
  verifyNativeAlaserZipRoundTrip(imported.curves,dir.filePath("synthetic_native.alaser"));
  const auto& c=imported.curves.contours.at(0);
  need(c.closed&&c.layer==2&&c.nodes.size()==3&&c.edgeKind.size()==3,
       "Closed native cubic topology changed");
  near(c.nodes[0].anchor.x,10.,"XForm anchor.x failed");
  near(c.nodes[0].anchor.y,20.,"XForm anchor.y failed");
  near(c.nodes[0].handleOut.x,12.,"XForm outgoing handle failed");
  near(c.nodes[1].handleIn.x,14.,"XForm incoming handle failed");
  const auto mid=cubicPoint(edgeAt(c,0),.5);
  near(mid.x,13.,"Exact Bezier evaluation changed");
  const auto& e=imported.curves.contours.at(2);
  need(e.closed&&e.nodes.size()==16&&e.layer==1,"Ellipse approximation invalid");
  near(e.nodes[0].anchor.x,31.,"Ellipse Rx failed");
  near(e.nodes[0].anchor.y,40.,"Ellipse translated center failed");

  // Shared PrimID and VertID may be defined on a later Shape; resolve after
  // parsing the ENTIRE file, never create artificial connections.
  const QString shared=fileWithShapes(dir.filePath("shared.lbrn2"),
    QStringLiteral("<Shape Type=\"Path\" CutIndex=\"0\" VertID=\"4\" PrimID=\"5\">"
      "<XForm>1 0 0 1 0 0</XForm></Shape>"
      "<Shape Type=\"Path\" CutIndex=\"0\" VertID=\"4\" PrimID=\"5\">"
      "<VertList>V0 0V1 0V1 1V0 1</VertList>"
      "<PrimList>LineClosed</PrimList></Shape>"));
  const auto reused=lightburn::readNativeGeometry199(shared);
  need(reused.curves.contours.size()==2&&reused.linearEdges==8,
       "Forward-shared geometry references are not preserved");

  const QString group=fileWithShapes(dir.filePath("group.lbrn2"),
    QStringLiteral("<Shape Type=\"Group\" CutIndex=\"0\">"
      "<Children><Shape Type=\"Ellipse\" CutIndex=\"0\" Rx=\"1\" Ry=\"1\"/>"
      "</Children></Shape>"));
  bool refusedGroup=false;
  try{(void)lightburn::readNativeGeometry199(group);}
  catch(const std::exception& ex){
   refusedGroup=std::string(ex.what()).find("group hierarchy")!=std::string::npos;
  }
  need(refusedGroup,"Unsafe flattening of native group was accepted");

  if(argc>1){
   const QString original=QString::fromLocal8Bit(argv[1]);
   const auto start=std::chrono::steady_clock::now();
   auto actual=lightburn::readNativeGeometry199(original);
   const auto parseMs=std::chrono::duration<double,std::milli>(
      std::chrono::steady_clock::now()-start).count();
   validateCurve(actual.curves);
   verifyNativeCodecRoundTrip(actual.curves);
   verifyNativeAlaserZipRoundTrip(actual.curves,dir.filePath("real_native_roundtrip.alaser"));
   need(actual.sourceShapes==2134&&actual.sourcePaths==2133&&actual.sourceEllipses==1,
        "Original LightBurn shape count unexpected");
   need(actual.curves.contours.size()==2134&&actual.nativeAnchors==17947,
        "Original LightBurn contour/anchor count mismatch");
   need(actual.cubicEdges==16417&&actual.linearEdges==1530,
        "Original LightBurn cubic/line edge count mismatch");
   std::cout<<"REAL LBRN2: parse_ms="<<parseMs<<" roundtrip_exact=true sourceShapes="<<actual.sourceShapes
            <<" nativeContours="<<actual.curves.contours.size()
            <<" nativeAnchors="<<actual.nativeAnchors
            <<" cubicEdges="<<actual.cubicEdges
            <<" straightEdges="<<actual.linearEdges
            <<" ellipsesApproximated="<<actual.ellipseApproximated<<"\n";
  }
  std::cout<<"PASS native LBRN2 19.9: cubic controls, shared refs, native v5 ZIP exact reopen, fabrication off\n";
  return 0;
 }catch(const std::exception& ex){
  std::cerr<<"FAIL native LBRN2 19.9: "<<ex.what()<<"\n";
  return 1;
 }
}
