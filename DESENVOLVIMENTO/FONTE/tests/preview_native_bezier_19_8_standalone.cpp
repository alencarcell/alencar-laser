// Alencar Laser 19.8: closed native curves are ONE editable object, also
// simulated as closed Fill on their assigned layer. Offline, no GRBL.
#include "../src/preview_native_19_8.hpp"
#include "../src/preview_sim_19_8.hpp"
#include "../src/preview_raster_19_8.hpp"
#include "../src/bezier_interaction.hpp"
#include "../src/bezier_package.hpp"
#include <iostream>
#include <stdexcept>
using namespace alaser;
static void expect(bool pass,const char* what){if(!pass)throw std::runtime_error(what);}
static CurveNode nd(double x,double y,Pt out){
 return {0,{x,y},{x,y},out};
}
int main(){
 try{
  Editor editor(Document{});
  editor.addLayer(1,"Preenchimento");
  editor.setLayerMode(1,"Fill");
  CurveDragSession native(CurveDocument{});
  const Key closed=native.addEditablePath({
    nd(12,12,{17,8}),nd(32,12,{32,12}),
    nd(32,32,{32,32}),nd(12,32,{12,32})
  },true,575,1);
  native.addEditableCubic({40,40},{45,35},{50,45},{60,40},576,0);
  const auto sourceBefore=encodeCurves(native.committed());
  const auto overlay=sim198::withNativeCurves(editor.state(),&native.committed());
  expect(overlay.paths==2&&overlay.closed==1&&overlay.open==1,
    "closed and open Bezier paths not preserved in virtual preview");
  expect(overlay.state.doc.contours.size()==2&&
    overlay.state.doc.contours.front().closed&&
    overlay.state.doc.contours.front().nodes.size()>4,
    "closed cubic must be approximated only on the read-only copy");
  expect(overlay.state.objects.at(1).layer==1&&
         overlay.state.objects.at(2).layer==0,
         "native path Fill/Line layers lost in preview");
  const auto lines=sim198::build(overlay.state);
  expect(lines.cutMm>0&&lines.ignoredFill==1,
         "open curve should be Line while closed Fill is deferred");
  sim198::Plan fill;
  sim198::raster::Settings raster;
  raster.intervalMm=0.5;
  sim198::raster::appendFill(fill,overlay.state,raster);
  expect(fill.cutMm>0&&fill.moves.size()>0,
         "closed Bezier does not fill as one closed object");
  expect(encodeCurves(native.committed())==sourceBefore,
         "preview MUST NOT mutate editable cubic source");
  const auto restored=decodeCurves(sourceBefore);
  expect(restored.contours.at(0).layer==1&&restored.contours.at(0).closed,
         "v5 layer and closure not persisted");
  auto legacy=sourceBefore;
  for(auto& contour:legacy["contours"].as_array())
    contour.as_object().erase("layerId");
  const auto oldFile=decodeCurves(legacy);
  expect(oldFile.contours.at(0).layer==0,
         "old v5 projects must default to layer 0");
  expect(native.setLayer(closed,0)&&native.committed().contours.at(0).layer==0,
         "editing native layer failed");
  expect(native.undo()&&native.committed().contours.at(0).layer==1,
         "undo of native layer failed");
  expect(native.redo()&&native.committed().contours.at(0).layer==0,
         "redo of native layer failed");
  expect(sim198::withNativeCurves(editor.state(),nullptr).paths==0,
         "legacy no-native preview changed");
  std::cout<<"PASS: Bezier v5 closed Fill/open Line offline, layer/codec, Undo/Redo and source untouched\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;
 }
}
