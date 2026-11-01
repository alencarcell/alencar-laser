#include "../src/editor_tools.hpp"
#include <iostream>
#include <cmath>
using namespace alaser;
static void need(bool result,const char* msg){if(!result)throw std::runtime_error(msg);}
static void near(double a,double b,const char* msg){need(std::abs(a-b)<1e-7,msg);}
static double low(const Editor& e,int id,bool horizontal){
 auto b=boundsOf(e.state().doc,{id});return horizontal?b.first.x:b.first.y;
}
int main(){
 try{
  for(bool horizontal:{false,true}){
   Editor e(Document{});
   const int a=e.rectangle({0,0},10,10);
   const int b=e.rectangle(horizontal?Pt{30,0}:Pt{0,30},20,10);
   const int c=e.rectangle(horizontal?Pt{100,0}:Pt{0,100},10,10);
   const int other=e.rectangle({400,400},10,10);
   const auto initial=e.state().doc;
   const size_t history=e.undoCount();
   e.distributeObjects({a,b,c},horizontal);
   near(low(e,b,horizontal),horizontal?45.0:50.0,"wrong equal spacing");
   need(e.undoCount()==history+1,"history is not atomic");
   near(low(e,other,horizontal),400,"unselected neighbor moved");
   near(low(e,a,horizontal),0,"first anchor moved");
   near(low(e,c,horizontal),100,"last anchor moved");
   const auto changed=e.state().doc;
   for(int j=0;j<3;++j){
    need(e.undo()&&sameDoc(initial,e.state().doc),"undo is incorrect");
    need(e.redo()&&sameDoc(changed,e.state().doc),"redo is incorrect");
   }
   const size_t before=e.undoCount();bool rejected=false;
   try{e.distributeObjects({a,b},horizontal);}catch(const std::exception&){rejected=true;}
   need(rejected&&e.undoCount()==before&&sameDoc(changed,e.state().doc),
        "invalid selection mutated state");
  }
  Editor e(Document{});
  const int left=e.rectangle({0,0},10,10);
  const int first=e.rectangle({30,0},10,10);
  const int second=e.rectangle({45,0},10,10);
  const int right=e.rectangle({100,0},10,10);
  e.group({first,second});const auto start=e.state().doc;
  e.distributeObjects({left,first,right},true);
  near(low(e,first,true),42.5,"group start position wrong");
  near(low(e,second,true),57.5,"unselected group member not moved");
  need(e.undo()&&sameDoc(e.state().doc,start),"group undo not exact");
  const auto prev=e.state().doc;
  e.setLayerFlags(0,true,true);
  const size_t history=e.undoCount();bool refused=false;
  try{e.distributeObjects({left,first,right},true);}catch(const std::exception&){refused=true;}
  need(refused&&e.undoCount()==history&&sameDoc(prev,e.state().doc),
       "locked group was modified");
  std::cout<<"DISTRIBUTE_19_8 PASS: horizontal, vertical, groups, locks, atomic undo/redo\n";
 }catch(const std::exception& ex){
  std::cerr<<"DISTRIBUTE_19_8 FAIL: "<<ex.what()<<'\n';return 1;
 }
}
