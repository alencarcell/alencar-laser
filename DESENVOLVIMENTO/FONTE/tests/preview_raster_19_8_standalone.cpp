// Regression of actual raster planner without Qt, networking or machine access.
// Build: g++ -std=c++20 -Wall -Wextra -Werror -O2 preview_raster_19_8_standalone.cpp -o raster_test
#include "../src/preview_raster_19_8.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace alaser;
int main(){
 sim198::raster::Settings cfg{1.,2.,true};
 Editor scene(Document{});
 const int outer=scene.rectangle({0,0},10,10);
 scene.setLayerMode(0,"Fill");
 sim198::Plan p;
 sim198::raster::appendFill(p,scene.state(),cfg);
 assert(std::abs(p.cutMm-100.)<1e-8);
 assert(p.travelMm>0&&p.moves.front().to.x<-1.9);
 const int hole=scene.rectangle({3,3},4,4);
 sim198::Plan hollow;
 sim198::raster::appendFill(hollow,scene.state(),cfg);
 assert(std::abs(hollow.cutMm-84.)<1e-8);
 const auto oldGeneration=scene.generation;
 sim198::Plan hollowAgain;
 sim198::raster::appendFill(hollowAgain,scene.state(),cfg);
 assert(scene.generation==oldGeneration);
 assert(std::abs(hollowAgain.cutMm-hollow.cutMm)<1e-10);
 scene.setLayerPreviewFlags(0,false,false);
 sim198::Plan off;sim198::raster::appendFill(off,scene.state(),cfg);
 assert(off.moves.empty());
 scene.setLayerPreviewFlags(0,true,false);
 scene.addLayer(1001,"T1");
 scene.moveToLayer(outer,1001);
 sim198::Plan guide;sim198::raster::appendFill(guide,scene.state(),cfg);
 assert(std::abs(guide.cutMm-16.)<1e-8);
 assert(!scene.state().layers.at(1001).output);
 // Two binary scan lines; image dimensions remain untouched.
 sim198::raster::Mask mask{4,2,{0,1,1,0,0,0,1,0}};
 cfg.intervalMm=10.;
 sim198::Plan pic;
 sim198::raster::appendMask(pic,mask,{0,0,40,20},cfg);
 assert(std::abs(pic.cutMm-30.)<1e-8);
 assert(std::abs(sim198::repeated(pic,2).cutMm-60.)<1e-8);
 // Unidirectional scanning must not change dark-pixel distances.
 cfg.bidirectional=false;
 sim198::Plan oneWay;
 sim198::raster::appendMask(oneWay,mask,{0,0,40,20},cfg);
 assert(std::abs(oneWay.cutMm-30.)<1e-8);
 bool rejected=false;
 try{
  sim198::raster::Mask invalid{4,2,{1,0}};
  sim198::Plan x;sim198::raster::appendMask(x,invalid,{0,0,40,20},cfg);
 }catch(const std::runtime_error&){rejected=true;}
 assert(rejected);
 rejected=false;
 try{
  cfg.intervalMm=.05;sim198::Plan x;
  sim198::raster::appendMask(x,mask,{0,0,40,100},cfg);
 }catch(const std::runtime_error&){rejected=true;}
 assert(rejected);
 rejected=false;
 try{
  cfg.intervalMm=10.;cfg.overscanMm=21.;sim198::Plan x;
  sim198::raster::appendMask(x,mask,{0,0,40,20},cfg);
 }catch(const std::runtime_error&){rejected=true;}
 assert(rejected);
 (void)hole;
 std::cout<<"PASS: even-odd Fill/hole, image mask, overscan, tool layers, flags, bounds, read-only\n";
 return 0;
}
