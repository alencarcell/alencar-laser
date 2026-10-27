// Alencar Laser, TESTE_7: simulação reproduzível do núcleo geométrico e seleção.
// SEM Qt, SEM GRBL, SEM hardware. Os números não são FPS.
#include "editor_tools.hpp"
#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
#include <set>
#include <array>
#include <cassert>
#include <numeric>
#include <fstream>
#include <filesystem>
using namespace alaser;
using Clock=std::chrono::steady_clock;
static double ms(Clock::time_point a,Clock::time_point b){return std::chrono::duration<double,std::milli>(b-a).count();}
static void verify(bool ok,const char*s){if(!ok)throw std::runtime_error(s);}
static double pct(std::vector<double> v,double p){std::sort(v.begin(),v.end());return v[std::min(v.size()-1,size_t((v.size()-1)*p))];}
static void bench(const Document& doc,const std::string& root){
 auto segs=segments(doc);SegmentBVH bvh(segs);
 std::mt19937_64 rng(20261022);std::uniform_int_distribution<size_t> contourD(0,doc.contours.size()-1);
 struct Data{double brute=0,bvh=0,adaptive=0;size_t candidates=0,hits=0;};
 std::ofstream f(root+"/resultados/selecoes_por_escala.csv");
 verify(bool(f),"cannot create output csv");
 f<<"zoom_px_mm,dpr,queries,brute_total_ms,bvh_total_ms,adaptive_total_ms,p95_brute_ms,p95_bvh_ms,p95_adaptive_ms,avg_bvh_candidates,identity_mismatches,adaptive_mode\n";
 size_t checks=0;int combos=0;size_t totalBrute=0,totalBvh=0;size_t failure=0;
 for(double zoom:{0.05,0.75,6.,80.,1200.})for(double dpr:{1.,1.25,1.5,2.}){
  double tol=6.0/(zoom*dpr);bool indexEnabled=(zoom*dpr>=0.5);
  std::vector<double> tb,ti,ta;size_t sumCandidates=0;size_t mismatches=0;
  // Escolhemos pontos de nós reais de contornos diversos e deslocamentos em px.
  // Mesmas consultas para os três métodos, sem falsificar equivalência de identidade.
  std::vector<Pt> queries;queries.reserve(105);
  for(int i=0;i<105;i++){
   const auto &c=doc.contours[contourD(rng)];
   const auto &node=c.nodes[size_t(rng()%c.nodes.size())];
   double dx=double(int(rng()%13)-6)/(zoom*dpr);
   double dy=double(int(rng()%13)-6)/(zoom*dpr);
   queries.push_back({node.p.x+dx,node.p.y+dy});
  }
  for(Pt q:queries){
   auto a=Clock::now();auto old=bruteHits(segs,q,tol);auto b=Clock::now();
   size_t n=0;auto fast=bvh.hits(q,tol,&n);auto c=Clock::now();
   auto chosen=indexEnabled?bvh.hits(q,tol):bruteHits(segs,q,tol);auto d=Clock::now();
   tb.push_back(ms(a,b));ti.push_back(ms(b,c));ta.push_back(ms(c,d));sumCandidates+=n;
   if(!sameHits(old,fast)||!sameHits(old,chosen))mismatches++;
   totalBrute+=segs.size();totalBvh+=n;checks++;
  }
  if(mismatches)failure+=mismatches;
  auto sum=[](const auto&v){return std::accumulate(v.begin(),v.end(),0.);};
  f<<zoom<<","<<dpr<<","<<queries.size()<<","<<sum(tb)<<","<<sum(ti)<<","<<sum(ta)<<","<<pct(tb,.95)<<","<<pct(ti,.95)<<","<<pct(ta,.95)<<","<<double(sumCandidates)/queries.size()<<","<<mismatches<<","<<(indexEnabled?"bvh":"direct")<<"\n";
  combos++;
 }
 verify(failure==0,"BVH/adaptive identity diverged from brute force");
 std::cout<<"selection combinations="<<combos<<" checks="<<checks<<" mismatches="<<failure<<"\n";
 std::cout<<"brute candidate inspections="<<totalBrute<<" BVH candidates="<<totalBvh<<"\n";
 // Verificação de invalidação: o BVH antigo não pode ser consultado após commit.
 auto mutated=doc;auto originalSegments=segments(mutated);SegmentBVH stale(originalSegments);
 int ntests=0;for(int i=0;i<75;i++){
  auto &c=mutated.contours[size_t(i)%mutated.contours.size()];
  verify(!c.nodes.empty(),"empty contour in fixture");
  auto &node=c.nodes[size_t(i*37)%c.nodes.size()];
  node.p.x+=400+i*2;node.p.y+=200+i;
  // A versão reconstruída deve acertar, a antiga pode não ter informações novas.
  auto updatedSegments=segments(mutated);SegmentBVH rebuilt(updatedSegments);
  Pt probe=node.p;double tolerance=0.04;
  verify(sameHits(rebuilt.hits(probe,tolerance),bruteHits(updatedSegments,probe,tolerance)),"rebuilt BVH did not match updated geometry");
  ntests++;
 }
 std::cout<<"commit/rebuild checks="<<ntests<<"\n";
 // Medida do custo de previsualização: não cria undo nem modifica documento.
 auto longest=std::max_element(doc.contours.begin(),doc.contours.end(),[](auto& a,auto& b){return a.nodes.size()<b.nodes.size();});
 verify(longest!=doc.contours.end(),"no contours");int64_t dragNode=longest->nodes[longest->nodes.size()/2].id;
 Pt base=longest->nodes[longest->nodes.size()/2].p;
 std::vector<double> previewTimes;previewTimes.reserve(240);
 for(int i=0;i<240;i++){
  auto st=Clock::now();auto preview=previewSegments(doc,longest->key,dragNode,{base.x+i*.01,base.y+i*.013});
  auto en=Clock::now();verify(!preview.empty(),"preview empty");previewTimes.push_back(ms(st,en));
 }
 std::ofstream dr(root+"/resultados/arraste_nucleo.csv");dr<<"largest_contour_nodes,positions,p50_ms,p95_ms,max_ms\n"<<longest->nodes.size()<<","<<previewTimes.size()<<","<<pct(previewTimes,.5)<<","<<pct(previewTimes,.95)<<","<<*std::max_element(previewTimes.begin(),previewTimes.end())<<"\n";
 // Ensaio explícito dos comandos de histórico na implementação TESTE_6.
 Editor ed(doc);auto key=longest->key;auto id=dragNode;auto saved=ed.state().doc;
 for(int i=0;i<25;i++){
  auto*cc=get(ed.state().doc,key);verify(cc!=nullptr,"node's contour not found");
  Pt before{};bool found=false;for(auto& n:cc->nodes)if(n.id==id){before=n.p;found=true;break;}
  verify(found,"node not found");
  ed.editNode(key,id,{before.x+0.5,before.y+0.25});
  verify(ed.undo(),"undo failed");verify(sameDoc(ed.state().doc,saved),"undo loss");
  verify(ed.redo(),"redo failed");verify(!sameDoc(ed.state().doc,saved),"redo missing");
  verify(ed.undo(),"undo failed 2");verify(sameDoc(ed.state().doc,saved),"undo roundtrip loss");
 }
 std::cout<<"drag preview frames=240 undo-redo loops=25\n";
 // Comparação estritamente de mutações de propriedades da cena.
 // No TESTE_6, activate() reescreve setPen e setZValue para cada item a cada clique.
 // TESTE_7 altera somente os objetos cujo estado de destaque mudou.
 std::set<int> previous,now;std::map<int,bool> oldStyles,newStyles;
 for(const auto &c:doc.contours){oldStyles[c.key.object]=false;newStyles[c.key.object]=false;}
 long long legacyWrites=0,deltaWrites=0;int changes=0;
 std::vector<int> keys;for(auto&[obj,v]:oldStyles)keys.push_back(obj);
 verify(!keys.empty(),"no objects for style test");
 for(int i=0;i<800;i++){
  if(i%4!=0){now.clear();now.insert(keys[(i*37)%keys.size()]);}
  // Alterna a multi-seleção em parte dos eventos, para cobrir conjuntos variados.
  if(i%13==0)now.insert(keys[(i*41+7)%keys.size()]);
  if(i%17==0&&now.size()>1)now.erase(*now.begin());
  for(auto &kv:oldStyles){kv.second=now.count(kv.first)>0;legacyWrites+=2;}
  std::vector<int> modified;
  std::set_symmetric_difference(previous.begin(),previous.end(),now.begin(),now.end(),std::back_inserter(modified));
  for(int obj:modified){newStyles[obj]=now.count(obj)>0;deltaWrites+=2;}
  verify(oldStyles==newStyles,"delta visual state != full update");
  changes+=(int)modified.size();previous=now;
 }
 std::ofstream h(root+"/resultados/destaque_seletivo.csv");
 h<<"events,objects,full_style_setter_calls,delta_style_setter_calls,changed_object_states,reduction_factor\n";
 h<<800<<","<<oldStyles.size()<<","<<legacyWrites<<","<<deltaWrites<<","<<changes<<","<<double(legacyWrites)/std::max(1ll,deltaWrites)<<"\n";
 std::cout<<"style mutations: legacy="<<legacyWrites<<" delta="<<deltaWrites<<" equivalence=PASS\n";
}
int main(int argc,char**argv){try{
 if(argc!=3)throw std::runtime_error("usage: alencar_perf7 <web_nodes_csv> <output_root>");
 auto doc=readCsv(argv[1]);
 verify(doc.contours.size()==575,"fixture contour count changed");
 verify(countNodes(doc)==13415,"fixture node count changed");
 auto start=Clock::now();bench(doc,argv[2]);
 std::cout<<"elapsed_ms="<<ms(start,Clock::now())<<"\n";
 return 0;
}catch(const std::exception& e){std::cerr<<"FAILED: "<<e.what()<<"\n";return 1;}}
