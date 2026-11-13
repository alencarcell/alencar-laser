// Alencar Laser 19.8 - native Bézier scissors: actual edge removal.
// Portable/offline regression, no Qt and absolutely no machine commands.
#include "../src/bezier_interaction.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
using namespace alaser;
static void req(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static CurveNode node(double x,double y=0){return {0,{x,y},{x,y},{x,y}};}
static CurveHit edgeHit(Key key,int i){
 CurveHit hit;hit.key=key;hit.edge=i;hit.part=CurvePart::Edge;hit.t=.5;return hit;
}
int main(){
 try{
  CurveDragSession one(CurveDocument{});
  const Key k=one.addEditableCubic({0,0},{3,5},{7,-5},{10,0},1);
  req(one.committed().contours.size()==1&&
      one.committed().contours[0].edgeKind[0]==1,"cubic fixture");
  req(one.cutContourAtEdge(edgeHit(k,0),99)&&one.committed().contours.empty(),
      "single-edge cubic must be physically removed from model");
  req(one.undo()&&one.committed().contours.size()==1&&
      one.committed().contours[0].edgeKind[0]==1,"undo exact cubic");
  req(one.redo()&&one.committed().contours.empty(),"redo removal");
  CurveDragSession open(CurveDocument{});
  const Key line=open.addEditablePath({node(0),node(10),node(20),node(30)},false,2);
  req(open.cutContourAtEdge(edgeHit(line,1),55),"interior edge cut");
  const auto& parts=open.committed().contours;
  req(parts.size()==2&&parts[0].key==line&&parts[1].key.object==55,
      "two independently editable remaining paths");
  req(parts[0].nodes.size()==2&&parts[1].nodes.size()==2&&
      parts[0].edgeKind.size()==1&&parts[1].edgeKind.size()==1,
      "no artificial bridge between retained parts");
  req(std::abs(parts[0].nodes.back().anchor.x-10)<1e-9&&
      std::abs(parts[1].nodes.front().anchor.x-20)<1e-9,
      "removed ten-millimetre interval was not restored");
  req(open.undo()&&open.committed().contours.size()==1&&
      open.committed().contours[0].edgeKind.size()==3,"open undo");
  req(open.redo()&&open.committed().contours.size()==2,"open redo");
  CurveDragSession ring(CurveDocument{});
  const Key closed=ring.addEditablePath(
      {node(0,0),node(10,0),node(10,10),node(0,10)},true,3);
  req(ring.cutContourAtEdge(edgeHit(closed,3),88),"closed edge cut");
  const auto& c=ring.committed().contours.at(0);
  req(!c.closed&&c.nodes.size()==4&&c.edgeKind.size()==3,
      "closed edge must open with one actual gap");
  req(std::abs(c.nodes.front().anchor.x)<1e-9&&
      std::abs(c.nodes.back().anchor.y-10)<1e-9,
      "closed ring lost ordered surviving geometry");
  req(ring.undo()&&ring.committed().contours.at(0).closed,
      "undo closes original ring");
  req(!ring.cutContourAtEdge(edgeHit(closed,-1),88),"invalid index rejected");
  req(!ring.cutContourAtEdge(edgeHit({999,0},0),88),"missing curve rejected");
  const Cubic rising{{0,0},{3,0},{7,10},{10,10}};
  const Cubic falling{{0,10},{3,10},{7,0},{10,0}};
  const auto crossings=findCubicIntersections(rising,falling,0.02);
  req(!crossings.empty(),"crossing cubic edges must be detected");
  req(std::abs(crossings.front().point.x-5.0)<0.08&&
      std::abs(crossings.front().point.y-5.0)<0.08,
      "intersection point must be refined on native cubics");
  // Two foreign paths delimit a removable native span on one target edge.
  CurveDragSession nativeSpan(CurveDocument{});
  const Key target=nativeSpan.addEditablePath({node(0,0),node(10,0)},false,101);
  nativeSpan.addEditablePath({node(3,-2),node(3,2)},false,102);
  nativeSpan.addEditablePath({node(7,-2),node(7,2)},false,103);
  CurveHit cursor=edgeHit(target,0);cursor.t=.5;cursor.point={5,0};
  const auto planned=findNativeScissorSpan(nativeSpan.committed(),cursor);
  req(planned.has_value(),"two crossings must delimit native scissor span");
  std::cerr<<"DEBUG planned start="<<planned->startEdge<<":"<<planned->startT
           <<" end="<<planned->endEdge<<":"<<planned->endT
           <<" closed="<<planned->closed<<"\\n";
  req(planned->startEdge==0&&planned->endEdge==0&&
      std::abs(planned->startT-.3)<1e-4&&std::abs(planned->endT-.7)<1e-4,
      "linear crossing parameters must be exact at 0.3 and 0.7");
  const auto preview=nativeScissorSpanGeometry(nativeSpan.committed(),*planned);
  req(preview.size()==1&&std::abs(preview.front().first.p0.x-3)<0.02&&
      std::abs(preview.front().first.p3.x-7)<0.02,
      "preview must contain only the intersection-delimited segment");
  CurveHit oneCross=edgeHit(target,0);oneCross.t=.2;oneCross.point={2,0};
  CurveDocument oneCrossDoc;
  oneCrossDoc.contours.push_back(nativeSpan.committed().contours[0]);
  oneCrossDoc.contours.push_back(nativeSpan.committed().contours[1]);
  oneCrossDoc.nextId=nativeSpan.committed().nextId;
  req(!findNativeScissorSpan(oneCrossDoc,oneCross),
      "one crossing is insufficient for a safe cut");
  req(nativeSpan.cutContourSpan(*planned,104),"native intersection span cut");
  req(nativeSpan.committed().contours.size()==4,"native span creates two independent remnants");
  const CurveContour* left=nullptr;const CurveContour* right=nullptr;
  for(const auto& curve:nativeSpan.committed().contours){
   if(curve.key.object==101)left=&curve;
   if(curve.key.object==104)right=&curve;
  }
  req(left&&right&&left->nodes.size()==2&&right->nodes.size()==2,
      "native span keeps two separate open paths");
  req(std::abs(left->nodes.back().anchor.x-3.0)<0.02&&
      std::abs(right->nodes.front().anchor.x-7.0)<0.02,
      "native span removes only the between-crossings geometry");
  req(nativeSpan.undo()&&nativeSpan.committed().contours.size()==3,
      "native intersection span undo restores original");
  req(nativeSpan.redo()&&nativeSpan.committed().contours.size()==4,
      "native intersection span redo reapplies cut");

  std::cout<<"PASS: native cuts, cubic intersections and native span planning; no hardware\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;
 }
}
