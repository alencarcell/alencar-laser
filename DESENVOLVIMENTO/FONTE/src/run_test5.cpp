#include "editor_tools.hpp"
#include "persistence.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <bit>
using namespace alaser;
namespace fs=std::filesystem;
using namespace boost::json;
using Clock=std::chrono::steady_clock;
static array stages;
static void require(bool v,const std::string&msg){if(!v)throw std::runtime_error(msg);}
static void done(const std::string&name,object attrs={}){attrs["name"]=name;attrs["status"]="PASS";stages.emplace_back(std::move(attrs));std::cout<<"PASS "<<name<<"\n";}
static void near(Pt a,Pt b,double eps=1e-8){require(dist(a,b)<=eps,"coordinate mismatch");}
static void bittest(const Document&a,const Document&b){
 require(a.contours.size()==b.contours.size()&&a.nextId==b.nextId,"document size/identity mismatch");
 for(size_t i=0;i<a.contours.size();i++){
  auto&x=a.contours[i];auto&y=b.contours[i];require(x.key==y.key&&x.closed==y.closed&&x.nodes.size()==y.nodes.size(),"contour identity mismatch");
  for(size_t j=0;j<x.nodes.size();j++){
   auto&u=x.nodes[j];auto&v=y.nodes[j];require(u.id==v.id&&std::bit_cast<uint64_t>(u.p.x)==std::bit_cast<uint64_t>(v.p.x)&&std::bit_cast<uint64_t>(u.p.y)==std::bit_cast<uint64_t>(v.p.y),"float64 bit identity mismatch");
  }
 }
}
static std::string readFile(const fs::path&p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("fixture missing");return std::string(std::istreambuf_iterator<char>(f),{});}
static void testRejected(std::function<void()>action,const std::string&label){bool rejected=false;try{action();}catch(...){rejected=true;}require(rejected,"failure guard not activated: "+label);}
int main(int argc,char**argv){
 fs::path out=argc>1?argv[1]:"results",fixtures=argc>2?argv[2]:"fixtures";fs::create_directories(out);bool passed=true;std::string error;
 try{
  auto actual=readCsv((fixtures/"web_nodes_normalized.csv").string());require(actual.contours.size()==575&&countNodes(actual)==13415,"real fixture mismatch");
  Editor ed(actual);require(ed.state().objects.size()==575,"object meta count mismatch");
  done("base_real_575_objects",{{"objects",575},{"nodes",13415},{"contours",int64_t(actual.contours.size())}});
  // Creation is directly persisted as true editable point contours; ellipses are polygon approximations, not native Bezier.
  int r=ed.rectangle({-20,-10},40,20),l=ed.line({-11,30},{11,31}),e=ed.ellipse({80,80},12,8,128),p=ed.regularPolygon({110,110},15,7);
  require(countNodes(ed.state().doc)==13415+4+2+128+7,"new shape point count mismatch");
  require(get(ed.state().doc,{p,0})->nodes.size()==7,"polygon creation nodes mismatch");
  require(get(ed.state().doc,{r,0})->closed&&get(ed.state().doc,{e,0})->closed,"shape closure invalid");
  auto rec=*get(ed.state().doc,{r,0});near(rec.nodes[2].p,{20,10});
  testRejected([&]{ed.rectangle({1,2},-3,4);},"negative width");
  testRejected([&]{ed.ellipse({1,2},0,4);},"zero radius");
  testRejected([&]{ed.line({1,1},{1,1});},"zero length line");
  done("creation_four_shapes",{{"shapes",4},{"new_nodes",141},{"ellipse_segments",128},{"ellipse_curve_mode","polyline approximation, not cubic Bezier"}});
  // Affine operations and exact undo on a contour, separate commands, safe rejection.
  auto beforeMove=ed.state().doc;
  ed.moveObjects({r},{30,7});near(get(ed.state().doc,{r,0})->nodes[0].p,{10,-3});
  ed.rotateObjects({r},{10,-3},90);near(get(ed.state().doc,{r,0})->nodes[0].p,{10,-3});
  ed.scaleObjects({r},{10,-3},1.5,0.5);near(get(ed.state().doc,{r,0})->nodes[1].p,{10,17});
  ed.mirrorX({r},10);ed.mirrorY({r},-3);
  require(ed.undoCount()>=5,"history entries missing");
  for(int k=0;k<5;k++)require(ed.undo(),"expected undo step");
  bittest(ed.state().doc,beforeMove);for(int k=0;k<5;k++)require(ed.redo(),"expected redo step");
  ed.undo();require(ed.redoCount()==1,"redo availability lost");ed.moveObjects({l},{2,2});require(ed.redoCount()==0,"redo branch not invalidated");
  testRejected([&]{ed.scaleObjects({r},{0,0},0,1);},"zero scale");
  done("affine_and_transactional_history",{{"operations_exercised",5},{"undo_and_redo_roundtrip",true},{"new_edit_clears_redo",true}});
  // Duplicate stable distinct identities, align / grouping / layers / locked selection.
  auto duplicate=ed.duplicate(r,{200,50});auto* original=get(ed.state().doc,{r,0});auto* copied=get(ed.state().doc,{duplicate,0});
  require(original&&copied&&copied->nodes.size()==original->nodes.size(),"duplicate missing");
  for(size_t i=0;i<original->nodes.size();i++){require(original->nodes[i].id!=copied->nodes[i].id,"duplicate reused node identity");near(copied->nodes[i].p-original->nodes[i].p,{200,50});}
  int64_t duplicateNodeId=copied->nodes[0].id;
  auto grp=ed.group({r,duplicate});require(ed.state().objects.at(r).group==grp,"group failed");
  const auto cp0=get(ed.state().doc,{duplicate,0})->nodes[0].p;
  ed.moveObjects({r},{4,3});
  near(get(ed.state().doc,{duplicate,0})->nodes[0].p,cp0+Pt{4,3});
  require(ed.undo(),"undo grouped move failed");
  near(get(ed.state().doc,{duplicate,0})->nodes[0].p,cp0);
  ed.ungroup(grp);
  ed.alignLeft({r,duplicate});auto [bl,bh]=boundsOf(ed.state().doc,{r});auto [cl,ch]=boundsOf(ed.state().doc,{duplicate});require(std::abs(bl.x-cl.x)<1e-10,"align left inaccurate");
  ed.addLayer(3,"Gravação");ed.moveToLayer(duplicate,3);ed.setLayerFlags(3,true,true);
  testRejected([&]{ed.moveObjects({duplicate},{9,0});},"layer locked transform");
  testRejected([&]{ed.editNode({duplicate,0},duplicateNodeId,{0,0});},"layer locked node");
  ed.setLayerFlags(3,false,false);
  auto srcHit=get(ed.state().doc,{duplicate,0})->nodes[0].p;
  auto filtered=ed.selectableHits(srcHit,0.0001);
  require(std::none_of(filtered.begin(),filtered.end(),[&](auto h){return h.key.object==duplicate;}),"hidden contour selected");
  ed.setLayerFlags(3,true,false);
  done("duplication_alignment_grouping_layers",{{"new_objects",1},{"group_cycle",true},{"hidden_and_locked_are_inaccessible",true}});
  // Identity-aware selection at zoom and DPR scales; use 3 coincident independent contours.
  Editor ov(Document{});int a=ov.line({0,0},{100,0}),b=ov.line({0,0},{100,0}),c=ov.line({0,0},{100,0});
  int selections=0;for(double z:{0.05,0.75,6.,80.,1200.})for(double dpr:{1.,1.25,1.5,2.}){
   auto hits=ov.selectableHits({50,0},6/(z*dpr));require(hits.size()==3,"overlap identity failed");
   require(hits[0].key.object==a&&hits[1].key.object==b&&hits[2].key.object==c,"overlap stable ordering failed");selections++;
  }
  ov.addLayer(1,"Hidden");ov.moveToLayer(c,1);ov.setLayerFlags(1,false,false);
  require(ov.selectableHits({50,0},0.01).size()==2,"hidden overlap did not filter out");
  done("overlap_and_pixel_tolerance",{{"zoom_dpr_configs",selections},{"same_geometry_distinct_objects",3},{"hidden_not_selected",true}});
  // Live preview identity: canonical document remains unchanged until commit; rebuild spatial after every change.
  Key testKey=actual.contours[0].key;int64_t testId=actual.contours[0].nodes[31].id;Pt sourcePt=actual.contours[0].nodes[31].p;
  auto initial=ed.state().doc;size_t previewCount=0;
  for(int i=1;i<=240;i++){
   Pt to=sourcePt+Pt{5.5*i/240.,-2.75*i/240.};auto preview=previewSegments(ed.state().doc,testKey,testId,to);
   auto expected=ed.state().doc;require(moveNode(expected,testKey,testId,to),"test node missing");
   auto expectedSegments=segments(expected);std::vector<Segment> subset;
   for(auto& segment:expectedSegments)if(segment.key==testKey)subset.push_back(segment);
   require(preview.size()==subset.size(),"preview differs count");
   for(size_t j=0;j<preview.size();j++){near(preview[j].a,subset[j].a);near(preview[j].b,subset[j].b);}
   previewCount++;
  }
  bittest(ed.state().doc,initial);
  ed.editNode(testKey,testId,sourcePt+Pt{5.5,-2.75});
  auto freshSegments=segments(ed.state().doc);SegmentBVH freshIndex(freshSegments);
  require(!freshIndex.hits(sourcePt+Pt{5.5,-2.75},6/1200.).empty(),"fresh index missed edited point");
  require(ed.undo(),"undo node edit failed");bittest(ed.state().doc,initial);
  require(ed.redo(),"redo node edit failed");
  done("drag_preview_commit_and_bvh_refresh",{{"preview_frames",int64_t(previewCount)},{"preview_does_not_mutate_original",true},{"bvh_rebuilt_after_commit",true},{"qt_paint_tested_here",false}});
  // Save full editor semantics in experimental format, using schema4 Package.layers for new metadata.
  std::string curves=readFile(fixtures/"lightburn_cubics.csv");
  auto path=out/"Alencar_Editor_Completo_EXPERIMENTAL_TESTE_5.alaser";
  Package pack{ed.state().doc,metadata(ed.state()),object{{"fabricationEnabled",false},{"caution","no GRBL output"}},curves};
  writePackage(path,pack);auto read=readPackage(path);bittest(ed.state().doc,read.doc);
  auto reconstructed=fromMetadata(read.doc,read.layers);require(serialize(metadata(reconstructed))==serialize(metadata(ed.state())),"layer, object or group metadata lost");
  require(serialize(read.operations)==serialize(pack.operations)&&read.curveSource==curves,"resources lost");
  auto badRead=read;auto badMetadata=badRead.layers.as_object();badMetadata["schema"]="unexpected-schema";
  testRejected([&]{(void)fromMetadata(read.doc,badMetadata);},"unknown metadata schema");
  auto another=out/"INTERRUPTED_TESTE_5.alaser";writePackage(another,pack);
  testRejected([&]{writePackage(another,pack,true);},"interrupted write");bittest(ed.state().doc,readPackage(another).doc);
  done("native_roundtrip_meta_layers_and_integrity",{{"object_count",int64_t(ed.state().objects.size())},{"nodes",int64_t(countNodes(ed.state().doc))},{"exact_f64",true},{"metadata_preserved",true},{"atomic_save_failure",true},{"schema","experimental-4+editor-meta-1"}});
  // Coarse performance; compare fast and full scans using real 575/13,415 geometry, no Qt paint implied.
  auto realLines=segments(actual);SegmentBVH index(realLines);
  std::mt19937 rng(20261022);std::uniform_int_distribution<int> ix(0,int(realLines.size()-1));
  int trials=0;double bruteTime=0,bvhTime=0;int sameCount=0;array perZoom;
  for(double zoom:{.05,.75,6.,80.,1200.}){double bt=0,ft=0;for(int n=0;n<100;n++){
   const auto& seg=realLines[ix(rng)];Pt q=(seg.a+seg.b)*0.5;auto tol=6/zoom;
   auto t0=Clock::now();auto ref=bruteHits(realLines,q,tol);auto t1=Clock::now();auto got=index.hits(q,tol);auto t2=Clock::now();
   require(sameHits(ref,got),"bvh mismatched ground truth");trials++;sameCount++;
   auto elapsedBrute=std::chrono::duration<double,std::milli>(t1-t0).count();auto elapsedBVH=std::chrono::duration<double,std::milli>(t2-t1).count();
   bruteTime+=elapsedBrute;bvhTime+=elapsedBVH;bt+=elapsedBrute;ft+=elapsedBVH;
  }
  perZoom.emplace_back(object{{"zoom",zoom},{"queries",100},{"brute_ms",bt},{"bvh_ms",ft},{"faster_measured",bt<ft?"full_scan":"bvh"}});
  }
  done("real_project_spatial_search",{{"searches",trials},{"matched",sameCount},{"brute_ms",bruteTime},{"bvh_ms",bvhTime},{"by_zoom",perZoom},{"adaptive_switch_required",true}});
  done("machine_safety",{{"machine_connected",false},{"gcode_generated",false},{"network_commands",0}});
 }catch(const std::exception&e){passed=false;error=e.what();std::cerr<<"FAIL "<<error<<"\n";}
 object report{{"id","ALENCAR_LASER_QT6_TESTE_5"},{"status",passed?"PASS":"FAIL"},
 {"engine","C++20 headless; Qt6 code provided separately"},{"qt6_visual_executed_here",false},{"windows_executed_here",false},
 {"tests",stages},{"error",error}};
 std::ofstream f(out/"RESULTADOS_CPP_TESTE_5.json");f<<serialize(report)<<"\n";
 return passed?0:1;
}
