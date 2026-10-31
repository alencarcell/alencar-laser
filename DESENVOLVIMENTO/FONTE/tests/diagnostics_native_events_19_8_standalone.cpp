// Alencar Laser TESTE 19.8: diagnósticos locais de Bézier nativa.
// Sem Qt, dados de clientes, rede, G-code ou comandos de máquina.
#include "../src/diagnostics_collector.hpp"
#include <iostream>
#include <stdexcept>
using namespace alaser;
static void expect(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){
 try{
  DiagnosticsCollector collector;
  expect(!collector.record("bezier_contour_scissor_cut",1),
      "opt-in OFF must not record native cuts");
  expect(collector.records().empty()&&collector.counters().empty(),
      "collector recorded diagnostics while OFF");
  collector.start();
  expect(collector.record("bezier_contour_scissor_cut",1),
      "native cut should be recorded");
  expect(collector.record("bezier_undo",1),
      "native undo should be recorded");
  expect(collector.record("bezier_redo",1),
      "native redo should be recorded");
  expect(collector.counters().at("bezier_contour_scissor_cut")==1&&
         collector.counters().at("bezier_undo")==1&&
         collector.counters().at("bezier_redo")==1,
         "native diagnostic counters not updated");
  expect(collector.records().size()==3,
         "native diagnostics not persisted in bounded event list");
  bool rejected=false;
  try{collector.record("bezier_undo:x=35.11,y=22.94",1);}
  catch(const std::invalid_argument&){rejected=true;}
  expect(rejected,"unknown events/coordinates must stay blocked");
  collector.stop();
  expect(!collector.record("bezier_redo",1)&&collector.records().size()==3,
      "collector continued collecting after opt-out");
  collector.clear();
  expect(collector.records().empty()&&collector.counters().empty(),
      "clear should erase local diagnostic history");
  std::cout<<"PASS: opt-in Bezier cut, undo/redo allowlist and private collector lifecycle (offline)\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;
 }
}
