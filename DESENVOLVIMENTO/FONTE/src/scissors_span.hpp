#pragma once
// Tesoura de trechos: geometria pura; SEM Qt, SEM comandos de maquina.
// Intervalos sobre uma unica polilinha, delimitados por cruzamentos reais.
#include "geometry.hpp"
#include <map>
#include <functional>
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
inline double wrapScissorParam(double u,int edges){
 if(edges<=0)throw std::runtime_error("caminho sem arestas");
 double w=std::fmod(u,double(edges));
 if(w<0)w+=edges;
 return w;
}
inline std::vector<Pt> spanPoints(const Contour& c,double a,double b){
 const int m=int(c.nodes.size())-(c.closed?0:1);
 if(m<1||b<=a||b-a>m+1e-7)throw std::runtime_error("intervalo de tesoura invalido");
 std::vector<Pt> p{pointAt(c,c.closed?wrapScissorParam(a,m):a)};
 // Indices internos de arestas, incluindo a costura do contorno fechado.
 for(int j=int(std::floor(a))+1;double(j)<b-1e-8;j++){
  const Pt at=c.nodes[size_t(c.closed?((j%int(c.nodes.size())+int(c.nodes.size()))%int(c.nodes.size())):j)].p;
  if(dist(p.back(),at)>1e-10)p.push_back(at);
 }
 const Pt end=pointAt(c,c.closed?wrapScissorParam(b,m):b);
 if(dist(p.back(),end)>1e-10)p.push_back(end);
 return p;
}
inline double cross2(Pt a,Pt b){return a.x*b.y-a.y*b.x;}
// Arquivos vetoriais antigos podem ter o primeiro no repetido no fim mas
// closed=false. A geometria e um anel real, somente o metadado ficou aberto.
// Usar fecho virtual APENAS para preview/tesoura; nao mudar os dados originais.
inline bool hasExactDuplicateClosure(const Contour& contour){
 return !contour.closed && contour.nodes.size()>=4 &&
  dist(contour.nodes.front().p,contour.nodes.back().p)<=1e-9;
}
inline Contour scissorLogicalContour(const Contour& contour){
 Contour result=contour;
 if(hasExactDuplicateClosure(contour)){
  result.nodes.pop_back();result.closed=true;
 }
 return result;
}
class ScissorSpanPlanner {
 struct Boundaries {std::vector<double> crossings;std::vector<std::pair<double,double>> unsafe;};
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
   if(rr<=1e-18){out.unsafe.push_back({double(i),double(i+1)});continue;}
   const auto candidates=index_->candidateIdsBox(std::min(a.x,b.x),std::min(a.y,b.y),
                                                   std::max(a.x,b.x),std::max(a.y,b.y));
   for(int id:candidates){
    const auto& s=segments_[size_t(id)];
    if(s.key==c.key){
     // Arestas vizinhas partilham vertices por definicao. Cruzamentos com
     // outras arestas do mesmo contorno tambem delimitam o recorte.
     const int delta=std::abs(s.idx-i);
     if(delta<=1||(c.closed&&delta==edges-1))continue;
    }
    Pt q=s.a,t=s.b-s.a;const double tt=norm2(t);
    if(tt<=1e-18)continue;
    const double d=cross2(r,t);
    const double scale=std::sqrt(rr*tt);
    if(std::abs(d)<=1e-11*scale){
     // Sobreposicao colinear, caso ambiguo: recusamos modificacao destrutiva.
     if(std::abs(cross2(q-a,r))<=1e-10*std::sqrt(rr)*std::max(1.,dist(q,a))){
      const double v0=((q-a).x*r.x+(q-a).y*r.y)/rr;
      const double v1=((s.b-a).x*r.x+(s.b-a).y*r.y)/rr;
      if(std::min(v0,v1)<1.-1e-8&&std::max(v0,v1)>1e-8){
       out.unsafe.push_back({i+std::max(0.,std::min(v0,v1)),
                             i+std::min(1.,std::max(v0,v1))});
      }
     }
     continue;
    }
    const double x=cross2(q-a,t)/d,y=cross2(q-a,r)/d;
    if(x< -1e-9||x>1.+1e-9||y< -1e-9||y>1.+1e-9)continue;
    // Intersecoes exatamente em vertices podem ser tangencias; recusamos
    // estes casos ate haver classificacao topologica de ambos os lados.
    if(x<1e-7||x>1.-1e-7||y<1e-7||y>1.-1e-7){
     const double p=double(i)+std::clamp(x,0.,1.);
     out.unsafe.push_back({p-1e-6,p+1e-6});continue;
    }
    out.crossings.push_back(double(i)+x);
   }
  }
  std::sort(out.crossings.begin(),out.crossings.end());
  out.crossings.erase(std::unique(out.crossings.begin(),out.crossings.end(),
    [](double a,double b){return std::abs(a-b)<1e-7;}),out.crossings.end());
  return out;
 }
public:
 void reset(const Document& d,const std::function<bool(Key)>& isVisible={}){
  segments_=alaser::segments(d);
  if(isVisible)segments_.erase(std::remove_if(segments_.begin(),segments_.end(),
    [&](const Segment& s){return !isVisible(s.key);}),segments_.end());
  index_=std::make_unique<SegmentBVH>(segments_);
  cache_.clear();
 }
 std::optional<ScissorSpan> candidate(const Document& d,const Hit& hit){
  const Contour* original=get(d,hit.key);
  if(!original||original->nodes.size()<2)return {};
  // Tratar costura duplicada como anel fechado em arquivos ja existentes.
  Contour logical=scissorLogicalContour(*original);
  const Contour* c=&logical;
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
  // LightBurn Trim Shapes: sem nenhum cruzamento, a forma INTEIRA
  // (inclusive circulo/retangulo recem-criado) e o candidato a exclusao.
  // Exatamente um cruzamento no contorno fechado continua ambiguo.
  if(c->closed&&bs.crossings.size()==1)return {};
  if(bs.crossings.empty()){
   // Sobreposicoes e tangencias nao resolvidas precisam ser recusadas.
   if(!bs.unsafe.empty())return {};
   return ScissorSpan{hit.key,0.,double(m),c->closed,
                      spanPoints(*c,0.,double(m))};
  }
  std::vector<double> edges=bs.crossings;
  if(!c->closed){edges.insert(edges.begin(),0.);edges.push_back(double(m));}
  const auto next=std::upper_bound(edges.begin(),edges.end(),pos+1e-8);
  double lo=0,hi=0;
  if(c->closed){
   lo=(next==edges.begin()?edges.back()-m:*(next-1));
   // Se o cursor estiver antes do primeiro cruzamento, o limite direito
   // e o PRIMEIRO cruzamento neste ciclo, nao no ciclo seguinte.
   // Somar m aqui duplica o arco, supera a volta inteira e apaga a previa.
   hi=(next==edges.begin()?edges.front():
       next==edges.end()?edges.front()+m:*next);
   if(pos<lo)lo-=m;
  }else{
   if(next==edges.begin()||next==edges.end())return {};
   lo=*(next-1);hi=*next;
  }
  if(pos-lo<1e-7||hi-pos<1e-7||hi-lo<1e-7||hi-lo>m+1e-7)return {};
  // Bloqueia apenas spans que atravessam sobreposicoes/tangencias ambiguas,
  // sem inutilizar os outros arcos do mesmo objeto.
  for(const auto& bad:bs.unsafe){
   for(int shift=-1;shift<=1;shift++){
    const double offset=c->closed?double(shift*m):0.;
    if(bad.second+offset>lo+1e-8&&bad.first+offset<hi-1e-8)return {};
    if(!c->closed)break;
   }
  }
  // Janela de previsualizacao deve ser igual ao intervalo aplicado no commit.
  ScissorSpan result{hit.key,lo,hi,c->closed,spanPoints(*c,lo,hi)};
  if(result.preview.size()<2)return {};
  return result;
 }
};
} // namespace alaser
