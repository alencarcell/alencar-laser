#pragma once
// Alencar Laser 20s — independent offline ESCAPE WHEEL study.
// All output is nominal vector geometry. Neither Graham anchor kinetics nor
// kerf/material tolerances are certified; anchor coordinates are GUIDES ONLY.
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <vector>
namespace alaser::escape20s {
struct Pt { double x=0., y=0.; };
inline Pt polar(double r,double angle){return {r*std::cos(angle),r*std::sin(angle)};}
inline double dist(Pt a,Pt b){return std::hypot(a.x-b.x,a.y-b.y);}
enum class Rotation { Clockwise=0, Counterclockwise=1 };
struct Settings {
 int teeth=30;
 double outerDiameterMm=80.;
 double rootFraction=.75;
 double axleHoleMm=6.;
 double tipThicknessDeg=.6;
 double trailingRootOffsetDeg=4.2;
 double leadingRootOffsetDeg=2.6;
 double palletSpanTeeth=7.5;
 Rotation rotation=Rotation::Clockwise;
};
struct Study {
 std::vector<Pt> wheel,axleHole;
 Pt wheelCenter{},anchorPivot{},entryReference{},exitReference{};
 std::vector<Pt> anchorReferenceArc;
 double rootRadius=0.,outerRadius=0.,pitchDeg=0.;
 double guidePivotDistanceMm=0.;
 double guidePalletArmMm=0.;
 bool anchorContactValidated=false;
 bool mechanicalFitValidated=false;
};
inline Study generate(const Settings& p){
 if(p.teeth<15||p.teeth>90||!std::isfinite(p.outerDiameterMm)||
    p.outerDiameterMm<20.||p.outerDiameterMm>400.||
    !std::isfinite(p.rootFraction)||p.rootFraction<.60||p.rootFraction>.90||
    !std::isfinite(p.axleHoleMm)||p.axleHoleMm<0.||
    !std::isfinite(p.tipThicknessDeg)||p.tipThicknessDeg<.15||
    !std::isfinite(p.trailingRootOffsetDeg)||
    !std::isfinite(p.leadingRootOffsetDeg)||
    !std::isfinite(p.palletSpanTeeth)||p.palletSpanTeeth<2.5||
    p.rotation!=Rotation::Clockwise&&p.rotation!=Rotation::Counterclockwise)
  throw std::invalid_argument("parametros da roda de escape invalidos");
 const double pitch=360./p.teeth;
 const double trailing=p.trailingRootOffsetDeg;
 const double leading=p.leadingRootOffsetDeg;
 const double tip=p.tipThicknessDeg;
 if(tip>=pitch/4.||trailing<=tip/2.+.25||
    leading<=tip/2.+.25||trailing+leading>=pitch-1.||
    trailing>pitch*.46||leading>pitch*.46)
  throw std::invalid_argument(
   "faces do dente se sobrepoem; ajuste espessura da ponta e afastamentos");
 const double outer=p.outerDiameterMm/2.,root=outer*p.rootFraction;
 if(p.axleHoleMm>=2.*root-3.)
  throw std::invalid_argument("furo central deixa parede radial insuficiente");
 const double spanAngle=p.palletSpanTeeth*pitch;
 if(spanAngle<45.||spanAngle>135.)
  throw std::invalid_argument("arco entre palhetas deve ficar entre 45 e 135 graus");
 const double toRad=std::numbers::pi/180.;
 const double halfSpan=spanAngle/2.*toRad;
 Study s;
 s.outerRadius=outer;s.rootRadius=root;s.pitchDeg=pitch;
 s.wheel.reserve(size_t(p.teeth)*5);
 const double entryOffset=(p.rotation==Rotation::Clockwise)?trailing:leading;
 const double exitOffset=(p.rotation==Rotation::Clockwise)?leading:trailing;
 for(int i=0;i<p.teeth;++i){
  const double middle=(90.+i*pitch)*toRad;
  s.wheel.push_back(polar(root,middle-entryOffset*toRad));
  s.wheel.push_back(polar(outer,middle-tip*toRad/2.));
  s.wheel.push_back(polar(outer,middle+tip*toRad/2.));
  s.wheel.push_back(polar(root,middle+exitOffset*toRad));
  s.wheel.push_back(polar(root,middle+pitch*toRad/2.));
 }
 if(p.axleHoleMm>0.){
  s.axleHole.reserve(96);
  for(int i=95;i>=0;--i)
   s.axleHole.push_back(polar(p.axleHoleMm/2.,2.*std::numbers::pi*i/96.));
 }
 // Square arrangement guide: wheel center (0,0), pivot above it. This is a
 // construction reference, not a generated anchor profile. At 30 teeth and
 // 7.5 tooth span this gives sqrt(2)*R, as in the classical square layout.
 const double d=2.*outer*std::cos(halfSpan);
 s.anchorPivot={0.,d};
 s.guidePivotDistanceMm=d;
 s.entryReference=polar(outer,std::numbers::pi/2.-halfSpan);
 s.exitReference=polar(outer,std::numbers::pi/2.+halfSpan);
 s.guidePalletArmMm=dist(s.anchorPivot,s.entryReference);
 // Arc represents a reference circle centered on the anchor pivot; it is
 // intentionally NOT added to the editor's cutting contours.
 const double a=std::atan2(s.entryReference.y-d,s.entryReference.x);
 const double b=std::atan2(s.exitReference.y-d,s.exitReference.x);
 s.anchorReferenceArc.reserve(65);
 double sweep=b-a;
 while(sweep<0)sweep+=2.*std::numbers::pi;
 if(sweep>std::numbers::pi)sweep-=2.*std::numbers::pi;
 for(int i=0;i<=64;++i){
  const double t=a+sweep*i/64.;
  s.anchorReferenceArc.push_back({
   d==0?0:s.anchorPivot.x+s.guidePalletArmMm*std::cos(t),
   s.anchorPivot.y+s.guidePalletArmMm*std::sin(t)});
 }
 return s;
}
inline double signedArea(const std::vector<Pt>& polygon){
 double sum=0.;
 if(polygon.size()<3)return 0.;
 for(size_t i=0;i<polygon.size();++i){
  auto a=polygon[i],b=polygon[(i+1)%polygon.size()];
  sum+=a.x*b.y-a.y*b.x;
 }
 return .5*sum;
}
} // namespace alaser::escape20s
