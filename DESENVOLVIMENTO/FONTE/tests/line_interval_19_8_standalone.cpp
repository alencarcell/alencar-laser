// Alencar Laser 19.8 - offline per-layer scan interval regression.
// No Qt, GRBL, G-code, serial or physical machine operations.
#include "../src/preview_raster_19_8.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
using namespace alaser;
namespace sr=alaser::sim198::raster;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){
 try{
  Editor editor(Document{});
  editor.rectangle({0,0},10,10,0);
  editor.addLayer(1,"Teste camada 1");
  editor.rectangle({30,0},10,10,1);
  auto layer0=editor.state().layers.at(0);
  layer0.mode="Fill";layer0.powerMode="M4";layer0.lineIntervalMm=1.0;
  auto layer1=editor.state().layers.at(1);
  layer1.mode="Fill";layer1.powerMode="M3";layer1.lineIntervalMm=0.5;
  const auto countUndo=editor.undoCount();
  editor.configureLayer(0,layer0);
  editor.configureLayer(1,layer1);
  require(editor.undoCount()==countUndo+2,"each layer edit must be atomic");
  const auto saved=metadata(editor.state());
  const auto revived=fromMetadata(editor.state().doc,saved);
  require(revived.layers.at(0).lineIntervalMm==1.0&&
          revived.layers.at(1).lineIntervalMm==0.5,
          "layer interval not persisted in metadata");
  auto old=saved;
  for(auto& item:old["layers"].as_array())
   item.as_object().erase("lineIntervalMm");
  const auto legacy=fromMetadata(editor.state().doc,old);
  require(std::abs(legacy.layers.at(0).lineIntervalMm-0.20)<1e-9&&
          std::abs(legacy.layers.at(1).lineIntervalMm-0.20)<1e-9,
          "legacy metadata default must be 0.20 mm");
  sr::Settings config;
  config.intervalMm=0.20;config.overscanMm=0;config.bidirectional=true;
  sim198::Plan perLayer;
  sr::appendFill(perLayer,editor.state(),config,250000,true);
  std::size_t c0=0,c1=0;
  for(const auto& m:perLayer.moves)if(m.laser){
   if(m.layer==0)++c0;else if(m.layer==1)++c1;
  }
  require(c0==10&&c1==20,"virtual scan rows must reflect each layer pitch");
  sim198::Plan global;
  sr::appendFill(global,editor.state(),config);
  c0=c1=0;
  for(const auto& m:global.moves)if(m.laser){
   if(m.layer==0)++c0;else if(m.layer==1)++c1;
  }
  require(c0==50&&c1==50,"global fallback pitch must be preserved");
  require(editor.undo()&&std::abs(editor.state().layers.at(1).lineIntervalMm-0.20)<1e-9,
          "Undo should restore previous layer pitch");
  require(editor.redo()&&editor.state().layers.at(1).lineIntervalMm==0.5,
          "Redo should restore edited layer pitch");
  bool rejected=false;
  try{auto invalid=layer0;invalid.lineIntervalMm=0.01;editor.configureLayer(0,invalid);}
  catch(const std::exception&){rejected=true;}
  require(rejected,"invalid interval must be rejected");
  std::cout<<"PASS: 1.000/0.500mm per-layer Fill, 0.200mm global fallback, old metadata, undo/redo; M3/M4 independent; offline only\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;
 }
}
