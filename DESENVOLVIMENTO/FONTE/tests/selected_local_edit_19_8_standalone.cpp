// Localized editing regression, TESTE_19.8. Core-only, offline and no hardware.
// Removing one node or edge of the ACTIVE object must never delete neighbors.
#include "../src/editor_tools.hpp"
#include "../src/bezier_interaction.hpp"
#include "../src/diagnostics_collector.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace alaser;
static void require(bool pass,const char* message){if(!pass)throw std::runtime_error(message);}
static CurveNode point(double x,double y){return {0,{x,y},{x,y},{x,y}};}
int main(){
 try{
  Editor ed(Document{});
  const int target=ed.rectangle({0,0},20,10);
  const int untouched=ed.rectangle({60,0},15,15);
  const auto initial=ed.state().doc;
  const Key key{target,0},neighbor{untouched,0};
  const int64_t node=get(ed.state().doc,key)->nodes.at(1).id;
  ed.erase(key,node);
  require(get(ed.state().doc,key)->nodes.size()==3,
          "delete only one node of selected rectangle");
  require(get(ed.state().doc,neighbor)->nodes.size()==4&&
          ed.state().doc.contours.size()==2,"node deletion affected another object");
  require(ed.undo()&&sameDoc(ed.state().doc,initial),"node Undo not exact");
  require(ed.redo()&&get(ed.state().doc,key)->nodes.size()==3,"node Redo failed");
  require(ed.undo()&&sameDoc(ed.state().doc,initial),"node Undo after Redo failed");
  ed.removePolylineEdge(key,1);
  const auto* opened=get(ed.state().doc,key);
  require(opened&&!opened->closed&&opened->nodes.size()==4,
          "single selected side of closed rectangle must open only that shape");
  require(get(ed.state().doc,neighbor)->closed&&
          get(ed.state().doc,neighbor)->nodes.size()==4,
          "side deletion affected the other rectangle");
  require(ed.undo()&&sameDoc(ed.state().doc,initial),"edge Undo not exact");
  require(ed.redo()&&!get(ed.state().doc,key)->closed,"edge Redo failed");
  require(ed.undo()&&sameDoc(ed.state().doc,initial),"edge second Undo failed");
  bool rejected=false;
  try{ed.removePolylineEdge(key,999);}catch(const std::exception&){rejected=true;}
  require(rejected&&sameDoc(ed.state().doc,initial),
          "invalid side operation must have no partial effects");
  Editor openEd(Document{});
  const int openId=openEd.addContour(
      {{0,0},{5,0},{10,0},{15,0},{20,0}},false);
  const int otherId=openEd.rectangle({90,90},3,3);
  const auto preCut=openEd.state().doc;
  const int rightId=openEd.removePolylineEdge({openId,0},2);
  require(rightId!=openId&&rightId>0&&openEd.state().doc.contours.size()==3,
          "interior open edge must leave two distinct editable paths");
  const auto* left=get(openEd.state().doc,{openId,0});
  const auto* right=get(openEd.state().doc,{rightId,0});
  require(left&&right&&left->nodes.size()==3&&right->nodes.size()==2&&
          std::abs(left->nodes.back().p.x-right->nodes.front().p.x)>4.9,
          "deleted edge must not leave an artificial bridge");
  require(get(openEd.state().doc,{otherId,0})!=nullptr,
          "other unselected contour disappeared");
  require(openEd.undo()&&sameDoc(openEd.state().doc,preCut),
          "open edge Undo did not restore original document");
  CurveDragSession curves(CurveDocument{});
  const Key selected=curves.addEditablePath(
      {point(0,0),point(10,0),point(10,10),point(0,10)},true,101);
  const Key distant=curves.addEditablePath(
      {point(40,0),point(50,0),point(50,10)},false,102);
  const auto before=curves.committed();
  require(curves.cutContourAtEdge(
      {selected,1,0,CurvePart::Edge,0,.5,{10,5}},999),
      "selected native Bezier side was not removed");
  require(curves.committed().contours.size()==2&&
          !curves.committed().contours.at(0).closed&&
          curves.committed().contours.at(1).key==distant,
          "Bezier cut removed or changed neighbor");
  require(curves.undo()&&curves.committed().contours.at(0).closed&&
          curves.committed().contours.at(1).nodes.size()==before.contours.at(1).nodes.size(),
          "Bezier side Undo failed");
  require(curves.redo()&&!curves.committed().contours.at(0).closed,
          "Bezier side Redo failed");
  DiagnosticsCollector diagnostics;
  require(!diagnostics.record("polyline_node_deleted",1),
          "opt-out diagnostics must remain silent");
  diagnostics.start();
  require(diagnostics.record("polyline_node_deleted",1)&&
          diagnostics.record("polyline_edge_deleted",1)&&
          diagnostics.counters().at("polyline_node_deleted")==1&&
          diagnostics.counters().at("polyline_edge_deleted")==1,
          "local geometry events were not accepted in opt-in diagnostics");
  std::cout<<"PASS: selected-only node/edge edits, other objects untouched, "
             "open/closed polylines, Bezier, Undo/Redo, opt-in diagnostics; offline\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;
 }
}
