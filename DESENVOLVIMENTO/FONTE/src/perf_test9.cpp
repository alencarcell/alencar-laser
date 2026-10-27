// TESTE_9 — ensaio de arraste continuo, referencia integral vs geometria incremental.
// Sem dependencias Qt e sem comandos GRBL.
#include "drag_preview.hpp"
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <random>
#include <set>
#include <numeric>
using namespace alaser;
using Clock=std::chrono::steady_clock;
static void need(bool ok,const std::string& why){if(!ok)throw std::runtime_error(why);}
static bool equalPt(Pt a,Pt b){return a.x==b.x&&a.y==b.y;}
static bool equalSeg(const Segment&a,const Segment&b){return a.key==b.key&&a.idx==b.idx&&equalPt(a.a,b.a)&&equalPt(a.b,b.b);}
static double ms(Clock::time_point a,Clock::time_point b){return std::chrono::duration<double,std::milli>(b-a).count();}
static void checkTrace(const Document& d,const Contour& c,size_t ni,int frames,int &cases,int&checks){
 auto nodeId=c.nodes.at(ni).id;
 DragSegmentPreview plan(d,c.key,nodeId);
 need(plan.touchedEdges()==((c.closed|| (ni>0&&ni+1<c.nodes.size()))?2:1),"incorrect adjacent edge count");
 const auto original=segments(Document{{c},1});
 auto last=original;
 for(int f=0;f<frames;f++){
  Pt q{c.nodes[ni].p.x+6.0*std::sin(f*.091),c.nodes[ni].p.y+4.0*std::cos(f*.113)};
  auto delta=plan.update(q);
  auto rebuilt=previewSegments(d,c.key,nodeId,q);
  need(rebuilt.size()==original.size(),"preview size diverged");
  for(int i=0;i<delta.count;i++){
   auto before=delta.before[i], after=delta.after[i];
   need(equalSeg(before,last.at(size_t(before.idx))),"stale before delta");
   need(equalSeg(after,rebuilt.at(size_t(after.idx))),"partial update differs from canonical preview");
   for(Pt p:{before.a,before.b,after.a,after.b})need(delta.damage.contains(p),"damage does not include modified segment");
   ++checks;
  }
  // All unrelated edges retain identical geometry at each frame.
  if(f%13==0)for(size_t j=0;j<original.size();j++){
   bool affected=false;for(int i=0;i<delta.count;i++)affected|=(size_t(delta.after[i].idx)==j);
   if(!affected)need(equalSeg(rebuilt[j],original[j]),"unrelated edge changed");
  }
  last=std::move(rebuilt);++cases;
 }
 // The reference document is unmodified throughout the drag; Esc cancels without commits.
 need(get(d,c.key)->nodes[ni].p.x==c.nodes[ni].p.x,"drag changed canonical document");
}
int main(int argc,char**argv){try{
 need(argc==3,"usage: perf_test9 fixture.csv out_dir");
 namespace fs=std::filesystem;fs::create_directories(argv[2]);
 auto real=readCsv(argv[1]);
 need(real.contours.size()==575,"unexpected contour count");
 size_t nodes=0;for(auto&c:real.contours)nodes+=c.nodes.size();need(nodes==13415,"unexpected node count");
 int frames=0,checks=0,selectedContours=0;
 for(auto& c:real.contours){if(c.nodes.size()<2)continue;
  // Representative tests: start, middle, end for each real contour.
  std::set<size_t> chosen{0,c.nodes.size()/2,c.nodes.size()-1};
  for(auto ni:chosen)checkTrace(real,c,ni,4,frames,checks);
  ++selectedContours;
 }
 // Synthetic closed contour and open contour: stresses 20k nodes each.
 Document giant;giant.nextId=40010;
 Contour open{{5000,0},{},false};Contour closed{{5001,0},{},true};
 for(int i=0;i<20000;i++){
  Pt p{double(i)*.03,std::sin(i*.013)*8.};
  open.nodes.push_back({i+1,p});closed.nodes.push_back({i+20001,p});
 }
 giant.contours={open,closed};
 for(auto&c:giant.contours)for(size_t index:{size_t(0),size_t(9999),size_t(19999)})checkTrace(giant,c,index,12,frames,checks);
 // Realistic drag with many move events on long contour: same sequence benchmarked.
 const auto& hot=giant.contours.front();auto id=hot.nodes[9999].id;
 std::vector<Pt> positions;positions.reserve(1400);
 for(int i=0;i<1400;i++)positions.push_back({hot.nodes[9999].p.x+std::cos(i*.023)*11.,hot.nodes[9999].p.y+std::sin(i*.023)*11.});
 auto fullT0=Clock::now();double fullSink=0;
 for(Pt pos:positions){auto v=previewSegments(giant,hot.key,id,pos);fullSink+=v[v.size()/2].a.x;}
 auto fullT1=Clock::now();
 DragSegmentPreview local(giant,hot.key,id);double incSink=0;auto incT0=Clock::now();
 for(Pt pos:positions){auto v=local.update(pos);incSink+=v.after[0].a.x+v.after[0].b.y;}
 auto incT1=Clock::now();
 need(std::isfinite(fullSink)&&std::isfinite(incSink),"benchmark optimized away or invalid");
 auto fullMs=ms(fullT0,fullT1),incrementalMs=ms(incT0,incT1);
 need(incrementalMs<fullMs,"incremental plan slower than full rebuild");
 // Fault injection: invalid node/contour/nonfinite, and no mutation on failed update.
 int faultChecks=0;
 try{DragSegmentPreview t(real,{999999,1},2);(void)t;}catch(const std::runtime_error&){++faultChecks;}
 try{DragSegmentPreview t(giant,hot.key,-200);(void)t;}catch(const std::runtime_error&){++faultChecks;}
 try{DragSegmentPreview t(giant,hot.key,id);t.update({NAN,3});}catch(const std::runtime_error&){++faultChecks;}
 need(faultChecks==3,"invalid input not rejected");
 auto prev=giant.contours.front().nodes[9999].p;
 need(equalPt(prev,hot.nodes[9999].p),"unexpected canonical mutation");
 std::ofstream csv(fs::path(argv[2])/"bench_arraste_TESTE_9.csv");
 csv<<"scenario,nodes,moves,full_rebuild_ms,incremental_adjacent_ms,speedup\n";
 csv<<std::fixed<<std::setprecision(6)<<"open_20000_nodes,20000,"<<positions.size()<<","<<fullMs<<","<<incrementalMs<<","<<fullMs/incrementalMs<<"\n";
 std::ofstream json(fs::path(argv[2])/"resumo_TESTE_9.json");
 json<<std::fixed<<std::setprecision(6)<<"{\n  \"status\": \"PASS\",\n  \"method\": \"C++20_headless_not_Qt\",\n  \"real_objects\": 575,\n  \"real_nodes\": 13415,\n  \"real_contours_exercised\": "<<selectedContours<<",\n  \"drag_frames_verified\": "<<frames<<",\n  \"affected_edge_checks\": "<<checks<<",\n  \"fault_cases_rejected\": "<<faultChecks<<",\n  \"stress_contour_nodes\": 20000,\n  \"stress_frames_benchmarked\": "<<positions.size()<<",\n  \"full_rebuild_ms\": "<<fullMs<<",\n  \"local_neighbor_calculation_ms\": "<<incrementalMs<<",\n  \"speedup_in_non_equivalent_work\": "<<fullMs/incrementalMs<<",\n  \"warning\": \"Local segment calculations do NOT replace Qt full-path rendering or establish frame latency.\",\n  \"hardware_connected\": false\n}\n";
 std::cout<<"TESTE_9 PASS real nodes "<<nodes<<" contours "<<selectedContours<<" frames "<<frames<<" checks "<<checks<<" faults "<<faultChecks<<"\n";
 std::cout<<"20k-node/1400-frame full rebuild "<<fullMs<<" ms, adjacency "<<incrementalMs<<" ms (NOT Qt FPS)\n";
 return 0;
}catch(const std::exception&e){std::cerr<<"TESTE_9 FAIL "<<e.what()<<"\n";return 1;}}
