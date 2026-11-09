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
// The angular outlines below are editorial/laser cut shapes, NOT a certified
// gear-meshing profile. Keep Involute as the backwards-compatible default.
enum class ToothProfile { Involute=0, Angular=1, Sawtooth=2 };
struct Parameters {
 int teeth=24;
 double moduleMm=3.;
 double pressureDeg=20.;
 double boreMm=6.;
 int flankSamples=12;
 ToothProfile profile=ToothProfile::Involute;
 double angularBasePercent=75.; // percentage of a single tooth pitch
 double tipPositionPercent=80.; // sawtooth apex location from leading root
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
// A pointed/serrated wheel as in the hand sketch: each tooth has straight
// sides and a single sharp apex. Contour is CCW and closed by the editor;
// no redundant final point. One root midpoint bridges adjacent teeth.
inline Result generateAngularOutline(const Parameters& p){
 if(p.profile!=ToothProfile::Angular && p.profile!=ToothProfile::Sawtooth)
  throw std::invalid_argument("perfil angular desconhecido");
 if(!std::isfinite(p.angularBasePercent)||
    p.angularBasePercent<45.||p.angularBasePercent>90.||
    !std::isfinite(p.tipPositionPercent)||
    p.tipPositionPercent<15.||p.tipPositionPercent>85.)
  throw std::invalid_argument("largura da base ou inclinacao fora dos limites");
 const double pi=std::numbers::pi;
 const double rp=p.teeth*p.moduleMm/2.;
 const double ra=rp+p.moduleMm,rf=rp-1.25*p.moduleMm;
 if(rf<=0.||p.boreMm>=2.*(rf-0.5*p.moduleMm))
  throw std::invalid_argument("furo central excede o nucleo estrutural da engrenagem");
 Result out;
 out.pitchRadius=rp;out.outerRadius=ra;out.rootRadius=rf;
 out.baseRadius=rp*std::cos(p.pressureDeg*pi/180.);
 out.undercutWarning=false; // not an involute tooth
 const double step=2.*pi/double(p.teeth);
 const double half=step*p.angularBasePercent/200.;
 const double apexFraction=(p.profile==ToothProfile::Angular)
                            ?0.5:p.tipPositionPercent/100.;
 const double gap=step-2.*half;
 out.outer.reserve(size_t(p.teeth)*4);
 for(int i=0;i<p.teeth;++i){
  const double center=(i+0.25)*step;
  const double left=center-half,right=center+half;
  const double apex=left+(right-left)*apexFraction;
  append(out.outer,polar(rf,left));
  append(out.outer,polar(ra,apex));
  append(out.outer,polar(rf,right));
  append(out.outer,polar(rf,right+gap/2.));
  // The next tooth starts at this last angular position. The closed
  // polygon supplies the final segment without duplicating the first node.
 }
 if(p.boreMm>0.){
  out.bore.reserve(96);
  for(int i=95;i>=0;--i)
   out.bore.push_back(polar(p.boreMm/2.,2.*pi*i/96.));
 }
 return out;
}
inline Result generate(const Parameters& p){
 if(p.teeth<12||p.teeth>120||!std::isfinite(p.moduleMm)||
    p.moduleMm<0.5||p.moduleMm>20.||
    !std::isfinite(p.pressureDeg)||p.pressureDeg<14.||p.pressureDeg>30.||
    !std::isfinite(p.boreMm)||p.boreMm<0.||p.flankSamples<6||p.flankSamples>20)
  throw std::invalid_argument("parametros de engrenagem fora dos limites");
 if(p.profile==ToothProfile::Angular||p.profile==ToothProfile::Sawtooth)
  return generateAngularOutline(p);
 if(p.profile!=ToothProfile::Involute)
  throw std::invalid_argument("perfil de dente desconhecido");
 const double pi=std::numbers::pi;
 const double phi=p.pressureDeg*pi/180.;
 const double rp=p.teeth*p.moduleMm/2.,rb=rp*std::cos(phi);
 const double ra=rp+p.moduleMm,rf=rp-1.25*p.moduleMm;
 if(rf<=0.||p.boreMm>=2.*(rf-0.5*p.moduleMm))
  throw std::invalid_argument("furo central excede o nucleo estrutural da engrenagem");
 const double flankStart=std::max(rf,rb);
 const double baseHalf=pi/(2.*p.teeth)+involute(rp,rb);
 const auto halfAt=[&](double r){return baseHalf-involute(r,rb);};
 if(halfAt(ra)<=0.002 || halfAt(flankStart)>=pi/p.teeth)
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
  for(int j=1;j<=5;++j){
   if(i==p.teeth-1&&j==5)continue; // closing edge supplied by closed path
   append(out.outer,polar(rf,center+startHalf+gap*j/5.));
  }
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

/* Parâmetros editoriais: nenhuma função aciona máquina ou exporta G-code.
 * Passo circular = pi * módulo. Não confundir passo circular com circunferência.
 */
inline double circularPitchFromModule(double moduleMm){
 if(!std::isfinite(moduleMm)||moduleMm<0.5||moduleMm>20.)
  throw std::invalid_argument("modulo fora dos limites");
 return std::numbers::pi*moduleMm;
}
inline double moduleFromCircularPitch(double pitchMm){
 const double m=pitchMm/std::numbers::pi;
 if(!std::isfinite(pitchMm)||!std::isfinite(m)||m<0.5-1e-6||m>20.+1e-6)
  throw std::invalid_argument("passo circular fora dos limites");
 return std::clamp(m,0.5,20.);
}

// Geometria nominal do par; a fase dos dentes e a folga ainda NAO sao resolvidas.
// Nao oferecer o resultado como par mecanicamente homologado.
struct PairParameters{
 Parameters driver{};
 int drivenTeeth=36;
 double drivenBoreMm=6.;
};
struct PairResult{
 Result driver,driven;
 double centerDistanceMm=0.;
 double ratio=0.; // magnitude da redução = dentes acionados / dentes motores
 Point driverCenter{},drivenCenter{};
 bool meshingValidated=false;
};
inline PairResult generatePairNominal(const PairParameters& request){
 auto second=request.driver;
 second.teeth=request.drivenTeeth;
 second.boreMm=request.drivenBoreMm;
 PairResult out;
 out.driver=generate(request.driver);
 out.driven=generate(second);
 out.centerDistanceMm=out.driver.pitchRadius+out.driven.pitchRadius;
 out.ratio=double(second.teeth)/request.driver.teeth;
 out.driverCenter={0.,0.};
 out.drivenCenter={out.centerDistanceMm,0.};
 return out;
}

// Cremalheira teórica de dentes trapezoidais, sem arredondamento/raiz de fresa.
// Mantém a mesma referência módulo/ângulo do pinhão, mas não valida encaixe real.
struct RackParameters{
 int teeth=8;
 double moduleMm=3.;
 double pressureDeg=20.;
 double baseThicknessMm=9.;
};
struct RackResult{
 std::vector<Point> outline;
 double circularPitchMm=0.;
 double widthMm=0.,heightMm=0.;
 bool meshingValidated=false;
};
inline RackResult generateRackNominal(const RackParameters& params){
 if(params.teeth<1||params.teeth>120||
    !std::isfinite(params.moduleMm)||params.moduleMm<0.5||params.moduleMm>20.||
    !std::isfinite(params.pressureDeg)||params.pressureDeg<14.||params.pressureDeg>30.||
    !std::isfinite(params.baseThicknessMm)||
    params.baseThicknessMm<0.5*params.moduleMm||
    params.baseThicknessMm>30.*params.moduleMm)
  throw std::invalid_argument("parametros da cremalheira fora dos limites");
 const double m=params.moduleMm,alpha=params.pressureDeg*std::numbers::pi/180.;
 const double pitch=circularPitchFromModule(m),a=m,d=1.25*m;
 const double flank=std::tan(alpha);
 if(pitch/2.-2*d*flank<=0.||pitch/2.-2*a*flank<=0.)
  throw std::invalid_argument("espacamento do dente da cremalheira invalido");
 RackResult out;
 out.circularPitchMm=pitch;out.widthMm=params.teeth*pitch;
 out.heightMm=params.baseThicknessMm+a+d;
 const double left=-pitch/2.,right=(params.teeth-.5)*pitch;
 append(out.outline,{left,-d-params.baseThicknessMm});
 append(out.outline,{left,-d});
 for(int i=0;i<params.teeth;++i){
  const double x=i*pitch;
  append(out.outline,{x-pitch/4.-d*flank,-d});
  append(out.outline,{x-pitch/4.+a*flank,a});
  append(out.outline,{x+pitch/4.-a*flank,a});
  append(out.outline,{x+pitch/4.+d*flank,-d});
 }
 append(out.outline,{right,-d});
 append(out.outline,{right,-d-params.baseThicknessMm});
 std::reverse(out.outline.begin(),out.outline.end()); // contorno externo anti-horário
 return out;
}
} // namespace alaser::gear
