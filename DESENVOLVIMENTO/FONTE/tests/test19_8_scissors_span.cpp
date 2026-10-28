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

  // Regressao real do GUI 19.8: cruzamentos nas arestas verticais de
  // retangulo e clique na aresta superior, antes do PRIMEIRO cruzamento
  // na sequencia circular dos nos (intervalo que cruza a origem).
  Editor wrap(Document{});
  const int box=wrap.rectangle({100,100},20,20);
  const int divider=wrap.line({90,110},{130,110});
  const Document wrapBefore=wrap.state().doc;
  planner.reset(wrap.state().doc);
  auto top=planner.candidate(wrap.state().doc,{{box,0},0,0,{110,100}});
  require(bool(top),"arco anterior ao primeiro cruzamento ficou sem destaque");
  close(top->from,-0.5);close(top->to,1.5);
  require(top->preview.size()==4,"preview circular superior fragmentado");
  close(top->preview.front().y,110);close(top->preview.back().y,110);
  const int wrapResult=wrap.removeScissorSpan(*top);
  require(wrapResult==box,"recorte de arco fechado mudou id original");
  const auto* remainBox=get(wrap.state().doc,{box,0});
  require(remainBox&&!remainBox->closed&&remainBox->nodes.size()==4,
          "recorte circular deixou arco antigo por tras");
  require(get(wrap.state().doc,{divider,0})!=nullptr,
          "recorte de arco removeu limite usado como referencia");
  require(wrap.undo()&&sameDoc(wrapBefore,wrap.state().doc),
          "desfazer recorte que cruza origem nao restaurou contorno");

  // Outro lado da costura: cursor apos ultimo cruzamento deve enxergar
  // o mesmo intervalo circular, sem avancar duas voltas.
  planner.reset(wrap.state().doc);
  auto beforeOrigin=planner.candidate(wrap.state().doc,{{box,0},3,0,{100,105}});
  require(bool(beforeOrigin),"arco apos ultimo cruzamento sem destaque");
  close(beforeOrigin->from,3.5);close(beforeOrigin->to,5.5);
  require(beforeOrigin->preview.size()==4,
          "arco de retorno recebeu limite incorreto");

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

  // Curva que cruza a si mesma: a intersecao tambem delimita dois arcos.
  Editor bow(Document{});
  const int bowId=bow.addContour({{-2,-2},{2,2},{-2,2},{2,-2}},true);
  const Document bowBefore=bow.state().doc;
  planner.reset(bow.state().doc);
  auto bowSpan=planner.candidate(bow.state().doc,{{bowId,0},1,0,{0,2}});
  require(bool(bowSpan)&&bowSpan->preview.size()>2,"autocruzamento nao delimita trecho");
  bow.removeScissorSpan(*bowSpan);
  require(get(bow.state().doc,{bowId,0})&&!get(bow.state().doc,{bowId,0})->closed,
          "autocruzamento nao removeu arco");
  require(bow.undo()&&sameDoc(bowBefore,bow.state().doc),"undo autocruzamento falhou");

  // Uma camada invisivel nao pode criar limites de corte fantasma.
  Editor invis(Document{});
  const int face=invis.rectangle({0,0},10,10);
  const int helper=invis.line({-5,5},{15,5});
  invis.addLayer(4,"Limite oculto");invis.moveToLayer(helper,4);
  invis.setLayerFlags(4,false,false);
  const auto& st=invis.state();
  planner.reset(st.doc,[&](Key key){
   auto meta=st.objects.find(key.object);
   return meta!=st.objects.end()&&st.layers.at(meta->second.layer).visible;
  });
  // O corte oculto nao pode criar duas fronteiras; como nao ha mais
  // cruzamentos visiveis, Trim Shapes deve destacar a forma INTEIRA.
  const Document invisBefore=invis.state().doc;
  auto isolated=planner.candidate(st.doc,{{face,0},0,0,{5,0}});
  require(bool(isolated)&&isolated->closed&&isolated->preview.size()==5,
          "camada oculta criou fronteira fantasma ou bloqueou forma inteira");
  require(invis.removeScissorSpan(*isolated)==0,
          "Tesoura nao removeu forma isolada diante de camada invisivel");
  require(!get(invis.state().doc,{face,0})&&get(invis.state().doc,{helper,0}),
          "Tesoura apagou tambem o contorno oculto ou deixou restos do alvo");
  require(invis.undo()&&sameDoc(invis.state().doc,invisBefore),
          "Undo com camada oculta nao restaurou o documento");
  require(!invis.state().layers.at(4).visible,
          "Undo da Tesoura tornou visivel a camada oculta");
  Editor noCross(Document{});
  int plain=noCross.ellipse({0,0},10,10,64);
  planner.reset(noCross.state().doc);
  const auto* nc=get(noCross.state().doc,{plain,0});
  Pt p=(nc->nodes[10].p+nc->nodes[11].p)*0.5;
  auto entireCircle=planner.candidate(noCross.state().doc,{{plain,0},10,0,p});
  require(bool(entireCircle)&&entireCircle->preview.size()>=64,
          "Tesoura nao destaca circulo sem cruzamentos por inteiro");
  const auto circleBefore=noCross.state().doc;
  require(noCross.removeScissorSpan(*entireCircle)==0,
          "Tesoura nao excluiu circulo completo");
  require(noCross.state().doc.contours.empty(),
          "Tesoura deixou restos de circulo");
  require(noCross.undo()&&sameDoc(circleBefore,noCross.state().doc),
          "Undo do circulo isolado falhou");
  Editor soloRect(Document{});
  const int idRect=soloRect.rectangle({2,3},20,10);
  planner.reset(soloRect.state().doc);
  auto rect=planner.candidate(soloRect.state().doc,{{idRect,0},0,0,{12,3}});
  require(bool(rect)&&rect->preview.size()==5,
          "Retangulo novo nao recebe destaque integral");
  require(soloRect.removeScissorSpan(*rect)==0&&soloRect.state().doc.contours.empty(),
          "Retangulo novo nao foi excluido por inteiro");

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

  std::cout<<"PASS: circulo/retangulo isolados, trechos densos, origem circular, elipse 64, autocruzamentos, invisibilidade, previas, undo/redo, guardas\n";
  return 0;
 }catch(const std::exception& ex){
  std::cerr<<"FAIL: "<<ex.what()<<"\n";return 1;
 }
}
