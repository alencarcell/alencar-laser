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
 scene.setLayerFlags(0,false,false); // Show OFF must NOT disable Output
 sim198::Plan hidden;sim198::raster::appendFill(hidden,scene.state(),cfg);
 assert(std::abs(hidden.cutMm-84.)<1e-8);
 scene.setLayerFlags(0,true,false);
 scene.addLayer(1001,"T1");
 scene.moveToLayer(outer,1001);
 sim198::Plan guide;sim198::raster::appendFill(guide,scene.state(),cfg);
 assert(std::abs(guide.cutMm-16.)<1e-8);
 assert(!scene.state().layers.at(1001).output);
 // Two binary scan lines; image dimensions remain untouched.
 sim198::raster::Mask mask{4,2,{0,1,1,0,0,0,1,0}};
 cfg.intervalMm=5.; // maximum supported by the simulator/UI
 sim198::raster::validate(cfg); // valid at the upper boundary
 bool intervalRejected=false;
 try{
  auto invalidInterval=cfg;invalidInterval.intervalMm=10.;
  sim198::raster::validate(invalidInterval);
 }catch(const std::runtime_error&){intervalRejected=true;}
 assert(intervalRejected); // 10 mm is intentionally NOT a supported pitch
 sim198::Plan pic;
 sim198::raster::appendMask(pic,mask,{0,0,40,10},cfg);
 assert(std::abs(pic.cutMm-30.)<1e-8);
 assert(std::abs(sim198::repeated(pic,2).cutMm-60.)<1e-8);
 // Unidirectional scanning must not change dark-pixel distances.
 cfg.bidirectional=false;
 sim198::Plan oneWay;
 sim198::raster::appendMask(oneWay,mask,{0,0,40,10},cfg);
 assert(std::abs(oneWay.cutMm-30.)<1e-8);
 // Image height that is not a multiple of the interval must NOT compress
 // the requested pitch (old implementation used 12.5/3 rather than 5 mm).
 cfg.bidirectional=true;
 sim198::raster::Mask allBlack{4,2,{1,1,1,1,1,1,1,1}};
 sim198::Plan fixedPitch;
 sim198::raster::appendMask(fixedPitch,allBlack,{0,0,40,12.5},cfg);
 std::vector<double> imageRows;
 for(const auto& move:fixedPitch.moves)
  if(move.laser)imageRows.push_back(move.from.y);
 assert(imageRows.size()==2);
 assert(std::abs(imageRows[0]-2.5)<1e-8&&
        std::abs(imageRows[1]-7.5)<1e-8);
 sim198::Plan tiny;
 sim198::raster::appendMask(tiny,allBlack,{0,0,40,2},cfg);
 int tinyOn=0;
 for(const auto& move:tiny.moves)if(move.laser){
  ++tinyOn;assert(std::abs(move.from.y-1.)<1e-8);
 }
 assert(tinyOn==1);
 bool rejected=false;
 try{
  sim198::raster::Mask invalid{4,2,{1,0}};
  sim198::Plan x;sim198::raster::appendMask(x,invalid,{0,0,40,10},cfg);
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
  Editor expensive(Document{});
  expensive.ellipse({0,50},10,50,4096);
  expensive.ellipse({30,50},10,50,4096);
  expensive.setLayerMode(0,"Fill");
  sim198::raster::Settings dense{0.1,0.0,true};
  sim198::Plan x;
  sim198::raster::appendFill(x,expensive.state(),dense);
 }catch(const std::runtime_error&){rejected=true;}
 assert(rejected); // 8192 vertices x 1000 scanlines > 6M budget
 rejected=false;
 try{
  cfg.intervalMm=5.;cfg.overscanMm=21.;sim198::Plan x;
  sim198::raster::appendMask(x,mask,{0,0,40,10},cfg);
 }catch(const std::runtime_error&){rejected=true;}
 assert(rejected);
 (void)hole;
 std::cout<<"PASS: even-odd Fill/hole, image mask, overscan, tool layers, flags, bounds, read-only\n";
 return 0;
}
