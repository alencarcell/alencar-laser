#pragma once
// Previa geométrica APENAS de leitura. Nunca gera G-code nem acessa transporte.
// Primeira etapa: percursos Line em ordem do documento, sem otimização ou aceleração.
// Fill/Image devem ser simulados por motor próprio antes de qualquer execução física.
#include "editor_tools.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>
namespace alaser::sim198 {
struct Move {
 Pt from{},to{};
 bool laser=false;
 int layer=0;
};
struct Plan {
 std::vector<Move> moves;
 double cutMm=0,travelMm=0;
 size_t ignoredFill=0,ignoredDisabled=0,ignoredTool=0;
};
inline Plan build(const EditorState& state,size_t maxSegments=300000){
 Plan out;
 Pt last{0,0};
 for(const auto& c:state.doc.contours){
  if(c.nodes.size()<2)continue;
  auto meta=state.objects.find(c.key.object);
  if(meta==state.objects.end())continue;
  auto li=state.layers.find(meta->second.layer);
  if(li==state.layers.end())continue;
  const auto& layer=li->second;
  if(layer.id==1001||layer.id==1002){++out.ignoredTool;continue;}
  if(!layer.output){++out.ignoredDisabled;continue;}
  if(layer.mode!="Line"){++out.ignoredFill;continue;}
  const Pt start=c.nodes.front().p;
  auto append=[&](Pt a,Pt b,bool laser){
   const double d=std::hypot(b.x-a.x,b.y-a.y);
   if(!std::isfinite(d))throw std::runtime_error("Simulacao com coordenada invalida");
   if(d<1e-9)return;
   if(out.moves.size()>=maxSegments)throw std::runtime_error("Previa excedeu 300 mil segmentos");
   out.moves.push_back({a,b,laser,layer.id});
   if(laser)out.cutMm+=d;else out.travelMm+=d;
  };
  append(last,start,false);
  for(size_t i=1;i<c.nodes.size();++i)
   append(c.nodes[i-1].p,c.nodes[i].p,true);
  if(c.closed)append(c.nodes.back().p,start,true);
  last=c.closed?start:c.nodes.back().p;
 }
 return out;
}
// Repete o plano vetorial completo, NAO a ordenacao por camada/forma do LightBurn.
// Entre passagens, retorna virtualmente a origem antes de reiniciar o plano.
// Nao produz comandos nem escreve no documento original.
inline Plan repeated(const Plan& single,int passes,size_t maxMoves=300000){
 if(passes<1||passes>20)throw std::runtime_error("Passagens virtuais fora de 1..20");
 if(!single.moves.empty()&&single.moves.size()>maxMoves/size_t(passes))
  throw std::runtime_error("Passagens excedem limite de movimentos virtuais");
 Plan output;
 output.ignoredFill=single.ignoredFill;
 output.ignoredDisabled=single.ignoredDisabled;
 output.ignoredTool=single.ignoredTool;
 if(single.moves.empty())return output;
 const Pt start=single.moves.front().from;
 output.moves.reserve(single.moves.size()*size_t(passes)+size_t(passes)-1);
 for(int pass=0;pass<passes;++pass){
  if(pass>0){
   const Pt end=output.moves.back().to;
   const double gap=std::hypot(start.x-end.x,start.y-end.y);
   if(gap>1e-9){
    if(output.moves.size()>=maxMoves)
     throw std::runtime_error("Limite virtual excedido entre passagens");
    output.moves.push_back({end,start,false,single.moves.front().layer});
    output.travelMm+=gap;
   }
  }
  for(const auto& m:single.moves){
   if(output.moves.size()>=maxMoves)
    throw std::runtime_error("Limite de movimentos virtuais excedido");
   output.moves.push_back(m);
   const double length=std::hypot(m.to.x-m.from.x,m.to.y-m.from.y);
   if(m.laser)output.cutMm+=length;else output.travelMm+=length;
  }
 }
 return output;
}
// Preview-only passes. Repeats each contiguous group of one layer according
// to its saved pass count; then repeats the overall job by globalPasses.
// This is a simple ordering approximation, NOT LightBurn sub-layer scheduling.
inline Plan repeatedByLayer(const Plan& base,const EditorState& state,int globalPasses,
                            std::size_t cap=300000){
 if(globalPasses<1||globalPasses>20)
  throw std::runtime_error("Passagens globais virtuais fora do limite");
 Plan out;
 out.ignoredFill=base.ignoredFill;out.ignoredDisabled=base.ignoredDisabled;
 out.ignoredTool=base.ignoredTool;
 auto append=[&](Pt next,bool laser,int layer){
  const Pt from=out.moves.empty()?Pt{}:out.moves.back().to;
  const double d=std::hypot(next.x-from.x,next.y-from.y);
  if(!std::isfinite(d))throw std::runtime_error("Passagem virtual invalida");
  if(d<1e-10)return;
  if(out.moves.size()>=cap)throw std::runtime_error("Limite virtual de movimentos excedido");
  out.moves.push_back({from,next,laser,layer});
  if(laser)out.cutMm+=d;else out.travelMm+=d;
 };
 for(int global=0;global<globalPasses;++global){
  for(std::size_t first=0;first<base.moves.size();){
   const int layer=base.moves[first].layer;
   std::size_t end=first+1;
   while(end<base.moves.size()&&base.moves[end].layer==layer)++end;
   int pass=1;
   const auto it=state.layers.find(layer);
   if(it!=state.layers.end())pass=it->second.passes;
   if(pass<1||pass>20)throw std::runtime_error("Passagem de camada invalida");
   for(int turn=0;turn<pass;++turn){
    for(std::size_t i=first;i<end;++i){
     const auto& m=base.moves[i];
     const Pt current=out.moves.empty()?Pt{}:out.moves.back().to;
     // At the start of a repeated group, avoid returning to the old
     // group's travel origin: go directly to the first destination.
     if(i==first&&!m.laser){append(m.to,false,layer);continue;}
     if(std::hypot(current.x-m.from.x,current.y-m.from.y)>1e-9)
      append(m.from,false,layer);
     append(m.to,m.laser,layer);
    }
   }
   first=end;
  }
 }
 return out;
}
inline double minutes(const Plan& plan,double cutMmMin,double rapidMmMin){
 if(!std::isfinite(cutMmMin)||!std::isfinite(rapidMmMin)||cutMmMin<=0||rapidMmMin<=0)
  throw std::runtime_error("Velocidades virtuais invalidas");
 return plan.cutMm/cutMmMin+plan.travelMm/rapidMmMin;
}
} // namespace alaser::sim198
