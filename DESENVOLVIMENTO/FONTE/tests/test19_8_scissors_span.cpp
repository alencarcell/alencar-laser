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

  // Caso real: depois de remover uma ponta da linha que cruza uma guia,
  // o resto termina exatamente no cruzamento antigo. Deve ser possivel
  // destacar e excluir o restante; tangencias INTERIORES ficam bloqueadas.
  Editor residual(Document{});
  const int segment=residual.line({0,0},{10,0});
  const int boundary=residual.line({5,-3},{5,3});
  planner.reset(residual.state().doc);
  auto firstTail=planner.candidate(residual.state().doc,
                      {{segment,0},0,0,{2.5,0}});
  require(bool(firstTail)&&firstTail->preview.size()==2,
          "nao identificou a primeira ponta excedente");
  residual.removeScissorSpan(*firstTail);
  planner.reset(residual.state().doc);
  auto remaining=planner.candidate(residual.state().doc,
                     {{segment,0},0,0,{7.5,0}});
  require(bool(remaining)&&remaining->preview.size()==2,
          "extremidade ja cortada bloqueou restante da linha aberta");
  require(residual.removeScissorSpan(*remaining)==0,
          "tesoura nao removeu o segundo excedente/contorno remanescente");
  require(!get(residual.state().doc,{segment,0})&&
          get(residual.state().doc,{boundary,0}),
          "segundo corte alterou linha de referencia");
  require(residual.undo()&&get(residual.state().doc,{segment,0}),
          "undo do segundo corte nao restaurou linha remanescente");

  Editor unsafeInterior(Document{});
  const int base=unsafeInterior.addContour({{0,0},{5,0},{10,0}},false);
  unsafeInterior.line({5,-3},{5,3});
  planner.reset(unsafeInterior.state().doc);
  auto ambiguous=planner.candidate(unsafeInterior.state().doc,
                     {{base,0},0,0,{2.5,0}});
  require(!ambiguous,"cruzamento no vertice interior foi excluido sem seguranca");
  require(planner.lastRejectionReason()=="vertex_or_tangent",
          "intersecao em vertice nao recebeu motivo seguro");
  // Overlapping collinear segments are not unique trim boundaries.
  Editor overlap(Document{});
  const int longLine=overlap.line({0,0},{10,0});
  overlap.line({3,0},{7,0});
  planner.reset(overlap.state().doc);
  auto overlapHit=planner.candidate(overlap.state().doc,
                           {{longLine,0},0,0,{5,0}});
  require(!overlapHit&&planner.lastRejectionReason()=="collinear_overlap",
          "sobreposicao colinear foi recortada ou nao classificada");
  // No intersections: the LightBurn-style whole-shape preview is retained.
  Editor isolatedCheck(Document{});
  const int isolatedId=isolatedCheck.rectangle({0,0},10,10);
  planner.reset(isolatedCheck.state().doc);
  auto whole=planner.candidate(isolatedCheck.state().doc,
                     {{isolatedId,0},0,0,{5,0}});
  require(bool(whole)&&planner.lastRejectionReason()=="none",
          "retangulo isolado deixou de oferecer recorte integral");


  // 20u: a foreign path vertex that genuinely crosses the target
  // must delimit the same span as a crossing inside a single edge.
  Editor vertexPass(Document{});
  const int horizontal=vertexPass.line({0,0},{10,0});
  const int joint=vertexPass.addContour({{5,-4},{5,0},{5,4}},false);
  const int outerLimit=vertexPass.line({8,-4},{8,4});
  const Document vertexBefore=vertexPass.state().doc;
  planner.reset(vertexPass.state().doc);
  auto safeVertex=planner.candidate(vertexPass.state().doc,
                      {{horizontal,0},0,0,{6.5,0}});
  require(bool(safeVertex),"cruzamento transversal em vertice externo foi recusado");
  close(safeVertex->from,0.5);close(safeVertex->to,0.8);
  close(safeVertex->preview.front().x,5.);
  close(safeVertex->preview.back().x,8.);
  vertexPass.removeScissorSpan(*safeVertex);
  require(get(vertexPass.state().doc,{joint,0})&&
          get(vertexPass.state().doc,{outerLimit,0}),
          "corte no vertice externo alterou linhas de referencia");
  require(vertexPass.undo()&&sameDoc(vertexBefore,vertexPass.state().doc),
          "undo do cruzamento transversal nao restaurou geometria");

  // A corner touching the line and returning to the same side is NOT a cut.
  Editor tangentVertex(Document{});
  const int target=tangentVertex.line({0,0},{10,0});
  tangentVertex.addContour({{5,-4},{5,0},{6,-4}},false);
  planner.reset(tangentVertex.state().doc);
  auto tangentSpan=planner.candidate(tangentVertex.state().doc,
                              {{target,0},0,0,{5.5,0}});
  require(!tangentSpan&&planner.lastRejectionReason()=="vertex_or_tangent",
          "tangencia de vertice foi tratada como cruzamento seguro");

  // An open foreign endpoint meeting the target is a T, not a
  // transverse two-sided crossing.
  Editor tee(Document{});
  const int horizontalT=tee.line({0,0},{10,0});
  tee.line({5,-4},{5,0});
  planner.reset(tee.state().doc);
  require(!planner.candidate(tee.state().doc,
                     {{horizontalT,0},0,0,{6,0}})&&
          planner.lastRejectionReason()=="vertex_or_tangent",
          "ponta externa em T foi promovida a intersecao transversal");

  // 20x: the CUT PATH crosses a straight foreign line exactly at its
  // own interior vertices. This is a true transverse crossing, not a touch.
  Editor targetVertex(Document{});
  const int zig=targetVertex.addContour({{0,-3},{5,0},{6,2},{8,0},{10,-3}},false);
  const int guide=targetVertex.line({0,0},{10,0});
  const Document targetVertexBefore=targetVertex.state().doc;
  planner.reset(targetVertex.state().doc);
  auto zigSpan=planner.candidate(targetVertex.state().doc,
                       {{zig,0},2,0,{7,1}});
  require(bool(zigSpan),"20x: alvo atravessando reta no proprio vertice recusado");
  close(zigSpan->from,1);close(zigSpan->to,3);
  close(zigSpan->preview.front().x,5);
  close(zigSpan->preview.back().x,8);
  targetVertex.removeScissorSpan(*zigSpan);
  require(get(targetVertex.state().doc,{guide,0})!=nullptr,
          "20x: guia transversal foi removida indevidamente");
  require(targetVertex.undo()&&sameDoc(targetVertexBefore,targetVertex.state().doc),
          "20x: Undo nao restaurou geometria do alvo");
  Editor targetTangent(Document{});
  const int apex=targetTangent.addContour({{0,3},{5,0},{10,3}},false);
  targetTangent.line({0,0},{10,0});
  planner.reset(targetTangent.state().doc);
  auto illegal=planner.candidate(targetTangent.state().doc,
                        {{apex,0},0,0,{3,1.8}});
  require(!illegal&&planner.lastRejectionReason()=="vertex_or_tangent",
          "20x: tangencia no vertice do proprio alvo foi cortada");

  // 20v atomic weld contract. This artificially specified valid interval
  // isolates the *post-cut join* from the planner's separate T-junction gate.
  Editor weld(Document{});
  const int cutLine=weld.line({0,0},{10,0});
  const int continuation=weld.line({3,0},{3,4});
  const Document weldBefore=weld.state().doc;
  const auto undoBefore=weld.undoCount();
  ScissorSpan trim{{cutLine,0},.3,.7,false,{{3,0},{7,0}}};
  weld.removeScissorSpan(trim,true);
  require(weld.undoCount()==undoBefore+1,"20v: weld created multiple Undo states");
  require(weld.state().doc.contours.size()==2,
          "20v: unique matching endpoint did not weld");
  require(!get(weld.state().doc,{continuation,0}),
          "20v: welded object retained as duplicate");
  require(weld.undo()&&sameDoc(weldBefore,weld.state().doc),
          "20v: Undo did not restore cut and weld atomically");
  // Ctrl-style no-join leaves the cut pieces and the other path separate.
  weld.removeScissorSpan(trim,false);
  require(weld.state().doc.contours.size()==3&&
          get(weld.state().doc,{continuation,0}),
          "20v: no-join modifier failed");
  // Different layers never weld, even with exact endpoints.
  Editor otherLayer(Document{});
  const int source=otherLayer.line({0,0},{10,0});
  const int next=otherLayer.line({3,0},{3,4});
  otherLayer.addLayer(2,"second");
  otherLayer.moveToLayer(next,2);
  otherLayer.removeScissorSpan({{source,0},.3,.7,false,{{3,0},{7,0}}},true);
  require(otherLayer.state().doc.contours.size()==3,
          "20v: joined contours across different layers");
  // Reproducao numerica da geometria enviada pelo operador: objeto 575.
  // O caminho aberto tem uma ponta acima do triangulo e termina com um
  // vertice sobre a PRIMEIRA aresta, uma junção em T de ponta externa.
  Editor realTriangle(Document{});
  const int triangle=realTriangle.addContour({
     {36.42090666666665,171.17826133333327},
     {36.42090666666666,234.15374697766455},
     {85.90379017754454,234.15374697766455},
     {36.42090666666665,184.67086346678667}},false);
  const Document triangleBefore=realTriangle.state().doc;
  planner.reset(realTriangle.state().doc);
  auto excess=planner.candidate(realTriangle.state().doc,
          {{triangle,0},0,0,{36.42090666666665,174.}});
  require(bool(excess),"objeto 575: ponta externa na junta T nao ficou em laranja");
  require(excess->from<1e-7&&excess->to>0.20&&excess->to<0.23,
          "objeto 575: ponta excedente nao termina na junta T");
  realTriangle.removeScissorSpan(*excess);
  const auto* onlyShape=get(realTriangle.state().doc,{triangle,0});
  require(onlyShape&&hasExactDuplicateClosure(*onlyShape),
          "objeto 575: primeiro recorte nao deixou triangulo fechado logicamente");
  planner.reset(realTriangle.state().doc);
  auto remainder=planner.candidate(realTriangle.state().doc,
          {{triangle,0},1,0,{60.,234.15374697766455}});
  require(bool(remainder)&&remainder->closed,
          "objeto 575: resto do triangulo nao recebeu contorno laranja");
  require(realTriangle.removeScissorSpan(*remainder)==0&&
          realTriangle.state().doc.contours.empty(),
          "objeto 575: Tesoura nao apagou segunda forma restante");
  require(realTriangle.undo()&&get(realTriangle.state().doc,{triangle,0}),
          "objeto 575: segundo recorte nao pode ser desfeito");
  require(realTriangle.undo()&&sameDoc(triangleBefore,realTriangle.state().doc),
          "objeto 575: segundo Undo nao restaurou original binario");

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
  const auto* circleRemainder=get(ring.state().doc,{circle,0});
  require(circleRemainder&&!circleRemainder->closed,"arco fechado nao virou caminho aberto");
  require(circleRemainder->nodes.size()<n0&&circleRemainder->nodes.size()>5,"arco removido de maneira fragmentada");
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
