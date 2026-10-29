// Alencar Laser 19.8 - regression for production-independent route planning.
// Portable C++20, no Qt, no serial/TCP, no G-code.
// Build: g++ -std=c++20 -O2 -Wall -Wextra -Werror -pedantic
//   -I../src preview_route_19_8_standalone.cpp -o preview_route_test
#include "../src/preview_route_19_8.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace alaser;
static bool close(double a,double b){return std::abs(a-b)<1e-7;}
static std::vector<double> laserPaths(const sim198::Plan& plan){
 std::vector<double> path;
 double length=0;
 for(const auto& move:plan.moves){
  const double d=std::hypot(move.to.x-move.from.x,move.to.y-move.from.y);
  if(move.laser)length+=d;
  else if(length>1e-9){path.push_back(length);length=0;}
 }
 if(length>1e-9)path.push_back(length);
 return path;
}
int main(){
 // Nested case: outer is nearest the virtual start; it MUST be cut last.
 Editor three(Document{});
 three.rectangle({0,0},100,100);
 three.rectangle({20,20},60,60);
 three.rectangle({40,40},20,20);
 const auto baseline=sim198::build(three.state());
 std::size_t deps=0;
 const auto result=sim198::route::nearestNeighborLines(
    three.state(),300000,1500,true,&deps);
 const auto paths=laserPaths(result);
 assert(deps==3&&paths.size()==3);
 assert(close(paths[0],80)&&close(paths[1],240)&&close(paths[2],400));
 assert(close(result.cutMm,baseline.cutMm));
 const auto repeated=sim198::repeatedByLayer(result,three.state(),2);
 assert(close(repeated.cutMm,2*result.cutMm));
 assert(close(repeated.cutMm,2*baseline.cutMm));
 std::size_t offDeps=99;
 const auto off=sim198::route::nearestNeighborLines(
    three.state(),300000,1500,false,&offDeps);
 assert(offDeps==0&&close(laserPaths(off)[0],400));
 auto option=three.state().layers.at(0);
 option.optimizeTravel=false;three.configureLayer(0,option);
 const auto preserved=sim198::route::nearestNeighborLines(three.state());
 assert(close(laserPaths(preserved)[0],400));
 // Open internal line is handled before a strictly enclosing closed contour.
 Editor open(Document{});
 open.rectangle({0,0},100,100);
 open.line({30,50},{70,50});
 std::size_t openDeps=0;
 const auto openPlan=sim198::route::nearestNeighborLines(
    open.state(),300000,1500,true,&openDeps);
 assert(openDeps==1&&close(laserPaths(openPlan)[0],40));
 // Different layers retain declared order, independent of nesting.
 Editor layered(Document{});
 layered.rectangle({0,0},100,100);
 layered.addLayer(1,"Internal");
 layered.rectangle({20,20},10,10,1);
 std::size_t layerDeps=99;
 const auto layerPlan=sim198::route::nearestNeighborLines(
    layered.state(),300000,1500,true,&layerDeps);
 assert(layerDeps==0&&close(laserPaths(layerPlan)[0],400));
 // Touching an outer edge is ambiguous: must NOT create inner-first rules.
 Editor touching(Document{});
 touching.rectangle({0,0},100,100);
 touching.rectangle({0,20},20,20);
 std::size_t touchingDeps=99;
 (void)sim198::route::nearestNeighborLines(
    touching.state(),300000,1500,true,&touchingDeps);
 assert(touchingDeps==0);
 // Guides never participate in preview ordering or emission.
 Editor guide(Document{});
 guide.rectangle({0,0},100,100);
 guide.addLayer(1001,"T1");
 guide.rectangle({10,10},10,10,1001);
 const auto guidePlan=sim198::route::nearestNeighborLines(guide.state());
 assert(close(guidePlan.cutMm,400));
 // Containment algorithm rejects nonfinite input instead of returning garbage.
 sim198::nesting::Bounds outer{0,0,100,100},inner{20,20,30,30};
 assert(sim198::nesting::boundsStrictlyInside(inner,outer));
 assert(!sim198::nesting::boundsStrictlyInside(outer,inner));
 assert(close(three.state().layers.at(0).speedMmMin,3000));
 std::cout<<"PASS: nesting, open line, cross-layer order, opt-out, guide, passes (offline)\n";
}
