// Alencar Laser 19.8 - affine native Bezier regression, offline only.
#include "../src/bezier_interaction.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace alaser;
static void req(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
static bool near(Pt a,Pt b){return dist(a,b)<1e-8;}
int main(){
 try{
  CurveDragSession edit(CurveDocument{});
  Key key=edit.addEditablePath({
   {0,{10,10},{10,10},{10,10}},
   {0,{30,10},{20,0},{40,20}},
   {0,{50,10},{50,10},{50,10}}
  },false,501);
  const auto original=edit.committed().contours.front();
  const int64_t anchorId=original.nodes[1].id;
  const auto before=edit.undoCount();
  req(edit.transformSelectedContour({10,10},1.5,2.0,0), "affine scale did not commit");
  const auto& scaled=edit.committed().contours.front();
  req(scaled.nodes.size()==original.nodes.size()&&scaled.edgeKind==original.edgeKind,
      "scale changed Bezier topology");
  req(scaled.nodes[1].id==anchorId&&near(scaled.nodes[1].anchor,{40,10})&&
      near(scaled.nodes[1].handleIn,{25,-10})&&near(scaled.nodes[1].handleOut,{55,30}),
      "anchors and cubic handles were not scaled together");
  req(edit.undoCount()==before+1&&edit.undo(),"scaling did not record one undo");
  for(size_t i=0;i<original.nodes.size();++i){
   const auto& n=edit.committed().contours.front().nodes[i];
   req(near(n.anchor,original.nodes[i].anchor)&&
       near(n.handleIn,original.nodes[i].handleIn)&&
       near(n.handleOut,original.nodes[i].handleOut),"scale Undo not exact");
  }
  req(edit.redo()&&near(edit.committed().contours.front().nodes[1].anchor,{40,10}),
      "scale Redo not exact");
  req(edit.transformSelectedContour({40,10},1,1,90),
      "native rotation missing");
  req(near(edit.committed().contours.front().nodes[0].anchor,{40,-35}),
      "rotation did not transform anchor");
  req(edit.undo()&&near(edit.committed().contours.front().nodes[1].anchor,{40,10}),
      "rotation Undo lost prior scale");
  req(edit.transformSelectedContour({0,0},1,1,0,{5,7}),"move transform missing");
  req(near(edit.committed().contours.front().nodes[1].anchor,{45,17}),
      "move did not preserve transformed curve");
  bool invalid=false;
  const auto history=edit.undoCount();
  try{(void)edit.transformSelectedContour({0,0},0,1,0);}catch(const std::invalid_argument&){invalid=true;}
  req(invalid&&history==edit.undoCount(),"invalid zero scale corrupted Undo");
  req(edit.selected()&&*edit.selected()==key,"transform dropped current selection");
  const auto doc=edit.committed();validateCurve(doc);
  std::cout<<"PASS native Bezier scale, rotate, move, exact handles, undo/redo and invalid guards\n";
  return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
