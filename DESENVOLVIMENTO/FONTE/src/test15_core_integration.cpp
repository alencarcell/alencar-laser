// TESTE_15: integra geometria nativa, transacoes e persistencia v5 no processo C++.
// Executa independentemente do Qt; --verify e processo separado do --write.
#include "bezier_package.hpp"
#include "bezier_interaction.hpp"
#include "editor_tools.hpp"
#include <filesystem>
#include <iostream>
#include <string>
#include <stdexcept>
using namespace alaser;
namespace fs=std::filesystem;
static void require(bool cond,const char* msg){if(!cond)throw std::runtime_error(msg);}
static CurveDocument makeCurves(){
 CurveDocument d;d.nextId=10;CurveContour c;c.key={45,1};c.nodes={
  {1,{0,0},{-1,-1},{6,8}},
  {2,{20,0},{16,7},{26,-7}},
  {3,{40,0},{34,-7},{46,7}},
  {4,{60,0},{54,8},{61,1}}};c.edgeKind={1,1,1};d.contours.push_back(c);
 validateCurve(d);return d;
}
static bool sameCurve(const CurveDocument& a,const CurveDocument& b){return serialize(encodeCurves(a))==serialize(encodeCurves(b));}
int main(int argc,char** argv){
 try{
  require(argc==3,"usage: alencar_test15_core --write|--verify path.alaser");
  auto path=fs::path(argv[2]);std::string mode=argv[1];
  if(mode=="--write"){
   CurveDragSession s(makeCurves());const auto original=s.committed();s.select({45,1});
   CurveHit h;h.key={45,1};h.nodeId=2;h.part=CurvePart::HandleOut;
   require(s.start(h),"failed starting handle drag");
   for(int i=0;i<150;i++)s.update({26.+i*.03,-7.+i*.02});
   require(s.undoCount()==0,"gesture unexpectedly committed during preview");
   require(sameCurve(s.committed(),original),"preview mutated original");
   require(s.commit()&&s.undoCount()==1,"gesture did not commit exactly once");
   auto after=s.committed();require(s.undo(),"undo failed");
   require(sameCurve(s.committed(),original),"undo did not restore all handles");
   require(s.redo()&&sameCurve(s.committed(),after),"redo differs");
   CurveHit edge;edge.key={45,1};edge.part=CurvePart::Edge;edge.edge=1;
   int64_t created=s.insertAnchor(edge,.5);require(s.committed().contours[0].nodes.size()==5,"split failed");
   require(s.deleteAnchor(created),"delete split node failed");
   require(s.committed().contours[0].nodes.size()==4,"deletion failed");
   // A remoção da âncora INTERNA usa ponte reta, como no LightBurn.
   require(s.deleteAnchor(3),"delete internal node failed");
   const auto& c=s.committed().contours[0];
   require(c.nodes.size()==3&&c.edgeKind.size()==2&&c.edgeKind[1]==0,"straight bridge failed");
   IntegratedProject pkg;pkg.version=5;pkg.legacy.doc.nextId=1;
   pkg.legacy.layers=boost::json::object{};
   pkg.legacy.operations=boost::json::object{{"fabricationEnabled",false}};
   pkg.curves=s.committed();
   writeIntegratedProject(path,pkg);
   auto restored=readIntegratedProject(path);
   require(sameCurve(restored.curves,s.committed()),"same-process codec mismatch");
   std::cout<<"TESTE15 WRITE PASS nodes="<<c.nodes.size()<<" undo="<<s.undoCount()<<"\n";
  }else if(mode=="--verify"){
   auto file=readIntegratedProject(path);require(file.version==5,"wrong version");
   validateCurve(file.curves);const auto& c=file.curves.contours.at(0);
   require(c.key==Key{45,1}&&c.nodes.size()==3&&c.edgeKind.size()==2,"wrong persisted geometry");
   require(c.nodes[0].id==1&&c.nodes[1].id==2&&c.nodes[2].id==4,"node IDs not preserved");
   require(c.edgeKind[1]==0,"bridge was not a line");
   CurveDragSession s(file.curves);s.select(c.key);
   CurveHit h;h.key=c.key;h.nodeId=2;h.part=CurvePart::Anchor;
   require(s.start(h),"cannot edit after reopen");s.update({21,1});require(s.commit(),"cannot commit after reopen");
   require(s.undo(),"cannot undo after reopen");
   require(sameCurve(s.committed(),file.curves),"undo after reopen mismatch");
   std::cout<<"TESTE15 VERIFY PASS version=5 nodes="<<c.nodes.size()<<" post_reload_edit=1\n";
  }else throw std::runtime_error("unknown mode");
  return 0;
 }catch(const std::exception& e){std::cerr<<"TESTE15 FAIL: "<<e.what()<<"\n";return 1;}
}
