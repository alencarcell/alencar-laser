// TESTE_16 — reproduz perda silenciosa anterior em v5 e confirma correção.
//  --write INPUT OUTPUT    --verify INPUT OUTPUT   --selftest
#include "integrated_project_edit.hpp"
#include "bezier_interaction.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
using namespace alaser;
namespace fs=std::filesystem;
static void required(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static bool same(const boost::json::value& a,const boost::json::value& b){return serialize(a)==serialize(b);}
static const boost::json::object& lookup(const boost::json::array& rows,int64_t id){
 for(const auto& v:rows)if(v.as_object().at("id").as_int64()==id)return v.as_object();
 throw std::runtime_error("metadata ID not found");
}
static void inject(IntegratedProject& in){
 // Fixture antigo com {"source":"TESTE_13"} não tem editor-meta-1.
 const auto oldLayers=in.legacy.layers;
 const auto state=editorStateForIntegratedProject(in);
 in.legacy.layers=preserveEditorMetadata(oldLayers,state);
 auto& meta=in.legacy.layers.as_object();
 meta["futureEditorFlag"]=boost::json::object{{"vendor","external"},{"version",17}};
 auto& layer=meta.at("layers").as_array().front().as_object();layer["originalOpacity"]=0.35;
 auto& object=meta.at("objects").as_array().front().as_object();object["customLaserNote"]="do-not-drop";
 in.legacy.operations=boost::json::object{{"fabricationEnabled",true},{"feedMmMin",1234},
  {"customOperation",boost::json::object{{"passes",3},{"label","preserve"}}}};
 in.legacy.curveSource="LIGHTBURN, source, original;\nB,1,2,3\n";
}
static void verifyMetadata(const IntegratedProject& old,const IntegratedProject& updated){
 required(updated.version==5,"saved file is not v5");
 required(same(updated.legacy.layers.as_object().at("futureEditorFlag"),old.legacy.layers.as_object().at("futureEditorFlag")),"root extension lost");
 const auto idLayer=0LL;
 const auto idObject=old.legacy.doc.contours.front().key.object;
 required(same(lookup(updated.legacy.layers.as_object().at("layers").as_array(),idLayer).at("originalOpacity"),
  lookup(old.legacy.layers.as_object().at("layers").as_array(),idLayer).at("originalOpacity")),"layer extension lost");
 required(same(lookup(updated.legacy.layers.as_object().at("objects").as_array(),idObject).at("customLaserNote"),
  lookup(old.legacy.layers.as_object().at("objects").as_array(),idObject).at("customLaserNote")),"object extension lost");
 required(updated.legacy.curveSource==old.legacy.curveSource,"original curve source dropped");
 const auto& operations=updated.legacy.operations.as_object();
 required(operations.at("fabricationEnabled").as_bool()==false,"fabrication was enabled");
 required(same(operations.at("customOperation"),old.legacy.operations.as_object().at("customOperation")),"operation options lost");
 required(same(operations.at("feedMmMin"),old.legacy.operations.as_object().at("feedMmMin")),"speed setting lost");
}
static int action(int argc,char** argv){
 required(argc==4,"usage: --write|--verify INPUT OUTPUT");
 auto base=readIntegratedProject(argv[2]);
 required(base.version==5,"require real v5 project");
 required(!base.legacy.doc.contours.empty(),"real polygon fixture absent");
 required(!base.curves.contours.empty(),"real bezier fixture absent");
 inject(base);
 if(std::string(argv[1])=="--write"){
  Editor edited(editorStateForIntegratedProject(base));
  edited.line({6,7},{8,9});
  CurveDragSession curve(base.curves);
  auto key=curve.committed().contours.front().key;
  auto first=curve.committed().contours.front().nodes.front();
  curve.select(key);
  CurveHit part;part.key=key;part.nodeId=first.id;part.part=CurvePart::Anchor;
  required(curve.start(part),"drag start failed");
  curve.update({first.anchor.x+0.1,first.anchor.y+0.15});
  required(curve.commit(),"no bezier edit");
  const auto merged=buildEditedIntegratedProject(edited.state(),curve.committed(),&base);
  verifyMetadata(base,merged);
  required(merged.legacy.doc.contours.size()==base.legacy.doc.contours.size()+1,"legacy edit lost");
  writeIntegratedProject(argv[3],merged);
  required(readIntegratedProject(argv[3]).curves.contours.size()==base.curves.contours.size(),"v5 codec mismatch");
  std::cout<<"TESTE16 WRITE PASS polygons="<<merged.legacy.doc.contours.size()<<" curves="<<merged.curves.contours.size()<<"\n";
 }else if(std::string(argv[1])=="--verify"){
  const auto fresh=readIntegratedProject(argv[3]);verifyMetadata(base,fresh);
  required(fresh.legacy.doc.contours.size()==base.legacy.doc.contours.size()+1,"reopen lost polyline");
  required(fresh.curves.contours.size()==base.curves.contours.size(),"reopen lost curves");
  required(!same(encodeCurves(fresh.curves),encodeCurves(base.curves)),"reopen lost handle modification");
  Editor edited(editorStateForIntegratedProject(fresh));
  auto again=buildEditedIntegratedProject(edited.state(),fresh.curves,&fresh);
  verifyMetadata(base,again);
  required(same(again.legacy.layers,fresh.legacy.layers),"second save changed metadata");
  required(same(again.legacy.operations,fresh.legacy.operations),"second save changed operations");
  std::cout<<"TESTE16 VERIFY PASS lossless_extensions=true source_preserved=true fabrication_disabled=true\n";
 }else throw std::runtime_error("unsupported phase");
 return 0;
}
static void failureCases(){
 Document d;CurveDocument curves;Editor e(d);
 const auto blank=buildEditedIntegratedProject(e.state(),curves);
 required(blank.version==5&&blank.legacy.operations.as_object().at("fabricationEnabled")==false,"default project invalid");
 auto noSchema=blank;noSchema.legacy.layers=boost::json::object{{"source","old experimental"}};
 auto upgraded=buildEditedIntegratedProject(editorStateForIntegratedProject(noSchema),curves,&noSchema);
 required(upgraded.legacy.layers.as_object().at("schema").as_string()=="editor-meta-1", "v5 metadata not upgraded");
 required(upgraded.legacy.layers.as_object().at("source").as_string()=="old experimental", "old provenance not preserved");
 auto custom=blank;custom.legacy.layers.as_object()["sentinel"]=123;
 auto altered=buildEditedIntegratedProject(e.state(),curves,&custom);
 required(altered.legacy.layers.as_object().at("sentinel").as_int64()==123,"preservation missing");
 custom.legacy.layers.as_object()["schema"]="unknown-v2";
 bool threw=false;try{(void)buildEditedIntegratedProject(e.state(),curves,&custom);}catch(const std::exception&){threw=true;}
 required(threw,"unsupported schema must not be silently overwritten");
 std::cout<<"TESTE16 SELFTEST PASS default_safe=true unknown_schema_rejected=true\n";
}
int main(int argc,char** argv){try{if(argc==2&&std::string(argv[1])=="--selftest"){failureCases();return 0;}return action(argc,argv);}catch(const std::exception& e){std::cerr<<"TESTE16 FAIL "<<e.what()<<"\n";return 1;}}
