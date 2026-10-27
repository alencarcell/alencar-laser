// TESTE_12: benchmark de previsualizacao continua sem emendas graficas.
// Compara, frame a frame, o caminho canonico completo com o preview por blocos,
// mas NAO assume que blocos sejam visualmente equivalentes (TESTE_11 reprovou isso na rasterizacao).
#include "geometry.hpp"
#include "render_chunks.hpp"
#include <chrono>
#include <random>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
using namespace alaser;
using Clock=std::chrono::steady_clock;
static double ms(Clock::time_point start){return std::chrono::duration<double,std::milli>(Clock::now()-start).count();}
static void require(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
static bool sameSeg(const Segment&a,const Segment&b){return a.key==b.key&&a.idx==b.idx&&a.a.x==b.a.x&&a.a.y==b.a.y&&a.b.x==b.b.x&&a.b.y==b.b.y;}
static std::vector<Segment> cloneAndBuild(const Contour&c,int64_t id,Pt to){
 Contour copy=c;bool found=false;
 for(auto&n:copy.nodes)if(n.id==id){n.p=to;found=true;break;}
 require(found,"node id not found");Document d;d.contours.push_back(std::move(copy));return segments(d);
}
struct Stats{long long frames=0,edgeChecks=0, mismatches=0;double fullMs=0,chunkMs=0;};
static void experiment(const Contour &c,int iterations,Stats &st){
 if(c.nodes.size()<2)return;
 const size_t j=c.nodes.size()/2;auto original=c.nodes[j].p;
 ChunkedContourPreview chunk(c,c.nodes[j].id,256);
 std::vector<Segment> immutable=cloneAndBuild(c,c.nodes[j].id,original);
 require(immutable.size()==chunk.edgeCount(),"different edge counts");
 for(int i=0;i<iterations;++i){
  double t=i*.137;Pt target={original.x+std::sin(t)*2.,original.y+std::cos(t)*3.};
  auto t0=Clock::now();auto full=cloneAndBuild(c,c.nodes[j].id,target);st.fullMs+=ms(t0);
  t0=Clock::now();auto dirty=chunk.update(target);auto partial=chunk.flatten();st.chunkMs+=ms(t0);
  require(dirty.updatedEdges>=1&&dirty.updatedEdges<=2,"dirty edge count is invalid");
  require(full.size()==partial.size(),"edge count mismatch");
  for(size_t k=0;k<full.size();k++){st.edgeChecks++;if(!sameSeg(full[k],partial[k]))st.mismatches++;}
  st.frames++;
 }
 chunk.cancel();auto restored=chunk.flatten();
 for(size_t k=0;k<immutable.size();k++){st.edgeChecks++;if(!sameSeg(immutable[k],restored[k]))st.mismatches++;}
}
static Contour makeSynthetic(int n,bool closed){
 Contour c;c.key={200000+n,0};c.closed=closed;
 for(int i=0;i<n;i++)c.nodes.push_back({i+1,{double(i)*.001,std::sin(i*.02)*.4}});
 return c;
}
int main(int argc,char**argv){
 try{
  if(argc<3)throw std::runtime_error("usage: test12 fixture.csv result.json");
  auto doc=readCsv(argv[1]);require(doc.contours.size()==575,"expected 575 contours");
  Stats real,synthetic;
  for(const auto&c:doc.contours)experiment(c,12,real);
  experiment(makeSynthetic(20000,false),90,synthetic);
  experiment(makeSynthetic(100000,true),45,synthetic);
  require(real.mismatches==0&&synthetic.mismatches==0,"geometry diverged");
  std::ofstream out(argv[2]);require(bool(out),"cannot write results");out<<std::setprecision(10);
  out<<"{\n\"test\":12,\"mode\":\"continuous_path_is_default\",\n";
  out<<"\"real\":{\"contours\":575,\"frames\":"<<real.frames<<",\"edge_checks\":"<<real.edgeChecks<<",\"mismatches\":"<<real.mismatches<<",\"full_ms\":"<<real.fullMs<<",\"chunk_ms\":"<<real.chunkMs<<"},\n";
  out<<"\"stress\":{\"max_nodes\":100000,\"frames\":"<<synthetic.frames<<",\"edge_checks\":"<<synthetic.edgeChecks<<",\"mismatches\":"<<synthetic.mismatches<<",\"full_ms\":"<<synthetic.fullMs<<",\"chunk_ms\":"<<synthetic.chunkMs<<"},\n";
  out<<"\"qt6_rendered\":false,\"machine_connected\":false,\"homologated_for_windows\":false\n}\n";
  std::cout<<"PASS test12 real frames="<<real.frames<<" stress frames="<<synthetic.frames<<" edges="<<real.edgeChecks+synthetic.edgeChecks<<"\n";
 }catch(const std::exception&e){std::cerr<<"FAIL test12: "<<e.what()<<"\n";return 1;}
}
