#pragma once
// TESTE_10: particionamento do tracado em arestas estaveis.
// A geometria canonica nao e alterada durante o arraste.
#include "geometry.hpp"
#include <set>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <cmath>
namespace alaser {
struct EdgeChunk {size_t first=0;std::vector<Segment> edges;};
struct ChunkChange {std::vector<size_t> dirtyChunkIds;size_t updatedEdges=0;};
class ChunkedContourPreview {
 const Contour* src_;
 size_t nodeIndex_=0, chunkSize_=256;
 Pt current_;
 std::vector<EdgeChunk> chunks_;
 static Segment edge(const Contour& c,size_t i,size_t moved,Pt current){
  size_t n=c.nodes.size();Pt a=i==moved?current:c.nodes[i].p;
  size_t j=(i+1)%n;Pt b=j==moved?current:c.nodes[j].p;
  return {c.key,int(i),a,b,std::min(a.x,b.x),std::max(a.x,b.x),std::min(a.y,b.y),std::max(a.y,b.y)};
 }
 void patchEdge(size_t index){auto&ch=chunks_[index/chunkSize_];ch.edges.at(index-ch.first)=edge(*src_,index,nodeIndex_,current_);}
public:
 ChunkedContourPreview(const Contour& c,int64_t nodeId,size_t chunkSize=256):src_(&c),chunkSize_(chunkSize){
  if(chunkSize<1||c.nodes.size()<2)throw std::runtime_error("chunk count or contour too small");
  auto it=std::find_if(c.nodes.begin(),c.nodes.end(),[&](const Node& n){return n.id==nodeId;});
  if(it==c.nodes.end())throw std::runtime_error("chunk drag node not found");
  nodeIndex_=size_t(it-c.nodes.begin());current_=it->p;
  auto edgeCount=c.nodes.size()-1+(c.closed?1:0);
  for(size_t start=0;start<edgeCount;start+=chunkSize_){
   EdgeChunk chunk;chunk.first=start;
   for(size_t i=start;i<std::min(edgeCount,start+chunkSize_);i++)chunk.edges.push_back(edge(c,i,nodeIndex_,current_));
   chunks_.push_back(std::move(chunk));
  }
 }
 size_t chunkCount()const{return chunks_.size();}
 size_t nodeCount()const{return src_->nodes.size();}
 size_t edgeCount()const{return src_->nodes.size()-1+(src_->closed?1:0);}
 size_t chunkSize()const{return chunkSize_;}
 const std::vector<EdgeChunk>& chunks()const{return chunks_;}
 const EdgeChunk& chunk(size_t id)const{return chunks_.at(id);}
 Pt current()const{return current_;}
 size_t index()const{return nodeIndex_;}
 ChunkChange update(Pt to){
  if(!std::isfinite(to.x)||!std::isfinite(to.y))throw std::runtime_error("nonfinite drag");
  current_=to;std::set<size_t> affected;
  if(nodeIndex_>0)affected.insert(nodeIndex_-1);
  else if(src_->closed)affected.insert(src_->nodes.size()-1);
  if(nodeIndex_+1<src_->nodes.size())affected.insert(nodeIndex_);
  else if(src_->closed)affected.insert(src_->nodes.size()-1);
  ChunkChange result;result.updatedEdges=affected.size();
  for(auto edgeIndex:affected){patchEdge(edgeIndex);result.dirtyChunkIds.push_back(edgeIndex/chunkSize_);}
  std::sort(result.dirtyChunkIds.begin(),result.dirtyChunkIds.end());
  result.dirtyChunkIds.erase(std::unique(result.dirtyChunkIds.begin(),result.dirtyChunkIds.end()),result.dirtyChunkIds.end());
  return result;
 }
 ChunkChange cancel(){return update(src_->nodes[nodeIndex_].p);}
 std::vector<Segment> flatten()const{
  std::vector<Segment> all;all.reserve(edgeCount());
  for(const auto&chunk:chunks_)all.insert(all.end(),chunk.edges.begin(),chunk.edges.end());
  return all;
 }
};
}
