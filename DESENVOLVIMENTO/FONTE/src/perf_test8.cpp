// Alencar Laser TESTE_8 — laboratorio C++ de cache de selecao e invalidacao
// Sem Qt, sem GRBL, sem IO de maquina.
#include "editor_tools.hpp"
#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <array>
#include <cmath>
#include <sstream>
using namespace alaser;
using Clock=std::chrono::steady_clock;
static void must(bool v,const char *s){if(!v)throw std::runtime_error(s);}
static double elapsed(Clock::time_point a,Clock::time_point b){return std::chrono::duration<double,std::milli>(b-a).count();}
static double percentile(std::vector<double> a,double p){std::sort(a.begin(),a.end());return a[std::min(a.size()-1,size_t(std::ceil(p*a.size())-1))];}
static std::vector<Hit> bruteEligible(const Editor& e,const std::vector<Segment>& s,Pt p,double tol){
 auto hits=bruteHits(s,p,tol);
 hits.erase(std::remove_if(hits.begin(),hits.end(),[&](auto h){return !editable(e.state(),h.key.object);}),hits.end());return hits;
}
int main(int argc,char** argv){try{
 must(argc==3,"usage: perf_test8 <fixture.csv> <output_directory>");
 namespace fs=std::filesystem;
 fs::create_directories(argv[2]);
 const auto doc=readCsv(argv[1]);must(doc.contours.size()==575&&countNodes(doc)==13415,"fixture changed");
 Editor editor(doc);CachedHits cache;
 std::mt19937_64 rng(424242);
 std::uniform_int_distribution<size_t> pick(0,doc.contours.size()-1);
 const auto baseline=segments(doc);
 std::ofstream csv(fs::path(argv[2])/"selecao_teste8.csv");must(bool(csv),"cannot create CSV");
 csv<<"zoom_px_mm,dpr,queries,cached_total_ms,reference_total_ms,cached_p95_ms,reference_p95_ms,mismatches,cache_rebuilds\n";
 int queries=0,mismatches=0;
 for(double zoom:{.05,.75,6.,80.,1200.}) for(double dpr:{1.,1.25,1.5,2.}){
  std::vector<double> fastTimes,referenceTimes;int mismatchesAtScale=0;int startRebuild=int(cache.rebuilds());
  for(int j=0;j<130;j++){
   const auto &c=doc.contours[pick(rng)];auto const& node=c.nodes[rng()%c.nodes.size()];
   Pt p{node.p.x+(double(int(rng()%17)-8)/(zoom*dpr)),node.p.y+(double(int(rng()%17)-8)/(zoom*dpr))};
   auto st=Clock::now();auto h=cache.query(editor,p,6.,zoom,dpr);auto mt=Clock::now();
   auto ref=bruteEligible(editor,baseline,p,6./(zoom*dpr));auto en=Clock::now();
   if(!sameHits(h,ref))++mismatchesAtScale;
   ++queries;fastTimes.push_back(elapsed(st,mt));referenceTimes.push_back(elapsed(mt,en));
  }
  auto sum=[](auto v){return std::accumulate(v.begin(),v.end(),0.);};
  csv<<zoom<<","<<dpr<<","<<fastTimes.size()<<","<<sum(fastTimes)<<","<<sum(referenceTimes)<<","<<percentile(fastTimes,.95)<<","<<percentile(referenceTimes,.95)<<","<<mismatchesAtScale<<","<<cache.rebuilds()-startRebuild<<"\n";
  mismatches+=mismatchesAtScale;
 }
 must(queries==2600&&mismatches==0,"selected identity diverged");
 must(cache.rebuilds()==1,"cache was rebuilt on zoom or each click");
 std::cout<<"selection: "<<queries<<" queries, mismatches "<<mismatches<<", cache rebuilds "<<cache.rebuilds()<<"\n";
 // Em camadas ocultas/bloqueadas/transferidas, a geometria e o BVH permanecem identicos,
 // mas a elegibilidade dos objetos muda imediatamente em todos os zooms.
 auto probeContour=doc.contours.at(14);must(probeContour.nodes.size()>=2,"fixture contour empty");
 int obj=probeContour.key.object;auto & node=probeContour.nodes[0];Pt q=node.p;
 editor.addLayer(7,"Teste 8");editor.moveToLayer(obj,7);
 size_t before=cache.rebuilds();
 cache.query(editor,q,6.,80.);must(cache.rebuilds()==before,"metadata invalidated geometry index");
 editor.setLayerFlags(7,false,false);auto hidden=cache.query(editor,q,6.,80.);for(auto h:hidden)must(h.key.object!=obj,"hidden object returned");
 editor.setLayerFlags(7,true,true);auto locked=cache.query(editor,q,6.,80.);for(auto h:locked)must(h.key.object!=obj,"locked object returned");
 editor.setLayerFlags(7,true,false);auto unlocked=cache.query(editor,q,6.,80.);
 bool found=false;for(auto h:unlocked)if(h.key.object==obj)found=true;
 must(found,"editable object missing after unhide/unlock");
 editor.group({obj,doc.contours.at(40).key.object});
 cache.query(editor,q,6.,80.);
 must(cache.rebuilds()==before,"grouping should not rebuild geometry index");
 std::cout<<"metadata only: hidden/locked/group checked without rebuild\n";
 // Geometria editada: BVH precisa invalidar uma vez, nao a cada selecao.
 Pt edited{q.x+270.,q.y+310.};editor.editNode(probeContour.key,node.id,edited);
 auto hit=cache.query(editor,edited,6.,80.);(void)hit;
 must(cache.rebuilds()==before+1,"editing node did not rebuild index");
 auto current=segments(editor.state().doc);
 must(sameHits(cache.query(editor,edited,6.,80.),bruteEligible(editor,current,edited,6./80.)),"stale index after editing");
 must(cache.rebuilds()==before+1,"excessive post-edit rebuild");
 must(editor.undo(),"undo failed");
 auto undoHits=cache.query(editor,q,6.,80.);(void)undoHits;
 must(cache.rebuilds()==before+2,"undo failed to invalidate cache");
 must(editor.redo(),"redo failed");
 auto redoHits=cache.query(editor,edited,6.,80.);(void)redoHits;
 must(cache.rebuilds()==before+3,"redo failed to invalidate cache");
 std::cout<<"geometry commits, undo, redo: 3 invalidations only\n";
 // Linhas coincidentes devem permanecer IDs distintos e alternaveis.
 Editor overlap(Document{});
 int idA=overlap.line({0,0},{30,0});
 int idB=overlap.line({0,0},{30,0});
 CachedHits overlapPicker;
 auto coincident=overlapPicker.query(overlap,{15,0},6.,200.);
 must(coincident.size()==2&&coincident[0].key.object!=coincident[1].key.object,"overlapping contours collapsed into one");
 for(double z:{.05,.75,6.,80.,1200.})for(double d:{1.,1.25,1.5,2.}){
  auto hs=overlapPicker.query(overlap,{15,0},6.,z,d);
  must(hs.size()==2&&hs[0].key.object==idA&&hs[1].key.object==idB,"overlap order not stable");
 }
 std::cout<<"overlap: 20 zoom-DPR combinations preserve separate keys\n";
 // Rejeitar escalas invalidas: evita selecao silenciosa sobre areas incorretas.
 int badInputs=0;
 for(std::array<double,3> invalid:std::vector<std::array<double,3>>{{0,6,1},{6,0,1},{6,6,0},{6,NAN,1},{6,6,INFINITY}}){
  try{overlapPicker.query(overlap,{10,0},invalid[0],invalid[1],invalid[2]);}
  catch(const std::runtime_error&){++badInputs;}
 }
 must(badInputs==5,"invalid view parameters accepted");
 // Abrir outro arquivo reinicia geometryGeneration para zero: invalidacao explicita obrigatoria.
 // Sem ela, o BVH poderia apontar para o documento anterior mesmo com a mesma revisao.
 Editor reopened(doc);
 CachedHits filePicker;
 auto beforeOpen=filePicker.query(reopened,q,6.,80.);(void)beforeOpen;
 auto countBeforeFile=filePicker.rebuilds();
 auto alternate=doc;
 alternate.contours[14].nodes[0].p={q.x+460,q.y+365};
 Editor newFile(alternate);reopened=std::move(newFile); // ambas revisoes podem ser 0
 filePicker.invalidate();
 auto afterOpen=filePicker.query(reopened,alternate.contours[14].nodes[0].p,6.,80.);
 auto openedSegments=segments(reopened.state().doc);
 must(sameHits(afterOpen,bruteEligible(reopened,openedSegments,alternate.contours[14].nodes[0].p,6./80.)),"stale cache after opening different document");
 must(filePicker.rebuilds()==countBeforeFile+1,"open did not rebuild the index");
 std::cout<<"file replacement: cache explicitly invalidated and regenerated\n";
 // Baseline TESTE_7 and optimized linear scanner should agree on all queries.
 int refChecks=0;
 for(int i=0;i<400;i++){
  auto& c=doc.contours[pick(rng)];auto n=c.nodes[rng()%c.nodes.size()];
  Pt p{n.p.x+double(int(rng()%13)-6),n.p.y+double(int(rng()%13)-6)};
  must(sameHits(bruteHits(baseline,p,6.),bruteHitsNoAlloc(baseline,p,6.)),"allocation-free scanner changed identity");
  ++refChecks;
 }
 // Atualizacao de estilos: a maquina de estado nao deve modificar outros objetos.
 std::map<int,bool> full,diff;for(auto const& c:doc.contours){full[c.key.object]=false;diff[c.key.object]=false;}
 std::set<int> prev,selection;int64_t oldSetterCalls=0,newSetterCalls=0;
 for(int i=0;i<800;++i){selection.clear();if(i%5!=0)selection.insert(doc.contours[(i*31)%doc.contours.size()].key.object);
  if(i%23==0)selection.insert(doc.contours[(i*47)%doc.contours.size()].key.object);
  for(auto&[k,v]:full){v=selection.contains(k);oldSetterCalls+=2;}
  std::vector<int> changed;std::set_symmetric_difference(prev.begin(),prev.end(),selection.begin(),selection.end(),std::back_inserter(changed));
  for(int id:changed){diff[id]=selection.contains(id);newSetterCalls+=2;}
  must(full==diff,"style state differs after delta updates");prev=selection;
 }
 must(oldSetterCalls>newSetterCalls,"no advantage from delta updates");
 std::ofstream meta(fs::path(argv[2])/"resumo_teste8.json");
 meta<<"{\n  \"status\": \"PASS\",\n  \"source\": \"core_C++20_not_Qt\",\n  \"objects\": 575,\n  \"nodes\": 13415,\n  \"queries\": "<<queries<<",\n  \"selection_mismatches\": "<<mismatches<<",\n  \"rebuilds_after_undo_redo\": "<<cache.rebuilds()<<",\n  \"overlap_view_combinations\": 20,\n  \"legacy_style_calls\": "<<oldSetterCalls<<",\n  \"delta_style_calls\": "<<newSetterCalls<<",\n  \"linear_scanner_parity\": "<<refChecks<<",\n  \"hardware_connected\": false\n}\n";
 std::cout<<"style old="<<oldSetterCalls<<" new="<<newSetterCalls<<" scanner parity="<<refChecks<<"\n";
 return 0;
}catch(const std::exception& e){std::cerr<<"TESTE_8 REPROVADO: "<<e.what()<<"\n";return 1;}}
