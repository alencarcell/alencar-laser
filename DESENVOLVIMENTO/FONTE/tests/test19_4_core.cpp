#include "editor_tools.hpp"
#include "bezier_interaction.hpp"
#include "diagnostics_collector.hpp"
#include <cassert>
#define expect(x) do {if(!(x))throw std::runtime_error(#x);}while(0)
#include <iostream>
#include <stdexcept>
using namespace alaser;
static CurveDocument curves(){
 CurveDocument d;d.nextId=3;
 CurveContour c;c.key={7,0};c.nodes={
  {1,{0,0},{-1,0},{1,0}},
  {2,{4,0},{3,1},{5,1}}
 };c.edgeKind={1};d.contours.push_back(c);validateCurve(d);return d;
}
int main(){
 {Editor ed(Document{});
  int a=ed.rectangle({0,0},10,5), b=ed.line({20,0},{30,0});
  ed.moveObjects({a},{2,3});
  expect(ed.state().doc.contours[0].nodes[0].p.x==2);
  expect(ed.undoCount()==3);expect(ed.undo());expect(ed.state().doc.contours[0].nodes[0].p.x==0);
  expect(ed.redo());expect(ed.state().doc.contours[0].nodes[0].p.y==3);
  ed.group({a,b});
  ed.deleteObjects({a});
  expect(ed.state().doc.contours.empty()&&ed.state().objects.empty());
  expect(ed.undo());expect(ed.state().doc.contours.size()==2);
  expect(ed.redo());expect(ed.state().doc.contours.empty());
 }
 {Editor ed(Document{});
  int a=ed.rectangle({0,0},10,5), b=ed.line({20,0},{30,0});
  ed.group({a,b});ed.addLayer(1,"locked");ed.moveToLayer(b,1);ed.setLayerFlags(1,true,true);
  const auto prior=ed.state().doc.contours.size();const auto undo=ed.undoCount();
  bool failed=false;try{ed.deleteObjects({a});}catch(const std::exception&){failed=true;}
  expect(failed&&ed.state().doc.contours.size()==prior&&ed.undoCount()==undo);
 }
 {CurveDragSession curve(curves());
  Key key{7,0};curve.select(key);
  const auto before=curve.committed().contours[0].nodes;
  expect(curve.startContourMove(key,{0,0}));curve.updateContourMove({10,20});
  expect(curve.committed().contours[0].nodes[0].anchor.x==before[0].anchor.x);
  expect(curve.visible().contours[0].nodes[1].handleOut.y==before[1].handleOut.y+20);
  curve.cancel();expect(curve.committed().contours[0].nodes[0].anchor.x==before[0].anchor.x);
  expect(curve.startContourMove(key,{0,0}));curve.updateContourMove({10,20});
  expect(curve.commitContourMove({10,20}));
  expect(curve.undoCount()==1&&curve.committed().contours[0].nodes[0].anchor.x==10);
  expect(curve.committed().contours[0].nodes[0].handleOut.y==before[0].handleOut.y+20);
  expect(curve.undo());expect(curve.committed().contours[0].nodes[0].anchor.x==0);
  expect(curve.redo());expect(curve.committed().contours[0].nodes[1].anchor.x==14);
  expect(curve.deleteSelectedContour());expect(curve.committed().contours.empty());
  expect(curve.undo());expect(curve.committed().contours.size()==1);
  expect(curve.redo());expect(curve.committed().contours.empty());
 }
 {DiagnosticsCollector d;
  expect(!d.record("shape_created"));d.start();expect(d.record("shape_created"));
  expect(d.record("shapes_moved",2));expect(d.records().size()==2&&d.counters().at("shapes_moved")==1);
  d.stop();expect(!d.record("shapes_deleted"));expect(d.records().size()==2);
  bool rejected=false;try{d.start();d.record("full_document",5);}catch(const std::exception&){rejected=true;}expect(rejected);
  d.clear();expect(d.records().empty()&&d.counters().empty());
 }
 std::cout<<"TESTE 19.4: ALL CORE ASSERTIONS PASS\n";
}
