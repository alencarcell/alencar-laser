// Portable regression: g++ -std=c++20 -Wall -Wextra -Werror -O2
// -I ../src preview_motion_19_8_standalone.cpp -o preview_motion_test
#include "../src/preview_motion_19_8.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>
struct Pt{double x=0,y=0;};
struct Move{Pt from,to;bool laser=false;int layer=0;};
struct Plan{std::vector<Move> moves;};
static bool near(double a,double b,double tolerance=1e-8){return std::abs(a-b)<tolerance;}
int main(){
 using namespace alaser::sim198::motion;
 assert(near(startStopSeconds(10,10,10),2));
 assert(near(startStopSeconds(100,10,10),11));
 assert(near(startStopSeconds(0,10,10),0));
 // Exemplos matematicos, sem comando GRBL: M3 mantem nominal;
 // M4 reduz proporcionalmente a velocidade e zera em velocidade zero.
 assert(near(virtualPowerPercent(40.,"M3",500.,1000.),40.));
 assert(near(virtualPowerPercent(40.,"M4",500.,1000.),20.));
 assert(near(virtualPowerPercent(40.,"M4",0.,1000.),0.));
 assert(near(virtualPowerPercent(40.,"M4",2000.,1000.),40.));

 Settings cfg;cfg.maxXmmMin=600;cfg.maxYmmMin=1200;
 assert(near(segmentSpeed(10,0,true,cfg),10));
 assert(near(segmentSpeed(0,10,true,cfg),20));
 assert(near(segmentSpeed(10,10,true,cfg),600*std::sqrt(2.)/60));
 assert(near(segmentAccel(10,0,true,cfg),cfg.cutAccelX));
 assert(near(segmentAccel(0,10,true,cfg),cfg.cutAccelY));
 Plan p{{{{0,0},{10,0},true,1},{{10,0},{20,0},true,1},
         {{20,0},{20,10},true,1},{{20,10},{10,10},false,1}}};
 auto t=estimate(p,cfg);
 // GUI and exported JSON must share the same nominal, per-layer model.
 Plan feeds{{{{0,0},{100,0},true,2},{{100,0},{200,0},false,2}}};
 Settings rated;
 rated.cutMmMin=3000.;rated.rapidMmMin=6000.;
 assert(near(nominalSecondsWithFeeds(feeds,rated,[&](int){return 6000.;}),2.));
 assert(near(nominalSecondsWithFeeds(feeds,rated,[&](int){return 3000.;}),3.));
 bool invalidFeed=false;
 try{(void)nominalSecondsWithFeeds(feeds,rated,[&](int){return 0.;});}
 catch(const std::runtime_error&){invalidFeed=true;}
 assert(invalidFeed);
 assert(t.endSeconds.size()==4&&t.runs==3);
 assert(t.totalSeconds>0&&near(t.totalSeconds,t.lineSeconds+t.travelSeconds));
 assert(t.endSeconds[1]>t.endSeconds[0]&&t.endSeconds[2]>t.endSeconds[1]);
 assert(moveIndexAt(p,t,t.endSeconds[0]+1e-4)==1);
 assert(moveIndexAt(p,t,t.totalSeconds+1)==3);
 Settings huge=cfg;huge.cutAccelX=huge.cutAccelY=1e8;
 huge.rapidAccelX=huge.rapidAccelY=1e8;
 assert(estimate(p,huge).totalSeconds<t.totalSeconds);
 bool bad=false;try{auto z=cfg;z.cutAccelX=0;estimate(p,z);}catch(const std::runtime_error&){bad=true;}assert(bad);
 bad=false;try{auto z=cfg;z.continuityDegrees=99;estimate(p,z);}catch(const std::runtime_error&){bad=true;}assert(bad);
 bad=false;try{startStopSeconds(-1,10,10);}catch(const std::runtime_error&){bad=true;}assert(bad);
 bad=false;try{startStopSeconds(1,0,1);}catch(const std::runtime_error&){bad=true;}assert(bad);
 assert(estimate(Plan{},cfg).endSeconds.empty());
 // 1000 deterministic seeded random paths: timestamps monotonic, finite,
 // estimated time >= bounded straight-line constant-speed baseline.
 std::mt19937_64 rng(198);
 std::uniform_real_distribution<double> d(-10,10);
 std::uniform_int_distribution<int> active(0,1);
 for(int sample=0;sample<1000;++sample){
  Plan test;Pt last{0,0};double baseline=0;
  for(int i=0;i<25;++i){
   Pt to{last.x+d(rng),last.y+d(rng)};
   bool laser=active(rng)>0;
   test.moves.push_back({last,to,laser,i%3});
   const double dx=to.x-last.x,dy=to.y-last.y;
   baseline+=std::hypot(dx,dy)/segmentSpeed(dx,dy,laser,cfg);
   last=to;
  }
  auto timeline=estimate(test,cfg);
  assert(timeline.endSeconds.size()==test.moves.size());
  assert(std::isfinite(timeline.totalSeconds));
  assert(timeline.totalSeconds+1e-10>=baseline);
  assert(near(timeline.totalSeconds,timeline.endSeconds.back(),1e-5));
  for(std::size_t j=1;j<timeline.endSeconds.size();++j)
   assert(timeline.endSeconds[j]>=timeline.endSeconds[j-1]);
 }
 std::cout << "PASS: profiles, axis caps/accel, continuity, 1000 random trajectories\n";
}
