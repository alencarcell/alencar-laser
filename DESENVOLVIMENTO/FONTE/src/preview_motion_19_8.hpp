#pragma once
// Alencar Laser 19.8 - kinetic time estimator, offline/read-only.
// Deliberately independent of Qt, GRBL and all transport/G-code code.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

namespace alaser::sim198::motion {
struct Settings {
 double cutMmMin=3000.0, rapidMmMin=7000.0;
 double maxXmmMin=12000.0,maxYmmMin=12000.0;
 double cutAccelX=400.0,cutAccelY=300.0; // mm/s^2, hypothetical
 double rapidAccelX=500.0,rapidAccelY=400.0;
 double continuityDegrees=12.0;
};
struct Timeline {
 std::vector<double> endSeconds; // one end timestamp for each source move
 double lineSeconds=0.0,travelSeconds=0.0,totalSeconds=0.0;
 std::size_t runs=0;
};
inline void validate(const Settings& s){
 const double values[]={s.cutMmMin,s.rapidMmMin,s.maxXmmMin,s.maxYmmMin,
   s.cutAccelX,s.cutAccelY,s.rapidAccelX,s.rapidAccelY};
 for(double v:values)if(!(v>0.0)||!std::isfinite(v))
  throw std::runtime_error("Velocidade/aceleracao virtual invalida");
 if(!std::isfinite(s.continuityDegrees)||s.continuityDegrees<0||s.continuityDegrees>45)
  throw std::runtime_error("Angulo de continuidade invalido");
}
inline double segmentSpeed(double dx,double dy,bool line,const Settings& s){
 const double d=std::hypot(dx,dy);if(d<1e-12)return 0;
 const double ux=std::abs(dx)/d,uy=std::abs(dy)/d;
 double mmMin=line?s.cutMmMin:s.rapidMmMin;
 if(ux>1e-12)mmMin=std::min(mmMin,s.maxXmmMin/ux);
 if(uy>1e-12)mmMin=std::min(mmMin,s.maxYmmMin/uy);
 return mmMin/60.0;
}
inline double segmentAccel(double dx,double dy,bool line,const Settings& s){
 const double d=std::hypot(dx,dy);if(d<1e-12)return 0;
 const double ux=std::abs(dx)/d,uy=std::abs(dy)/d;
 double a=std::numeric_limits<double>::infinity();
 if(ux>1e-12)a=std::min(a,(line?s.cutAccelX:s.rapidAccelX)/ux);
 if(uy>1e-12)a=std::min(a,(line?s.cutAccelY:s.rapidAccelY)/uy);
 return a;
}
inline double startStopSeconds(double distance,double speed,double accel){
 if(!std::isfinite(distance)||distance<0||!(speed>0)||!(accel>0))
  throw std::runtime_error("Movimento invalido para simulacao");
 if(distance==0)return 0;
 const double rampDistance=speed*speed/accel; // accel + decel distances combined
 if(distance<=rampDistance)return 2.0*std::sqrt(distance/accel);
 return 2.0*speed/accel+(distance-rampDistance)/speed;
}
template<typename Plan>
Timeline estimate(const Plan& plan,const Settings& s){
 validate(s);
 Timeline timeline;
 timeline.endSeconds.resize(plan.moves.size());
 struct Run{double length=0,speed=0,accel=0;std::size_t first=0,last=0;
            bool line=false;int layer=0;double ux=0,uy=0;};
 Run run;bool has=false;
 const double cosine=std::cos(s.continuityDegrees*3.14159265358979323846/180.0);
 auto flush=[&](){
  if(!has)return;
  const double seconds=startStopSeconds(run.length,run.speed,run.accel);
  // Distribute run duration to edges by length: ordering is reliable,
  // intra-run instantaneous head position is approximate, not GRBL-exact.
  double elapsed=0;
  for(std::size_t i=run.first;i<=run.last;++i){
   const auto& m=plan.moves[i];
   elapsed+=seconds*std::hypot(m.to.x-m.from.x,m.to.y-m.from.y)/run.length;
   timeline.endSeconds[i]=timeline.totalSeconds+elapsed;
  }
  timeline.totalSeconds+=seconds;
  if(run.line)timeline.lineSeconds+=seconds;
  else timeline.travelSeconds+=seconds;
  ++timeline.runs;has=false;
 };
 for(std::size_t i=0;i<plan.moves.size();++i){
  const auto& m=plan.moves[i];
  const double dx=m.to.x-m.from.x,dy=m.to.y-m.from.y;
  const double d=std::hypot(dx,dy);
  if(!std::isfinite(d))throw std::runtime_error("Coordenada nao finita no plano");
  if(d<=1e-12){flush();timeline.endSeconds[i]=timeline.totalSeconds;continue;}
  const double ux=dx/d,uy=dy/d;
  const double speed=segmentSpeed(dx,dy,m.laser,s);
  const double accel=segmentAccel(dx,dy,m.laser,s);
  if(!(speed>0)||!(accel>0))throw std::runtime_error("Perfil de movimento invalido");
  const bool contiguous=has&&std::hypot(m.from.x-plan.moves[i-1].to.x,
                                            m.from.y-plan.moves[i-1].to.y)<1e-6;
  const bool same=contiguous&&m.laser==run.line&&m.layer==run.layer&&
                   ux*run.ux+uy*run.uy>=cosine;
  if(!same){flush();run={0.0,speed,accel,i,i,m.laser,m.layer,ux,uy};has=true;}
  run.length+=d;run.last=i;
  run.speed=std::min(run.speed,speed);run.accel=std::min(run.accel,accel);
  run.ux=ux;run.uy=uy;
 }
 flush();
 return timeline;
}
template<typename Plan>
std::size_t moveIndexAt(const Plan& plan,const Timeline& time,double elapsed){
 if(plan.moves.empty())return 0;
 if(!std::isfinite(elapsed))throw std::runtime_error("Tempo da previsualizacao invalido");
 const auto i=std::lower_bound(time.endSeconds.begin(),time.endSeconds.end(),
                               std::max(0.0,elapsed));
 return i==time.endSeconds.end()?plan.moves.size()-1:
      std::size_t(i-time.endSeconds.begin());
}
} // namespace alaser::sim198::motion
