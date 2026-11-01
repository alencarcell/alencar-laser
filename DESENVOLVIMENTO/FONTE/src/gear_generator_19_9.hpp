#pragma once
// Alencar Laser: original, independent offline gear generator, not Inkscape source.
// Approximate involute involute-tooth polyline. Needs physical fit/kerf validation.
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <vector>
namespace alaser::gear {
struct Point {double x=0,y=0;};
struct Parameters {
 int teeth=24;
 double moduleMm=3.;
 double pressureDeg=20.;
 double boreMm=6.;
 int flankSamples=12;
};
struct Result {
 std::vector<Point> outer, bore;
 double pitchRadius=0., outerRadius=0., rootRadius=0.;
 double baseRadius=0.;
 bool undercutWarning=false;
};
inline double involute(double radius,double base){
 if(radius<=base)return 0.;
 const double t=std::sqrt(std::max(0.,radius*radius/(base*base)-1.));
 return t-std::acos(std::clamp(base/radius,0.,1.));
}
inline Point polar(double r,double theta){
 return {r*std::cos(theta),r*std::sin(theta)};
}
inline void append(std::vector<Point>& points,Point value){
 if(!std::isfinite(value.x)||!std::isfinite(value.y))
  throw std::invalid_argument("coordenada de engrenagem invalida");
 if(points.empty()||std::hypot(points.back().x-value.x,points.back().y-value.y)>1e-10)
  points.push_back(value);
}
inline Result generate(const Parameters& p){
 if(p.teeth<12||p.teeth>120||!std::isfinite(p.moduleMm)||
    p.moduleMm<0.5||p.moduleMm>20.||
    !std::isfinite(p.pressureDeg)||p.pressureDeg<14.||p.pressureDeg>30.||
    !std::isfinite(p.boreMm)||p.boreMm<0.||p.flankSamples<6||p.flankSamples>20)
  throw std::invalid_argument("parametros de engrenagem fora dos limites");
 const double pi=std::numbers::pi;
 const double phi=p.pressureDeg*pi/180.;
 const double rp=p.teeth*p.moduleMm/2.,rb=rp*std::cos(phi);
 const double ra=rp+p.moduleMm,rf=rp-1.25*p.moduleMm;
 if(rf<=0.||p.boreMm>=2.*(rf-0.5*p.moduleMm))
  throw std::invalid_argument("furo central excede o nucleo estrutural da engrenagem");
 const double flankStart=std::max(rf,rb);
 const double baseHalf=pi/(2.*p.teeth)+involute(rp,rb);
 const auto halfAt=[&](double r){return baseHalf-involute(r,rb);};
 if(halfAt(ra)<=0.002 || baseHalf>=pi/p.teeth)
  throw std::invalid_argument("perfil de dente invalido: ajuste dentes/modulo/angulo");
 Result out;
 out.pitchRadius=rp;out.outerRadius=ra;out.rootRadius=rf;out.baseRadius=rb;
 out.undercutWarning=p.teeth<17&&p.pressureDeg<=20.;
 const double toothStep=2.*pi/p.teeth;
 out.outer.reserve(size_t(p.teeth)*(2*p.flankSamples+16));
 for(int i=0;i<p.teeth;++i){
  const double center=(i+0.25)*toothStep;
  const double startHalf=halfAt(flankStart);
  append(out.outer,polar(rf,center-startHalf));
  for(int j=0;j<=p.flankSamples;++j){
   const double r=flankStart+(ra-flankStart)*j/p.flankSamples;
   append(out.outer,polar(r,center-halfAt(r)));
  }
  const double tip=halfAt(ra);
  for(int j=1;j<=5;++j)
   append(out.outer,polar(ra,center-tip+2.*tip*j/5.));
  for(int j=p.flankSamples;j>=0;--j){
   const double r=flankStart+(ra-flankStart)*j/p.flankSamples;
   append(out.outer,polar(r,center+halfAt(r)));
  }
  append(out.outer,polar(rf,center+startHalf));
  const double gap=toothStep-2.*startHalf;
  for(int j=1;j<=5;++j)
   append(out.outer,polar(rf,center+startHalf+gap*j/5.));
 }
 if(p.boreMm>0.){
  out.bore.reserve(96);
  for(int i=95;i>=0;--i)
   out.bore.push_back(polar(p.boreMm/2.,2.*pi*i/96.));
 }
 if(out.outer.size()>10000)
  throw std::invalid_argument("contorno excessivamente complexo");
 return out;
}
} // namespace alaser::gear
