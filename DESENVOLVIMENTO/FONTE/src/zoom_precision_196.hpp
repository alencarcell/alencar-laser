#pragma once
// TESTE 19.6: numeric bounds only. No change to document geometry or device commands.
#include <algorithm>
#include <cmath>
namespace alaser::zoom196 {
constexpr double Minimum=0.03;
constexpr double Maximum=4096.0; // experimental precision viewer; physical work area remains unchanged
constexpr double WheelFactor=1.16;
inline double applyWheel(double oldScale,double wheelSteps){
 if(!std::isfinite(oldScale)||oldScale<=0||!std::isfinite(wheelSteps))return Minimum;
 const double d=oldScale*std::pow(WheelFactor,std::clamp(wheelSteps,-32.0,32.0));
 return std::clamp(d,Minimum,Maximum);
}
inline double clampScale(double v){return std::isfinite(v)?std::clamp(v,Minimum,Maximum):Minimum;}
}
