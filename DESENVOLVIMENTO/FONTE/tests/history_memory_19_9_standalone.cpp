#include "../src/editor_tools.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <cmath>
// Always evaluated, including Release builds compiled with -DNDEBUG.
static void require(bool condition,const char* label){
  if(!condition)throw std::runtime_error(label);
}
int main(){
  using namespace alaser;
  Editor editor(Document{});
  std::vector<Pt> points;
  points.reserve(4000);
  for(int i=0;i<4000;++i)points.push_back(Pt{double(i)*0.01,double(i%37)*0.5});
  const int id=editor.addContour(std::move(points),false,0);
  require(editor.state().doc.contours.front().nodes.size()==4000,"history-check-17");
  const Pt initial=editor.state().doc.contours.front().nodes.front().p;
  const auto before=editor.historyNodeFootprint();
  require(before.currentNodes==4000,"history-check-20");
  editor.moveObjects({id},Pt{3,5});
  const auto moved=editor.state().doc.contours.front().nodes.front().p;
  require(std::abs(moved.x-initial.x-3)<1e-9,"history-check-23");
  require(std::abs(moved.y-initial.y-5)<1e-9,"history-check-24");
  const auto moving=editor.historyNodeFootprint();
  require(moving.undoStates==2,"history-check-26");
  require(moving.undoNodes==4000,"history-check-27");
  require(moving.undoNodeBytes>=4000*sizeof(editor.state().doc.contours.front().nodes[0]),"history-check-28");
  require(editor.undo(),"history-check-29");
  const auto old=editor.state().doc.contours.front().nodes.front().p;
  require(std::abs(old.x-initial.x)<1e-9&&std::abs(old.y-initial.y)<1e-9,"history-check-31");
  const auto undone=editor.historyNodeFootprint();
  require(undone.redoNodes==4000,"history-check-33");
  require(editor.redo(),"history-check-34");
  const auto again=editor.state().doc.contours.front().nodes.front().p;
  require(std::abs(again.x-moved.x)<1e-9&&std::abs(again.y-moved.y)<1e-9,"history-check-36");
  editor.moveObjects({id},Pt{1,0});
  require(editor.redoCount()==0,"history-check-38");
  for(int i=0;i<44;++i)editor.moveObjects({id},Pt{0.01,0.0});
  require(editor.undoCount()==40,"history-check-40");
  require(editor.historyNodeFootprint().undoNodes==4000*40,"history-check-41");
  std::cout<<"PASS history-memory 19.9; undo="<<editor.undoCount()
           <<" retained_nodes="<<editor.historyNodeFootprint().undoNodes<<"\n";
}
