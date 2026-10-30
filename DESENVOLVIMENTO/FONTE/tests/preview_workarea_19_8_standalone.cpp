#include "../src/preview_workarea_19_8.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace alaser;
using namespace alaser::sim198;
static void req(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
int main(){
 try{
  Pt a{},b{};
  req(bed198::clip({-10,20},{40,20},100,100,a,b),"clip horizontal");
  req(std::abs(a.x)<1e-8&&std::abs(b.x-40)<1e-8,"horizontal limits");
  req(bed198::clip({20,-30},{20,130},100,100,a,b),"clip vertical");
  req(std::abs(a.y)<1e-8&&std::abs(b.y-100)<1e-8,"vertical limits");
  req(!bed198::clip({-10,-10},{-5,-5},100,100,a,b),"outside excluded");
  req(!bed198::clip({-10,0},{0,0},100,100,a,b),"zero-length tangent excluded");
  Plan full;
  full.moves={{{0,0},{-10,20},false,1},
              {{-10,20},{40,20},true,1},
              {{40,20},{120,20},true,1},
              {{120,20},{130,20},true,1},
              {{130,20},{120,40},false,1},
              {{120,40},{50,40},true,1}};
  bed198::Report summary;
  const Plan filtered=bed198::restrictToBed(full,100,100,&summary);
  req(summary.outsideSegments>=1&&summary.clippedSegments>=2,"out-of-bed counters");
  req(summary.outsideLaserSegments==1&&summary.clippedLaserSegments>=2,
      "laser-only outside counters must exclude travel");
  req(summary.omittedLaserMm>0,"excluded laser distance");
  req(!filtered.moves.empty(),"filtered plan");
  for(const auto& move:filtered.moves)
   req(bed198::inside(move.from,100,100)&&bed198::inside(move.to,100,100),
       "filtered move stays on bed");
  for(size_t i=1;i<filtered.moves.size();++i){
   const auto& prev=filtered.moves[i-1];
   const auto& cur=filtered.moves[i];
   req(std::hypot(prev.to.x-cur.from.x,prev.to.y-cur.from.y)<1e-8,
       "retained fragments connected");
  }
  req(filtered.cutMm<full.moves.size()*100.,"bounded distance");
  Plan inside;inside.moves={{{0,0},{10,10},false,2},{{10,10},{30,10},true,2}};
  const Plan same=bed198::restrictToBed(inside,100,100);
  req(same.moves.size()==inside.moves.size()&&std::abs(same.cutMm-20.)<1e-9,
      "all-inside plan unaffected");
  std::cout<<"PASS: preview restricted to bed, warnings, connected OFF travels; no hardware\n";
  return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
