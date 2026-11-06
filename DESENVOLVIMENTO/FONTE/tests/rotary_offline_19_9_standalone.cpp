#include "../src/rotary_geometry_19_9.hpp"
#include <iostream>
#include <limits>
using namespace alaser::rotary;
static void expect(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
template<class F> static bool rejects(F f){try{f();return false;}catch(const std::invalid_argument&){return true;}}
static bool approx(double a,double b){return std::abs(a-b)<1e-8;}
int main(){
 try{
  Profile p;
  expect(!p.previewEnabled&&p.mmPerRotation==0&&p.rollerDiameterMm==0,
    "Unknown roller must not have invented calibration");
  validate(p);
  expect(rejects([&]{(void)circumferenceMm(0.);}),"invalid diameter");
  expect(rejects([&]{(void)circumferenceMm(std::numeric_limits<double>::quiet_NaN());}),
    "NaN diameter");
  p.previewEnabled=true;
  expect(rejects([&]{validate(p);}),"incomplete activated preview must fail");
  p.rollerDiameterMm=20;p.objectDiameterMm=80;p.mmPerRotation=40;
  validate(p);
  expect(approx(circumferenceMm(p.objectDiameterMm),80*std::numbers::pi),
    "wrong object circumference");
  expect(approx(objectTurnDegrees(20*std::numbers::pi,p),90.0),
    "quarter turn math");
  expect(approx(rollerTurns(10*std::numbers::pi,p),0.5),
    "roller half turn math");
  expect(exceedsPerimeter(81*std::numbers::pi,p),"perimeter overrun not flagged");
  expect(!exceedsPerimeter(80*std::numbers::pi,p),"equal circumference flagged");
  p.type=Type::Chuck;p.rollerDiameterMm=0;validate(p);
  expect(rejects([&]{rollerTurns(40,p);}),"chuck cannot use roller formula");
  p.previewEnabled=false;
  p.mmPerRotation=std::numeric_limits<double>::infinity();
  expect(rejects([&]{validate(p);}),"invalid inactive setting accepted");
  std::cout<<"ROTARY_OFFLINE_19_9 PASS: safe defaults, profile, roller/chuck, units, invalid values and circumference\n";
 }catch(const std::exception& e){
  std::cerr<<"ROTARY_OFFLINE_19_9 FAIL: "<<e.what()<<"\n";return 1;
 }
}
