#include "zoom_precision_196.hpp"
#include "grbl_tcp_rules_196.hpp"
#include "grbl_observer.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
 using namespace alaser;
 assert(zoom196::applyWheel(240,1)>240);
 assert(zoom196::applyWheel(4096,30)==4096);
 assert(zoom196::applyWheel(.03,-100)==.03);
 assert(zoom196::clampScale(4000)==4000);
 for(const char* ip:{"192.168.0.15","10.8.4.2","172.16.0.1","172.31.255.254"})assert(tcp196::privateIpv4(ip));
 for(const char* ip:{"127.0.0.1","8.8.8.8","172.32.1.2","192.168.1.5:23","192.168.1","192.168.2.999","host.local",""})assert(!tcp196::privateIpv4(ip));
 assert(tcp196::validPort(23)&&!tcp196::validPort(0)&&!tcp196::validPort(65536));
 grbl::Observation obs;obs.accept("Grbl 1.1h");obs.accept("<Idle|MPos:0,0,0>");
 assert(grbl::probeBytes(grbl::Probe::Status,obs.grblBanner,obs.state)=="?");
 assert(grbl::probeBytes(grbl::Probe::Firmware,obs.grblBanner,obs.state)=="$I\n");
 obs.accept("<Run|MPos:0,0,0>");
 bool blocked=false;try{(void)grbl::probeBytes(grbl::Probe::Settings,obs.grblBanner,obs.state);}catch(...){blocked=true;}
 assert(blocked);
 std::cout<<"TESTE_19_6 zoom/wifi safety PASS\n";
}
