// Alencar Laser 20r: measured included-tip angles, not clock escapement certification.
#include "gear_generator_19_9.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <array>
using namespace alaser::gear;
static void check(bool b,const char*msg){if(!b)throw std::runtime_error(msg);}
static double cross(Point a,Point b,Point c){
 return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);
}
static void verifyAngular(const Parameters& p){
 const auto r=generate(p);
 check(r.outer.size()==size_t(p.teeth)*4u,"Incorrect external point count");
 check(r.bore.size()==(p.boreMm>0?96u:0u),"Incorrect bore");
 for(int i=0;i<p.teeth;++i){
  const auto a=r.outer[size_t(4*i)],b=r.outer[size_t(4*i+1)],c=r.outer[size_t(4*i+2)];
  const double measured=includedAngleAtTipDeg(a,b,c);
  check(std::abs(measured-p.includedTipAngleDeg)<1e-6,
        "Computed angle does not match requested interior tip angle");
  check(std::abs(std::hypot(b.x,b.y)-r.outerRadius)<1e-8,"Tip radius mismatch");
  check(std::abs(std::hypot(a.x,a.y)-r.rootRadius)<1e-8,"Root radius mismatch");
  check(r.outerRadius<=p.moduleMm*(p.teeth/2.+1.)+1e-8,
        "Angle solver exceeded nominal radial envelope");
  check(r.outerRadius-r.rootRadius>=.15*p.moduleMm-1e-8,
        "Angle solver produced too shallow a tooth");
  check(std::isfinite(cross(a,b,c)),"Invalid flank geometry");
 }
}
int main(){
 try{
  const auto reference=generate(Parameters{});
  check(reference.outer.size()>500,"Legacy involute default changed");
  for(int n:{12,24,60,120}){
   for(ToothProfile profile:{ToothProfile::Angular,ToothProfile::Sawtooth}){
    for(double angle:{60.,75.,90.,95.,96.,110.,120.,140.}){
     Parameters p;
     p.teeth=n;p.profile=profile;
     p.angularDefinition=AngularDefinition::ByIncludedTipAngle;
     p.includedTipAngleDeg=angle;
     p.angularBasePercent=75.;
     p.tipPositionPercent=80.;
     verifyAngular(p);
    }
   }
  }
  // Old width mode must remain exact when the new angle field is irrelevant.
  Parameters width; width.profile=ToothProfile::Sawtooth;
  const auto a=generate(width);
  width.includedTipAngleDeg=150.;
  const auto b=generate(width);
  check(a.outer.size()==b.outer.size(),"Legacy width mode changed count");
  for(size_t i=0;i<a.outer.size();++i)
   check(std::hypot(a.outer[i].x-b.outer[i].x,a.outer[i].y-b.outer[i].y)<1e-12,
         "Legacy width mode changed geometry");
  auto invalid=width;
  invalid.angularDefinition=AngularDefinition::ByIncludedTipAngle;
  invalid.includedTipAngleDeg=12.;
  bool rejected=false;
  try{(void)generate(invalid);}catch(const std::invalid_argument&){rejected=true;}
  check(rejected,"Below-range angle accepted");
  invalid.includedTipAngleDeg=165.;
  rejected=false;
  try{(void)generate(invalid);}catch(const std::invalid_argument&){rejected=true;}
  check(rejected,"Above-range angle accepted");
  invalid.includedTipAngleDeg=30.;
  rejected=false;
  try{(void)generate(invalid);}catch(const std::invalid_argument&){rejected=true;}
  check(rejected,"Unrealizable angle accepted instead of reporting error");
  std::cout<<"PASS GEAR_MEASURED_20R: tip angle verified (60/75/90/95/96/110/120/140 degrees), 12/24/60/120 teeth, symmetric/saw profiles, involute/width compatibility and invalid values\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"FAIL GEAR_MEASURED_20R: "<<e.what()<<"\n";return 1;
 }
}
