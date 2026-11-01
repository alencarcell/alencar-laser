#include "../src/gear_generator_19_9.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
using namespace alaser::gear;
static void require(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
template<class F> static bool rejects(F fn){try{fn();return false;}catch(const std::invalid_argument&){return true;}}
static double signedArea(const std::vector<Point>& poly){
 double area=0;
 for(size_t i=0;i<poly.size();++i){const auto&a=poly[i],&b=poly[(i+1)%poly.size()];
  area+=a.x*b.y-b.x*a.y;}
 return area*.5;
}
int main(){
 try{
  Parameters p;const auto a=generate(p);
  require(a.outer.size()>size_t(p.teeth)*20u&&a.outer.size()<50000u,"unexpected profile size");
  require(a.bore.size()==96,"bore sampling failed");
  require(std::abs(a.pitchRadius-36.)<1e-9,"incorrect pitch radius");
  require(std::abs(a.outerRadius-39.)<1e-9,"incorrect outer radius");
  require(std::abs(a.rootRadius-32.25)<1e-9,"incorrect root radius");
  require(signedArea(a.outer)>0.&&signedArea(a.bore)<0.,"outer/hole winding incorrect");
  for(const auto& x:a.outer)require(std::hypot(x.x,x.y)<=a.outerRadius+1e-8,"point outside tip radius");
  p.teeth=18;p.boreMm=0;
  const auto withoutHole=generate(p);require(withoutHole.bore.empty(),"zero bore not disabled");
  p.teeth=120;auto large=generate(p);require(large.outer.size()<10000,"large gear too complex");
  p.teeth=11;require(rejects([&]{generate(p);}),"low tooth count accepted");
  p.teeth=24;p.boreMm=200;require(rejects([&]{generate(p);}),"unsafe bore accepted");
  p.boreMm=4;p.moduleMm=0;require(rejects([&]{generate(p);}),"bad module accepted");
  p.moduleMm=3;p.pressureDeg=40;require(rejects([&]{generate(p);}),"bad pressure accepted");
  p.pressureDeg=20;p.flankSamples=100;require(rejects([&]{generate(p);}),"bad complexity accepted");
  std::cout<<"GEAR_OFFLINE_19_9 PASS: geometry, bore, bounds, winding, invalid inputs\n";
  return 0;
 }catch(const std::exception&e){std::cerr<<"GEAR_OFFLINE_19_9 FAIL: "<<e.what()<<"\n";return 1;}
}
