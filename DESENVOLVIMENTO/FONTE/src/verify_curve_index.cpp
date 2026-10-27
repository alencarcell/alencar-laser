#include "bezier_interaction.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
using namespace alaser;
static void check(bool value,const char* reason){if(!value)throw std::runtime_error(reason);}
static void compare(const std::vector<CurveHit>& a,const std::vector<CurveHit>& b){
 check(a.size()==b.size(),"hit count mismatch");
 for(size_t i=0;i<a.size();i++)check(a[i].key==b[i].key&&a[i].edge==b[i].edge&&a[i].distancePx==b[i].distancePx&&a[i].t==b[i].t&&a[i].point.x==b[i].point.x&&a[i].point.y==b[i].point.y,"indexed identity or geometry mismatch");
}
int main(int argc,char**argv){try{
 check(argc==4,"usage: index_check real_nodes.csv native_curves.csv report.json");
 auto source=readCsv(argv[1]);CurveDocument dense;dense.nextId=source.nextId;size_t nodes=0,edges=0;
 for(const auto& c:source.contours){CurveContour cc;cc.key=c.key;cc.closed=c.closed;for(auto n:c.nodes){cc.nodes.push_back({n.id,n.p,n.p,n.p});nodes++;}cc.edgeKind.resize(edgeCount(cc),0);edges+=edgeCount(cc);dense.contours.push_back(cc);}
 check(nodes==13415,"wrong real fixture");validateCurve(dense);
 auto native=loadLightBurnCurves(argv[2]);
 auto clock=std::chrono::steady_clock::now;auto start=clock();CurveSpatialIndex index(dense);
 double buildMs=std::chrono::duration<double,std::milli>(clock()-start).count();
 size_t queries=0,candidates=0;double indexedMs=0,bruteMs=0;
 for(double zoom:{.75,6.,80.,1200.})for(double dpr:{1.,2.})for(size_t i=0;i<500;i++){
  const auto& c=dense.contours[(i*37)%dense.contours.size()];Pt point=c.nodes[(i*13)%c.nodes.size()].anchor;
  point.x+=(int(i%9)-4)/(zoom*dpr);point.y+=(int(i%7)-3)/(zoom*dpr);
  size_t count=0;start=clock();auto actual=index.hits(point,zoom,dpr,6,&count);indexedMs+=std::chrono::duration<double,std::milli>(clock()-start).count();
  start=clock();auto expected=pickCurveEdges(dense,point,zoom,dpr);bruteMs+=std::chrono::duration<double,std::milli>(clock()-start).count();compare(actual,expected);queries++;candidates+=count;
 }
 CurveSpatialIndex nativeIndex(native);size_t nativeQueries=0;
 for(const auto& c:native.contours)for(size_t e=0;e<edgeCount(c);e++)for(double t:{0.,.25,.5,.75,1.})for(double z:{6.,80.,1200.}){
  auto p=cubicPoint(edgeAt(c,e),t);compare(nativeIndex.hits(p,z,2),pickCurveEdges(native,p,z,2));nativeQueries++;
 }
 // Force cache creation, then move a node far outside its former bounding box.
 CurveDragSession session(native);const auto& c=native.contours.front();auto n=c.nodes[1];session.select(c.key);
 auto verify=[&](Pt p){compare(session.pickEdges(p,80,2),pickCurveEdges(session.committed(),p,80,2));};
 verify(n.anchor);CurveHit h;h.key=c.key;h.nodeId=n.id;h.part=CurvePart::Anchor;
 check(session.start(h),"drag failed");Pt moved=n.anchor+Pt{1000,1000};session.update(moved);session.commit();verify(moved);verify(n.anchor);
 check(session.undo(),"undo failed");verify(moved);verify(n.anchor);check(session.redo(),"redo failed");verify(moved);verify(n.anchor);
 check(session.start(h),"cancel drag failed");session.update(n.anchor);session.cancel();verify(moved);
 CurveDocument empty;CurveSpatialIndex emptyIndex(empty);check(emptyIndex.hits({0,0},1,1).empty(),"empty index failed");
 std::ofstream report(argv[3]);check(bool(report),"cannot write report");
 report<<"{\n\"status\":\"PASS\",\n\"real_nodes\":"<<nodes<<",\n\"edges\":"<<edges<<",\n\"queries\":"<<queries<<",\n\"native_curve_queries\":"<<nativeQueries<<",\n\"candidate_total\":"<<candidates<<",\n\"build_ms\":"<<buildMs<<",\n\"indexed_query_ms_total\":"<<indexedMs<<",\n\"brute_query_ms_total\":"<<bruteMs<<",\n\"commit_undo_redo_cancel_cache\":true,\n\"identity_mismatches\":0\n}\n";
 std::cout<<"PASS indexed selection: "<<queries<<" dense queries + "<<nativeQueries<<" cubic queries; zero mismatches\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
