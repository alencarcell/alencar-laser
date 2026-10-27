#pragma once
#include "geometry.hpp"
#include <boost/json.hpp>
#include <minizip/zip.h>
#include <minizip/unzip.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <map>
#include <cstdint>
#include <bit>
#include <stdexcept>

namespace alaser {
using namespace boost::json;
inline std::string digest(const std::string& bytes){
 uint64_t h=14695981039346656037ULL;
 for(unsigned char b:bytes){h^=b;h*=1099511628211ULL;}
 std::ostringstream s;s<<std::hex<<std::setfill('0')<<std::setw(16)<<h;return s.str(); // FNV-1a integrity tag, not a cryptographic hash
}
inline std::string bitsToHex(double v){
 std::ostringstream s;s<<std::hex<<std::setfill('0')<<std::setw(16)<<std::bit_cast<uint64_t>(v);return s.str();
}
inline double hexToBits(const std::string& h){
 if(h.size()!=16||h.find_first_not_of("0123456789abcdefABCDEF")!=std::string::npos)throw std::runtime_error("invalid float64 bit string");
 return std::bit_cast<double>(std::stoull(h,nullptr,16));
}
inline object encode(const Document& d){
 object root;root["coordinateUnit"]="mm";root["nextNodeId"]=d.nextId;array cs;
 for(const auto& c:d.contours){object e;e["objectId"]=c.key.object;e["subpathId"]=c.key.sub;e["closed"]=c.closed;array ns;
  for(auto n:c.nodes){object p;p["id"]=n.id;p["x"]=n.p.x;p["y"]=n.p.y;p["xF64"]=bitsToHex(n.p.x);p["yF64"]=bitsToHex(n.p.y);ns.push_back(std::move(p));}
  e["nodes"]=std::move(ns);cs.push_back(std::move(e));}
 root["contours"]=std::move(cs);return root;
}
inline Document decode(const object& o){
 if(o.at("coordinateUnit").as_string()!="mm")throw std::runtime_error("coordinate unit unsupported");
 Document d;d.nextId=o.at("nextNodeId").as_int64();if(d.nextId<1)throw std::runtime_error("invalid nextId");
 std::map<Key,bool> seen;std::map<int64_t,bool> nodeIds;
 auto num=[](const value&v)->double{return v.is_int64()?double(v.as_int64()):v.is_uint64()?double(v.as_uint64()):v.as_double();};
 for(const auto& cv:o.at("contours").as_array()){
  const auto& c=cv.as_object();Contour z;z.key={int(c.at("objectId").as_int64()),int(c.at("subpathId").as_int64())};z.closed=c.at("closed").as_bool();
  if(seen[z.key])throw std::runtime_error("duplicate object/subpath identity");
  seen[z.key]=true;
  for(const auto& nv:c.at("nodes").as_array()){
   const auto &n=nv.as_object();Node k;k.id=n.at("id").as_int64();k.p={hexToBits(std::string(n.at("xF64").as_string())),hexToBits(std::string(n.at("yF64").as_string()))};
   if(std::abs(k.p.x-num(n.at("x")))>1e-10||std::abs(k.p.y-num(n.at("y")))>1e-10)throw std::runtime_error("numeric/bit representation conflict");
   if(k.id<1||nodeIds[k.id]||!std::isfinite(k.p.x)||!std::isfinite(k.p.y))throw std::runtime_error("invalid/duplicate node identity or coordinates");
   nodeIds[k.id]=true;z.nodes.push_back(k);
  }
  if(z.closed&&z.nodes.size()<3)throw std::runtime_error("invalid closed contour");
  d.contours.push_back(std::move(z));
 }
 if(!nodeIds.empty()&&d.nextId<=nodeIds.rbegin()->first)throw std::runtime_error("nextId must exceed existing node IDs");
 return d;
}
inline void addZip(zipFile z,const std::string& name,const std::string& data){
 zip_fileinfo info{};
 if(zipOpenNewFileInZip64(z,name.c_str(),&info,nullptr,0,nullptr,0,nullptr,Z_DEFLATED,6,0)!=ZIP_OK)throw std::runtime_error("cannot create ZIP member: "+name);
 if(zipWriteInFileInZip(z,data.data(),unsigned(data.size()))!=ZIP_OK){zipCloseFileInZip(z);throw std::runtime_error("write ZIP member failed");}
 if(zipCloseFileInZip(z)!=ZIP_OK)throw std::runtime_error("closing ZIP member failed");
}
inline std::string extract(unzFile z,const char* name){
 if(unzLocateFile(z,name,0)!=UNZ_OK)throw std::runtime_error(std::string("missing ZIP entry: ")+name);
 unz_file_info64 inf{};if(unzGetCurrentFileInfo64(z,&inf,nullptr,0,nullptr,0,nullptr,0)!=UNZ_OK||inf.uncompressed_size>64*1024*1024)throw std::runtime_error("ZIP size invalid");
 if(unzOpenCurrentFile(z)!=UNZ_OK)throw std::runtime_error("ZIP entry invalid");
 std::string out;out.reserve(size_t(inf.uncompressed_size));char block[16384];int r;
 while((r=unzReadCurrentFile(z,block,sizeof(block)))>0){out.append(block,r);if(out.size()>64*1024*1024)break;}
 int close=unzCloseCurrentFile(z);
 if(r<0||close!=UNZ_OK||out.size()!=inf.uncompressed_size)throw std::runtime_error(std::string("invalid ZIP content/CRC: ")+name);
 return out;
}
struct Package{Document doc;value layers;value operations;std::string curveSource;};
inline void writePackage(const std::filesystem::path& target,const Package& p,bool injectFailure=false){
 namespace fs=std::filesystem;
 const std::string geometry=serialize(encode(p.doc)),layers=serialize(p.layers),operations=serialize(p.operations);
 object manifest;manifest["format"]="ALENCAR-LASER-EXPERIMENTAL";manifest["schemaVersion"]=4;
 manifest["geometryDigestFNV1a64"]=digest(geometry);manifest["layersDigestFNV1a64"]=digest(layers);manifest["operationsDigestFNV1a64"]=digest(operations);manifest["curvesDigestFNV1a64"]=digest(p.curveSource);
 manifest["fabricationEnabled"]=false;manifest["description"]="TESTE_5 research-only; not a final .alaser specification";
 fs::path temp=target;temp+=".partial";
 fs::remove(temp);zipFile z=zipOpen64(temp.string().c_str(),APPEND_STATUS_CREATE);if(!z)throw std::runtime_error("cannot open temporary ZIP");
 try{addZip(z,"manifest.json",serialize(manifest));addZip(z,"design/geometry.json",geometry);addZip(z,"design/layers.json",layers);addZip(z,"fabrication/operations.json",operations);addZip(z,"design/original_bezier_samples.csv",p.curveSource);}
 catch(...){zipClose(z,nullptr);fs::remove(temp);throw;}
 if(zipClose(z,nullptr)!=ZIP_OK){fs::remove(temp);throw std::runtime_error("could not close ZIP");}
 if(injectFailure){fs::remove(temp);throw std::runtime_error("simulated interruption before atomic replacement");}
 // POSIX rename replaces atomically. On Windows use QSaveFile/ReplaceFileW with flush/backup in product.
 fs::rename(temp,target);
}
inline Package readPackage(const std::filesystem::path& file){
 unzFile z=unzOpen64(file.string().c_str());if(!z)throw std::runtime_error("invalid or truncated ZIP");
 std::string manifest,geometry,layers,operations,curves;
 try{manifest=extract(z,"manifest.json");geometry=extract(z,"design/geometry.json");layers=extract(z,"design/layers.json");operations=extract(z,"fabrication/operations.json");curves=extract(z,"design/original_bezier_samples.csv");}catch(...){unzClose(z);throw;}
 unzClose(z);auto m=parse(manifest).as_object();
 if(m.at("format").as_string()!="ALENCAR-LASER-EXPERIMENTAL"||m.at("schemaVersion").as_int64()!=4)throw std::runtime_error("unknown schema version");
 if(m.at("geometryDigestFNV1a64").as_string()!=digest(geometry)||m.at("layersDigestFNV1a64").as_string()!=digest(layers)||m.at("operationsDigestFNV1a64").as_string()!=digest(operations)||m.at("curvesDigestFNV1a64").as_string()!=digest(curves))throw std::runtime_error("geometry or resources have been modified");
 Package result;result.doc=decode(parse(geometry).as_object());result.layers=parse(layers);result.operations=parse(operations);result.curveSource=std::move(curves);return result;
}
}
