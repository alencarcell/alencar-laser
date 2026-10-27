#pragma once
// Pacote .alaser experimental v5: mantém geometria poligonal do v4 e adiciona curvas cubicas nativas.
// O importador v4 existente continua intacto. Leitor unificado aceita v4 e v5.
#include "persistence.hpp"
#include "bezier_native.hpp"
#include <filesystem>
#include <string>
#include <stdexcept>

namespace alaser {
inline object encodeCurves(const CurveDocument& d){
 validateCurve(d);object root;root["schema"]="bezier-1";root["coordinateUnit"]="mm";root["nextCurveNodeId"]=d.nextId;
 array curves;
 for(const auto& c:d.contours){
  object co;co["objectId"]=c.key.object;co["subpathId"]=c.key.sub;co["closed"]=c.closed;
  array edges;for(uint8_t e:c.edgeKind)edges.push_back(e?"cubic":"line");co["edgeKinds"]=std::move(edges);
  array nodes;
  for(const auto& n:c.nodes){
   nodes.push_back(object{{"id",n.id},{"anchorX64",bitsToHex(n.anchor.x)},{"anchorY64",bitsToHex(n.anchor.y)},
     {"inX64",bitsToHex(n.handleIn.x)},{"inY64",bitsToHex(n.handleIn.y)},
     {"outX64",bitsToHex(n.handleOut.x)},{"outY64",bitsToHex(n.handleOut.y)}});
  }
  co["nodes"]=std::move(nodes);curves.push_back(std::move(co));
 }
 root["contours"]=std::move(curves);return root;
}
inline CurveDocument decodeCurves(const object& root){
 if(root.at("schema").as_string()!="bezier-1"||root.at("coordinateUnit").as_string()!="mm")throw std::runtime_error("unsupported curve schema");
 CurveDocument d;d.nextId=root.at("nextCurveNodeId").as_int64();
 for(auto& cv:root.at("contours").as_array()){
  const auto& co=cv.as_object();CurveContour c;c.key={int(co.at("objectId").as_int64()),int(co.at("subpathId").as_int64())};c.closed=co.at("closed").as_bool();
  for(auto& edge:co.at("edgeKinds").as_array()){
   auto s=edge.as_string();if(s=="cubic")c.edgeKind.push_back(1);else if(s=="line")c.edgeKind.push_back(0);else throw std::runtime_error("invalid edge type");
  }
  for(auto& nv:co.at("nodes").as_array()){
   const auto& n=nv.as_object();auto read=[&](const char* field){return hexToBits(std::string(n.at(field).as_string()));};
   c.nodes.push_back({n.at("id").as_int64(),{read("anchorX64"),read("anchorY64")},
     {read("inX64"),read("inY64")},{read("outX64"),read("outY64")}});
  }
  d.contours.push_back(std::move(c));
 }
 validateCurve(d);return d;
}
struct IntegratedProject {Package legacy;CurveDocument curves;int version=5;};
inline void writeIntegratedProject(const std::filesystem::path& path,const IntegratedProject& project){
 namespace fs=std::filesystem;
 const std::string geometry=serialize(encode(project.legacy.doc));
 const std::string native=serialize(encodeCurves(project.curves));
 const std::string layers=serialize(project.legacy.layers),operations=serialize(project.legacy.operations);
 const std::string source=project.legacy.curveSource;
 object manifest{{"format","ALENCAR-LASER-EXPERIMENTAL"},{"schemaVersion",5},{"fabricationEnabled",false},
  {"geometryDigestFNV1a64",digest(geometry)},{"bezierDigestFNV1a64",digest(native)},
  {"layersDigestFNV1a64",digest(layers)},{"operationsDigestFNV1a64",digest(operations)},
  {"curvesDigestFNV1a64",digest(source)},
  {"description","TESTE_13 experimental - do not use for fabrication"}};
 fs::path temp=path;temp+=".partial";fs::remove(temp);
 zipFile zip=zipOpen64(temp.string().c_str(),APPEND_STATUS_CREATE);if(!zip)throw std::runtime_error("cannot write integrated ZIP");
 try{
  addZip(zip,"manifest.json",serialize(manifest));addZip(zip,"design/geometry.json",geometry);
  addZip(zip,"design/bezier.json",native);addZip(zip,"design/layers.json",layers);
  addZip(zip,"fabrication/operations.json",operations);addZip(zip,"design/original_bezier_samples.csv",source);
 }catch(...){zipClose(zip,nullptr);fs::remove(temp);throw;}
 if(zipClose(zip,nullptr)!=ZIP_OK){fs::remove(temp);throw std::runtime_error("failed to finish integrated ZIP");}
 try{fs::rename(temp,path);}catch(...){fs::remove(temp);throw;}
}
inline IntegratedProject readIntegratedProject(const std::filesystem::path& file){
 unzFile zip=unzOpen64(file.string().c_str());if(!zip)throw std::runtime_error("invalid ZIP file");
 int version=0;std::string manifest;
 try{manifest=extract(zip,"manifest.json");auto m=parse(manifest).as_object();
   if(m.at("format").as_string()!="ALENCAR-LASER-EXPERIMENTAL")throw std::runtime_error("unsupported format");
   version=int(m.at("schemaVersion").as_int64());
 }catch(...){unzClose(zip);throw;}
 if(version==4){unzClose(zip);IntegratedProject p;p.legacy=readPackage(file);p.version=4;return p;}
 if(version!=5){unzClose(zip);throw std::runtime_error("unsupported integrated version");}
 std::string g,b,l,o,s;
 try{
  g=extract(zip,"design/geometry.json");b=extract(zip,"design/bezier.json");
  l=extract(zip,"design/layers.json");o=extract(zip,"fabrication/operations.json");s=extract(zip,"design/original_bezier_samples.csv");
 }catch(...){unzClose(zip);throw;}
 unzClose(zip);
 auto m=parse(manifest).as_object();
 if(m.at("geometryDigestFNV1a64").as_string()!=digest(g)||m.at("bezierDigestFNV1a64").as_string()!=digest(b)
   ||m.at("layersDigestFNV1a64").as_string()!=digest(l)||m.at("operationsDigestFNV1a64").as_string()!=digest(o)
   ||m.at("curvesDigestFNV1a64").as_string()!=digest(s))throw std::runtime_error("integrated ZIP digest mismatch");
 IntegratedProject p;p.version=5;p.legacy={decode(parse(g).as_object()),parse(l),parse(o),s};p.curves=decodeCurves(parse(b).as_object());
 return p;
}
}
