#include "escapement_study_20s.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace alaser::escape20s;
static void need(bool cond,const char*reason){if(!cond)throw std::runtime_error(reason);}
int main(){
 try{
  Settings s;const auto r=generate(s);
  need(r.wheel.size()==150&&r.axleHole.size()==96,"nominal 30 tooth node count");
  need(std::abs(r.outerRadius-40.)<1e-10&&
       std::abs(r.rootRadius-30.)<1e-10,"radii");
  need(std::abs(r.pitchDeg-12.)<1e-10,"12 degree pitch");
  need(signedArea(r.wheel)>0.&&signedArea(r.axleHole)<0.,"opposite winding");
  need(std::abs(r.guidePivotDistanceMm-std::sqrt(2.)*40.)<1e-9,
       "classical square layout reference distance");
  need(dist(r.anchorPivot,r.entryReference)>0.&&
       std::abs(dist(r.anchorPivot,r.entryReference)-
                dist(r.anchorPivot,r.exitReference))<1e-9,"reference symmetry");
  need(r.anchorReferenceArc.size()==65&&
       dist(r.anchorReferenceArc.front(),r.entryReference)<1e-9&&
       dist(r.anchorReferenceArc.back(),r.exitReference)<1e-9,
       "guide arc endpoints");
  need(!r.anchorContactValidated&&!r.mechanicalFitValidated,
       "educational reference wrongly certified");
  for(int z:{15,24,30,45,60,90}){
   Settings alt=s;alt.teeth=z;
   alt.trailingRootOffsetDeg=360./z*.35;
   alt.leadingRootOffsetDeg=360./z*.22;
   alt.tipThicknessDeg=360./z*.05;
   alt.palletSpanTeeth=std::clamp(90./(360./z),2.5,13.);
   for(auto rotation:{Rotation::Clockwise,Rotation::Counterclockwise}){
    alt.rotation=rotation;
    const auto w=generate(alt);
    need(w.wheel.size()==size_t(z*5),"wrong node count");
    need(signedArea(w.wheel)>0.,"outer orientation");
    need(signedArea(w.axleHole)<0.,"bore orientation");
    for(size_t i=0;i<w.wheel.size();++i){
     const Pt p=w.wheel[i],q=w.wheel[(i+1)%w.wheel.size()];
     need(std::isfinite(p.x)&&std::isfinite(p.y)&&dist(p,q)>1e-8,
          "degenerate path");
    }
    need(dist(w.anchorReferenceArc.front(),w.entryReference)<1e-8,
         "entry reference incorrect");
   }
  }
  const auto inverse=[]{
   Settings s;s.rotation=Rotation::Counterclockwise;return generate(s);
  }();
  need(std::abs(r.wheel[0].x-inverse.wheel[0].x)>1e-5,
       "direction should mirror the tooth flank offsets");
  for(int defect=0;defect<5;++defect){
   Settings bad=s;
   if(defect==0)bad.teeth=3;
   if(defect==1)bad.rootFraction=.95;
   if(defect==2)bad.axleHoleMm=100.;
   if(defect==3)bad.tipThicknessDeg=10.;
   if(defect==4)bad.palletSpanTeeth=15.;
   bool rejected=false;
   try{(void)generate(bad);}catch(const std::invalid_argument&){rejected=true;}
   need(rejected,"invalid geometry accepted");
  }
  Settings noBore=s;noBore.axleHoleMm=0.;
  need(generate(noBore).axleHole.empty(),"bore=0 did not omit hole");
  std::cout<<"PASS ESCAPE_WHEEL_STUDY_20S: 30t square guide, 15/24/30/45/60/90t variants, CW/CCW, closed 2D wheel, bore, 5 invalid guards and no hardware\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"FAIL ESCAPE_WHEEL_STUDY_20S: "<<e.what()<<"\n";return 1;
 }
}
