#pragma once
// Alencar Laser 19.9 — calculo e biblioteca DIDATICA, somente editor/offline.
// Estes mecanismos NAO sao certificados para transmitir torque nem cortar.
#include "gear_generator_19_9.hpp"
#include <array>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <string>
#include <vector>
namespace alaser::mechanisms {
using alaser::gear::Point;
enum class Template { ClockTrain, ToyGears, SchoolRack };
enum class Material { MDF, Plywood, Acrylic };
struct Settings {
 Template preset=Template::ToyGears;
 Material material=Material::MDF;
 double thicknessMm=3.;
 double moduleMm=3.;
 double pressureDeg=20.;
 double kerfMm=0.15;          // Apenas informativo; nao aplicar ao contorno.
 double backlashMm=0.12;      // Folga nominal proposta; nao aplicar ao contorno.
 int teeth1=24,teeth2=48;     // Pinhao motor e roda movida
 double boreMm=6.;
};
struct Part {
 std::vector<Point> outer, bore;
 Point origin{};
 int teeth=0;
 bool rack=false;
 double phase=0.;
 double angularRatio=0.;  // radianos da peca por radiano do motor
 double slideRatio=0.;    // mm por radiano do motor (cremalheira)
};
struct Evaluation {
 std::vector<Part> parts;
 std::vector<std::string> warnings;
 double centerDistanceMm=0.;
 double speedRatio=0.;
 double estimatedKerfGapMm=0.; // Diferenca diametral ilustrativa; nunca CAM.
 bool physicallyCertified=false;
 bool hardwareEnabled=false;
};
inline const char* materialName(Material m){
 switch(m){case Material::MDF:return "MDF";case Material::Plywood:return "Compensado";
  case Material::Acrylic:return "Acrilico";}
 return "Nao definido";
}
inline const char* presetName(Template t){
 switch(t){case Template::ClockTrain:return "Trem de engrenagens para estudo de relogios";
  case Template::ToyGears:return "Transmissao para brinquedo didatico";
  case Template::SchoolRack:return "Pinhao e cremalheira para projeto escolar";}
 return "Nao definido";
}
inline Settings defaults(Template p){
 Settings s;s.preset=p;
 if(p==Template::ClockTrain){s.teeth1=24;s.teeth2=48;s.moduleMm=2.5;s.boreMm=4.;}
 else if(p==Template::ToyGears){s.teeth1=24;s.teeth2=36;s.moduleMm=3.;s.boreMm=6.;}
 else {s.teeth1=24;s.teeth2=12;s.moduleMm=3.;s.boreMm=5.;}
 return s;
}
inline Evaluation generate(const Settings& s){
 if(!std::isfinite(s.thicknessMm)||s.thicknessMm<1.||s.thicknessMm>30.||
    !std::isfinite(s.kerfMm)||s.kerfMm<0.||s.kerfMm>1.||
    !std::isfinite(s.backlashMm)||s.backlashMm<0.||
    !std::isfinite(s.moduleMm)||s.moduleMm<0.5||s.moduleMm>20.||
    s.backlashMm>std::min(s.moduleMm*0.4,3.)||
    !std::isfinite(s.pressureDeg)||s.pressureDeg<14.||s.pressureDeg>30.||
    !std::isfinite(s.boreMm)||s.boreMm<0.)
  throw std::invalid_argument("material, espessura, folga ou geometria fora do intervalo seguro de edicao");
 alaser::gear::Parameters base;
 base.teeth=s.teeth1;base.moduleMm=s.moduleMm;
 base.pressureDeg=s.pressureDeg;base.boreMm=s.boreMm;
 auto primary=alaser::gear::generate(base);
 Evaluation out;out.estimatedKerfGapMm=2.*s.kerfMm;
 out.parts.push_back({primary.outer,primary.bore,{},s.teeth1,false,0.,1.,0.});
 if(primary.undercutWarning)out.warnings.push_back("Pinhao com risco de undercut/interferencia.");
 if(s.preset==Template::SchoolRack){
  alaser::gear::RackParameters r;
  r.teeth=s.teeth2;r.moduleMm=s.moduleMm;
  r.pressureDeg=s.pressureDeg;
  r.baseThicknessMm=std::max(s.thicknessMm*3.,2.*s.moduleMm);
  const auto rail=alaser::gear::generateRackNominal(r);
  // Centro do pinhao acima da linha primitiva da cremalheira.
  out.parts.front().origin={0.,primary.pitchRadius};
  out.parts.push_back({rail.outline,{}, {},s.teeth2,true,0.,0.,primary.pitchRadius});
  out.centerDistanceMm=primary.pitchRadius;
  out.speedRatio=0.; // cremalheira translada, nao tem razao de rotacao
  out.warnings.push_back("Cremalheira tem curso teorico; nao inclui batentes nem suporte.");
 }else{
  auto followerParams=base;followerParams.teeth=s.teeth2;
  auto follower=alaser::gear::generate(followerParams);
  const double offset=primary.pitchRadius+follower.pitchRadius;
  out.centerDistanceMm=offset;
  out.speedRatio=double(s.teeth1)/s.teeth2;
  // Fase inicial ilustrativa, nao resolucao numerica de contato real.
  out.parts.push_back({follower.outer,follower.bore,{offset,0.},
                       s.teeth2,false,std::numbers::pi/s.teeth2,-out.speedRatio,0.});
  if(follower.undercutWarning)out.warnings.push_back("Roda movida com risco de undercut/interferencia.");
  if(s.preset==Template::ClockTrain){
   // Modulo exemplar com tres rodas: NAO e um relogio completo nem tem escape.
   auto third=base;third.teeth=24;third.boreMm=s.boreMm;
   auto g3=alaser::gear::generate(third);
   out.parts.push_back({g3.outer,g3.bore,
              {offset+follower.pitchRadius+g3.pitchRadius,0.},
              third.teeth,false,std::numbers::pi/third.teeth,
              double(s.teeth1)/third.teeth,0.});
   out.warnings.push_back("Trem didatico: sem escapamento, eixos, sustentacao ou fonte de energia.");
  }
 }
 if(s.backlashMm<2.*s.kerfMm)
  out.warnings.push_back("Folga indicada menor que duas larguras de kerf: ajuste por cupom de teste.");
 if(s.material==Material::Acrylic)
  out.warnings.push_back("Acrilico: verificar compatibilidade do material com laser de diodo e ventilacao.");
 if(s.thicknessMm<2.)
  out.warnings.push_back("Material fino: verificar resistencia e empenamento.");
 out.warnings.push_back("Perfil involuta aproximado e kerf/folga NAO aplicados aos contornos.");
 out.warnings.push_back("Simulacao cinematica idealizada; sem colisao, atrito ou analise de carga.");
 return out;
}
inline Point transform(const Part& part,Point local,double driverRadians){
 if(part.rack)return {part.origin.x+local.x+part.slideRatio*driverRadians,
                       part.origin.y+local.y};
 const double phi=part.phase+driverRadians*part.angularRatio;
 const double cs=std::cos(phi),sn=std::sin(phi);
 return {part.origin.x+local.x*cs-local.y*sn,part.origin.y+local.x*sn+local.y*cs};
}
} // namespace alaser::mechanisms
