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
 std::cout<<"PASS: atomic join, closed rectangle, cross-contour merge, snap constraints, mixed native Bézier, undo/redo\n";
}
