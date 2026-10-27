// TESTE_14 - integração selecao / manipulacao Bezier / persistencia sem Qt.
#include "bezier_interaction.hpp"
#include "bezier_package.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <random>
#include <bit>
#include <chrono>
using namespace alaser;
static void req(bool okay,const char* why){if(!okay)throw std::runtime_error(why);}
static bool bitsEq(Pt a,Pt b){return std::bit_cast<uint64_t>(a.x)==std::bit_cast<uint64_t>(b.x)&&std::bit_cast<uint64_t>(a.y)==std::bit_cast<uint64_t>(b.y);}
static bool identical(const CurveDocument& a,const CurveDocument& b){
 if(a.contours.size()!=b.contours.size()||a.nextId!=b.nextId)return false;
 for(size_t i=0;i<a.contours.size();i++){
  const auto&x=a.contours[i];const auto&y=b.contours[i];
  if(x.key!=y.key||x.closed!=y.closed||x.edgeKind!=y.edgeKind||x.nodes.size()!=y.nodes.size())return false;
  for(size_t j=0;j<x.nodes.size();j++){
   auto&m=x.nodes[j];auto&n=y.nodes[j];
   if(m.id!=n.id||!bitsEq(m.anchor,n.anchor)||!bitsEq(m.handleIn,n.handleIn)||!bitsEq(m.handleOut,n.handleOut))return false;
  }
 }return true;
}
static double residual(Cubic c,const std::vector<Pt>& poly){
 double maxDistance=0;
 for(int i=0;i<=1000;i++){
  Pt pos=cubicPoint(c,double(i)/1000),near{};double closest=1e99;
  for(size_t k=1;k<poly.size();k++){
   Segment s{{},0,poly[k-1],poly[k],0,0,0,0};
   closest=std::min(closest,distanceToSegment(pos,s,&near));
  }maxDistance=std::max(maxDistance,closest);
 }return maxDistance;
}
int main(int argc,char**argv){
 try{
  req(argc==4,"usage test14 lightburn_cubics.csv web_nodes.csv output_dir");
  namespace fs=std::filesystem;fs::path output=argv[3];fs::create_directories(output);
  CurveDocument doc=loadLightBurnCurves(argv[1]);validateCurve(doc);
  req(!doc.contours.empty(),"no Bezier contours");
  int cubicCount=0,straightCount=0;for(auto& c:doc.contours)for(auto k:c.edgeKind){cubicCount+=(k==1);straightCount+=(k==0);}
  req(cubicCount==37 && straightCount==4,"fixture changed");
  // Collinear overshoot: old distance-to-INFINITE-line flatness incorrectly accepted this.
  Cubic overshoot{{0,0},{40,0},{-40,0},{1,0}};
  auto poly=flatten(overshoot,.02);req(poly.size()>2,"overshoot cubic flattened into single chord");
  req(residual(overshoot,poly)<.021,"overshoot flatten geometric error too big");
  // Closed path: last edge connects to first node and must keep identity when split.
  CurveDocument closed;
  CurveContour loop;loop.key={60001,9};loop.closed=true;
  loop.nodes={{1,{0,0},{0,0},{2,0}},{2,{10,0},{8,0},{10,2}},
              {3,{10,10},{10,8},{8,10}},{4,{0,10},{2,10},{0,8}}};
  loop.edgeKind={1,0,0,1};closed.contours.push_back(loop);closed.nextId=5;
  validateCurve(closed);
  Cubic originalClosing=edgeAt(closed.contours[0],3);
  int64_t inserted=splitEdge(closed,0,3,.42);
  validateCurve(closed);
  req(closed.contours[0].nodes.size()==5&&closed.contours[0].nodes[4].id==inserted,
      "closed curve split did not preserve new node identity");
  for(int i=0;i<=100;i++){
   double u=i/100.;
   Pt compare=u<=.42?cubicPoint(edgeAt(closed.contours[0],3),u/.42):
                      cubicPoint(edgeAt(closed.contours[0],4),(u-.42)/.58);
   req(dist(compare,cubicPoint(originalClosing,u))<1e-10,"closed split differs");
  }
  size_t curveSelectionCases=0, misses=0;
  for(auto& c:doc.contours)for(size_t e=0;e<edgeCount(c);e++){
   auto curve=edgeAt(c,e);
   for(double t:{.15,.33,.57,.83}){
    Pt on=c.edgeKind[e]?cubicPoint(curve,t):lerp(curve.p0,curve.p3,t);
    for(double px:{.75,6.,80.,1200.}){
     auto hits=pickCurveEdges(doc,on,px,1.0,6);
     bool found=false;
     for(auto&hit:hits)if(hit.key==c.key&&hit.distancePx<=0.12){found=true;break;}
     misses+=!found;curveSelectionCases++;
    }
   }
  }
  req(misses==0,"real Bezier point selection missed contour");
  // Tolerancia expressa em pixels, nao mm; longe de contorno nao e permitido clicar.
  auto& target=doc.contours.front();Key key=target.key;auto first=target.nodes[1];
  CurveDocument separated;
  separated.contours.push_back({{50202,0},{{101,{0,0},{0,0},{0,0}},{102,{1000,1000},{1000,1000},{1000,1000}}},{0},false});
  separated.nextId=103;validateCurve(separated);
  for(double z:{.75,6.,80.,1200.}){
   Pt off{9/z,0};
   req(!pickCurveNode(separated,{50202,0},off,z,1.,6),"node picker ignores physical pixel tolerance");
  }
  // Duplicatas identicas devem ser escolhas distintas (alternancia pode usar o indice da lista).
  CurveDocument overlapping;auto clone=target;clone.key={80001,0};
  overlapping.contours={target,clone};
  for(auto& n:overlapping.contours[1].nodes)n.id+=100000;
  overlapping.nextId=200000;
  validateCurve(overlapping);
  Pt on=edgeAt(target,0).p0;
  auto duplicates=pickCurveEdges(overlapping,on,80,1.5,6);
  req(duplicates.size()==2,"overlapping curve IDs collapsed");
  req(duplicates[0].key!=duplicates[1].key,"overlap has identical keys");
  // Selection of handles only comes from active contour.
  auto activeNode=pickCurveNode(overlapping,key,first.anchor,80,1.5,7);
  req(activeNode&&activeNode->key==key&&activeNode->part==CurvePart::Anchor,"incorrect anchor identity");
  // Handles must obey the selected contour and physical pixel scale, including 200% DPI.
  CurveDocument handleFixture;
  CurveContour hc;hc.key={65000,0};hc.nodes={{1,{0,0},{-5,0},{5,0}},
    {2,{20,0},{15,0},{25,0}}};hc.edgeKind={1};handleFixture.contours={hc};handleFixture.nextId=3;
  validateCurve(handleFixture);
  auto pickedHandle=pickCurveNode(handleFixture,hc.key,{5,0},80,2.,7);
  req(pickedHandle&&pickedHandle->part==CurvePart::HandleOut&&pickedHandle->nodeId==1,
      "200pct DPI handle was not selected correctly");
  req(!pickCurveNode(handleFixture,hc.key,{5.06,0},80,2.,7),
      "200pct DPI allowed a handle click outside 7 physical pixels");
  CurveDragSession session(doc);session.select(key);
  auto pick=pickCurveNode(doc,key,first.anchor,80,1.25,7);
  req(pick&&pick->nodeId==first.id&&pick->part==CurvePart::Anchor,"anchor not picked");
  const CurveDocument initial=doc;
  req(session.start(*pick),"drag not started");
  for(int f=1;f<=120;f++){
   Pt now=first.anchor+Pt{f*.015,-f*.008};session.update(now);
   req(identical(session.committed(),initial),"preview changed committed geometry before mouse release");
   req(bitsEq(session.visible().contours.front().nodes[1].anchor,now),"preview anchor wrong");
  }
  req(session.commit(),"drag not committed");
  req(session.undoCount()==1,"120 mousemove frames created >1 undo entry");
  req(!identical(session.committed(),initial),"no committed edit");
  auto edited=session.committed();
  req(session.undo()&&identical(session.committed(),initial),"undo lost original");
  req(session.redo()&&identical(session.committed(),edited),"redo lost edited");
  session.select(key);
  auto originalCommitted=session.committed();
  auto targetHandle=session.committed().contours.front().nodes[1];
  CurveHit handle{key,-1,targetHandle.id,CurvePart::HandleOut,0,0,targetHandle.handleOut};
  req(session.start(handle),"handle drag failed");
  Pt out=targetHandle.handleOut+Pt{3,2};session.update(out);
  req(bitsEq(session.visible().contours.front().nodes[1].anchor,targetHandle.anchor),"handle moved anchor");
  req(bitsEq(session.visible().contours.front().nodes[1].handleOut,out),"handle preview failed");
  session.cancel();req(identical(session.visible(),originalCommitted),"cancel didn't restore geometry");
  req(!session.dragging(),"cancel left dragging active");
  req(session.start(handle),"handle re-drag failed");session.update(out);req(session.commit(),"handle commit failed");
  req(session.undoCount()==2,"handle drag must create one undo entry");
  req(session.undo(),"handle undo missing");req(identical(session.committed(),originalCommitted),"handle undo changed wrong geometry");
  // invalid drag targets reject without history mutation
  CurveDragSession safe(initial);safe.select(key);auto bogus=handle;bogus.nodeId=-15;
  req(!safe.start(bogus),"invalid node id accepted");req(safe.undoCount()==0,"invalid pick wrote history");
  // roundtrip shared format v5 after actual drag edit.
  auto legacy=readCsv(argv[2]);req(legacy.contours.size()==575,"real source not present");
  Package package{legacy,object{{"source","TESTE_14"}},object{{"fabricationEnabled",false}},std::string()};
  IntegratedProject full{package,edited,5};fs::path filename=output/"ALENCAR_TESTE_14_Bezier_Interativo_EXPERIMENTAL.alaser";
  writeIntegratedProject(filename,full);
  auto reopened=readIntegratedProject(filename);
  req(identical(reopened.curves,edited)&&sameDoc(reopened.legacy.doc,legacy),".alaser v5 save/load changed geometry");
  // Validate previous v4 remains readable without invented curves.
  auto old=output/"ALENCAR_TESTE_14_V4_EXPERIMENTAL.alaser";writePackage(old,package);
  auto v4=readIntegratedProject(old);req(v4.version==4&&v4.curves.contours.empty(),"v4 compatibility regression");
  std::ofstream json(output/"RESULTADOS_TESTE_14.json");
  json<<"{\n  \"test\":14,\n  \"native_curves\":"<<cubicCount<<",\n  \"native_lines\":"<<straightCount
      <<",\n  \"edge_click_cases\":"<<curveSelectionCases<<",\n  \"edge_click_misses\":"<<misses
      <<",\n  \"closed_curve_split_verified\":true,\n  \"dpi_200pct_handles_verified\":true,\n  \"drag_preview_frames\":120,\n  \"drag_transaction_undo_entries\":1,\n  \"overlap_contour_ids_preserved\":true,\n  \"collinear_overshoot_fixed\":true,\n  \"cancel_restores_geometry\":true,\n  \"handle_edit_keeps_anchor\":true,\n  \"v5_roundtrip\":true,\n  \"v4_backward_compatible\":true,\n  \"qt6_compiled_here\":false,\n  \"machine_connected\":false\n}\n";
  std::cout<<"PASS TESTE_14 cubic="<<cubicCount<<" lines="<<straightCount
    <<" edge_clicks="<<curveSelectionCases<<" misses="<<misses
    <<" drag_frames=120 undo=1 overshoot=OK overlap=OK handles=OK v5=OK v4=OK\n";
  return 0;
 }catch(const std::exception&e){std::cerr<<"FAIL TESTE_14 "<<e.what()<<"\n";return 1;}
}
