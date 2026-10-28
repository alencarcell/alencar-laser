#pragma once
// Alencar Laser 19.8: conservative topology for OFFLINE cut-order preview.
// Detects strict nesting only; contacts, overlaps and degenerate paths are
// intentionally NOT treated as containment. No G-code/machine interaction.
#include "preview_sim_19_8.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace alaser::sim198::nesting {
constexpr double epsilon=1e-7; // mm: conservative margin for touching edges
struct Bounds{double minX=0,minY=0,maxX=0,maxY=0;};
inline Bounds bounds(const Contour& c){
 if(c.nodes.empty())throw std::runtime_error("Contorno sem pontos");
 Bounds b{c.nodes.front().p.x,c.nodes.front().p.y,c.nodes.front().p.x,c.nodes.front().p.y};
 for(const auto& n:c.nodes){
  if(!std::isfinite(n.p.x)||!std::isfinite(n.p.y))
   throw std::runtime_error("Coordenada invalida na topologia virtual");
  b.minX=std::min(b.minX,n.p.x);b.minY=std::min(b.minY,n.p.y);
  b.maxX=std::max(b.maxX,n.p.x);b.maxY=std::max(b.maxY,n.p.y);
 }
 return b;
}
inline bool boundsStrictlyInside(const Bounds& inner,const Bounds& outer){
 return inner.minX>outer.minX+epsilon&&inner.minY>outer.minY+epsilon&&
        inner.maxX<outer.maxX-epsilon&&inner.maxY<outer.maxY-epsilon;
}
inline double orient(Pt a,Pt b,Pt c){
 return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);
}
inline bool onSegment(Pt p,Pt a,Pt b){
 const double span=std::hypot(b.x-a.x,b.y-a.y);
 if(span<=epsilon)return std::hypot(p.x-a.x,p.y-a.y)<=epsilon;
 return std::abs(orient(a,b,p))<=epsilon*span&&
        p.x>=std::min(a.x,b.x)-epsilon&&p.x<=std::max(a.x,b.x)+epsilon&&
        p.y>=std::min(a.y,b.y)-epsilon&&p.y<=std::max(a.y,b.y)+epsilon;
}
// 0 outside/on-boundary, 1 strictly inside (ray parity).
inline bool pointStrictlyInside(Pt point,const Contour& polygon,
                               std::size_t& checks,std::size_t maxChecks){
 if(!polygon.closed||polygon.nodes.size()<3)return false;
 bool inside=false;
 for(std::size_t i=0,j=polygon.nodes.size()-1;i<polygon.nodes.size();j=i++){
  if(++checks>maxChecks)throw std::runtime_error("Limite de topologia virtual excedido");
  const Pt a=polygon.nodes[j].p,b=polygon.nodes[i].p;
  if(onSegment(point,a,b))return false;
  if((a.y>point.y)!=(b.y>point.y)){
   const double crossing=a.x+(point.y-a.y)*(b.x-a.x)/(b.y-a.y);
   if(std::abs(crossing-point.x)<=epsilon)return false;
   if(crossing>point.x)inside=!inside;
  }
 }
 return inside;
}
inline bool segmentsMeet(Pt a,Pt b,Pt c,Pt d){
 if(onSegment(a,c,d)||onSegment(b,c,d)||onSegment(c,a,b)||onSegment(d,a,b))
  return true;
 const double ab1=orient(a,b,c),ab2=orient(a,b,d);
 const double cd1=orient(c,d,a),cd2=orient(c,d,b);
 return ((ab1>0&&ab2<0)||(ab1<0&&ab2>0))&&
        ((cd1>0&&cd2<0)||(cd1<0&&cd2>0));
}
inline bool strictlyContains(const Contour& outer,const Contour& inner,
                             const Bounds& outerBounds,const Bounds& innerBounds,
                             std::size_t& checks,std::size_t maxChecks){
 if(!outer.closed||outer.nodes.size()<3||inner.nodes.size()<2 ||
    !boundsStrictlyInside(innerBounds,outerBounds))return false;
 // If a self-intersection is supplied, caller is expected to inspect it:
 // this preview makes no promise about self-intersecting polygons.
 for(const auto& node:inner.nodes)
  if(!pointStrictlyInside(node.p,outer,checks,maxChecks))return false;
 const std::size_t edges=inner.closed?inner.nodes.size():inner.nodes.size()-1;
 for(std::size_t i=0;i<edges;++i){
  const Pt a=inner.nodes[i].p;
  const Pt b=inner.nodes[(i+1)%inner.nodes.size()].p;
  for(std::size_t j=0;j<outer.nodes.size();++j){
   if(++checks>maxChecks)
    throw std::runtime_error("Topologia virtual complexa demais; limite de comparacoes");
   const Pt c=outer.nodes[j].p,d=outer.nodes[(j+1)%outer.nodes.size()].p;
   if(segmentsMeet(a,b,c,d))return false;
  }
 }
 return true;
}
inline bool simpleClosed(const Contour& shape,
                         std::size_t& checks,std::size_t maxChecks){
 if(!shape.closed||shape.nodes.size()<3)return false;
 const std::size_t n=shape.nodes.size();
 for(std::size_t a=0;a<n;++a){
  const Pt p=shape.nodes[a].p,q=shape.nodes[(a+1)%n].p;
  if(std::hypot(p.x-q.x,p.y-q.y)<=epsilon)return false;
  for(std::size_t b=a+1;b<n;++b){
   if(b==a+1||(a==0&&b==n-1))continue;
   if(++checks>maxChecks)throw std::runtime_error("Limite de contornos complexos excedido");
   if(segmentsMeet(p,q,shape.nodes[b].p,shape.nodes[(b+1)%n].p))
    return false;
  }
 }
 return true;
}
struct Constraints{
 std::vector<std::vector<std::size_t>> children; // parent index -> nested children
 std::size_t nestedPairs=0;
};
// Worst-case NxN bounding-box work; expensive edge tests are budgeted.
inline Constraints build(const std::vector<const Contour*>& shapes,
                         std::size_t maxChecks=4000000){
 Constraints result;
 result.children.resize(shapes.size());
 std::vector<Bounds> boxes;boxes.reserve(shapes.size());
 for(const auto* c:shapes)boxes.push_back(bounds(*c));
 std::size_t checks=0;
 for(std::size_t parent=0;parent<shapes.size();++parent){
  if(!shapes[parent]->closed)continue;
  // Avoid O(nodes²) unless some other box can actually fit inside.
  bool possible=false;
  for(std::size_t child=0;child<shapes.size();++child)
   if(parent!=child&&boundsStrictlyInside(boxes[child],boxes[parent])){
    possible=true;break;
   }
  if(!possible||!simpleClosed(*shapes[parent],checks,maxChecks))continue;
  for(std::size_t child=0;child<shapes.size();++child){
   if(parent==child)continue;
   if(strictlyContains(*shapes[parent],*shapes[child],
                       boxes[parent],boxes[child],checks,maxChecks)){
    result.children[parent].push_back(child);
    ++result.nestedPairs;
   }
  }
 }
 return result;
}
} // namespace alaser::sim198::nesting
