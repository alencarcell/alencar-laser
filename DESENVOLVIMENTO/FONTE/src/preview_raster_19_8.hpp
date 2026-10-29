#pragma once
// Geometric scan simulator only: NO G-code, physical output, motors or laser control.
// Fill: even-odd pairing across all closed paths on the same layer.
// Image: binary mask only; Qt adapter handles threshold/dither without saving pixels.
// Overscan is a VISUAL off-segment and never authorizes machine motion.
#include "preview_sim_19_8.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

namespace alaser::sim198::raster {
struct Settings {
 double intervalMm=0.2;      // virtual scanline spacing
 double overscanMm=1.0;      // hypothetical off-distance on both ends
 bool bidirectional=true;
};
struct Mask {
 int width=0,height=0;
 std::vector<unsigned char> black; // 0=off, 1=on, row major
};
struct Bounds {double x=0,y=0,width=0,height=0;};
inline void validate(const Settings& s){
 if(!std::isfinite(s.intervalMm)||s.intervalMm<0.05||s.intervalMm>5 ||
    !std::isfinite(s.overscanMm)||s.overscanMm<0||s.overscanMm>20)
  throw std::runtime_error("Intervalo ou overscan virtual fora dos limites");
}
inline double distance(Pt a,Pt b){return std::hypot(a.x-b.x,a.y-b.y);}
inline void add(Plan& out,Pt end,bool on,int layer,std::size_t cap){
 Pt previous=out.moves.empty()?Pt{}:out.moves.back().to;
 const double len=distance(previous,end);
 if(!std::isfinite(len))throw std::runtime_error("Ponto nao finito no raster");
 if(len<1e-10)return;
 if(out.moves.size()>=cap)throw std::runtime_error("Raster virtual excedeu limite de movimentos");
 out.moves.push_back({previous,end,on,layer});
 if(on)out.cutMm+=len;else out.travelMm+=len;
}
inline void emitRow(Plan& out,double y,const std::vector<std::pair<double,double>>& spans,
                    bool reverse,int layer,const Settings& settings,std::size_t cap){
 if(spans.empty())return;
 const double minX=spans.front().first,maxX=spans.back().second;
 if(!reverse){
  add(out,{minX-settings.overscanMm,y},false,layer,cap);
  for(const auto& span:spans){
   add(out,{span.first,y},false,layer,cap);
   add(out,{span.second,y},true,layer,cap);
  }
  add(out,{maxX+settings.overscanMm,y},false,layer,cap);
 }else{
  add(out,{maxX+settings.overscanMm,y},false,layer,cap);
  for(auto it=spans.rbegin();it!=spans.rend();++it){
   add(out,{it->second,y},false,layer,cap);
   add(out,{it->first,y},true,layer,cap);
  }
  add(out,{minX-settings.overscanMm,y},false,layer,cap);
 }
}
inline void appendFill(Plan& out,const EditorState& state,const Settings& settings,
                       std::size_t cap=250000){
 validate(settings);
 // Detect open contours in Fill explicitly: no accidental closure or emission.
 std::map<int,std::vector<const Contour*>> groups;
 for(const auto& c:state.doc.contours){
  // Importacoes LightBurn podem representar fecho repetindo o primeiro vertice.
  if(c.nodes.size()<3||!(c.closed||hasExactDuplicateClosure(c)))continue;
  auto meta=state.objects.find(c.key.object);
  if(meta==state.objects.end())continue;
  auto layer=state.layers.find(meta->second.layer);
  if(layer==state.layers.end()||layer->first==1001||layer->first==1002 ||
      !layer->second.output||layer->second.mode!="Fill")continue;
  groups[layer->first].push_back(&c);
 }
 // The preview dialog recalculates synchronously; cap potential intersection
 // work before entering an O(rows * vertices) raster scan.
 std::size_t scanBudget=0;
 constexpr std::size_t maxScanChecks=6000000;
 for(const auto& [layer,paths]:groups){
  double ymin=std::numeric_limits<double>::infinity(),ymax=-ymin;
  for(const auto* c:paths)for(const auto& node:c->nodes){
   ymin=std::min(ymin,node.p.y);ymax=std::max(ymax,node.p.y);
  }
  if(!std::isfinite(ymin)||!std::isfinite(ymax))throw std::runtime_error("Fill invalido");
  const double range=ymax-ymin;
  if(!std::isfinite(range)||range>10000)
   throw std::runtime_error("Altura Fill virtual fora do limite");
  if(range<=0)continue;
  const double rows=std::ceil(range/settings.intervalMm);
  if(rows>1500)throw std::runtime_error("Fill ultrapassa 1500 linhas virtuais");
  std::size_t edges=0;
  for(const Contour* shape:paths){
   if(shape->nodes.size()>maxScanChecks)
    throw std::runtime_error("Fill com contorno complexo demais para a previa");
   edges+=shape->nodes.size();
   if(edges>maxScanChecks)
    throw std::runtime_error("Fill com vertices demais para a previa");
  }
  const auto rowCount=std::size_t(rows);
  if(rowCount>0&&edges>(maxScanChecks-scanBudget)/rowCount)
   throw std::runtime_error("Fill excedeu limite de intersecoes virtuais");
  scanBudget+=edges*rowCount;
  int row=0;
  for(double y=ymin+settings.intervalMm*0.5;y<ymax;++row,y=ymin+settings.intervalMm*(row+0.5)){
   std::vector<double> xs;
   for(const Contour* contour:paths){
    const auto& nodes=contour->nodes;
    for(std::size_t i=0;i<nodes.size();++i){
     const auto a=nodes[i].p,b=nodes[(i+1)%nodes.size()].p;
     // half-open rule removes double vertices and horizontal edges
     if((a.y<=y&&b.y>y)||(b.y<=y&&a.y>y)){
      const double x=a.x+(y-a.y)*(b.x-a.x)/(b.y-a.y);
      if(!std::isfinite(x))throw std::runtime_error("Fill intersection not finite");
      xs.push_back(x);
     }
    }
   }
   std::sort(xs.begin(),xs.end());
   if(xs.size()%2)throw std::runtime_error("Intersecoes Fill impares: contorno inconsistente");
   std::vector<std::pair<double,double>> spans;
   for(std::size_t i=0;i+1<xs.size();i+=2)
    if(xs[i+1]-xs[i]>1e-8)spans.emplace_back(xs[i],xs[i+1]);
   emitRow(out,y,spans,settings.bidirectional&&(row%2!=0),layer,settings,cap);
  }
 }
}
inline void appendMask(Plan& out,const Mask& mask,const Bounds& box,
                       const Settings& settings,std::size_t cap=250000){
 validate(settings);
 if(mask.width<1||mask.height<1||mask.width>1000||mask.height>1000 ||
    mask.black.size()!=std::size_t(mask.width)*std::size_t(mask.height))
  throw std::runtime_error("Mascara virtual invalida ou muito grande");
 if(!std::isfinite(box.x)||!std::isfinite(box.y)||!std::isfinite(box.width)||
    !std::isfinite(box.height)||box.width<=0||box.height<=0||
    box.width>10000||box.height>10000)
  throw std::runtime_error("Dimensoes da imagem virtual invalidas");
 // Keep the user-requested physical line interval. The previous version
 // redistributed a ceil() number of rows over the image height, changing
 // the selected pitch and therefore its nominal time and raster density.
 // For an image thinner than half the spacing, show a single centered row.
 const double pitch=settings.intervalMm;
 const int rows=std::max(1,int(std::ceil(box.height/pitch-0.5)));
 if(rows>1500)throw std::runtime_error("Imagem ultrapassa 1500 linhas virtuais");
 const double px=box.width/double(mask.width);
 for(int row=0;row<rows;++row){
  const double localY=box.height<pitch*0.5?box.height*0.5:(row+0.5)*pitch;
  const double y=box.y+localY;
  const int sy=std::clamp(int(mask.height*(localY/box.height)),0,mask.height-1);
  std::vector<std::pair<double,double>> spans;
  for(int col=0;col<mask.width;){
   if(!mask.black[std::size_t(sy)*mask.width+col]){++col;continue;}
   const int begin=col;
   while(col<mask.width&&mask.black[std::size_t(sy)*mask.width+col])++col;
   spans.push_back({box.x+begin*px,box.x+col*px});
  }
  emitRow(out,y,spans,settings.bidirectional&&(row%2!=0),-1,settings,cap);
 }
}
} // namespace alaser::sim198::raster
