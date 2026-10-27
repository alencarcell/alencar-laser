#pragma once
// Tesoura de trechos: geometria pura; SEM Qt, SEM comandos de maquina.
// Intervalos sobre uma unica polilinha, delimitados por cruzamentos reais.
#include "geometry.hpp"
#include <map>
#include <memory>
#include <optional>
#include <utility>

namespace alaser {
struct ScissorSpan {
 Key key{};
 double from=0, to=0; // posicoes em arestas; fechado pode ultrapassar edgeCount
 bool closed=false;
 std::vector<Pt> preview;
};
inline Pt pointAt(const Contour& c,double u){
 const size_t n=c.nodes.size();
 const int edges=int(n)-(c.closed?0:1);
 if(n<2||edges<=0||u<0||u>double(edges)+1e-7)throw std::runtime_error("parametro de tesoura invalido");
 double v=std::clamp(u,0.,double(edges));
 if(c.closed&&v>=edges)v-=edges;
 int edge=std::min(int(std::floor(v)),edges-1);
 double t=std::clamp(v-edge,0.,1.);
 return c.nodes[size_t(edge)].p+(c.nodes[size_t(edge+1)%n].p-c.nodes[size_t(edge)].p)*t;
}
inline std::vector<Pt> spanPoints(const Contour& c,double a,double b){
 const int m=int(c.nodes.size())-(c.closed?0:1);
 if(m<1||b<=a||b-a>m+1e-7)throw std::runtime_error("intervalo de tesoura invalido");
 std::vector<Pt> p{pointAt(c,std::fmod(a,double(m)))};
 // Indices internos de arestas, incluindo a costura do contorno fechado.
 for(int j=int(std::floor(a))+1;double(j)<b-1e-8;j++){
  const Pt at=c.nodes[size_t(c.closed?j%int(c.nodes.size()):j)].p;
  if(dist(p.back(),at)>1e-10)p.push_back(at);
 }
 const Pt end=pointAt(c,c.closed?std::fmod(b,double(m)):b);
 if(dist(p.back(),end)>1e-10)p.push_back(end);
 return p;
}
inline double cross2(Pt a,Pt b){return a.x*b.y-a.y*b.x;}
class ScissorSpanPlanner {
 struct Boundaries {std::vector<double> crossings;bool ambiguous=false;};
 std::vector<Segment> segments_;
 std::unique_ptr<SegmentBVH> index_;
 std::map<Key,Boundaries> cache_;
 Boundaries compute(const Contour& c)const{
  Boundaries out;
  if(!index_||c.nodes.size()<2)return out;
  const int edges=int(c.nodes.size())-(c.closed?0:1);
  for(int i=0;i<edges;i++){
   Pt a=c.nodes[size_t(i)].p,b=c.nodes[size_t(i+1)%c.nodes.size()].p;
   Pt r=b-a;const double rr=norm2(r);
   if(rr<=1e-18){out.ambiguous=true;return out;}
   const auto candidates=index_->candidateIdsBox(std::min(a.x,b.x),std::min(a.y,b.y),
                                                   std::max(a.x,b.x),std::max(a.y,b.y));
   for(int id:candidates){
    const auto& s=segments_[size_t(id)];
    if(s.key==c.key)continue; // cruzamento consigo mesmo requer regra adicional
    Pt q=s.a,t=s.b-s.a;const double tt=norm2(t);
    if(tt<=1e-18)continue;
    const double d=cross2(r,t);
    const double scale=std::sqrt(rr*tt);
    if(std::abs(d)<=1e-11*scale){
     // Sobreposicao colinear, caso ambiguo: recusamos modificacao destrutiva.
     if(std::abs(cross2(q-a,r))<=1e-10*std::sqrt(rr)*std::max(1.,dist(q,a))){
      const double v0=((q-a).x*r.x+(q-a).y*r.y)/rr;
      const double v1=((s.b-a).x*r.x+(s.b-a).y*r.y)/rr;
      if(std::min(v0,v1)<1.-1e-8&&std::max(v0,v1)>1e-8)out.ambiguous=true;
     }
     continue;
    }
    const double x=cross2(q-a,t)/d,y=cross2(q-a,r)/d;
    if(x< -1e-9||x>1.+1e-9||y< -1e-9||y>1.+1e-9)continue;
    // Intersecoes exatamente em vertices podem ser tangencias; recusamos
    // estes casos ate haver classificacao topologica de ambos os lados.
    if(x<1e-7||x>1.-1e-7||y<1e-7||y>1.-1e-7){out.ambiguous=true;continue;}
    out.crossings.push_back(double(i)+x);
   }
  }
  std::sort(out.crossings.begin(),out.crossings.end());
  out.crossings.erase(std::unique(out.crossings.begin(),out.crossings.end(),
    [](double a,double b){return std::abs(a-b)<1e-7;}),out.crossings.end());
  return out;
 }
public:
 void reset(const Document& d){
  segments_=alaser::segments(d);
  index_=std::make_unique<SegmentBVH>(segments_);
  cache_.clear();
 }
 std::optional<ScissorSpan> candidate(const Document& d,const Hit& hit){
  const Contour* c=get(d,hit.key);
  if(!c||c->nodes.size()<2)return {};
  const int m=int(c->nodes.size())-(c->closed?0:1);
  if(hit.segment<0||hit.segment>=m)return {};
  const Pt a=c->nodes[size_t(hit.segment)].p,b=c->nodes[size_t(hit.segment+1)%c->nodes.size()].p;
  const Pt r=b-a;const double dd=norm2(r);
  if(dd<1e-18)return {};
  const double t=std::clamp(((hit.projected-a).x*r.x+(hit.projected-a).y*r.y)/dd,0.,1.);
  const double pos=double(hit.segment)+t;
  auto it=cache_.find(hit.key);
  if(it==cache_.end())it=cache_.emplace(hit.key,compute(*c)).first;
  const auto& bs=it->second;
  if(bs.ambiguous||(c->closed&&bs.crossings.size()<2))return {};
  std::vector<double> edges=bs.crossings;
  if(!c->closed){edges.insert(edges.begin(),0.);edges.push_back(double(m));}
  const auto next=std::upper_bound(edges.begin(),edges.end(),pos+1e-8);
  double lo=0,hi=0;
  if(c->closed){
   lo=(next==edges.begin()?edges.back()-m:*(next-1));
   hi=(next==edges.end()?edges.front()+m:*next);
   if(pos<lo)lo-=m;
  }else{
   if(next==edges.begin()||next==edges.end())return {};
   lo=*(next-1);hi=*next;
  }
  if(pos-lo<1e-7||hi-pos<1e-7||hi-lo<1e-7||hi-lo>m+1e-7)return {};
  // Janela de previsualizacao deve ser igual ao intervalo aplicado no commit.
  ScissorSpan result{hit.key,lo,hi,c->closed,spanPoints(*c,lo,hi)};
  if(result.preview.size()<2)return {};
  return result;
 }
};
} // namespace alaser
