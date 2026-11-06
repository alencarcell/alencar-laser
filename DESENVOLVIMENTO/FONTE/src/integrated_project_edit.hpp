#pragma once
// TESTE_16: edição preservadora de dados auxiliares de projetos v4/v5.
// O editor visual é um cliente deste adaptador; este arquivo não depende de Qt.
#include "bezier_package.hpp"
#include "editor_tools.hpp"
#include <optional>
#include <map>
#include <stdexcept>

namespace alaser {
namespace json=boost::json;

// Preserva campos adicionais que o editor não interpreta, sem manter objetos
// excluídos e sem sobrepor atributos canônicos efetivamente modificados.
inline json::array overlayById(const json::array& source,const json::array& canonical){
 std::map<int64_t,json::object> original;
 for(const auto& record:source){
  if(!record.is_object())continue;
  const auto& item=record.as_object();
  auto it=item.find("id");
  if(it==item.end()||!it->value().is_int64())continue;
  original.emplace(it->value().as_int64(),item);
 }
 json::array merged;
 for(const auto& val:canonical){
  const auto& item=val.as_object();const int64_t id=item.at("id").as_int64();
  auto it=original.find(id);
  json::object row=it==original.end()?json::object{}:it->second;
  for(const auto& kv:item)row[kv.key()]=kv.value();
  merged.push_back(std::move(row));
 }
 return merged;
}
inline json::value preserveEditorMetadata(const json::value& source,const EditorState& edited){
 json::object canonical=metadata(edited);
 if(!source.is_object())return canonical;
 const auto& old=source.as_object();
 auto schema=old.find("schema");
 if(schema!=old.end()&&(!schema->value().is_string()||schema->value().as_string()!="editor-meta-1"))
  throw std::runtime_error("unsupported editor metadata schema: refusing lossy save");
 json::object output=old;
 for(const auto& kv:canonical){
  const auto key=kv.key();
  if((key=="layers"||key=="objects")&&output.if_contains(key)&&output.at(key).is_array())
   output[key]=overlayById(output.at(key).as_array(),kv.value().as_array());
  else output[key]=kv.value();
 }
 return output;
}
// 19.9 native LightBurn: retain every CutIndex as a real *editor* layer.
// Source LBRN2 CutIndex supplies an identity, NOT trusted speed/Fill/power values.
// This repairs older v5 projects in memory without reassigning native curves,
// touching their cubic handles, or enabling fabrication.
inline std::size_t registerNativeLayerReferences(EditorState& state,
                                                  const CurveDocument& native){
 std::size_t registered=0;
 for(const auto& contour:native.contours){
  if(contour.layer<0)throw std::runtime_error("native LightBurn layer is negative");
  if(state.layers.count(contour.layer))continue;
  Layer placeholder;
  placeholder.id=contour.layer;
  placeholder.name="LightBurn C"+std::to_string(contour.layer)+" (parametros nao verificados)";
  placeholder.mode="Line"; // editor display only, not original LightBurn operation
  placeholder.parametersVerified=false;
  placeholder.output=contour.layer!=1001&&contour.layer!=1002;
  state.layers.emplace(contour.layer,std::move(placeholder));
  ++registered;
 }
 return registered;
}
// A brand-new LBRN2 import cannot assume that even the default C00 settings
// represent those in LightBurn. Keep all imported layer IDs, flag every
// referenced layer's settings as unverified, and preserve the source geometry.
inline EditorState newEditorStateForNativeLightBurn(const CurveDocument& native){
 validateCurve(native);
 EditorState state=fromDocument(Document{});
 registerNativeLayerReferences(state,native);
 for(const auto& contour:native.contours)
  state.layers.at(contour.layer).parametersVerified=false;
 validate(state);
 return state;
}
// V5 experimental dos TESTES 13-15 inclui projects com layers={"source":...},
// sem schema do editor. Um projeto SEM schema utiliza a camada padrão,
// mantendo a origem em campo adicional ao ser salvo. Schema desconhecido não
// pode ser silenciosamente importado.
inline EditorState editorStateForIntegratedProject(const IntegratedProject& project){
 const auto& stored=project.legacy.layers;
 if(!stored.is_object())throw std::runtime_error("invalid editor layer metadata");
 const auto& root=stored.as_object();
 auto it=root.find("schema");
 EditorState state;
 if(it==root.end())state=fromDocument(project.legacy.doc);
 else{
  if(!it->value().is_string()||it->value().as_string()!="editor-meta-1")
   throw std::runtime_error("unknown editor schema: migration requires review");
  state=fromMetadata(project.legacy.doc,stored);
 }
 registerNativeLayerReferences(state,project.curves);
 validate(state);
 return state;
}
// A interface não habilita laser. Mantém os ajustes de trabalho e desabilita
// apenas o sinal explícito de fabricação, inclusive em projetos antigos.
inline json::value safeOperations(json::value operations){
 if(operations.is_object())operations.as_object()["fabricationEnabled"]=false;
 return operations;
}
inline IntegratedProject buildEditedIntegratedProject(const EditorState& state,
  const CurveDocument& curves,const IntegratedProject* previous=nullptr){
 validate(state);validateCurve(curves);
 IntegratedProject output;
 output.version=5;
 output.legacy.doc=state.doc;
 if(previous){
  output.legacy.layers=preserveEditorMetadata(previous->legacy.layers,state);
  output.legacy.operations=safeOperations(previous->legacy.operations);
  output.legacy.curveSource=previous->legacy.curveSource;
 }else{
  output.legacy.layers=metadata(state);
  output.legacy.operations=json::object{{"fabricationEnabled",false}};
 }
 output.curves=curves;
 return output;
}
} // namespace alaser
