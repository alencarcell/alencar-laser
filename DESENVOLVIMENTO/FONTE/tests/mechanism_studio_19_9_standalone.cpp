#include "../src/mechanism_studio_19_9.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <numbers>
#include <algorithm>
using namespace alaser::mechanisms;
static void check(bool yes,const char* why){if(!yes)throw std::runtime_error(why);}
int main(){
 try{
  const auto toy=generate(defaults(Template::ToyGears));
  check(toy.parts.size()==2,"toy must have two gears");
  check(toy.speedRatio>0.&&toy.parts[1].angularRatio<0.,"pair must rotate opposite");
  const double theta=2.*std::numbers::pi;
  check(std::abs(toy.centerDistanceMm-90.)<1e-9,"wrong toy center distance");
  const auto local=toy.parts[1].outer.front();
  const auto a=transform(toy.parts[1],local,0.);
  const auto b=transform(toy.parts[1],local,theta);
  check(std::hypot(b.x-a.x,b.y-a.y)>1e-6,"moving gear did not animate");
  check(!toy.physicallyCertified&&!toy.hardwareEnabled,"unsafe certification");
  check(!toy.warnings.empty(),"missing mechanical warning");
  const auto clock=generate(defaults(Template::ClockTrain));
  check(clock.parts.size()==3&&clock.parts[2].angularRatio>0.,"clock train motion invalid");
  const auto school=generate(defaults(Template::SchoolRack));
  check(school.parts.size()==2&&school.parts[1].rack,"school must use rack");
  const auto moved=transform(school.parts[1],{0.,0.},1.);
  check(std::abs(moved.x-school.parts[0].origin.y)<1e-8,
        "rack distance should equal pitch radius times angle");
  for(const auto preset:{Template::ClockTrain,Template::ToyGears,Template::SchoolRack}){
   auto s=defaults(preset);
   for(const auto material:{Material::MDF,Material::Plywood,Material::Acrylic}){
    s.material=material;
    const auto e=generate(s);
    check(!e.parts.empty()&&!e.hardwareEnabled,"empty/unsafe preset");
    for(const auto& part:e.parts){
     for(const auto& p:part.outer)check(std::isfinite(p.x)&&std::isfinite(p.y),
                                           "invalid vertex");
     for(const double angle:{0.,1.,3.,6.}){
      for(const auto& p:part.outer){
       const auto q=transform(part,p,angle);
       check(std::isfinite(q.x)&&std::isfinite(q.y),"invalid kinematics");
      }
     }
    }
   }
  }
  auto bad=defaults(Template::ToyGears);bad.kerfMm=1.1;
  bool thrown=false;try{(void)generate(bad);}catch(const std::invalid_argument&){thrown=true;}
  check(thrown,"unreasonable kerf accepted");
  bad=defaults(Template::ToyGears);bad.thicknessMm=0.;
  thrown=false;try{(void)generate(bad);}catch(const std::invalid_argument&){thrown=true;}
  check(thrown,"invalid material thickness accepted");
  bad=defaults(Template::ToyGears);bad.backlashMm=5.;
  thrown=false;try{(void)generate(bad);}catch(const std::invalid_argument&){thrown=true;}
  check(thrown,"invalid clearance accepted");
  std::cout<<"MECHANISM_STUDIO_19_9 PASS: 3 templates, 3 materials, ratios, animated kinematics, validations; no hardware\n";
  return 0;
 }catch(const std::exception& e){std::cerr<<"MECHANISM_STUDIO_19_9 FAIL: "<<e.what()<<"\n";return 1;}
}
