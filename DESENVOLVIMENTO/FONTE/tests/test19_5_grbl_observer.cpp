#include "grbl_observer.hpp"
#include "serial_observer_win.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace alaser::grbl;
static bool fails(Probe p,bool recognized,std::string_view state){
 try {(void)probeBytes(p,recognized,state);return false;}catch(const std::runtime_error&){return true;}
}
int main(){
 assert(probeBytes(Probe::Status,false,"Unknown")=="?");
 assert(fails(Probe::Firmware,false,"Idle"));
 assert(fails(Probe::Settings,false,"Idle"));
 for(auto state:{"Run","Hold","Alarm","Jog","Unknown","Door"}){
  assert(fails(Probe::Firmware,true,state));assert(fails(Probe::Settings,true,state));
 }
 assert(probeBytes(Probe::Firmware,true,"Idle")=="$I\n");
 assert(probeBytes(Probe::Settings,true,"Idle")=="$$\n");
 Observation o;
 o.accept("Grbl 1.1h ['$' for help]");assert(o.grblBanner);
 o.accept("<Idle|MPos:0.000,1.000,0.000|FS:0,0>");assert(o.state=="Idle");
 o.accept("<Run|MPos:1,2,3>");assert(o.state=="Run");
 o.accept("ALARM:2");assert(o.state=="Alarm");
 o.accept("<Fake|MPos:0>");assert(o.state=="Unknown");
 o.accept(std::string(601,'a'));assert(o.ignored==1);
 for(int n=0;n<250;++n)o.accept("ok");
 assert(o.recent.size()==80);
 std::cout<<"TESTE_19_5_GRBL_OBSERVER PASS - read-only gate and status parsing\n";
}
