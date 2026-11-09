// Phase 20q: deterministic, Qt-free regression for pointed/saw-tooth gear outlines.
// No G-code, USB, laser commands, file access or changes to saved projects.
#include "gear_generator_19_9.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace alaser::gear;
static void require(bool cond,const char* msg){if(!cond)throw std::runtime_error(msg);}
static double area(const std::vector<Point>& points){
 double sum=0.;
 for(size_t i=0;i<points.size();++i){
  const auto& a=points[i];const auto& b=points[(i+1)%points.size()];
  sum+=a.x*b.y-b.x*a.y;
 }
 return 0.5*sum;
}
static double radius(Point p){return std::hypot(p.x,p.y);}
static double dist(Point a,Point b){return std::hypot(a.x-b.x,a.y-b.y);}
static void verify(const Parameters& params){
 const auto result=generate(params);
 require(result.outer.size()==size_t(4*params.teeth),"Angular profile must use 4 vertices per tooth");
 require(result.bore.size()==(params.boreMm>0?96u:0u),"Bore count changed");
 require(area(result.outer)>0.,"Angular outline must wind CCW");
 if(!result.bore.empty())require(area(result.bore)<0.,"Bore must wind CW");
 require(dist(result.outer.front(),result.outer.back())>1e-7,"Redundant closing node");
 const double step=2*std::numbers::pi/double(params.teeth);
 for(int i=0;i<params.teeth;++i){
  auto left=result.outer.at(size_t(4*i));
  auto tip=result.outer.at(size_t(4*i+1));
  auto right=result.outer.at(size_t(4*i+2));
  auto valley=result.outer.at(size_t(4*i+3));
  require(std::abs(radius(left)-result.rootRadius)<1e-8,"Left root radius incorrect");
  require(std::abs(radius(right)-result.rootRadius)<1e-8,"Right root radius incorrect");
  require(std::abs(radius(valley)-result.rootRadius)<1e-8,"Valley radius incorrect");
  require(std::abs(radius(tip)-result.outerRadius)<1e-8,"Sharp tip radius incorrect");
  const double lhs=dist(left,tip),rhs=dist(tip,right);
  if(params.profile==ToothProfile::Angular){
   require(std::abs(lhs-rhs)<1e-8,"Symmetric angular tooth is asymmetric");
  }else{
   require(lhs>rhs*1.05,"Sawtooth tip should be inclined");
  }
  if(i>0){
   const auto prevTip=result.outer.at(size_t(4*(i-1)+1));
   double t=std::atan2(tip.y,tip.x)-std::atan2(prevTip.y,prevTip.x);
   if(t<0)t+=2*std::numbers::pi;
   require(std::abs(t-step)<1e-8,"Tooth periodicity broken");
  }
 }
 for(size_t j=0;j<result.outer.size();++j){
  const auto& a=result.outer[j];
  require(std::isfinite(a.x)&&std::isfinite(a.y),"Non-finite coordinate");
  require(dist(a,result.outer[(j+1)%result.outer.size()])>1e-7,"Degenerate segment");
 }
 require(!result.undercutWarning,"Angular shape incorrectly reported involute undercut");
}
int main(){
 try{
  Parameters baseline;
  const auto legacy=generate(baseline);
  require(legacy.outer.size()>500 && legacy.bore.size()==96,
          "Default involute geometry changed unexpectedly");
  require(baseline.profile==ToothProfile::Involute,"Involute must remain default");
  for(int n:{12,24,60,120}){
   for(auto profile:{ToothProfile::Angular,ToothProfile::Sawtooth}){
    Parameters p; p.teeth=n;p.profile=profile;
    p.angularBasePercent=75.;p.tipPositionPercent=80.;
    verify(p);
   }
  }
  Parameters noHole;noHole.profile=ToothProfile::Angular;noHole.boreMm=0.;
  verify(noHole);
  Parameters invalid;invalid.profile=ToothProfile::Angular;
  invalid.angularBasePercent=101.;
  bool caught=false;
  try{(void)generate(invalid);}catch(const std::invalid_argument&){caught=true;}
  require(caught,"Invalid tooth base accepted");
  invalid=Parameters{};invalid.profile=ToothProfile::Sawtooth;
  invalid.tipPositionPercent=99.;
  caught=false;
  try{(void)generate(invalid);}catch(const std::invalid_argument&){caught=true;}
  require(caught,"Invalid tip position accepted");
  invalid=Parameters{};invalid.profile=ToothProfile::Angular;
  invalid.boreMm=1000.;
  caught=false;
  try{(void)generate(invalid);}catch(const std::invalid_argument&){caught=true;}
  require(caught,"Oversized central bore accepted");
  std::cout<<"PASS GEAR_ANGULAR_20Q: involute default preserved, 2 pointed profiles, 12/24/60/120 teeth, 4 vertices per tooth, bores and guards\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"FAIL GEAR_ANGULAR_20Q: "<<e.what()<<"\n";
  return 1;
 }
}
