// TESTE_11 - verifica emendas geometricas e regioes sujas em arraste por blocos.
// Nao e renderizacao Qt: garante pontos/arestas e old/new dirty bounding regions.
#include "render_chunks.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
using namespace alaser;
namespace fs=std::filesystem;
struct Rect {double left=0,top=0,right=0,bottom=0;};
static void ensure(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static Rect bounds(const EdgeChunk& chunk,double scale,double dpr,double margin){
 ensure(!chunk.edges.empty(),"empty chunk");
 Rect r{1e300,1e300,-1e300,-1e300};
 for(const auto& e:chunk.edges)for(auto p:{e.a,e.b}){
  double x=p.x*scale*dpr,y=p.y*scale*dpr;
  r.left=std::min(r.left,x);r.right=std::max(r.right,x);
  r.top=std::min(r.top,y);r.bottom=std::max(r.bottom,y);
 }
 r.left-=margin;r.right+=margin;r.top-=margin;r.bottom+=margin;
 return r;
}
static Rect unite(Rect a,Rect b){return{std::min(a.left,b.left),std::min(a.top,b.top),std::max(a.right,b.right),std::max(a.bottom,b.bottom)};}
static bool contains(Rect r,Pt p,double scale,double dpr){double x=p.x*scale*dpr,y=p.y*scale*dpr;
 return x>=r.left-1e-8&&x<=r.right+1e-8&&y>=r.top-1e-8&&y<=r.bottom+1e-8;
}
static bool equal(const Segment&a,const Segment&b){return a.key==b.key&&a.idx==b.idx&&a.a.x==b.a.x&&a.a.y==b.a.y&&a.b.x==b.b.x&&a.b.y==b.b.y;}
static void runContour(const Contour&c,size_t idx,size_t chunkSize,int frames,
 long long&framesOK,long long&joinsOK,long long&dirtyChecks,long long& unchangedOK){
 ChunkedContourPreview p(c,c.nodes[idx].id,chunkSize);
 auto previous=p.flatten();auto base=c.nodes[idx].p;
 for(int f=0;f<frames;f++){
  Pt moved{base.x+std::sin(.17*f+.5)*3.4,base.y+std::cos(.21*f-.3)*2.1};
  auto oldChunks=p.chunks();auto dirty=p.update(moved);
  ensure(dirty.dirtyChunkIds.size()<=2,"more than 2 affected chunks");
  for(double zoom:{.05,.75,6.,80.,1200.})for(double dpr:{1.,1.25,1.5,2.}){
   Rect roi;bool first=true;
   for(auto id:dirty.dirtyChunkIds){Rect a=bounds(oldChunks.at(id),zoom,dpr,2.5);
    Rect b=bounds(p.chunk(id),zoom,dpr,2.5);
    if(first){roi=unite(a,b);first=false;}else roi=unite(roi,unite(a,b));
    for(const auto& edge:oldChunks.at(id).edges){ensure(contains(roi,edge.a,zoom,dpr)&&contains(roi,edge.b,zoom,dpr),"old edge excluded from invalidation");dirtyChecks+=2;}
    for(const auto& edge:p.chunk(id).edges){ensure(contains(roi,edge.a,zoom,dpr)&&contains(roi,edge.b,zoom,dpr),"new edge excluded from invalidation");dirtyChecks+=2;}
   }
  }
  auto now=p.flatten();ensure(now.size()==previous.size(),"edge count changed");
  for(size_t i=0;i<now.size();i++){
   auto a=now[i].a,b=now[i].b;
   ensure(a.x==((i==idx)?moved.x:c.nodes[i].p.x)&&a.y==((i==idx)?moved.y:c.nodes[i].p.y),"start of edge differs");
   size_t next=(i+1)%c.nodes.size();
   ensure(b.x==((next==idx)?moved.x:c.nodes[next].p.x)&&b.y==((next==idx)?moved.y:c.nodes[next].p.y),"end of edge differs");
   if(!dirty.dirtyChunkIds.empty()&&!std::binary_search(dirty.dirtyChunkIds.begin(),dirty.dirtyChunkIds.end(),i/chunkSize)){
    ensure(equal(now[i],previous[i]),"unmodified chunk changed");++unchangedOK;
   }
  }
  for(size_t ch=0;ch<p.chunkCount();ch++){
   if(ch+1>=p.chunkCount())break;
   const auto& left=p.chunk(ch).edges.back();const auto& right=p.chunk(ch+1).edges.front();
   ensure(left.b.x==right.a.x&&left.b.y==right.a.y,"GAP in chunk seam");++joinsOK;
  }
  ++framesOK;previous=std::move(now);
 }
 p.cancel();auto restored=p.flatten();for(size_t i=0;i<restored.size();i++){
  auto a=c.nodes[i].p,b=c.nodes[(i+1)%c.nodes.size()].p;
  ensure(restored[i].a.x==a.x&&restored[i].a.y==a.y&&restored[i].b.x==b.x&&restored[i].b.y==b.y,"cancel creates ghost");
 }
}
int main(int argc,char**argv){try{
 ensure(argc==3,"usage: core11 fixture.csv output.json");auto d=readCsv(argv[1]);ensure(d.contours.size()==575,"expected 575 contours");
 long long frames=0,joins=0,dirty=0,unchanged=0;size_t nodes=0;
 auto t0=std::chrono::steady_clock::now();
 for(const auto&c:d.contours){nodes+=c.nodes.size();if(c.nodes.size()<2)continue;
  size_t n=c.nodes.size();for(size_t idx:{size_t(0),n/2,n-1})runContour(c,idx,64,2,frames,joins,dirty,unchanged);
 }
 ensure(nodes==13415,"fixture node count changed");
 for(bool closed:{false,true})for(int n:{3,64,65,256,257,513,1024,20000}){
  Contour c{{n+(closed?100000:0),0},{},closed};for(int i=0;i<n;i++)c.nodes.push_back({i+1,{std::sin(i*.02)*20,i*.125}});
  for(size_t k:{size_t(0),size_t(1),size_t(n/2),size_t(n-1)})
   for(size_t size:{size_t(1),size_t(32),size_t(64),size_t(256),size_t(512)})runContour(c,k,size,4,frames,joins,dirty,unchanged);
 }
 double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t0).count();
 fs::create_directories(fs::path(argv[2]).parent_path());std::ofstream out(argv[2]);
 out<<"{\n\"status\":\"PASS\",\"engine\":\"C++20 no Qt\",\"objects\":575,\"nodes\":13415,\"frames\":"<<frames<<",\"edge_join_checks\":"<<joins<<",\"old_new_dirty_endpoint_checks\":"<<dirty<<",\"unmodified_edges_unchanged\":"<<unchanged<<",\"elapsed_ms\":"<<std::fixed<<std::setprecision(3)<<ms<<",\"qt_visual_validated\":false,\"hardware_connected\":false\n}\n";
 std::cout<<"TESTE_11 CORE PASS | frames="<<frames<<" joins="<<joins<<" dirty="<<dirty<<" unchanged="<<unchanged<<" ms="<<ms<<"\n";
 return 0;
 }catch(const std::exception&e){std::cerr<<"TESTE_11 FAIL: "<<e.what()<<"\n";return 1;}}
