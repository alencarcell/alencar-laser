#include "editor_tools.hpp"
#include "persistence.hpp"
#include <iostream>
#include <cmath>
#include <stdexcept>
using namespace alaser;
static void need(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
static void near(Pt a,Pt b){need(dist(a,b)<1e-9,"coordinate drift");}
static std::string snapshot(const Editor& e){return boost::json::serialize(encode(e.state().doc))+"\n"+boost::json::serialize(metadata(e.state()));}
int main(){
 try{
  Editor e(Document{});
  int open=e.addContour({{0,0},{10,0},{20,0},{30,0}},false);
  e.addLayer(5,"Corte");
  e.moveSelectionToLayer({open},5);
  need(e.state().objects.at(open).layer==5,"layer transfer failed");
  e.undo();need(e.state().objects.at(open).layer==0,"layer undo failed");
  e.redo();need(e.state().objects.at(open).layer==5,"layer redo failed");
  const auto before=snapshot(e);const size_t history=e.undoCount();
  int newId=e.cutPolylineEdge({open,0},1,{15,0});
  need(newId!=open,"split must create another object");
  need(e.undoCount()==history+1,"scissor should be atomic");
  auto a=get(e.state().doc,{open,0}),b=get(e.state().doc,{newId,0});
  need(a&&b&&a->nodes.size()==3&&b->nodes.size()==3,"open split topology failed");
  near(a->nodes.back().p,{15,0});near(b->nodes.front().p,{15,0});
  need(e.state().objects.at(newId).layer==5,"split must inherit layer");
  need(a->nodes.back().id!=b->nodes.front().id,"split endpoint IDs must differ");
  need(countNodes(e.state().doc)==6,"split node count invalid");
  need(e.undo(),"undo split failed");need(snapshot(e)==before,"undo split not exact");
  need(e.redo(),"redo split failed");need(e.state().objects.count(newId),"redo missing new object");
  const auto safe=snapshot(e);size_t old=e.undoCount();
  try{e.cutPolylineEdge({open,0},0,{0,0});throw std::runtime_error("accepted endpoint cut");}
  catch(const std::runtime_error& ex){need(std::string(ex.what())!="accepted endpoint cut","endpoint accepted");}
  need(snapshot(e)==safe&&e.undoCount()==old,"failed cut mutated state");
  e.setLayerFlags(5,true,true);
  try{e.cutPolylineEdge({newId,0},0,{17,0});throw std::runtime_error("locked cut permitted");}
  catch(const std::runtime_error& ex){need(std::string(ex.what())!="locked cut permitted","lock failed");}
  e.setLayerFlags(5,true,false);
  const int rect=e.rectangle({40,40},10,10);auto prior=snapshot(e);
  const size_t h=e.undoCount();
  int sameId=e.cutPolylineEdge({rect,0},1,{50,45});
  need(sameId==rect,"closed cut shouldn't create object");
  auto* r=get(e.state().doc,{rect,0});need(r&&!r->closed&&r->nodes.size()==6,"closed cut topology failed");
  near(r->nodes.front().p,{50,45});near(r->nodes.back().p,{50,45});
  need(r->nodes.front().id!=r->nodes.back().id,"closed seam IDs duplicate");
  need(e.undoCount()==h+1,"closed cut not atomic");
  e.undo();need(snapshot(e)==prior,"closed undo changed geometry");
  e.redo();need(!get(e.state().doc,{rect,0})->closed,"closed redo failed");
  int grouped=e.rectangle({60,60},5,5),other=e.line({70,70},{75,70});
  e.group({grouped,other});prior=snapshot(e);
  try{e.cutPolylineEdge({grouped,0},1,{65,62});throw std::runtime_error("group split permitted");}
  catch(const std::runtime_error& ex){need(std::string(ex.what())!="group split permitted","group guard failed");}
  need(snapshot(e)==prior,"group guard mutated state");
  // Tesoura v19.8: REMOCAO visual real da aresta, preservando a operacao de corte legado.
  Editor closed(Document{});
  const int c=closed.rectangle({0,0},10,10);
  const auto rectangleBefore=snapshot(closed);
  const size_t closedHistory=closed.undoCount();
  int current=closed.removePolylineEdge({c,0},1);
  need(current==c,"closed erase changed object id");
  const auto* opened=get(closed.state().doc,{c,0});
  need(opened&&!opened->closed&&opened->nodes.size()==4,"closed edge was not removed");
  near(opened->nodes.front().p,{10,10});
  near(opened->nodes.back().p,{10,0});
  need(closed.undoCount()==closedHistory+1,"closed removal not atomic");
  need(closed.undo()&&snapshot(closed)==rectangleBefore,"closed erase undo differs");
  need(closed.redo()&&!get(closed.state().doc,{c,0})->closed,"closed erase redo failed");

  Editor split(Document{});
  const int o=split.addContour({{0,0},{10,0},{20,0},{30,0}},false);
  split.addLayer(7,"Teste camada");split.moveToLayer(o,7);
  const auto openBefore=snapshot(split), undoBefore=split.undoCount();
  const int remaining=split.removePolylineEdge({o,0},1);
  need(remaining!=0&&remaining!=o,"open edge did not create independent contour");
  auto* l=get(split.state().doc,{o,0});
  auto* rr=get(split.state().doc,{remaining,0});
  need(l&&rr&&l->nodes.size()==2&&rr->nodes.size()==2,"open edge still present");
  near(l->nodes.back().p,{10,0});near(rr->nodes.front().p,{20,0});
  need(split.state().objects.at(remaining).layer==7,"layer lost on scissor removal");
  need(split.undoCount()==undoBefore+1,"open removal not atomic");
  need(split.undo()&&snapshot(split)==openBefore,"open removal undo differs");
  need(split.redo()&&get(split.state().doc,{remaining,0}),"open removal redo failed");
  split.setLayerFlags(7,true,true);
  const auto lockedState=snapshot(split), lockedHistory=split.undoCount();
  bool refused=false;
  try{split.removePolylineEdge({o,0},0);}catch(const std::runtime_error&){refused=true;}
  need(refused&&snapshot(split)==lockedState&&split.undoCount()==lockedHistory,"locked removal altered document");
  Editor lone(Document{});
  int single=lone.line({0,0},{10,0});
  need(lone.removePolylineEdge({single,0},0)==0,"single edge not deleted");
  need(lone.state().doc.contours.empty()&&lone.state().objects.empty(),"single edge left shadow geometry");
  need(lone.undo()&&get(lone.state().doc,{single,0}),"single edge undo failed");
  const auto decoded=fromMetadata(decode(encode(e.state().doc)),metadata(e.state()));
  validate(decoded);
  std::cout<<"PASS: tesoura de corte e remocao real de aresta, IDs unicos, metadados, bloqueios, camada, undo/redo, serializacao\n";
  return 0;
 }catch(const std::exception&ex){std::cerr<<"FAIL: "<<ex.what()<<"\n";return 1;}
}
