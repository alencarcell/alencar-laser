#pragma once
// TESTE_9 — planejamento de atualizacao incremental dos segmentos afetados.
// Geometria de origem permanece IMUTAVEL durante o arraste.
#include "geometry.hpp"
#include <array>
#include <stdexcept>
#include <algorithm>
#include <cmath>
namespace alaser {
struct DragDamage {
 double x0=0,y0=0,x1=0,y1=0;
 void encompass(Pt p){x0=std::min(x0,p.x);y0=std::min(y0,p.y);x1=std::max(x1,p.x);y1=std::max(y1,p.y);}
 bool contains(Pt p,double eps=1e-9)const{return p.x>=x0-eps&&p.x<=x1+eps&&p.y>=y0-eps&&p.y<=y1+eps;}
};
struct DragDelta {
 std::array<Segment,2> before{},after{};
 int count=0;
 Pt previous{},current{};
 DragDamage damage;
};
class DragSegmentPreview {
 const Contour* contour_=nullptr;
 size_t movedIndex_=0;
 std::array<size_t,2> affected_{};
 int count_=0;
 Pt current_{};
 Segment at(size_t edge,Pt pos)const{
  const auto& nodes=contour_->nodes;
  const size_t n=nodes.size();
  size_t a=edge,b=(edge+1)%n;
  Pt p=(a==movedIndex_)?pos:nodes[a].p;
  Pt q=(b==movedIndex_)?pos:nodes[b].p;
  return {contour_->key,int(edge),p,q,std::min(p.x,q.x),std::max(p.x,q.x),std::min(p.y,q.y),std::max(p.y,q.y)};
 }
public:
 DragSegmentPreview(const Document& d,Key key,int64_t nodeId){
  contour_=get(d,key);
  if(!contour_||contour_->nodes.size()<2)throw std::runtime_error("drag: contour missing or too short");
  auto& nodes=contour_->nodes;
  auto it=std::find_if(nodes.begin(),nodes.end(),[&](const Node& n){return n.id==nodeId;});
  if(it==nodes.end())throw std::runtime_error("drag: node not found");
  movedIndex_=size_t(it-nodes.begin());current_=it->p;
  const size_t n=nodes.size();
  if(movedIndex_>0)affected_[count_++]=movedIndex_-1;
  else if(contour_->closed)affected_[count_++]=n-1;
  if(movedIndex_+1<n)affected_[count_++]=movedIndex_;
  else if(contour_->closed)affected_[count_++]=n-1;
  if(count_==2 && affected_[0]==affected_[1])count_=1;
 }
 int touchedEdges()const{return count_;}
 size_t movedIndex()const{return movedIndex_;}
 Pt current()const{return current_;}
 DragDelta update(Pt to){
  if(!std::isfinite(to.x)||!std::isfinite(to.y))throw std::runtime_error("drag: non finite position");
  DragDelta delta;delta.count=count_;delta.previous=current_;delta.current=to;
  delta.damage={std::min(to.x,current_.x),std::min(to.y,current_.y),std::max(to.x,current_.x),std::max(to.y,current_.y)};
  for(int i=0;i<count_;++i){
   delta.before[i]=at(affected_[i],current_);delta.after[i]=at(affected_[i],to);
   for(Pt p:{delta.before[i].a,delta.before[i].b,delta.after[i].a,delta.after[i].b})delta.damage.encompass(p);
  }
  current_=to;
  return delta;
 }
};
}
