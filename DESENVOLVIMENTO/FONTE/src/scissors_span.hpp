#pragma once
// Tesoura de trechos: geometria pura; SEM Qt, SEM comandos de maquina.
// Intervalos sobre uma unica polilinha, delimitados por cruzamentos reais.
#include "geometry.hpp"
#include <map>
#include <functional>
#include <memory>
#include <optional>
#include <string>
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
 struct Boundaries {
  std::vector<double> crossings;
  std::vector<std::pair<double,double>> unsafe;
  // Index-aligned classification for safe refusal; no geometry is altered.
  std::vector<const char*> unsafeKinds;
 };
 std::vector<Segment> segments_;
 std::map<Key,Contour> contourLookup_;
 std::unique_ptr<SegmentBVH> index_;
 std::map<Key,Boundaries> cache_;
 std::string lastRejection_="none";
 Boundaries compute(const Contour& c)const{
  Boundaries out;
  if(!index_||c.nodes.size()<2)return out;
  const int edges=int(c.nodes.size())-(c.closed?0:1);
  // A genuine transverse crossing at an INTERNAL vertex of another path
  // is safe: its two incident sides must lie strictly on opposite sides of
  // the target line. A tangency (same side), T endpoint or degenerate corner
  // is still ambiguous. No synthetic snapping or coordinate modification.
  auto transverseForeignVertex=[&](const Segment& s,double y,Pt a,Pt r,double rr){
   auto it=contourLookup_.find(s.key);
   if(it==contourLookup_.end())return false;
   const auto& other=it->second;
   const int count=int(other.nodes.size());
   if(count<3)return false;
   const bool atStart=y<1e-7,atEnd=y>1.-1e-7;
   if(!atStart&&!atEnd)return false;
   const int vertex=s.idx+(atEnd?1:0);
   const int v=other.closed?vertex%count:vertex;
   if(v<0||v>=count||(!other.closed&&(v==0||v==count-1)))
    return false;
   const Pt point=other.nodes[size_t(v)].p;
   const Pt before=other.nodes[size_t((v-1+count)%count)].p;
   const Pt after=other.nodes[size_t((v+1)%count)].p;
   const Pt left=before-point,right=after-point;
   const double lenA=norm2(left),lenB=norm2(right);
   if(lenA<=1e-18||lenB<=1e-18)return false;
   const double ca=cross2(r,left),cb=cross2(r,right);
   const double epsA=1e-9*std::sqrt(rr*lenA);
   const double epsB=1e-9*std::sqrt(rr*lenB);
   if(std::abs(ca)<=epsA||std::abs(cb)<=epsB)return false;
   return (ca<0&&cb>0)||(ca>0&&cb<0);
  };
  // 20x: a corner on the TARGET may also be a true crossing, provided
  // the two incident target edges lie strictly on opposite sides of the
  // foreign edge's supporting line. A tangent/touch never qualifies.
  auto transverseTargetVertex=[&](int i,double x,const Segment& other,Pt a,Pt b){
   if(x>1e-7&&x<1.-1e-7)return false;
   const int n=int(c.nodes.size());
   int at=(x<1e-7?i:i+1);
   if(c.closed)at%=n;
   if(at<=0&&!c.closed||at>=n-1&&!c.closed)return false;
   if(at<0||at>=n)return false;
   const Pt p=c.nodes[size_t(at)].p;
   const Pt prev=c.nodes[size_t((at-1+n)%n)].p;
   const Pt next=c.nodes[size_t((at+1)%n)].p;
   const Pt dir=other.b-other.a;
   const double dd=norm2(dir),da=norm2(prev-p),db=norm2(next-p);
   if(dd<=1e-18||da<=1e-18||db<=1e-18)return false;
   const double first=cross2(dir,prev-p),second=cross2(dir,next-p);
   if(std::abs(first)<=1e-9*std::sqrt(dd*da)||
      std::abs(second)<=1e-9*std::sqrt(dd*db))return false;
   return (first<0&&second>0)||(first>0&&second<0);
  };
  for(int i=0;i<edges;i++){
   Pt a=c.nodes[size_t(i)].p,b=c.nodes[size_t(i+1)%c.nodes.size()].p;
   Pt r=b-a;const double rr=norm2(r);
   if(rr<=1e-18){
    out.unsafe.push_back({double(i),double(i+1)});
    out.unsafeKinds.push_back("degenerate_edge");continue;
   }
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
       out.unsafeKinds.push_back("collinear_overlap");
      }
     }
     continue;
    }
    const double x=cross2(q-a,t)/d,y=cross2(q-a,r)/d;
    if(x< -1e-9||x>1.+1e-9||y< -1e-9||y>1.+1e-9)continue;
    // Um caminho ABERTO pode voltar com sua ponta ao interior de uma
    // aresta anterior, formando o 'T' da tesoura (fixture real: objeto 575).
    // So a ponta EXTERNA (primeiro/ultimo no) recebe tratamento especial:
    // criar limite na aresta INTERIOR e ignorar o duplicado na ponta.
    // Cruzamentos em vertices interiores continuam recusados.
    if(s.key==c.key&&!c.closed){
     const bool targetInterior=x>1e-7&&x<1.-1e-7;
     const bool otherInterior=y>1e-7&&y<1.-1e-7;
     const bool otherOpenTip=(s.idx==0&&y<1e-7)||
                             (s.idx==edges-1&&y>1.-1e-7);
     const bool targetOpenTip=(i==0&&x<1e-7)||
                              (i==edges-1&&x>1.-1e-7);
     if(targetInterior&&otherOpenTip){
      out.crossings.push_back(double(i)+x);continue;
     }
     if(otherInterior&&targetOpenTip)continue;
    }
    // The target vertex remains unsafe. A *foreign* inner vertex is
    // allowed only with a strict sign-changing (transverse) crossing.
    if(s.key!=c.key&&x>1e-7&&x<1.-1e-7&&
       (y<1e-7||y>1.-1e-7)&&
       transverseForeignVertex(s,y,a,r,rr)){
     out.crossings.push_back(double(i)+x);continue;
    }
    if(s.key!=c.key&&(x<1e-7||x>1.-1e-7)&&
       y>1e-7&&y<1.-1e-7&&
       transverseTargetVertex(i,x,s,a,b)){
     out.crossings.push_back(double(i)+std::clamp(x,0.,1.));
     continue;
    }
    if(x<1e-7||x>1.-1e-7||y<1e-7||y>1.-1e-7){
     const double p=double(i)+std::clamp(x,0.,1.);
     out.unsafe.push_back({p-1e-6,p+1e-6});
     out.unsafeKinds.push_back("vertex_or_tangent");continue;
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
 const std::string& lastRejectionReason()const noexcept{return lastRejection_;}
 void reset(const Document& d,const std::function<bool(Key)>& isVisible={}){
  segments_=alaser::segments(d);
  contourLookup_.clear();
  for(const auto& c:d.contours)contourLookup_.emplace(c.key,scissorLogicalContour(c));
  if(isVisible)segments_.erase(std::remove_if(segments_.begin(),segments_.end(),
    [&](const Segment& s){return !isVisible(s.key);}),segments_.end());
  index_=std::make_unique<SegmentBVH>(segments_);
  cache_.clear();lastRejection_="none";
 }
 std::optional<ScissorSpan> candidate(const Document& d,const Hit& hit){
  lastRejection_="none";
  const Contour* original=get(d,hit.key);
  if(!original||original->nodes.size()<2){lastRejection_="missing_contour";return {};}
  // Tratar costura duplicada como anel fechado em arquivos ja existentes.
  Contour logical=scissorLogicalContour(*original);
  const Contour* c=&logical;
  const int m=int(c->nodes.size())-(c->closed?0:1);
  if(hit.segment<0||hit.segment>=m){lastRejection_="invalid_segment";return {};}
  const Pt a=c->nodes[size_t(hit.segment)].p,b=c->nodes[size_t(hit.segment+1)%c->nodes.size()].p;
  const Pt r=b-a;const double dd=norm2(r);
  if(dd<1e-18){lastRejection_="degenerate_edge";return {};}
  const double t=std::clamp(((hit.projected-a).x*r.x+(hit.projected-a).y*r.y)/dd,0.,1.);
  const double pos=double(hit.segment)+t;
  auto it=cache_.find(hit.key);
  if(it==cache_.end())it=cache_.emplace(hit.key,compute(*c)).first;
  const auto& bs=it->second;
  // LightBurn Trim Shapes: sem nenhum cruzamento, a forma INTEIRA
  // (inclusive circulo/retangulo recem-criado) e o candidato a exclusao.
  // Exatamente um cruzamento no contorno fechado continua ambiguo.
  if(c->closed&&bs.crossings.size()==1){
   lastRejection_="single_closed_intersection";return {};
  }
  if(bs.crossings.empty()){
   // Apos o primeiro corte, a extremidade do caminho aberto pode coincidir
   // exatamente com a linha usada como limite. Essa intersecao terminal
   // NAO deve impedir o recorte posterior do resto (caso real 29/10).
   // Manter bloqueios para sobreposicoes e tangencias no MEIO do caminho.
   for(size_t k=0;k<bs.unsafe.size();++k){
    const auto& bad=bs.unsafe[k];
    const bool onlyOpenEndpoint=!c->closed&&
      (bad.second<=1e-5||bad.first>=double(m)-1e-5);
    if(!onlyOpenEndpoint){
     lastRejection_=bs.unsafeKinds.at(k);return {};
    }
   }
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
   if(next==edges.begin()||next==edges.end()){
    lastRejection_="no_interval";return {};
   }
   lo=*(next-1);hi=*next;
  }
  if(pos-lo<1e-7||hi-pos<1e-7||hi-lo<1e-7||hi-lo>m+1e-7){
   lastRejection_="boundary_proximity";return {};
  }
  // Bloqueia apenas spans que atravessam sobreposicoes/tangencias ambiguas,
  // sem inutilizar os outros arcos do mesmo objeto.
  for(size_t k=0;k<bs.unsafe.size();++k){
   const auto& bad=bs.unsafe[k];
   for(int shift=-1;shift<=1;shift++){
    const double offset=c->closed?double(shift*m):0.;
    if(bad.second+offset>lo+1e-8&&bad.first+offset<hi-1e-8){
     lastRejection_=bs.unsafeKinds.at(k);return {};
    }
    if(!c->closed)break;
   }
  }
  // Janela de previsualizacao deve ser igual ao intervalo aplicado no commit.
  ScissorSpan result{hit.key,lo,hi,c->closed,spanPoints(*c,lo,hi)};
  if(result.preview.size()<2){lastRejection_="short_preview";return {};}
  return result;
 }
};
} // namespace alaser
