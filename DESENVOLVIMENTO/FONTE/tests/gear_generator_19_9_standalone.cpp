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

  // Referência visual Inkscape: 19 dentes, passo circular 10 mm, pressão 30°, furo 10 mm.
  require(std::abs(circularPitchFromModule(3.)-3.*std::numbers::pi)<1e-12,
          "circular pitch calculation failed");
  for(double m:{0.5,1.,3.,7.25,20.}){
   require(std::abs(moduleFromCircularPitch(circularPitchFromModule(m))-m)<1e-10,
           "module/circular pitch roundtrip failed");
  }
  require(rejects([]{(void)moduleFromCircularPitch(0.);}),"zero pitch accepted");
  require(rejects([]{(void)circularPitchFromModule(25.);}),"oversize module accepted");
  Parameters ink;
  ink.teeth=19;ink.moduleMm=moduleFromCircularPitch(10.);
  ink.pressureDeg=30.;ink.boreMm=10.;
  const auto example=generate(ink);
  require(std::abs(2.*example.pitchRadius-190./std::numbers::pi)<1e-9,
          "Inkscape reference pitch diameter failed");
  require(std::abs(2.*example.outerRadius-210./std::numbers::pi)<1e-9,
          "Inkscape reference outside diameter failed");
  require(example.bore.size()==96,"Inkscape bore size incorrect");

  PairParameters pair;
  pair.driver=ink;pair.drivenTeeth=38;pair.drivenBoreMm=10.;
  const auto pairResult=generatePairNominal(pair);
  require(std::abs(pairResult.ratio-2.)<1e-12,"gear pair ratio incorrect");
  require(std::abs(pairResult.centerDistanceMm-285./std::numbers::pi)<1e-9,
          "gear pair center spacing incorrect");
  require(pairResult.drivenCenter.x==pairResult.centerDistanceMm &&
          !pairResult.meshingValidated,"nominal pair incorrectly certified");
  pair.drivenTeeth=5;
  require(rejects([&]{(void)generatePairNominal(pair);}),"unsafe follower accepted");

  RackParameters rack;
  rack.teeth=10;rack.moduleMm=ink.moduleMm;
  rack.pressureDeg=ink.pressureDeg;rack.baseThicknessMm=10.;
  const auto rail=generateRackNominal(rack);
  require(rail.outline.size()>40u,"rack contour incomplete");
  require(std::abs(rail.widthMm-100.)<1e-9,"rack pitch or width incorrect");
  require(std::abs(rail.heightMm-(10.+2.25*ink.moduleMm))<1e-9,
          "rack height incorrect");
  require(signedArea(rail.outline)>0.,"rack outer contour winding incorrect");
  require(!rail.meshingValidated,"rack incorrectly certified");
  rack.baseThicknessMm=0.;
  require(rejects([&]{(void)generateRackNominal(rack);}),"unsafe rack base accepted");
  std::cout<<"GEAR_OFFLINE_19_9 PASS: geometry, bore, bounds, winding, invalid inputs, circular pitch, nominal pair/rack\n";
  return 0;
 }catch(const std::exception&e){std::cerr<<"GEAR_OFFLINE_19_9 FAIL: "<<e.what()<<"\n";return 1;}
}
