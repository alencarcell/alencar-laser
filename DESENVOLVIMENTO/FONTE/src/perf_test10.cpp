// TESTE_10: compara a representacao completa com chunks de arestas identicas.
// Nenhum toolkit grafico, rede ou firmware utilizado.
#include "render_chunks.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <random>
using namespace alaser;
namespace fs=std::filesystem;
using Clock=std::chrono::steady_clock;
static volatile double observed=0;
static void need(bool ok,const std::string& msg){if(!ok)throw std::runtime_error(msg);}
static bool same(const Segment&a,const Segment&b){return a.key==b.key&&a.idx==b.idx&&a.a.x==b.a.x&&a.a.y==b.a.y&&a.b.x==b.b.x&&a.b.y==b.b.y&&a.minx==b.minx&&a.maxx==b.maxx&&a.miny==b.miny&&a.maxy==b.maxy;}
static void equivalence(const Contour& c, size_t idx,size_t chunkSize,int frames,int&cases,int64_t&edgeChecks,int64_t&recreatedEdges){
 auto before= c.nodes[idx].p;
 ChunkedContourPreview p(c,c.nodes[idx].id,chunkSize);
 const auto stable= p.flatten();
 for(int f=0;f<frames;f++){
  Pt q{before.x+2.3*std::sin(.13*f+1.0),before.y+1.7*std::cos(.07*f)};
  auto changed=p.update(q);
  need(changed.updatedEdges>=1&&changed.updatedEdges<=2,"too many affected edges");
  need(changed.dirtyChunkIds.size()<=2,"too many dirty chunks");
  std::set<size_t> touched(changed.dirtyChunkIds.begin(),changed.dirtyChunkIds.end());
  for(auto cidx:changed.dirtyChunkIds)recreatedEdges+=p.chunk(cidx).edges.size();
  auto now=p.flatten();
  need(now.size()==stable.size(),"edge count changed");
  for(size_t j=0;j<now.size();j++){
   Pt a=(j==idx)?q:c.nodes[j].p;
   size_t endpoint=(j+1)%c.nodes.size();
   Pt b=(endpoint==idx)?q:c.nodes[endpoint].p;
   need(now[j].idx==int(j)&&now[j].a.x==a.x&&now[j].a.y==a.y&&now[j].b.x==b.x&&now[j].b.y==b.y,"canonical edge mismatch");
   ++edgeChecks;
   if(!touched.count(j/chunkSize))need(same(now[j],stable[j]),"untouched chunk changed");
  }
  ++cases;
 }
 p.cancel();auto restored=p.flatten();for(size_t i=0;i<stable.size();i++)need(same(stable[i],restored[i]),"cancel left ghost geometry");
 need(c.nodes[idx].p.x==before.x&&c.nodes[idx].p.y==before.y,"source mutated during drag");
}
static double ms(Clock::time_point a, Clock::time_point b){return std::chrono::duration<double,std::milli>(b-a).count();}
int main(int argc,char**argv){try{
 need(argc==3,"usage: test10 fixture.csv results_dir");fs::create_directories(argv[2]);
 Document original=readCsv(argv[1]);need(original.contours.size()==575,"not 575 contours");
 size_t count=0;for(auto&c:original.contours)count+=c.nodes.size();need(count==13415,"unexpected number of nodes");
 int cases=0;int64_t edgeChecks=0,recreatedEdges=0;int contours=0;
 for(const auto&c:original.contours){if(c.nodes.size()<2)continue;std::set<size_t> nodes{0,c.nodes.size()/2,c.nodes.size()-1};
  for(size_t i:nodes)equivalence(c,i,64,3,cases,edgeChecks,recreatedEdges);
  ++contours;
 }
 // Boundary tests: node touching two chunks and last->first edge of closed contours.
 for(bool closed:{false,true})for(int size:{2,3,64,65,256,257,513}){
  if(closed&&size<3)continue;
  Contour c{{5000+size+(closed?1000:0),0},{},closed};
  for(int i=0;i<size;i++)c.nodes.push_back({int64_t(i+1),{double(i)*.12,std::sin(i*.2)}});
  std::set<size_t> indices{0,size_t(size-1),size_t(size/2),size_t(std::min(size-1,64)),size_t(std::min(size-1,256))};
  for(int cs:{1,7,64,128,256,512})for(size_t index:indices)equivalence(c,index,size_t(cs),5,cases,edgeChecks,recreatedEdges);
 }
 // Performance over long contours: baseline full per-frame reconstruction against
 // incremental chunk regeneration, both produce exact same edge geometry.
 std::ofstream csv(fs::path(argv[2])/"comparativo_blocos_TESTE_10.csv");csv<<"nodes,closed,chunk_edges,moves,initial_chunks,baseline_full_ms,partial_changed_ms,baseline_edges_processed,partial_edges_processed,partial_percent,comment\n";
 for(int n:{20000,100000})for(bool closed:{false,true}){
  Contour c{{7100+n+(closed?1:0),0},{},closed};c.nodes.reserve(n);
  for(int i=0;i<n;i++)c.nodes.push_back({int64_t(i+1),{double(i)*.04,5*std::sin(i*.01)}});
  size_t ni=size_t(n/2);std::vector<Pt> moves;for(int f=0;f<750;f++)moves.push_back({c.nodes[ni].p.x+std::sin(f*.08)*2,c.nodes[ni].p.y+std::cos(f*.08)*4});
  double fullElapsed=0;uint64_t fullEdges=0;
  {auto t0=Clock::now();double checksum=0;
   std::vector<Pt> path;path.reserve(c.nodes.size()+1);
   for(Pt q:moves){path.clear();for(size_t i=0;i<c.nodes.size();i++)path.push_back(i==ni?q:c.nodes[i].p);
    if(c.closed)path.push_back(path.front());
    fullEdges+=path.size()-1;checksum+=path[path.size()/2].x;
   }
   fullElapsed=ms(t0,Clock::now());observed=observed+checksum;
  }
  for(size_t cs:{32,128,256,512,1024}){
   ChunkedContourPreview chunk(c,c.nodes[ni].id,cs);
   auto t0=Clock::now();uint64_t edgesRegen=0;double checksum=0;
   std::vector<Pt> path;path.reserve(cs+1);
   for(Pt q:moves){auto changed=chunk.update(q);for(auto id:changed.dirtyChunkIds){auto &ch=chunk.chunk(id);path.clear();
     path.push_back(ch.edges.front().a);for(const auto&e:ch.edges)path.push_back(e.b);
     edgesRegen+=ch.edges.size();checksum+=path[path.size()/2].x;
   }}
   auto elapsed=ms(t0,Clock::now());observed=observed+checksum;
   // The final updated chunks MUST describe the same geometry as a full build.
   Contour final=c;final.nodes[ni].p=moves.back();auto reference=segments(Document{{final},1});auto all=chunk.flatten();
   need(all.size()==reference.size(),"stress size mismatch");
   for(size_t i=0;i<all.size();i++)need(same(all[i],reference[i]),"stress geometry mismatch");
   auto before=chunk.flatten();chunk.cancel();auto cancelled=chunk.flatten();need(before.size()==cancelled.size(),"cancel mismatch");
   csv<<n<<","<<int(closed)<<","<<cs<<","<<moves.size()<<","<<chunk.chunkCount()<<","<<std::fixed<<std::setprecision(6)<<fullElapsed<<","<<elapsed<<","<<fullEdges<<","<<edgesRegen<<","<<100.0*edgesRegen/fullEdges<<",not_Qt_render_or_fps\n";
  }
 }
 int rejected=0;
 try{Contour c{{99,0},{{1,{0,0}},{2,{1,0}}},false};ChunkedContourPreview p(c,9);(void)p;}catch(const std::runtime_error&){++rejected;}
 try{Contour c{{99,0},{{1,{0,0}}},false};ChunkedContourPreview p(c,1);(void)p;}catch(const std::runtime_error&){++rejected;}
 try{Contour c{{99,0},{{1,{0,0}},{2,{1,0}}},false};ChunkedContourPreview p(c,1);p.update({NAN,4});}catch(const std::runtime_error&){++rejected;}
 need(rejected==3,"adversarial tests failed");
 std::ofstream json(fs::path(argv[2])/"resultado_TESTE_10.json");
 json<<"{\n  \"status\":\"PASS\",\n  \"runtime\":\"C++20 sem Qt\",\n  \"objects\":575,\n  \"nodes\":13415,\n  \"real_contours\":"<<contours<<",\n  \"drag_frames_verified\":"<<cases<<",\n  \"edge_checks\":"<<edgeChecks<<",\n  \"dirty_edges_equivalent\":true,\n  \"safe_cancel\":true,\n  \"adversarial_cases_rejected\":3,\n  \"qt_visual_homologated\":false,\n  \"hardware_connected\":false\n}\n";
 std::cout<<"TESTE_10 PASS | "<<contours<<" contornos reais, "<<cases<<" quadros de arraste, "<<edgeChecks<<" arestas comparadas\n";
 return 0;
}catch(const std::exception&e){std::cerr<<"TESTE_10 FAIL: "<<e.what()<<"\n";return 1;}}
