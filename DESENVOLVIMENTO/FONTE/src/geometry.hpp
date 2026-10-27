#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <numeric>
#include <chrono>

namespace alaser {
struct Pt { double x=0,y=0; };
inline Pt operator+(Pt a,Pt b){return {a.x+b.x,a.y+b.y};}
inline Pt operator-(Pt a,Pt b){return {a.x-b.x,a.y-b.y};}
inline Pt operator*(Pt a,double t){return {a.x*t,a.y*t};}
inline double norm2(Pt p){return p.x*p.x+p.y*p.y;}
inline double dist(Pt a,Pt b){return std::sqrt(norm2(a-b));}
struct Key{int object=0, sub=0;auto operator<=>(const Key&)const=default;};
struct Node{int64_t id=0;Pt p;};
struct Contour{Key key;std::vector<Node> nodes;bool closed=false;};
struct Document{std::vector<Contour> contours;int64_t nextId=1;};
struct Segment{Key key;int idx=0;Pt a,b;double minx,maxx,miny,maxy;};
struct Hit{Key key;int segment=-1;double distance=0;Pt projected;};
inline double distanceToSegment(Pt p,const Segment& s,Pt *out=nullptr){Pt v=s.b-s.a;double d=norm2(v);double t=d?std::clamp(((p.x-s.a.x)*v.x+(p.y-s.a.y)*v.y)/d,0.,1.):0.;Pt q=s.a+v*t;if(out)*out=q;return dist(p,q);}
inline Document readCsv(const std::string& filename){
 std::ifstream f(filename);if(!f)throw std::runtime_error("fixture CSV nao encontrado: "+filename);
 Document doc;std::map<Key,size_t> keys;std::string line;int64_t id=1;
 while(std::getline(f,line)){
  if(line.empty())continue;
  std::stringstream ss(line);std::vector<std::string> a;std::string col;
  while(std::getline(ss,col,','))a.push_back(col);
  if(a.size()<12)throw std::runtime_error("fixture CSV incompleto");
  Key key{std::stoi(a[0]),std::stoi(a[1])};auto it=keys.find(key);if(it==keys.end()){keys[key]=doc.contours.size();doc.contours.push_back({key,{},false});it=keys.find(key);}
  Pt p{std::stod(a[10]),std::stod(a[11])};if(!std::isfinite(p.x)||!std::isfinite(p.y))throw std::runtime_error("coordenada invalida");
  doc.contours[it->second].nodes.push_back({id++,p});
 }
 doc.nextId=id;return doc;
}
inline std::vector<Segment> segments(const Document& d){
 std::vector<Segment> out;for(const auto& c:d.contours){auto n=c.nodes.size();
  if(n==1){auto p=c.nodes.front().p;out.push_back({c.key,-1,p,p,p.x,p.x,p.y,p.y});continue;}
  if(n<2)continue;
  for(size_t j=0;j<n-1+(c.closed?1:0);++j){auto a=c.nodes[j].p,b=c.nodes[(j+1)%n].p;
   out.push_back({c.key,int(j),a,b,std::min(a.x,b.x),std::max(a.x,b.x),std::min(a.y,b.y),std::max(a.y,b.y)});
  }
 }return out;
}
inline bool sameDoc(const Document& a,const Document& b,double eps=0){
 if(a.nextId!=b.nextId||a.contours.size()!=b.contours.size())return false;
 for(size_t i=0;i<a.contours.size();++i){auto& x=a.contours[i];auto& y=b.contours[i];
  if(x.key!=y.key||x.closed!=y.closed||x.nodes.size()!=y.nodes.size())return false;
  for(size_t j=0;j<x.nodes.size();++j)if(x.nodes[j].id!=y.nodes[j].id||dist(x.nodes[j].p,y.nodes[j].p)>eps)return false;
 }return true;
}
inline Contour* get(Document& d,Key k){for(auto& c:d.contours)if(c.key==k)return &c;return nullptr;}
inline const Contour* get(const Document& d,Key k){for(auto& c:d.contours)if(c.key==k)return &c;return nullptr;}
inline bool moveNode(Document& d,Key k,int64_t id,Pt p){if(auto*c=get(d,k))for(auto& n:c->nodes)if(n.id==id){n.p=p;return true;}return false;}
inline void insertNode(Document& d,Key k,size_t idx,Pt p){auto*c=get(d,k);if(!c||idx>c->nodes.size())throw std::runtime_error("insert invalid key or index");c->nodes.insert(c->nodes.begin()+idx,{d.nextId++,p});}
inline void eraseNode(Document& d,Key k,int64_t id){auto*c=get(d,k);if(!c)throw std::runtime_error("erase invalid key");auto n=c->nodes.size();std::erase_if(c->nodes,[&](const Node& x){return x.id==id;});if(n==c->nodes.size())throw std::runtime_error("erase invalid node id");if(c->nodes.size()<3)c->closed=false;if(c->nodes.empty())std::erase_if(d.contours,[&](auto& t){return t.key==k;});}
inline std::vector<Segment> previewSegments(const Document& d,Key key,int64_t nodeId,Pt p){const auto*c=get(d,key);if(!c)return {};Contour copy=*c;bool found=false;for(auto& n:copy.nodes)if(n.id==nodeId){n.p=p;found=true;}if(!found)throw std::runtime_error("preview: node mismatch");Document only;only.contours.push_back(std::move(copy));return segments(only);}
inline bool hitLess(const Hit& a,const Hit& b){auto da=std::llround(a.distance*1e6),db=std::llround(b.distance*1e6);if(da!=db)return da<db;if(a.key!=b.key)return a.key<b.key;return a.segment<b.segment;}
inline std::vector<Hit> evaluate(const std::vector<Segment>& segs,const std::vector<int>& indices,Pt p,double tolerance){
 std::map<Key,Hit> best;
 for(int idx:indices){const auto &s=segs[idx];Pt projected;auto d=distanceToSegment(p,s,&projected);
  if(d>tolerance+1e-9)continue;
  auto it=best.find(s.key);Hit h{s.key,s.idx,d,projected};
  if(it==best.end()||hitLess(h,it->second))best[s.key]=h;
 }
 std::vector<Hit> hits;for(auto& [k,h]:best)hits.push_back(h);
 std::sort(hits.begin(),hits.end(),hitLess);return hits;
}
inline std::vector<Hit> bruteHits(const std::vector<Segment>& segs,Pt p,double tolerance){std::vector<int> all(segs.size());std::iota(all.begin(),all.end(),0);return evaluate(segs,all,p,tolerance);}
// Seleção linear de referência sem criar um vetor de índices de 0..N-1 por clique.
// Mantém mesma política de comparação e desempate que evaluate().
inline std::vector<Hit> bruteHitsNoAlloc(const std::vector<Segment>& segs,Pt p,double tolerance){
 std::map<Key,Hit> best;
 for(const auto& s:segs){Pt projected;auto d=distanceToSegment(p,s,&projected);
  if(d>tolerance+1e-9)continue;
  auto it=best.find(s.key);Hit h{s.key,s.idx,d,projected};
  if(it==best.end()||hitLess(h,it->second))best[s.key]=h;
 }
 std::vector<Hit> hits;hits.reserve(best.size());
 for(auto&[k,h]:best)hits.push_back(h);
 std::sort(hits.begin(),hits.end(),hitLess);return hits;
}

// Immutable BVH over exact segment AABBs. Build once; rebuild on geometric commit.
class SegmentBVH{
 struct Box{double x0,y0,x1,y1;};
 struct Branch{Box bbox;int start=0,count=0,left=-1,right=-1;};
 std::vector<Branch> tree;std::vector<int> order;const std::vector<Segment>*source=nullptr;
 static Box bounds(const Segment& s){return{s.minx,s.miny,s.maxx,s.maxy};}
 static Box merge(Box a,Box b){return{std::min(a.x0,b.x0),std::min(a.y0,b.y0),std::max(a.x1,b.x1),std::max(a.y1,b.y1)};}
 static bool overlaps(Box b,Pt p,double tol){return p.x>=b.x0-tol&&p.x<=b.x1+tol&&p.y>=b.y0-tol&&p.y<=b.y1+tol;}
 int build(int begin,int end){
  Box b=bounds((*source)[order[begin]]);for(int i=begin+1;i<end;i++)b=merge(b,bounds((*source)[order[i]]));
  int pos=int(tree.size());tree.push_back({b,begin,end-begin,-1,-1});
  if(end-begin>12){bool x=(b.x1-b.x0)>=(b.y1-b.y0);int mid=(begin+end)/2;
   std::nth_element(order.begin()+begin,order.begin()+mid,order.begin()+end,[&](int a,int c){
    auto& sa=(*source)[a];auto& sc=(*source)[c];double pa=x?sa.minx+sa.maxx:sa.miny+sa.maxy,pb=x?sc.minx+sc.maxx:sc.miny+sc.maxy;
    return pa==pb?a<c:pa<pb;
   });
   auto left=build(begin,mid);auto right=build(mid,end);tree[pos].left=left;tree[pos].right=right;tree[pos].count=0;
  }return pos;
 }
 void candidates(int node,Pt p,double tol,std::vector<int>&out)const{
  const auto&t=tree[node];if(!overlaps(t.bbox,p,tol))return;
  if(t.left>=0){candidates(t.left,p,tol,out);candidates(t.right,p,tol,out);}else for(int i=t.start;i<t.start+t.count;i++){
   const auto& s=(*source)[order[i]];if(overlaps(bounds(s),p,tol))out.push_back(order[i]);
  }
 }
public:
 SegmentBVH()=default;explicit SegmentBVH(const std::vector<Segment>&s){reset(s);}
 void reset(const std::vector<Segment>&s){source=&s;tree.clear();order.resize(s.size());std::iota(order.begin(),order.end(),0);if(!order.empty())build(0,int(order.size()));}
 std::vector<int> candidateIds(Pt p,double tol)const{
  if(!source)throw std::runtime_error("BVH not initialized");
  std::vector<int> ids;if(!tree.empty())candidates(0,p,tol,ids);return ids;
 }
 std::vector<Hit> hits(Pt p,double tol,size_t* candidateCount=nullptr)const{
  if(!source)throw std::runtime_error("BVH not initialized");
  std::vector<int> ids;
  if(!tree.empty())candidates(0,p,tol,ids);
  if(candidateCount)*candidateCount=ids.size();
  return evaluate(*source,ids,p,tol);
 }
};
inline bool sameHits(const std::vector<Hit>&a,const std::vector<Hit>&b){
 if(a.size()!=b.size())return false;
 for(size_t i=0;i<a.size();i++){
  if(a[i].key!=b[i].key||a[i].segment!=b[i].segment||std::abs(a[i].distance-b[i].distance)>1e-7)return false;
 }return true;
}
}
