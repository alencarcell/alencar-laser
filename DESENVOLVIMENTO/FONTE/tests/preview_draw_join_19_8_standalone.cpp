// Alencar Laser 19.8 — interactive drawing regression, offline only.
// g++ -std=c++20 -Wall -Wextra -O2 preview_draw_join_19_8_standalone.cpp -o draw_join.exe
#include "../src/editor_tools.hpp"
#include "../src/bezier_interaction.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace alaser;
static bool eq(Pt p,Pt q){return dist(p,q)<1e-8;}
int main(){
 // Missing one side in an open rectangle: joining D -> A must CLOSE THE
 // EXISTING contour, not add an overlapping separate line/object.
 Editor e(Document{});
 int box=e.addContour({{0,0},{30,0},{30,20},{0,20}},false);
 const auto undoBefore=e.undoCount();
 int same=e.commitJoinedPolyline({{0,20},{0,0}},false,0,1e-5);
 assert(same==box&&e.state().doc.contours.size()==1);
 assert(e.state().doc.contours.front().closed);
 assert(e.state().doc.contours.front().nodes.size()==4);
 assert(e.undoCount()==undoBefore+1);
 assert(e.undo()&&!e.state().doc.contours.front().closed);
 assert(e.redo()&&e.state().doc.contours.front().closed);
 // Reverse direction also closes the same original contour.
 assert(e.undo());
 e.commitJoinedPolyline({{0,0},{0,20}},false,0,1e-5);
 assert(e.state().doc.contours.size()==1&&e.state().doc.contours.front().closed);
 // Two separately drawn lines bridged by a new segment yield one path.
 Editor two(Document{});
 const int first=two.line({0,0},{10,0});
 const int second=two.line({20,0},{30,0});
 const auto oldHistory=two.undoCount();
 const int joined=two.commitJoinedPolyline({{10,0},{20,0}},false,0,1e-5);
 assert(joined==first);
 assert(two.state().doc.contours.size()==1);
 assert(two.state().objects.size()==1);
 assert(two.state().doc.contours.front().nodes.size()==4);
 assert(eq(two.state().doc.contours.front().nodes.back().p,{30,0}));
 assert(two.undoCount()==oldHistory+1);
 assert(two.undo()&&two.state().doc.contours.size()==2);
 assert(two.state().objects.count(second)>0);
 assert(two.redo()&&two.state().doc.contours.size()==1);
 // Explicit Alt+J joins coincident endpoints, including the common
 // imported case in which both shapes use exactly the same endpoint.
 Editor selected(Document{});
 const int s1=selected.line({0,0},{10,0});
 const int s2=selected.line({10,0},{20,0});
 const auto previousUndo=selected.undoCount();
 const int kept=selected.joinTwoOpenContours(s1,s2,0.5);
 assert(kept==s1&&selected.state().doc.contours.size()==1);
 assert(selected.state().doc.contours.front().nodes.size()==3);
 assert(selected.undoCount()==previousUndo+1);
 assert(selected.undo()&&selected.state().doc.contours.size()==2);
 // Guard against joining shapes that are too far apart or on other layers.
 Editor far(Document{});
 const int f1=far.line({0,0},{10,0});
 const int f2=far.line({20,0},{30,0});
 const auto farHistory=far.undoCount();
 bool farRejected=false;
 try{(void)far.joinTwoOpenContours(f1,f2,0.5);}
 catch(const std::runtime_error&){farRejected=true;}
 assert(farRejected&&far.undoCount()==farHistory);
 // Single endpoint extends existing polyline. Multiple points survive.
 Editor extend(Document{});
 int line=extend.line({0,0},{10,0});
 extend.commitJoinedPolyline({{10,0},{15,5},{20,10}},false,0,1e-5);
 assert(extend.state().doc.contours.size()==1&&
        extend.state().doc.contours.front().nodes.size()==4);
 assert(eq(extend.state().doc.contours.front().nodes.back().p,{20,10}));
 // Snapping ignores locked, other-layer, grouped and already closed paths.
 Editor filtered(Document{});
 const int locked=filtered.line({0,0},{10,0});
 filtered.addLayer(1,"other");
 filtered.line({30,0},{40,0},1);
 filtered.rectangle({60,0},10,10);
 assert(!nearestOpenEndpoint(filtered.state(),{30,0},0,.5));
 assert(!nearestOpenEndpoint(filtered.state(),{60,0},0,.5));
 filtered.setLayerFlags(0,true,true);
 assert(!nearestOpenEndpoint(filtered.state(),{10,0},0,.5));
 filtered.setLayerFlags(0,true,false);
 assert(nearestOpenEndpoint(filtered.state(),{10.1,0},0,.2));
 (void)locked;
 // Native mixed Line/Bezier path uses straight click anchors and smooth
 // click-drag handles; all 3 nodes are stored as one native v5 contour.
 CurveDragSession native(CurveDocument{});
 std::vector<CurveNode> pts{
  {0,{0,0},{0,0},{0,0}},
  {0,{10,0},{7,-5},{13,5}},
  {0,{20,0},{20,0},{20,0}}
 };
 const auto curveUndo=native.undoCount();
 const Key key=native.addEditablePath(pts,false,201);
 assert(key.object==201&&native.committed().contours.size()==1);
 const auto& shape=native.committed().contours.front();
 assert(shape.nodes.size()==3&&shape.edgeKind.size()==2&&
        shape.edgeKind[0]==1&&shape.edgeKind[1]==1);
 assert(native.undoCount()==curveUndo+1);
 assert(native.undo()&&native.committed().contours.empty());
 assert(native.redo()&&native.committed().contours.size()==1);
 const Key straight=native.addEditablePath(
  {{0,{30,0},{30,0},{30,0}},{0,{40,0},{40,0},{40,0}}},false,202);
 assert(straight.object==202&&native.committed().contours.back().edgeKind[0]==0);
 // Regression: an already finished native cubic must be selectable,
 // editable, cut without flattening, deletable and fully undoable.
 CurveDragSession lifecycle(CurveDocument{});
 const Key original=lifecycle.addEditablePath(
  {{0,{0,0},{0,0},{3,8}},
   {0,{10,0},{7,8},{10,0}}},false,301);
 const Cubic baseline=edgeAt(lifecycle.committed().contours.front(),0);
 assert(lifecycle.selected()&&*lifecycle.selected()==original);
 lifecycle.clearSelection();
 assert(!lifecycle.selected());
 lifecycle.select(original);
 CurveHit cut{original,0,0,CurvePart::Edge,0,0.5,{5,6}};
 assert(lifecycle.cutContourAtEdge(cut,302));
 assert(lifecycle.committed().contours.size()==2);
 const auto& left=lifecycle.committed().contours[0];
 const auto& right=lifecycle.committed().contours[1];
 assert(left.nodes.size()==2&&right.nodes.size()==2);
 assert(left.edgeKind[0]==1&&right.edgeKind[0]==1);
 assert(eq(cubicPoint(edgeAt(left,0),0.5),cubicPoint(baseline,0.25)));
 assert(eq(cubicPoint(edgeAt(right,0),0.5),cubicPoint(baseline,0.75)));
 assert(lifecycle.undo()&&lifecycle.committed().contours.size()==1);
 assert(lifecycle.redo()&&lifecycle.committed().contours.size()==2);
 assert(lifecycle.deleteSelectedContour()&&lifecycle.committed().contours.size()==1);
 assert(lifecycle.undo()&&lifecycle.committed().contours.size()==2);
 assert(lifecycle.undo()&&lifecycle.committed().contours.size()==1);
 assert(lifecycle.undo()&&lifecycle.committed().contours.empty());
 assert(!lifecycle.selected()); // no dangling selected key after undo of creation
 assert(lifecycle.redo()&&lifecycle.committed().contours.size()==1);
 CurveDragSession closedNative(CurveDocument{});
 const Key ring=closedNative.addEditablePath(
  {{0,{0,0},{0,0},{0,0}},{0,{10,0},{10,0},{10,0}},
   {0,{10,10},{10,10},{10,10}}},true,305);
 assert(closedNative.cutContourAtEdge(
  {ring,1,0,CurvePart::Edge,0,0.5,{10,5}},306));
 assert(closedNative.committed().contours.size()==1);
 assert(!closedNative.committed().contours.front().closed);
 assert(closedNative.committed().contours.front().nodes.size()==5);
 assert(closedNative.undo()&&closedNative.committed().contours.front().closed);
 std::cout<<"PASS: atomic join, closed rectangle, cross-contour merge, snap constraints, mixed native Bézier, undo/redo\n";
}
