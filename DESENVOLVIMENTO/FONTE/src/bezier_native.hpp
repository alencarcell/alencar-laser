#pragma once
// TESTE_13 - curvas cubicas nativas. Nao fabrica nem envia comandos GRBL.
#include "geometry.hpp"
#include <array>
#include <bit>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace alaser {
struct CurveNode {
 int64_t id=0;
 Pt anchor{}, handleIn{}, handleOut{};
};
struct CurveContour {
 Key key{};
 std::vector<CurveNode> nodes;
 std::vector<uint8_t> edgeKind; // 0=reta, 1=cubica; tamanho n-1 (aberto) ou n (fechado)
 bool closed=false;
 int layer=0; // layer of this native object, persisted in bezier-1 (legacy default 0)
};
struct CurveDocument {std::vector<CurveContour> contours;int64_t nextId=1;};
struct Cubic {Pt p0,p1,p2,p3;};
inline Pt lerp(Pt a,Pt b,double t){return a+(b-a)*t;}
inline Pt cubicPoint(Cubic c,double t){auto a=lerp(c.p0,c.p1,t),b=lerp(c.p1,c.p2,t),d=lerp(c.p2,c.p3,t);return lerp(lerp(a,b,t),lerp(b,d,t),t);}
inline size_t edgeCount(const CurveContour& c){return c.nodes.empty()?0:c.nodes.size()-1+(c.closed?1:0);}
inline Cubic edgeAt(const CurveContour& c,size_t edge){
 if(edge>=edgeCount(c)||c.edgeKind.size()!=edgeCount(c))throw std::runtime_error("invalid Bezier edge");
 auto& a=c.nodes[edge];auto& b=c.nodes[(edge+1)%c.nodes.size()];
 if(c.edgeKind[edge]==0)return {a.anchor,a.anchor,b.anchor,b.anchor};
 return {a.anchor,a.handleOut,b.handleIn,b.anchor};
}
inline void validateCurve(const CurveDocument& d){
 std::map<Key,bool> seen;std::map<int64_t,bool> ids;int64_t maxId=0;
 for(auto& c:d.contours){
  if(seen[c.key])throw std::runtime_error("duplicate curve key");
  seen[c.key]=true;
  if(c.nodes.size()<2||(c.closed&&c.nodes.size()<3)||c.edgeKind.size()!=edgeCount(c)||c.layer<0)
    throw std::runtime_error("invalid curve topology/layer");
  for(auto e:c.edgeKind)if(e>1)throw std::runtime_error("unsupported curve edge");
  for(auto& n:c.nodes){
   if(n.id<=0||ids[n.id])throw std::runtime_error("duplicate curve node ID");
   ids[n.id]=true;maxId=std::max(maxId,n.id);
   for(Pt p:{n.anchor,n.handleIn,n.handleOut})if(!std::isfinite(p.x)||!std::isfinite(p.y))throw std::runtime_error("nonfinite curve coordinate");
  }
 }
 if(d.nextId<=maxId)throw std::runtime_error("invalid curve next node ID");
}
inline int64_t splitEdge(CurveDocument& d,size_t contour,size_t edge,double t){
 if(!(t>0&&t<1)||!std::isfinite(t)||contour>=d.contours.size())throw std::runtime_error("invalid split");
 CurveContour& c=d.contours[contour];if(edge>=edgeCount(c))throw std::runtime_error("invalid split edge");
 Cubic v=edgeAt(c,edge);bool cubic=c.edgeKind[edge]!=0;size_t next=(edge+1)%c.nodes.size();
 Pt p{};CurveNode added;added.id=d.nextId++;
 if(cubic){
  Pt a=lerp(v.p0,v.p1,t),b=lerp(v.p1,v.p2,t),cc=lerp(v.p2,v.p3,t);
  Pt ab=lerp(a,b,t),bc=lerp(b,cc,t);p=lerp(ab,bc,t);
  c.nodes[edge].handleOut=a;c.nodes[next].handleIn=cc;
  added={added.id,p,ab,bc};
 }else {p=lerp(v.p0,v.p3,t);added={added.id,p,p,p};}
 c.nodes.insert(c.nodes.begin()+edge+1,added);
 c.edgeKind.insert(c.edgeKind.begin()+edge+1,uint8_t(cubic?1:0));
 return added.id;
}
inline void moveCurveNode(CurveDocument& d,Key key,int64_t id,Pt pos){
 if(!std::isfinite(pos.x)||!std::isfinite(pos.y))throw std::runtime_error("invalid move");
 for(auto& c:d.contours)if(c.key==key)for(auto& n:c.nodes)if(n.id==id){Pt delta=pos-n.anchor;n.anchor=pos;n.handleIn=n.handleIn+delta;n.handleOut=n.handleOut+delta;return;}
 throw std::runtime_error("curve node not found");
}
// Remove a âncora id do contorno key.
//
// Política geométrica explícita:
// Duas cúbicas consecutivas A→B e B→C NÃO são fundidas numa única cúbica
// (isso alteraria a trajetória). O trecho que saltava o nó removido vira
// uma aresta RETA (edgeKind=0) entre os vizinhos que permaneceram.
// Arestas que não tocavam o nó removido preservam edgeKind e handles.
// IDs dos nós restantes e Key do contorno permanecem estáveis.
//
// Invariantes após a operação (validateCurve):
//  - aberto: nodes.size() >= 2, edgeKind.size() == nodes.size()-1
//  - fechado: nodes.size() >= 3, edgeKind.size() == nodes.size()
inline void removeCurveNode(CurveDocument& d,Key key,int64_t id){
 for(auto& c:d.contours)if(c.key==key){
  const size_t minNodes=c.closed?3u:2u;
  if(c.nodes.size()<=minNodes)
   throw std::runtime_error("cannot delete: topology would become invalid");
  size_t idx=c.nodes.size();
  for(size_t i=0;i<c.nodes.size();++i)if(c.nodes[i].id==id){idx=i;break;}
  if(idx==c.nodes.size())throw std::runtime_error("delete target node missing");
  if(c.edgeKind.size()!=edgeCount(c))throw std::runtime_error("invalid curve topology");

  const size_t n=c.nodes.size();
  std::vector<uint8_t> nextKind;
  nextKind.reserve(c.closed?n-1:n-2);

  if(c.closed){
   // Percorre os nós restantes em ordem cíclica; preserva aresta original
   // quando os índices eram adjacentes, senão insere reta (ponte sobre o nó).
   std::vector<size_t> keep;
   keep.reserve(n-1);
   for(size_t i=0;i<n;++i)if(i!=idx)keep.push_back(i);
   for(size_t k=0;k<keep.size();++k){
    const size_t a=keep[k];
    const size_t b=keep[(k+1)%keep.size()];
    if((a+1)%n==b)nextKind.push_back(c.edgeKind[a]);
    else nextKind.push_back(0); // saltou o nó removido
   }
  }else{
   // Caminho aberto: para cada nó restante i, liga ao próximo restante j.
   for(size_t i=0;i<n;++i){
    if(i==idx)continue;
    size_t j=i+1;
    while(j<n&&j==idx)++j;
    if(j>=n)break;
    if(j==i+1)nextKind.push_back(c.edgeKind[i]); // aresta original intacta
    else nextKind.push_back(0);                   // ponte reta sobre o removido
   }
  }

  c.nodes.erase(c.nodes.begin()+static_cast<std::ptrdiff_t>(idx));
  c.edgeKind=std::move(nextKind);
  // validateCurve será chamado pelo histórico; checagem local defensiva:
  if(c.edgeKind.size()!=edgeCount(c))
   throw std::runtime_error("removeCurveNode: edgeKind size mismatch after rebuild");
  return;
 }
 throw std::runtime_error("delete target contour missing");
}
// Estimativa conservadora da distancia max. dos controles a corda;
// subdivide ate atingir tolerancia mm. Nao destrói as cubicas nativas.
// Distancia ao SEGMENTO, nao a reta infinita. Curvas com controles colineares
// mas fora da corda (overshoot) precisam ser subdivididas tambem.
inline double pointLineDistance(Pt p,Pt a,Pt b){
 Pt v=b-a;double len=norm2(v);
 if(len<1e-30)return dist(p,a);
 double t=std::clamp(((p.x-a.x)*v.x+(p.y-a.y)*v.y)/len,0.,1.);
 return dist(p,a+v*t);
}
inline void flattenRecursive(Cubic c,double tol,int depth,std::vector<Pt>& out){
 double flat=std::max(pointLineDistance(c.p1,c.p0,c.p3),pointLineDistance(c.p2,c.p0,c.p3));
 if(flat<=tol || depth==28){out.push_back(c.p3);return;}
 Pt a=lerp(c.p0,c.p1,.5),b=lerp(c.p1,c.p2,.5),d=lerp(c.p2,c.p3,.5);
 Pt ab=lerp(a,b,.5),bd=lerp(b,d,.5),m=lerp(ab,bd,.5);
 flattenRecursive({c.p0,a,ab,m},tol,depth+1,out);
 flattenRecursive({m,bd,d,c.p3},tol,depth+1,out);
}
inline std::vector<Pt> flatten(Cubic c,double tolerance){
 if(!std::isfinite(tolerance)||tolerance<=0)throw std::runtime_error("invalid flatten tolerance");
 std::vector<Pt> out{c.p0};flattenRecursive(c,tolerance,0,out);return out;
}
// Historico de transacoes atomicas, para evitar alteracoes em modelos parciais.
class CurveHistory {
 CurveDocument current_;
 std::vector<CurveDocument> undo_,redo_;
public:
 explicit CurveHistory(CurveDocument doc):current_(std::move(doc)){validateCurve(current_);}
 const CurveDocument& document()const{return current_;}
 template<class Fn> void change(Fn fn){
  CurveDocument proposed=current_;fn(proposed);validateCurve(proposed);
  undo_.push_back(current_);current_=std::move(proposed);redo_.clear();
 }
 bool undo(){if(undo_.empty())return false;redo_.push_back(current_);current_=std::move(undo_.back());undo_.pop_back();return true;}
 bool redo(){if(redo_.empty())return false;undo_.push_back(current_);current_=std::move(redo_.back());redo_.pop_back();return true;}
 size_t undoCount()const{return undo_.size();}
 size_t redoCount()const{return redo_.size();}
};
// CSV do LightBurn: origem,indice,B/L,p0x,p0y,p1x,p1y,p2x,p2y,p3x,p3y.
inline CurveDocument loadLightBurnCurves(const std::string& file){
 std::ifstream in(file);if(!in)throw std::runtime_error("LightBurn fixture unavailable");
 CurveDocument d;std::string line;int sub=0;
 while(std::getline(in,line)){
  if(line.empty())continue;
  std::stringstream ss(line);std::vector<std::string> fields;std::string v;
  while(std::getline(ss,v,','))fields.push_back(v);
  if(fields.size()!=11)throw std::runtime_error("invalid LightBurn curve CSV");
  Cubic c{{std::stod(fields[3]),std::stod(fields[4])},{std::stod(fields[5]),std::stod(fields[6])},{std::stod(fields[7]),std::stod(fields[8])},{std::stod(fields[9]),std::stod(fields[10])}};
  bool cubic=fields[2]=="B";if(!cubic&&fields[2]!="L")throw std::runtime_error("invalid edge kind");
  if(d.contours.empty()||dist(d.contours.back().nodes.back().anchor,c.p0)>1e-8){
   CurveContour newContour;newContour.key={9000,sub++};newContour.nodes.push_back({d.nextId++,c.p0,c.p0,c.p0});d.contours.push_back(std::move(newContour));
  }
  auto& cur=d.contours.back();cur.nodes.back().handleOut=cubic?c.p1:cur.nodes.back().anchor;
  cur.nodes.push_back({d.nextId++,c.p3,cubic?c.p2:c.p3,c.p3});cur.edgeKind.push_back(cubic?1:0);
 }
 validateCurve(d);return d;
}
}
