// Alencar Laser - Fase 20m: duplicacao em lote atomica, SEM Qt/hardware.
#include "editor_tools.hpp"
#include <cmath>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>
#include <string>
using namespace alaser;

static void require(bool b,const char* message){
 if(!b)throw std::runtime_error(message);
}
static Document rectangles(int count){
 Document d;
 std::int64_t nextNode=1;
 for(int i=1;i<=count;++i){
  Contour c;c.key={i,0};c.closed=true;
  const double x=double(i%25)*12.,y=double(i/25)*12.;
  // 24 nodes per object: 575 objects = 13,800 nodes, near the
  // 13,415-node workload observed in a real Windows diagnosis.
  for(int k=0;k<24;++k){
   const double a=6.283185307179586*double(k)/24.;
   const Pt p{x+4.+4.*std::cos(a),y+4.+4.*std::sin(a)};
   c.nodes.push_back({nextNode++,p});
  }
  d.contours.push_back(std::move(c));
 }
 d.nextId=nextNode;
 return d;
}
static void stress575(){
 Editor editor(rectangles(575));
 const auto beforeNodes=countNodes(editor.state().doc);
 require(editor.state().doc.contours.size()==575,"fixture invalid");
 std::vector<int> ids;
 for(int i=1;i<=575;++i)ids.push_back(i);
 const auto original=editor.state().doc.contours.front().nodes.front().p;
 const auto historyBefore=editor.undoCount();
 const auto copies=editor.duplicateObjects(ids,{3,3});
 require(copies.size()==575&&copies.front()==576&&copies.back()==1150,
   "575 clones/IDs incorrect");
 require(editor.undoCount()==historyBefore+1,"batch created multiple Undo states");
 require(editor.state().doc.contours.size()==1150,"contours not duplicated");
 require(countNodes(editor.state().doc)==beforeNodes*2,"nodes not duplicated");
 require(editor.state().doc.contours.front().nodes.front().p.x==original.x,
   "source object unexpectedly moved");
 const auto clone=editor.state().doc.contours.at(575).nodes.front().p;
 require(std::abs(clone.x-original.x-3)<1e-9&&
         std::abs(clone.y-original.y-3)<1e-9,"offset mismatch");
 validate(editor.state());
 require(editor.undo(),"undo missing");
 require(editor.state().doc.contours.size()==575&&
         countNodes(editor.state().doc)==beforeNodes,"undo did not restore");
 require(editor.redo(),"redo missing");
 require(editor.state().doc.contours.size()==1150&&
         countNodes(editor.state().doc)==beforeNodes*2,"redo not atomic");
 validate(editor.state());
}
static void groups(){
 auto st=fromDocument(rectangles(4));
 st.objects.at(1).group=7;st.objects.at(2).group=7;
 st.nextGroup=8;
 Editor editor(st);
 auto copied=editor.duplicateObjects({1},{3,3});
 require(copied.size()==2,"partial group duplicated");
 const int grpA=editor.state().objects.at(copied[0]).group;
 const int grpB=editor.state().objects.at(copied[1]).group;
 require(grpA==grpB&&grpA!=7&&grpA>0,"cloned group identity not preserved");
 require(editor.state().objects.at(1).group==7,"source group changed");
 require(editor.undoCount()==1,"group clone Undo not atomic");
}
static void lockedAndInvalidRollback(){
 auto st=fromDocument(rectangles(4));
 Layer blocked=st.layers.at(0);
 blocked.id=1;blocked.locked=true;
 st.layers[1]=blocked;st.objects.at(2).layer=1;
 Editor editor(st);
 const auto gen=editor.generation;
 for(auto source:std::vector<std::vector<int>>{{1,2},{1,9999}}){
  bool failed=false;
  try{editor.duplicateObjects(source,{3,3});}
  catch(const std::exception&){failed=true;}
  require(failed,"locked/missing source was accepted");
  require(editor.generation==gen&&editor.undoCount()==0&&
          editor.state().doc.contours.size()==4,
          "failed duplicate left partial changes or Undo");
 }
}
static void editableTextAndGroup(){
 auto st=fromDocument(rectangles(4));
 st.objects.at(1).group=11;st.objects.at(2).group=11;
 st.nextGroup=12;
 EditableText t;t.text="Teste";t.fontFamily="Arial";
 t.members={1,2};t.anchor={4,5};t.sourceBoundsMin={1,2};
 st.editableTexts[1]=t;
 Editor editor(st);
 auto copies=editor.duplicateObjects({2},{3,3});
 require(copies.size()==2,"text partial member clone");
 require(copies[0]==5&&copies[1]==6,"text new IDs incorrect");
 const auto found=editor.state().editableTexts.find(5);
 require(found!=editor.state().editableTexts.end(),"editable text metadata missing");
 require(found->second.members==copies,"editable text members incorrect");
 require(std::abs(found->second.anchor.x-7)<1e-9&&
         std::abs(found->second.sourceBoundsMin.y-5)<1e-9,
         "editable text geometry metadata not offset");
 require(editor.state().editableTexts.at(1).members.size()==2,
         "source editable text changed");
 validate(editor.state());
 require(editor.undo(),"text undo missing");
 require(editor.state().editableTexts.size()==1,"text undo did not clear clone");
 require(editor.redo(),"text redo missing");
 require(editor.state().editableTexts.size()==2,"text redo did not restore clone");
}
static void limits(){
 Editor editor(rectangles(1001));
 std::vector<int> ids;for(int i=1;i<=1001;++i)ids.push_back(i);
 bool blocked=false;
 try{editor.duplicateObjects(ids,{3,3});}
 catch(const std::exception&){blocked=true;}
 require(blocked&&editor.undoCount()==0,"duplicate limit did not protect state");
}
int main(){
 try{
  stress575();groups();lockedAndInvalidRollback();editableTextAndGroup();limits();
  std::cout<<"PASS DUPLICACAO_20M: 575 objetos, Undo/Redo, grupos, texto, bloqueios e limite; sem hardware\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"FAIL DUPLICACAO_20M: "<<e.what()<<"\n";
  return 1;
 }
}
