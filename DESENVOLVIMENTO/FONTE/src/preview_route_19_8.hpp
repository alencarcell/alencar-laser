#pragma once
// Preview-only nearest-neighbor route heuristic. NO CAM, G-code or machine writes.
// Respects ordered layer groups and output filtering, does not edit source geometry.
// Unlike LightBurn's planner, no group priorities, inside-first, corner heuristics
// or collision checking. Closed contours may start at any EXISTING vertex.
#include "preview_sim_19_8.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>

namespace alaser::sim198::route {
struct Choice {std::size_t contourIndex=0;std::size_t startIndex=0;bool reverse=false;};
inline Plan nearestNeighborLines(const EditorState& state,std::size_t cap=300000,
                                  std::size_t maxContours=1500){
 Plan out;
 std::map<int,std::vector<const Contour*>> groups;
 for(const auto& contour:state.doc.contours){
  if(contour.nodes.size()<2)continue;
  const auto meta=state.objects.find(contour.key.object);
  if(meta==state.objects.end())continue;
  const auto layer=state.layers.find(meta->second.layer);
  if(layer==state.layers.end())continue;
  if(layer->first==1001||layer->first==1002){++out.ignoredTool;continue;}
  if(!layer->second.output){++out.ignoredDisabled;continue;}
  if(layer->second.mode!="Line"){++out.ignoredFill;continue;}
  if(!contour.closed&&contour.nodes.size()<2)continue;
  groups[layer->first].push_back(&contour);
 }
 std::size_t totalContours=0;
 for(const auto& pair:groups)totalContours+=pair.second.size();
 if(totalContours>maxContours)
  throw std::runtime_error("Rota virtual: limite de 1500 contornos");
 Pt current{0,0};
 auto add=[&](Pt next,bool laser,int layer){
  const double d=std::hypot(next.x-current.x,next.y-current.y);
  if(!std::isfinite(d))throw std::runtime_error("Rota virtual: ponto invalido");
  if(d<1e-10){current=next;return;}
  if(out.moves.size()>=cap)throw std::runtime_error("Rota virtual acima do limite");
  out.moves.push_back({current,next,laser,layer});
  if(laser)out.cutMm+=d;else out.travelMm+=d;
  current=next;
 };
 for(const auto& [layer,paths]:groups){
  std::vector<bool> used(paths.size(),false);
  for(std::size_t n=0;n<paths.size();++n){
   double best=std::numeric_limits<double>::infinity();
   Choice chosen;
   for(std::size_t j=0;j<paths.size();++j){
    if(used[j])continue;
    const Contour& c=*paths[j];
    // Open lines allow endpoint reversal; closed paths can start at any node.
    const std::size_t options=c.closed?c.nodes.size():2;
    for(std::size_t k=0;k<options;++k){
     const std::size_t i=c.closed?k:(k?c.nodes.size()-1:0);
     const Pt p=c.nodes[i].p;
     const double d=std::hypot(p.x-current.x,p.y-current.y);
     if(!std::isfinite(d))throw std::runtime_error("Rota virtual: coordenada nao finita");
     if(d<best-1e-9){
      best=d;chosen={j,i,!c.closed&&k==1};
     }
    }
   }
   const Contour& c=*paths.at(chosen.contourIndex);
   used[chosen.contourIndex]=true;
   const std::size_t count=c.nodes.size();
   const Pt start=c.nodes[chosen.startIndex].p;
   add(start,false,layer);
   if(c.closed){
    for(std::size_t i=1;i<count;++i)
     add(c.nodes[(chosen.startIndex+i)%count].p,true,layer);
    add(start,true,layer);
   }else if(chosen.reverse){
    for(std::size_t i=chosen.startIndex;i>0;--i)add(c.nodes[i-1].p,true,layer);
   }else{
    for(std::size_t i=1;i<count;++i)add(c.nodes[i].p,true,layer);
   }
  }
 }
 return out;
}
} // namespace alaser::sim198::route
