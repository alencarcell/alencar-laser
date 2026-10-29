#pragma once
// TESTE_14 - hit testing pixel-aware + drag transacional de curvas nativas.
// Nenhum comando GRBL; nenhuma GUI necessaria para validar o nucleo.
#include "bezier_native.hpp"
#include <optional>
#include <set>
#include <limits>
#include <tuple>
#include <cmath>
#include <memory>

namespace alaser {
enum class CurvePart : uint8_t { Anchor, HandleIn, HandleOut, Edge };
struct CurveHit {
 Key key{};
 int edge=-1;
 int64_t nodeId=0;
 CurvePart part=CurvePart::Edge;
 double distancePx=std::numeric_limits<double>::infinity();
 double t=0;
 Pt point{};
};

struct CubicSample {Pt p{};double t=0;};
inline void sampleCubicRec(Cubic c,double t0,double t1,double tol,int depth,std::vector<CubicSample>& out){
 double flat=std::max(pointLineDistance(c.p1,c.p0,c.p3),pointLineDistance(c.p2,c.p0,c.p3));
 if(flat<=tol||depth>=24){out.push_back({c.p3,t1});return;}
 Pt a=lerp(c.p0,c.p1,.5),b=lerp(c.p1,c.p2,.5),d=lerp(c.p2,c.p3,.5);
 Pt ab=lerp(a,b,.5),bd=lerp(b,d,.5),m=lerp(ab,bd,.5);double mid=(t0+t1)*.5;
 sampleCubicRec({c.p0,a,ab,m},t0,mid,tol,depth+1,out);
 sampleCubicRec({m,bd,d,c.p3},mid,t1,tol,depth+1,out);
}
inline std::vector<CubicSample> sampleCubic(Cubic c,double toleranceMm){
 if(!(toleranceMm>0)||!std::isfinite(toleranceMm))throw std::invalid_argument("invalid cubic sample tolerance");
 std::vector<CubicSample> out{{c.p0,0}};
 sampleCubicRec(c,0,1,toleranceMm,0,out);return out;
}
inline std::pair<double,double> nearestEdgeApprox(Cubic c,bool cubic,Pt at,double toleranceMm,double* tOut=nullptr,Pt* closestOut=nullptr){
 auto pts=cubic?sampleCubic(c,toleranceMm):std::vector<CubicSample>{{c.p0,0},{c.p3,1}};
 double best=std::numeric_limits<double>::infinity(),bestT=0;Pt bestPos{};
 for(size_t i=1;i<pts.size();i++){
  Pt v=pts[i].p-pts[i-1].p;double denom=norm2(v);
  double frac=denom>0?std::clamp(((at.x-pts[i-1].p.x)*v.x+(at.y-pts[i-1].p.y)*v.y)/denom,0.,1.):0.;
  Pt pos=pts[i-1].p+v*frac;double d=dist(at,pos);double t=pts[i-1].t+(pts[i].t-pts[i-1].t)*frac;
  if(d<best){best=d;bestT=t;bestPos=pos;}
 }
 if(tOut)*tOut=bestT;
 if(closestOut)*closestOut=bestPos;
 return {best,bestT};
}
inline double physicalPixelScale(double pxPerMm,double devicePixelRatio){
 if(!(pxPerMm>0)||!std::isfinite(pxPerMm)||!(devicePixelRatio>0)||!std::isfinite(devicePixelRatio))
  throw std::invalid_argument("invalid display scale");
 return pxPerMm*devicePixelRatio;
}
inline std::vector<CurveHit> pickCurveEdges(const CurveDocument& doc,Pt click,double pxPerMm,double dpr,double pickRadiusPhysicalPx=6,const std::vector<std::pair<size_t,size_t>>* candidates=nullptr){
 const double scale=physicalPixelScale(pxPerMm,dpr);
 if(!(pickRadiusPhysicalPx>0)||!std::isfinite(pickRadiusPhysicalPx))throw std::invalid_argument("invalid click radius");
 const double tol=pickRadiusPhysicalPx/scale;
 // Amostragem adaptativa com tolerancia menor que 0.1 pixel fisico.
 const double curveSampleTol=std::max(1e-8,0.08/scale);
 std::map<Key,CurveHit> bestByContour;
 auto evaluateEdge=[&](const CurveContour& contour,size_t i){
   const Cubic c=edgeAt(contour,i);
   // Bounding box dos quatro controles inclui a curva verdadeira.
   const double minX=std::min({c.p0.x,c.p1.x,c.p2.x,c.p3.x});
   const double maxX=std::max({c.p0.x,c.p1.x,c.p2.x,c.p3.x});
   const double minY=std::min({c.p0.y,c.p1.y,c.p2.y,c.p3.y});
   const double maxY=std::max({c.p0.y,c.p1.y,c.p2.y,c.p3.y});
   if(click.x<minX-tol||click.x>maxX+tol||click.y<minY-tol||click.y>maxY+tol)return;
   double t;Pt projection;
   double mm=nearestEdgeApprox(c,contour.edgeKind[i]==1,click,curveSampleTol,&t,&projection).first;
   double px=mm*scale;
   if(px>pickRadiusPhysicalPx+1e-8)return;
   CurveHit hit{contour.key,int(i),0,CurvePart::Edge,px,t,projection};
   auto it=bestByContour.find(contour.key);
   if(it==bestByContour.end()||std::tie(hit.distancePx,hit.edge)<std::tie(it->second.distancePx,it->second.edge))bestByContour[contour.key]=hit;
 };
 if(candidates){for(auto [ci,ei]:*candidates)evaluateEdge(doc.contours.at(ci),ei);}
 else for(const auto& contour:doc.contours)for(size_t i=0;i<edgeCount(contour);i++)evaluateEdge(contour,i);
 std::vector<CurveHit> result;for(auto&[_,hit]:bestByContour)result.push_back(hit);
 std::sort(result.begin(),result.end(),[](const CurveHit&a,const CurveHit&b){
  return std::tie(a.distancePx,a.key,a.edge)<std::tie(b.distancePx,b.key,b.edge);
 });
 return result;
}
// Owns its snapshot and BVH backing storage; copying/moving would invalidate BVH pointers.
class CurveSpatialIndex {
 CurveDocument document_;
 std::vector<Segment> boxes_;
 std::vector<std::pair<size_t,size_t>> edges_;
 SegmentBVH bvh_;
public:
 explicit CurveSpatialIndex(const CurveDocument& doc):document_(doc){
  validateCurve(document_);
  for(size_t ci=0;ci<doc.contours.size();ci++)for(size_t ei=0;ei<edgeCount(doc.contours[ci]);ei++){
   auto c=edgeAt(doc.contours[ci],ei);
   boxes_.push_back({doc.contours[ci].key,int(ei),c.p0,c.p3,
    std::min({c.p0.x,c.p1.x,c.p2.x,c.p3.x}),std::max({c.p0.x,c.p1.x,c.p2.x,c.p3.x}),
    std::min({c.p0.y,c.p1.y,c.p2.y,c.p3.y}),std::max({c.p0.y,c.p1.y,c.p2.y,c.p3.y})});
   edges_.push_back({ci,ei});
  }
  bvh_.reset(boxes_);
 }
 CurveSpatialIndex(const CurveSpatialIndex&)=delete;
 CurveSpatialIndex& operator=(const CurveSpatialIndex&)=delete;
 std::vector<CurveHit> hits(Pt at,double zoom,double dpr,double radius=6,size_t* count=nullptr)const{
  if(!(radius>0)||!std::isfinite(radius))throw std::invalid_argument("invalid click radius");
  auto ids=bvh_.candidateIds(at,radius/physicalPixelScale(zoom,dpr));
  std::vector<std::pair<size_t,size_t>> candidates;candidates.reserve(ids.size());
  for(auto id:ids)candidates.push_back(edges_.at(id));
  if(count)*count=candidates.size();
  return pickCurveEdges(document_,at,zoom,dpr,radius,&candidates);
 }
};
// Handles of the SELECTED contour only. A click cannot silently change another object.
inline std::optional<CurveHit> pickCurveNode(const CurveDocument& doc,Key active,Pt click,double pxPerMm,double dpr,double radiusPx=7){
 double scale=physicalPixelScale(pxPerMm,dpr);
 const CurveContour* contour=nullptr;
 for(const auto& c:doc.contours)if(c.key==active){contour=&c;break;}
 if(!contour)return std::nullopt;
 std::optional<CurveHit> best;
 for(const auto& n:contour->nodes){
  for(const auto& p:std::array<std::pair<CurvePart,Pt>,3>{{{CurvePart::Anchor,n.anchor},{CurvePart::HandleIn,n.handleIn},{CurvePart::HandleOut,n.handleOut}}}){
   // Anchor wins any coincidence with its handles.
   double distance=dist(p.second,click)*scale;
   if(distance>radiusPx)continue;
   CurveHit candidate{active,-1,n.id,p.first,distance,0,p.second};
   auto order=[](CurvePart p){return p==CurvePart::Anchor?0:p==CurvePart::HandleIn?1:2;};
   if(!best||distance<best->distancePx-1e-7||
      (std::abs(distance-best->distancePx)<=1e-7&&order(p.first)<order(best->part)))best=candidate;
  }
 }
 return best;
}
inline void setCurvePart(CurveDocument& doc,const CurveHit& target,Pt targetPosition){
 if(!std::isfinite(targetPosition.x)||!std::isfinite(targetPosition.y))throw std::invalid_argument("non-finite drag");
 for(auto& c:doc.contours)if(c.key==target.key){
  for(auto& n:c.nodes)if(n.id==target.nodeId){
   if(target.part==CurvePart::Anchor){Pt delta=targetPosition-n.anchor;n.anchor=targetPosition;n.handleIn=n.handleIn+delta;n.handleOut=n.handleOut+delta;}
   else if(target.part==CurvePart::HandleIn)n.handleIn=targetPosition;
   else if(target.part==CurvePart::HandleOut)n.handleOut=targetPosition;
   else throw std::invalid_argument("cannot drag entire edge as a node");
   return;
  }
 }
 throw std::runtime_error("drag target does not exist");
}

class CurveDragSession {
 CurveHistory history_;
 std::unique_ptr<CurveSpatialIndex> index_;
 std::optional<CurveDocument> preview_;
 std::optional<CurveHit> dragging_;
 std::optional<Key> contourMoving_;
 Pt contourStart_{};
 std::optional<Key> selected_;
 Pt startPosition_{};
public:
 explicit CurveDragSession(CurveDocument doc):history_(std::move(doc)){}
 const CurveDocument& committed()const{return history_.document();}
 const CurveDocument& visible()const{return preview_?*preview_:history_.document();}
 std::optional<Key> selected()const{return selected_;}
 bool dragging()const{return dragging_.has_value()||contourMoving_.has_value();}
 // Draw Lines interaction: corner nodes are clicks, smooth nodes derive
 // symmetric tangents from click-and-drag. One atomic history entry.
 Key addEditablePath(std::vector<CurveNode> points,bool closed,int objectId){
  if(dragging()||objectId<=0||points.size()<2||points.size()>20000||
     (closed&&points.size()<3))
   throw std::runtime_error("invalid native path creation");
  for(const auto& n:points){
   for(Pt p:{n.anchor,n.handleIn,n.handleOut})
    if(!std::isfinite(p.x)||!std::isfinite(p.y))
     throw std::runtime_error("nonfinite native path point");
  }
  Key key{objectId,0};
  for(const auto& old:committed().contours)
   if(old.key==key)throw std::runtime_error("native path ID already used");
  history_.change([&](CurveDocument& d){
   CurveContour contour;contour.key=key;contour.closed=closed;
   for(auto p:points){
    p.id=d.nextId++;
    contour.nodes.push_back(p);
   }
   const std::size_t edges=contour.nodes.size()-1+(closed?1:0);
   for(std::size_t i=0;i<edges;++i){
    const auto& a=contour.nodes[i];
    const auto& b=contour.nodes[(i+1)%contour.nodes.size()];
    const bool curve=dist(a.handleOut,a.anchor)>1e-8||
                     dist(b.handleIn,b.anchor)>1e-8;
    contour.edgeKind.push_back(curve?1:0);
   }
   d.contours.push_back(std::move(contour));
  });
  selected_=key;index_.reset();
  return key;
 }
 // Nova curva cúbica nativa com âncoras/alças e Ctrl+Z na mesma sessão.
 Key addEditableCubic(Pt a,Pt out,Pt in,Pt b,int objectId){
  if(dragging()||objectId<=0||dist(a,b)<1e-6)
   throw std::runtime_error("invalid cubic creation");
  const Key key{objectId,0};
  history_.change([&](CurveDocument& d){
   for(const auto& c:d.contours)if(c.key==key)
    throw std::runtime_error("duplicate native curve object");
   CurveContour c;c.key=key;c.closed=false;c.edgeKind={1};
   c.nodes.push_back({d.nextId++,a,a,out});
   c.nodes.push_back({d.nextId++,b,in,b});
   d.contours.push_back(std::move(c));
  });
  selected_=key;index_.reset();return key;
 }
 void select(Key key){
  if(dragging())throw std::runtime_error("cannot select while dragging");
  for(auto& c:history_.document().contours)if(c.key==key){selected_=key;return;}
  throw std::runtime_error("selection key not found");
 }
 bool start(const CurveHit& h){
  if(dragging()||!selected_||*selected_!=h.key||h.part==CurvePart::Edge)return false;
  for(auto& c:history_.document().contours)if(c.key==h.key)for(auto& n:c.nodes)if(n.id==h.nodeId){
   startPosition_=(h.part==CurvePart::Anchor?n.anchor:h.part==CurvePart::HandleIn?n.handleIn:n.handleOut);
   dragging_=h;preview_=history_.document();return true;
  }
  return false;
 }
 void update(Pt target){
  if(!dragging_)throw std::runtime_error("no drag active");
  preview_=history_.document();setCurvePart(*preview_,*dragging_,target);validateCurve(*preview_);
 }
 std::vector<CurveHit> pickEdges(Pt at,double zoom,double dpr,double radius=6){
  if(!index_)index_=std::make_unique<CurveSpatialIndex>(committed());
  return index_->hits(at,zoom,dpr,radius);
 }
 bool commit(){
  if(!dragging_)return false;
  const CurveHit target=*dragging_;
  const auto& view=visible();
  Pt destination=startPosition_;
  for(const auto& c:view.contours)if(c.key==target.key)for(const auto& n:c.nodes)if(n.id==target.nodeId)
   destination=(target.part==CurvePart::Anchor?n.anchor:target.part==CurvePart::HandleIn?n.handleIn:n.handleOut);
  bool changed=dist(startPosition_,destination)>0;
  // Transaction stores one history entry regardless of number of mousemove events.
  if(changed){history_.change([&](CurveDocument& d){setCurvePart(d,target,destination);});index_.reset();}
  dragging_.reset();preview_.reset();return changed;
 }
 void cancel(){preview_.reset();dragging_.reset();contourMoving_.reset();}
 bool undo(){if(dragging())cancel();bool changed=history_.undo();if(changed)index_.reset();return changed;}
 bool redo(){if(dragging())cancel();bool changed=history_.redo();if(changed)index_.reset();return changed;}
 size_t undoCount()const{return history_.undoCount();}
 size_t redoCount()const{return history_.redoCount();}
 // Inserção de âncora no parâmetro t da aresta atingida (0<t<1). Retorna o id do novo nó.
 int64_t insertAnchor(const CurveHit& edgeHit,double t){
  if(dragging())throw std::runtime_error("cannot insert while dragging");
  if(edgeHit.part!=CurvePart::Edge||!(t>0&&t<1)||!std::isfinite(t))
   throw std::invalid_argument("insertAnchor requires Edge hit and open parameter t");
  int64_t created=0;
  history_.change([&](CurveDocument& d){
   size_t ci=0;bool found=false;
   for(;ci<d.contours.size();++ci)if(d.contours[ci].key==edgeHit.key){found=true;break;}
   if(!found)throw std::runtime_error("insert target contour missing");
   created=splitEdge(d,ci,size_t(edgeHit.edge),t);
  });
  index_.reset();
  return created;
 }
 // Remove âncora do contorno selecionado.
 // Delega a removeCurveNode (política: ponte reta sobre o nó removido).
 bool deleteAnchor(int64_t nodeId){
  if(dragging())throw std::runtime_error("cannot delete while dragging");
  if(!selected_)return false;
  history_.change([&](CurveDocument& d){
   removeCurveNode(d,*selected_,nodeId);
  });
  index_.reset();
  return true;
 }
 // Move um contorno inteiro (âncoras E alças) com uma única entrada no histórico.
 // A prévia não altera o documento confirmado nem o índice espacial.
 bool startContourMove(Key key,Pt at){
  if(dragging()||!std::isfinite(at.x)||!std::isfinite(at.y))return false;
  bool found=false;
  for(const auto& c:committed().contours)if(c.key==key){found=true;break;}
  if(!found)return false;
  selected_=key;contourMoving_=key;contourStart_=at;preview_=committed();return true;
 }
 void updateContourMove(Pt at){
  if(!contourMoving_||!std::isfinite(at.x)||!std::isfinite(at.y))throw std::runtime_error("no curve contour move");
  const Pt delta=at-contourStart_;
  preview_=committed();
  for(auto& c:preview_->contours)if(c.key==*contourMoving_)for(auto& n:c.nodes){
   n.anchor=n.anchor+delta;n.handleIn=n.handleIn+delta;n.handleOut=n.handleOut+delta;
  }
  validateCurve(*preview_);
 }
 bool commitContourMove(Pt at){
  if(!contourMoving_)return false;
  if(!std::isfinite(at.x)||!std::isfinite(at.y))throw std::runtime_error("invalid curve destination");
  const Key key=*contourMoving_;const Pt delta=at-contourStart_;
  const bool changed=dist(Pt{},delta)>1e-10;
  if(changed){
   history_.change([&](CurveDocument& d){for(auto& c:d.contours)if(c.key==key)for(auto& n:c.nodes){
     n.anchor=n.anchor+delta;n.handleIn=n.handleIn+delta;n.handleOut=n.handleOut+delta;
   }});
   index_.reset();
  }
  contourMoving_.reset();preview_.reset();return changed;
 }
 bool deleteSelectedContour(){
  if(dragging()||!selected_)return false;
  const Key key=*selected_;
  history_.change([&](CurveDocument& d){
   auto it=std::find_if(d.contours.begin(),d.contours.end(),[&](const CurveContour& c){return c.key==key;});
   if(it==d.contours.end())throw std::runtime_error("curve not found");
   d.contours.erase(it);
  });
  selected_.reset();index_.reset();return true;
 }
 void replaceDocument(CurveDocument doc){
  if(dragging())cancel();
  history_=CurveHistory(std::move(doc));
  selected_.reset();
  index_.reset();
 }
};
}
