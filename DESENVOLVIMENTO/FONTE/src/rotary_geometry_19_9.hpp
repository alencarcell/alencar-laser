#pragma once
// Rotativo 19.9: MODELO MATEMATICO OFFLINE. Nunca gera movimento, GRBL ou G-code.
#include <cmath>
#include <numbers>
#include <stdexcept>
namespace alaser::rotary {
enum class Type { Roller=0, Chuck=1 };
enum class Axis { Y=0, Z=1, A=2 };
struct Profile {
 Type type=Type::Roller;
 Axis requestedAxis=Axis::Y; // escolha editorial, NÃO conexão confirmada
 bool previewEnabled=false;
 bool mirrorPreview=false;
 double mmPerRotation=0.;    // desconhecido até consultar manual e calibrar
 double rollerDiameterMm=0.; // só aplicável a Roller
 double objectDiameterMm=0.; // valor medido do objeto
};
inline bool validOptional(double value){
 return std::isfinite(value)&&value>=0.&&value<=100000.;
}
inline double circumferenceMm(double diameter){
 if(!std::isfinite(diameter)||diameter<=0.||diameter>100000.)
  throw std::invalid_argument("diametro invalido para circunferencia");
 return std::numbers::pi*diameter;
}
inline void validate(const Profile& p){
 if(p.type!=Type::Roller&&p.type!=Type::Chuck)
  throw std::invalid_argument("tipo rotativo invalido");
 if(p.requestedAxis!=Axis::Y&&p.requestedAxis!=Axis::Z&&p.requestedAxis!=Axis::A)
  throw std::invalid_argument("eixo rotativo invalido");
 if(!validOptional(p.mmPerRotation)||!validOptional(p.rollerDiameterMm)||
    !validOptional(p.objectDiameterMm))
  throw std::invalid_argument("parametro rotativo invalido");
 if(p.previewEnabled){
  if(p.mmPerRotation<=0.||p.objectDiameterMm<=0.||
     (p.type==Type::Roller&&p.rollerDiameterMm<=0.))
    throw std::invalid_argument("perfil rotativo incompleto: confirmar medidas");
 }
}
inline double objectTurnDegrees(double surfaceTravelMm,const Profile& p){
 validate(p);
 if(!std::isfinite(surfaceTravelMm)||p.objectDiameterMm<=0.)
  throw std::invalid_argument("sem diametro de objeto valido");
 return 360.0*surfaceTravelMm/circumferenceMm(p.objectDiameterMm);
}
inline double rollerTurns(double surfaceTravelMm,const Profile& p){
 validate(p);
 if(p.type!=Type::Roller||p.rollerDiameterMm<=0.||!std::isfinite(surfaceTravelMm))
  throw std::invalid_argument("rolete nao configurado");
 return surfaceTravelMm/circumferenceMm(p.rollerDiameterMm);
}
inline bool exceedsPerimeter(double extentMm,const Profile& p){
 if(!std::isfinite(extentMm)||extentMm<0.||p.objectDiameterMm<=0.)
  throw std::invalid_argument("arte ou objeto sem medidas");
 return extentMm>circumferenceMm(p.objectDiameterMm)+1e-7;
}
} // namespace alaser::rotary
