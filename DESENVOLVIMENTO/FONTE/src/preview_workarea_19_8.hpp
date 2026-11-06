#pragma once
// Somente visualizacao offline. Recorta trajetos AO LIMITE DA MESA, sem mudar
// geometria .alaser, sem G-code e sem liberar hardware.
#include "preview_sim_19_8.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace alaser::sim198::bed198 {
struct Report {
 std::size_t outsideSegments=0, clippedSegments=0, inputSegments=0;
 // Laser-ON statistics exclude positioning travels, avoiding false alarms
 // about non-cutting motion that happens to run outside the visual rectangle.
 std::size_t outsideLaserSegments=0,clippedLaserSegments=0;
 double omittedLaserMm=0.,clippedLaserMm=0.;
};
// A completely external object can be kept as an offline spare in the
// editor. Only a laser-ON segment crossing the physical bed is a warning.
// The full outside count is retained in diagnostic JSON, not a danger banner.
inline bool partiallyOutsideActiveGeometry(const Report& report){
 return report.clippedLaserSegments>0;
}
inline bool inside(Pt p,double width,double height,double epsilon=1e-7){
 return p.x>=-epsilon&&p.y>=-epsilon&&p.x<=width+epsilon&&p.y<=height+epsilon;
}
// Liang-Barsky clipping: endpoints inclusive, robust for vertical/horizontal.
inline bool clip(Pt a,Pt b,double w,double h,Pt& outA,Pt& outB){
 if(!(std::isfinite(w)&&std::isfinite(h)&&w>0&&h>0&&
      std::isfinite(a.x)&&std::isfinite(a.y)&&std::isfinite(b.x)&&std::isfinite(b.y)))
  throw std::runtime_error("Area ou coordenadas nao finitas na previa offline");
 double t0=0.,t1=1.;
 const double dx=b.x-a.x,dy=b.y-a.y;
 const double p[4]={-dx,dx,-dy,dy};
 const double q[4]={a.x,w-a.x,a.y,h-a.y};
 for(int i=0;i<4;++i){
  if(std::abs(p[i])<1e-13){if(q[i]<0.)return false;continue;}
  const double t=q[i]/p[i];
  if(p[i]<0.)t0=std::max(t0,t);
  else t1=std::min(t1,t);
  if(t0>t1)return false;
 }
 outA={std::clamp(a.x+dx*t0,0.,w),std::clamp(a.y+dy*t0,0.,h)};
 outB={std::clamp(a.x+dx*t1,0.,w),std::clamp(a.y+dy*t1,0.,h)};
 return std::hypot(outB.x-outA.x,outB.y-outA.y)>1e-10;
}
inline Plan restrictToBed(const Plan& original,double width,double height,Report* report=nullptr){
 Report r;Plan out;
 out.ignoredFill=original.ignoredFill;
 out.ignoredTool=original.ignoredTool;
 out.ignoredDisabled=original.ignoredDisabled;
 Pt last{0.,0.};
 auto append=[&](Pt a,Pt b,bool laser,int layer){
  const double length=std::hypot(b.x-a.x,b.y-a.y);
  if(length<=1e-9)return;
  out.moves.push_back({a,b,laser,layer});
  if(laser)out.cutMm+=length;else out.travelMm+=length;
  last=b;
 };
 for(const auto& move:original.moves){
  ++r.inputSegments;
  Pt start{},end{};
  const double total=std::hypot(move.to.x-move.from.x,move.to.y-move.from.y);
  const bool wasInside=inside(move.from,width,height)&&inside(move.to,width,height);
  if(!clip(move.from,move.to,width,height,start,end)){
   ++r.outsideSegments;
   if(move.laser){++r.outsideLaserSegments;r.omittedLaserMm+=total;}
   continue;
  }
  if(!wasInside){
   ++r.clippedSegments;
   if(move.laser){
    ++r.clippedLaserSegments;
    const double excluded=std::max(0.,total-std::hypot(end.x-start.x,end.y-start.y));
    r.omittedLaserMm+=excluded;
    r.clippedLaserMm+=excluded;
   }
  }
  // Rejoin retained fragments as OFF travel, never as an ON jump.
  if(std::hypot(start.x-last.x,start.y-last.y)>1e-7)append(last,start,false,move.layer);
  append(start,end,move.laser,move.layer);
 }
 if(report)*report=r;
 return out;
}
} // namespace alaser::sim198::bed198
