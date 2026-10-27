// Regressao 19.8: tesoura por intervalos inteiros entre cruzamentos.
// Sem Qt, sem USB, sem TCP nem controles da maquina.
#include "editor_tools.hpp"
#include <iostream>
#include <cmath>
#include <stdexcept>
using namespace alaser;
static void require(bool ok,const char* m){if(!ok)throw std::runtime_error(m);}
static void close(double a,double b){require(std::abs(a-b)<1e-6,"drift na intersecao");}
int main(){
 try{
  Editor e(Document{});
  std::vector<Pt> dense;
  for(int i=0;i<=10;i++)dense.push_back({double(i),0});
  const int track=e.addContour(dense,false);
  const int cross1=e.line({2.5,-5},{2.5,5});
  const int cross2=e.line({7.5,-5},{7.5,5});
  const Document original=e.state().doc;
  ScissorSpanPlanner planner;planner.reset(e.state().doc);
  auto span=planner.candidate(e.state().doc,{{track,0},5,0,{5.2,0}});
  require(bool(span),"trecho denso entre cruzamentos nao encontrado");
  close(span->from,2.5);close(span->to,7.5);
  require(span->preview.size()>=6,"hover marcou somente uma aresta");
  close(span->preview.front().x,2.5);close(span->preview.back().x,7.5);
  const size_t history=e.undoCount();
  const int second=e.removeScissorSpan(*span);
  require(second!=track&&second>0,"resto a direita sem identidade propria");
  const auto* left=get(e.state().doc,{track,0});
  const auto* right=get(e.state().doc,{second,0});
  require(left&&right&&!left->closed&&!right->closed,"contornos restantes invalidos");
  close(left->nodes.back().p.x,2.5);close(right->nodes.front().p.x,7.5);
  require(left->nodes.size()==4&&right->nodes.size()==4,"restou geometria oculta no intervalo");
  require(get(e.state().doc,{cross1,0})&&get(e.state().doc,{cross2,0}),"cortou objeto cruzador");
  require(e.undoCount()==history+1,"corte gerou varios passos de historico");
  require(e.undo()&&sameDoc(original,e.state().doc),"undo nao restaurou geometria integral");
  require(e.redo()&&get(e.state().doc,{second,0}),"redo nao restaurou segunda peca");
  validate(e.state());

  Editor ring(Document{});
  const int circle=ring.ellipse({0,0},10,10,64);
  const int knife=ring.line({-20,2},{20,2});
  const Document ringBefore=ring.state().doc;
  planner.reset(ring.state().doc);
  const auto* c=get(ring.state().doc,{circle,0});
  Pt mid=(c->nodes[16].p+c->nodes[17].p)*0.5;
  auto arc=planner.candidate(ring.state().doc,{{circle,0},16,0,mid});
  require(bool(arc),"arco de 64 arestas nao detectado");
  require(arc->preview.size()>20,"pre-visualizacao so pegou uma aresta");
  const auto n0=get(ring.state().doc,{circle,0})->nodes.size();
  const int result=ring.removeScissorSpan(*arc);
  require(result==circle,"arco fechado deveria manter objeto original");
  const auto* remainder=get(ring.state().doc,{circle,0});
  require(remainder&&!remainder->closed,"arco fechado nao virou caminho aberto");
  require(remainder->nodes.size()<n0&&remainder->nodes.size()>5,"arco removido de maneira fragmentada");
  require(get(ring.state().doc,{knife,0})&&get(ring.state().doc,{knife,0})->nodes.size()==2,
          "corte alterou linha cruzadora");
  require(ring.undo()&&sameDoc(ringBefore,ring.state().doc),"undo do arco falhou");
  validate(ring.state());

  Editor noCross(Document{});
  int plain=noCross.ellipse({0,0},10,10,64);
  planner.reset(noCross.state().doc);
  const auto* nc=get(noCross.state().doc,{plain,0});
  Pt p=(nc->nodes[10].p+nc->nodes[11].p)*0.5;
  require(!planner.candidate(noCross.state().doc,{{plain,0},10,0,p}),
    "Tesoura destrutiva aceita sem limites em contorno fechado");

  Editor lock(Document{});
  const int locked=lock.line({0,0},{10,0});
  planner.reset(lock.state().doc);
  auto full=planner.candidate(lock.state().doc,{{locked,0},0,0,{5,0}});
  require(bool(full),"linha aberta nao tem limites de extremidades");
  const Document before=lock.state().doc;
  lock.setLayerFlags(0,true,true);
  const auto prior=lock.undoCount();
  bool rejected=false;
  try{lock.removeScissorSpan(*full);}catch(const std::runtime_error&){rejected=true;}
  require(rejected&&lock.undoCount()==prior&&sameDoc(before,lock.state().doc),
          "Tesoura modificou camada bloqueada");
  lock.setLayerFlags(0,true,false);
  require(lock.removeScissorSpan(*full)==0,"caminho aberto inteiro nao foi removido");
  require(lock.state().doc.contours.empty()&&lock.state().objects.empty(),
          "objeto vazio deixou geometria fantasma");
  validate(lock.state());

  std::cout<<"PASS: trechos densos, elipse 64 segmentos, previas completas, limites, undo/redo, guardas\n";
  return 0;
 }catch(const std::exception& ex){
  std::cerr<<"FAIL: "<<ex.what()<<"\n";return 1;
 }
}
