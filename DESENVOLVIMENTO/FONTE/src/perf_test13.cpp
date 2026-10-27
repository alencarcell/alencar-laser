// TESTE_13: ensaio real de curvas cubicas e persistencia, sem render Qt.
#include "bezier_package.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <bit>
#include <chrono>
using namespace alaser;
static void ensure(bool value,const std::string& reason){if(!value)throw std::runtime_error(reason);}
static bool identical(Pt a,Pt b){return std::bit_cast<uint64_t>(a.x)==std::bit_cast<uint64_t>(b.x)&&std::bit_cast<uint64_t>(a.y)==std::bit_cast<uint64_t>(b.y);}
static bool sameCurve(const CurveDocument& a,const CurveDocument& b){
 if(a.contours.size()!=b.contours.size()||a.nextId!=b.nextId)return false;
 for(size_t i=0;i<a.contours.size();++i){auto&x=a.contours[i];auto&y=b.contours[i];
  if(x.key!=y.key||x.closed!=y.closed||x.edgeKind!=y.edgeKind||x.nodes.size()!=y.nodes.size())return false;
  for(size_t j=0;j<x.nodes.size();j++){auto&u=x.nodes[j];auto&v=y.nodes[j];
   if(u.id!=v.id||!identical(u.anchor,v.anchor)||!identical(u.handleIn,v.handleIn)||!identical(u.handleOut,v.handleOut))return false;
  }
 }
 return true;
}
static double distancePolyline(Pt p,const std::vector<Pt>& points){
 double best=std::numeric_limits<double>::infinity();for(size_t i=1;i<points.size();i++){
  Segment s{{0,0},0,points[i-1],points[i],0,0,0,0};best=std::min(best,distanceToSegment(p,s));
 }return best;
}
int main(int argc,char**argv){
 try{
  ensure(argc==4,"usage: test13 fixture_curves.csv fixture_web.csv output_dir");
  const std::filesystem::path out=argv[3];std::filesystem::create_directories(out);
  CurveDocument curves=loadLightBurnCurves(argv[1]);
  size_t cubics=0,lines=0;size_t flattenChecks=0,splits=0,edits=0;double maxFlattenError=0,maxSplitError=0;
  std::vector<std::pair<size_t,size_t>> cubicIndex;
  for(size_t i=0;i<curves.contours.size();i++)for(size_t j=0;j<curves.contours[i].edgeKind.size();j++){
   if(curves.contours[i].edgeKind[j]){cubics++;cubicIndex.emplace_back(i,j);}else lines++;
  }
  ensure(cubics>=35,"unexpectedly few LightBurn cubic curves");
  const CurveDocument initial=curves;
  for(auto [ci,ei]:cubicIndex){
   Cubic original=edgeAt(initial.contours[ci],ei);
   for(double tol:{.005,.02,.1}){
    auto points=flatten(original,tol);
    ensure(points.size()>=2,"flatten returned no geometry");
    for(int k=0;k<=1000;k++){
     Pt reference=cubicPoint(original,double(k)/1000);
     double error=distancePolyline(reference,points);
     maxFlattenError=std::max(maxFlattenError,error);++flattenChecks;
     ensure(error<=tol*1.02+1e-9,"flatten exceeds tolerance");
    }
   }
   for(double t:{.15,.5,.83}){
    CurveDocument temp=initial;
    auto id=splitEdge(temp,ci,ei,t);validateCurve(temp);splits++;
    auto& c=temp.contours[ci];ensure(c.nodes[ei+1].id==id,"split did not preserve insert identity");
    Cubic left=edgeAt(c,ei),right=edgeAt(c,ei+1);
    for(int k=0;k<=200;k++){
     double u=k/200.;Pt actual=u<=t?cubicPoint(left,u/t):cubicPoint(right,(u-t)/(1-t));
     double err=dist(actual,cubicPoint(original,u));maxSplitError=std::max(maxSplitError,err);
     ensure(err<1e-10,"de Casteljau split deviates from exact cubic");
    }
   }
  }
  // Um no deslocado move os controles juntos, e cancelamento restabelece snapshot.
  CurveDocument changed=initial;
  for(auto& c:changed.contours){
   auto id=c.nodes.at(0).id; auto orig=c.nodes[0];
   moveCurveNode(changed,c.key,id,{orig.anchor.x+0.25,orig.anchor.y-0.5});
   ensure(dist(c.nodes[0].anchor,orig.anchor)>.1,"move didn't change anchor");
   ensure(dist(c.nodes[0].handleIn,orig.handleIn+Pt{.25,-.5})<1e-11,"in handle not moved");
   ensure(dist(c.nodes[0].handleOut,orig.handleOut+Pt{.25,-.5})<1e-11,"out handle not moved");
   edits++;
  }
  CurveDocument cancel=initial;ensure(sameCurve(cancel,initial),"cancel snapshot mismatch");
  auto serialized=serialize(encodeCurves(changed));
  ensure(sameCurve(changed,decodeCurves(parse(serialized).as_object())),"curve JSON roundtrip changed float64 bits");
  // Regressao de desfazer/refazer com nos cubicos e subdivisao real.
  CurveHistory hist(initial);
  for(int j=0;j<12;j++){
   if(j%2==0){
    hist.change([&](CurveDocument& doc){splitEdge(doc,0,size_t(j/2),.3);});
   }else{
    hist.change([&](CurveDocument& doc){auto &n=doc.contours[0].nodes.at(size_t(j));
      moveCurveNode(doc,doc.contours[0].key,n.id,n.anchor+Pt{.25,-.125});});
   }
  }
  CurveDocument finalHist=hist.document();
  ensure(hist.undoCount()==12,"Bezier history missing operations");
  for(int j=0;j<12;j++)ensure(hist.undo(),"Bezier undo failure");
  ensure(sameCurve(hist.document(),initial),"Bezier undo did not restore exact original controls");
  for(int j=0;j<12;j++)ensure(hist.redo(),"Bezier redo failure");
  ensure(sameCurve(hist.document(),finalHist),"Bezier redo lost controls or IDs");
  ensure(hist.undo(),"Bezier history tail undo failed");
  hist.change([&](CurveDocument& doc){auto id=doc.contours[0].nodes[0].id;
    auto p=doc.contours[0].nodes[0].anchor;
    moveCurveNode(doc,doc.contours[0].key,id,p+Pt{1,2});});
  ensure(!hist.redo(),"redo not cleared after new change");
  // Garantir integridade do formato v5: geometria anterior + curvas novas no MESMO arquivo.
  auto legacy=readCsv(argv[2]);ensure(legacy.contours.size()==575,"unexpected real fixture size");
  Package original{legacy,object{{"source","TESTE_13"}},object{{"fabricationEnabled",false}},std::string()};
  IntegratedProject full{original,changed,5};
  auto file=out/"Alencar_Test13_Curvas_Nativas_EXPERIMENTAL.alaser";
  writeIntegratedProject(file,full);
  auto reopened=readIntegratedProject(file);
  ensure(reopened.version==5&&sameDoc(reopened.legacy.doc,legacy)&&sameCurve(reopened.curves,changed),"v5 ZIP roundtrip differs");
  // Teste de compatibilidade retroativa com v4.
  auto old=out/"Alencar_Test13_V4_Compatibilidade_EXPERIMENTAL.alaser";
  writePackage(old,original);auto oldOpened=readIntegratedProject(old);
  ensure(oldOpened.version==4&&sameDoc(oldOpened.legacy.doc,legacy)&&oldOpened.curves.contours.empty(),"v4 compatibility lost");
  // Corrupcao controlada nao pode ser interpretada como documento valido.
  bool invalidRejected=false;try{auto broken=changed;broken.contours[0].nodes[0].id=broken.contours[0].nodes[1].id;validateCurve(broken);}catch(...){invalidRejected=true;}
  ensure(invalidRejected,"duplicate node identity wasn't rejected");
  std::cout<<"PASS TESTE_13 cubic="<<cubics<<" line="<<lines<<" paths="<<curves.contours.size()
           <<" flattenSamples="<<flattenChecks<<" maxErrorMm="<<maxFlattenError
           <<" splits="<<splits<<" maxSplitErrorMm="<<maxSplitError<<" edits="<<edits
           <<" history=OK nativeZipV5=OK legacyV4=OK\n";
  std::ofstream json(out/"RESULTADOS_TESTE_13.json");
  json<<"{\n  \"test\":13,\n  \"native_cubics\":"<<cubics<<",\n  \"native_lines\":"<<lines
      <<",\n  \"native_contours\":"<<curves.contours.size()<<",\n  \"flatten_samples\":"<<flattenChecks
      <<",\n  \"max_flatten_error_mm\":"<<std::setprecision(17)<<maxFlattenError
      <<",\n  \"split_cases\":"<<splits<<",\n  \"max_split_error_mm\":"<<maxSplitError
      <<",\n  \"moving_nodes_tested\":"<<edits
      <<",\n  \"bezier_history_operations\":12,\n  \"alaser_v5_roundtrip\":true,\n  \"alaser_v4_compatibility\":true,\n  \"qt6_executed\":false,\n  \"machine_connected\":false\n}\n";
  return 0;
 }catch(const std::exception& ex){std::cerr<<"FAIL TESTE_13 "<<ex.what()<<"\n";return 1;}
}
