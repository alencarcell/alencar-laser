// Alencar Laser TESTE 19.8 - alignment/mirroring after bed centering.
// Portable-only regression; no GUI, device access or machine commands.
#include "../src/editor_tools.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace alaser;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static bool near(double x,double y){return std::abs(x-y)<1e-7;}
int main(){
 try{
  Editor editor(Document{});
  const int first=editor.addContour({{10,10},{20,10},{18,25}},true);
  const int second=editor.addContour({{80,30},{110,30},{95,40}},true);
  editor.alignObjects({first,second},6,first);
  auto [alignedA0,alignedA1]=boundsOf(editor.state().doc,{first});
  auto [alignedB0,alignedB1]=boundsOf(editor.state().doc,{second});
  require(near(alignedA0.x+alignedA1.x,alignedB0.x+alignedB1.x)&&
          near(alignedA0.y+alignedA1.y,alignedB0.y+alignedB1.y),
          "both centers did not align");
  // The optional spare drawing stays outside the bed, untouched.
  const int spare=editor.addContour({{510,10},{530,10},{520,25}},true);
  const auto spareBounds=boundsOf(editor.state().doc,{spare});
  const auto [lo,hi]=boundsOf(editor.state().doc,{first,second});
  const Pt displacement{200.-(lo.x+hi.x)/2.,200.-(lo.y+hi.y)/2.};
  editor.moveObjects({first,second},displacement);
  const auto [centeredLo,centeredHi]=boundsOf(editor.state().doc,{first,second});
  require(near((centeredLo.x+centeredHi.x)/2.,200.)&&
          near((centeredLo.y+centeredHi.y)/2.,200.),
          "whole selection not centered at bed center");
  const auto spareAfter=boundsOf(editor.state().doc,{spare});
  require(near(spareBounds.first.x,spareAfter.first.x)&&
          near(spareBounds.second.y,spareAfter.second.y),
          "unselected spare drawing moved");
  const auto [mirrorLo,mirrorHi]=boundsOf(editor.state().doc,{first});
  const double sumY=mirrorLo.y+mirrorHi.y;
  const auto* object=get(editor.state().doc,{first,0});
  require(object!=nullptr&&!object->nodes.empty(),"mirror target missing");
  std::vector<Pt> original;
  for(const auto& n:object->nodes)original.push_back(n.p);
  const auto beforeUndo=editor.undoCount();
  editor.mirrorY({first},sumY/2.);
  const auto* reflected=get(editor.state().doc,{first,0});
  require(reflected&&reflected->nodes.size()==original.size(),"mirror topology changed");
  for(size_t i=0;i<original.size();++i)
   require(near(reflected->nodes[i].p.y,sumY-original[i].y)&&
           near(reflected->nodes[i].p.x,original[i].x),
           "not all nodes reflected around CURRENT axis");
  require(editor.undoCount()==beforeUndo+1&&editor.undo(),
          "mirror should be one undo transaction");
  const auto* undone=get(editor.state().doc,{first,0});
  require(undone&&undone->nodes.size()==original.size(),"undo lost target");
  for(size_t i=0;i<original.size();++i)
   require(dist(undone->nodes[i].p,original[i])<1e-7,"undo not exact");
  std::cout<<"PASS: alignment, centering, spare art and dynamic-axis mirror with undo (offline)\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;
 }
}
