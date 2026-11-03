#include "../src/editor_tools.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>
int main(){
  using namespace alaser;
  Editor editor(Document{});
  std::vector<Pt> points;
  points.reserve(4000);
  for(int i=0;i<4000;++i)points.push_back(Pt{double(i)*0.01,double(i%37)*0.5});
  const int id=editor.addContour(std::move(points),false,0);
  assert(editor.state().doc.contours.front().nodes.size()==4000);
  const Pt initial=editor.state().doc.contours.front().nodes.front().p;
  const auto before=editor.historyNodeFootprint();
  assert(before.currentNodes==4000);
  editor.moveObjects({id},Pt{3,5});
  const auto moved=editor.state().doc.contours.front().nodes.front().p;
  assert(std::abs(moved.x-initial.x-3)<1e-9);
  assert(std::abs(moved.y-initial.y-5)<1e-9);
  const auto moving=editor.historyNodeFootprint();
  assert(moving.undoStates==2);
  assert(moving.undoNodes==4000);
  assert(moving.undoNodeBytes>=4000*sizeof(editor.state().doc.contours.front().nodes[0]));
  assert(editor.undo());
  const auto old=editor.state().doc.contours.front().nodes.front().p;
  assert(std::abs(old.x-initial.x)<1e-9&&std::abs(old.y-initial.y)<1e-9);
  const auto undone=editor.historyNodeFootprint();
  assert(undone.redoNodes==4000);
  assert(editor.redo());
  const auto again=editor.state().doc.contours.front().nodes.front().p;
  assert(std::abs(again.x-moved.x)<1e-9&&std::abs(again.y-moved.y)<1e-9);
  editor.moveObjects({id},Pt{1,0});
  assert(editor.redoCount()==0);
  for(int i=0;i<44;++i)editor.moveObjects({id},Pt{0.01,0.0});
  assert(editor.undoCount()==40);
  assert(editor.historyNodeFootprint().undoNodes==4000*40);
  std::cout<<"PASS history-memory 19.9; undo="<<editor.undoCount()
           <<" retained_nodes="<<editor.historyNodeFootprint().undoNodes<<"\n";
}
