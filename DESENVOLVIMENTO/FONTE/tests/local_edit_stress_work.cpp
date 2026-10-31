// Deterministic exhaustive edge/node matrix, including coincident neighbors.
#include "../src/editor_tools.hpp"
#include <iostream>
using namespace alaser;
static void need(bool x,const char* why){if(!x)throw std::runtime_error(why);}
static Document onlyOthers(Document d,Key k){
 d.contours.erase(std::remove_if(d.contours.begin(),d.contours.end(),[&](const Contour& c){return c.key==k;}),d.contours.end());
 d.nextId=0;return d;
}
int main(){try{
 int cases=0;
 for(bool closed:{false,true})for(int n=3;n<=40;++n){
  std::vector<Pt> p;for(int i=0;i<n;++i)p.push_back({10*std::cos(i*6.283185307179586/n),10*std::sin(i*6.283185307179586/n)});
  for(int mode=0;mode<2;++mode)for(int j=0;j<(mode?n-(closed?0:1):n);++j){
   Editor e(Document{});const Key target{e.addContour(p,closed),0};
   const int neighbor=e.addContour(p,closed); // Exactly coincident, different IDs.
   const auto initial=e.state().doc;const auto history=e.undoCount();
   const auto originalNeighbor=*get(initial,{neighbor,0});
   if(mode)e.removePolylineEdge(target,j);else e.erase(target,get(initial,target)->nodes[j].id);
   const auto changed=e.state().doc;
   const auto* untouched=get(changed,{neighbor,0});
   need(untouched&&untouched->nodes.size()==originalNeighbor.nodes.size(),"neighbor missing");
   Document a,b;a.contours={*untouched};b.contours={originalNeighbor};
   need(sameDoc(a,b),"coincident neighbor changed");
   need(e.undoCount()==history+1,"non-atomic history");
   for(int repeat=0;repeat<5;++repeat){
    need(e.undo()&&sameDoc(e.state().doc,initial),"undo identity mismatch");
    need(e.redo()&&sameDoc(e.state().doc,changed),"redo identity mismatch");
   }
   const auto count=e.undoCount();bool refused=false;
   try{e.removePolylineEdge({999999,0},0);}catch(const std::exception&){refused=true;}
   need(refused&&sameDoc(e.state().doc,changed)&&e.undoCount()==count,"invalid operation changed state");
   ++cases;
  }
 }
 std::cout<<"PASS "<<cases<<" cases; coincident neighbors; 5 Undo/Redo cycles each; invalid target atomic\n";
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
