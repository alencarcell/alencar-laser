// TESTE15: roundtrip v5 com projeto real do laboratorio e curvas reais LightBurn.
#include "bezier_interaction.hpp"
#include "bezier_package.hpp"
#include <iostream>
#include <stdexcept>
#include <filesystem>
using namespace alaser;
static void check15(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static bool same(const CurveDocument&a,const CurveDocument&b){return serialize(encodeCurves(a))==serialize(encodeCurves(b));}
int main(int argc,char** argv){
 try{
  check15(argc==3,"usage test15_real_roundtrip input.alaser output.alaser");
  const auto original=readIntegratedProject(argv[1]);
  check15(original.version==5,"real fixture not v5");
  check15(original.curves.contours.size()>0,"no real curves");
  const auto legacyBefore=serialize(encode(original.legacy.doc));
  CurveDragSession session(original.curves);
  const auto c=original.curves.contours.front();
  check15(c.nodes.size()>=3,"real curve lacks enough nodes");
  session.select(c.key);
  CurveHit hit;hit.key=c.key;hit.nodeId=c.nodes[1].id;hit.part=CurvePart::HandleOut;
  check15(session.start(hit),"cannot start real curve edit");
  auto oldPt=c.nodes[1].handleOut;
  session.update({oldPt.x+.002,oldPt.y+.005});
  check15(session.commit()&&session.undoCount()==1,"real curve transaction failed");
  auto updated=session.committed();
  check15(session.undo()&&same(original.curves,session.committed()),"undo real curve lost fidelity");
  check15(session.redo()&&same(updated,session.committed()),"redo real curve lost fidelity");
  IntegratedProject result=original;result.curves=updated;
  writeIntegratedProject(argv[2],result);
  auto reopened=readIntegratedProject(argv[2]);
  check15(same(reopened.curves,result.curves),"real v5 curve roundtrip differs");
  check15(serialize(encode(reopened.legacy.doc))==legacyBefore,"real project polylines modified");
  check15(reopened.curves.contours.size()==original.curves.contours.size(),"real curve count lost");
  CurveDragSession second(reopened.curves);second.select(c.key);
  check15(second.undoCount()==0,"transient undo history persisted unexpectedly");
  check15(second.committed().contours[0].nodes[1].id==c.nodes[1].id,"real node ID lost");
  std::cout<<"TESTE15 REAL PASS poly_contours="<<reopened.legacy.doc.contours.size()
           <<" curves="<<reopened.curves.contours.size()<<"\n";
  return 0;
 }catch(const std::exception& e){std::cerr<<"TESTE15 REAL FAIL "<<e.what()<<"\n";return 1;}
}
