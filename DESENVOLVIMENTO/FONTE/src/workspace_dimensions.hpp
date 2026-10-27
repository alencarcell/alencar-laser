#pragma once
// TESTE_19.2: dimensões de visualização; NÃO definem curso físico nem autorizam movimento.
#include <cmath>
#include <stdexcept>
#include <string>
namespace alaser {
struct WorkspaceConfig {
 double drawingWidth=400.0, drawingHeight=400.0;
 double machineWidth=400.0, machineHeight=400.0;
 bool showGrid=true, showMachineFrame=true, showOrigin=true;
 bool gridToBed=true; // A grade pode seguir a mesa visual, independentemente do desenho.
 std::string profile="AlgoLaser Alpha Mk2 20W";
 // Limite numérico preventivo para QGraphics/Zoom; não é limite do equipamento.
 static constexpr double MaxEditableDimension=1000000.0;
 static bool goodDimension(double mm) {
  return std::isfinite(mm) && mm>=1.0 && mm<=MaxEditableDimension;
 }
 void validate() const {
  if(!goodDimension(drawingWidth)||!goodDimension(drawingHeight)||
     !goodDimension(machineWidth)||!goodDimension(machineHeight))
    throw std::invalid_argument("Dimensoes invalidas: cada eixo deve estar entre 1 e 1000000 mm");
  if(profile.empty()||profile.size()>120)throw std::invalid_argument("Perfil de maquina invalido");
 }
};
}
