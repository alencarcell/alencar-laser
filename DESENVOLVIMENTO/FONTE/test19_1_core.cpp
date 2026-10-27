#include "editor_tools.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace alaser;
int main(){
 Editor e(Document{});
 const int first=e.rectangle({10,20},8,4);
 assert(e.state().doc.contours.size()==1);
 const int second=e.duplicate(first,{3,3});
 assert(second!=first && e.state().doc.contours.size()==2);
 const auto* d=get(e.state().doc,{second,0});
 assert(d && d->nodes.size()==4);
 assert(std::abs(d->nodes[0].p.x-13)<1e-9);
 auto [lo,hi]=boundsOf(e.state().doc,{second});
 e.mirrorX({second},(lo.x+hi.x)/2);
 const auto* mirrored=get(e.state().doc,{second,0});
 assert(mirrored && std::abs(mirrored->nodes[0].p.x-21)<1e-9);
 auto [lo2,hi2]=boundsOf(e.state().doc,{second});
 e.rotateObjects({second},{(lo2.x+hi2.x)/2,(lo2.y+hi2.y)/2},90);
 const auto* rotated=get(e.state().doc,{second,0});
 assert(rotated && std::abs(rotated->nodes[0].p.x-19)<1e-9);
 assert(e.undo());
 assert(std::abs(get(e.state().doc,{second,0})->nodes[0].p.x-21)<1e-9);
 assert(e.undo());
 assert(std::abs(get(e.state().doc,{second,0})->nodes[0].p.x-13)<1e-9);
 assert(e.redo());
 assert(std::abs(get(e.state().doc,{second,0})->nodes[0].p.x-21)<1e-9);
 std::cout<<"TESTE_19_1_CORE_TRANSFORMS PASS: duplicate, mirrorX, rotate90, undo, redo\n";
}
