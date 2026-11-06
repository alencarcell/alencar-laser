#pragma once
// Alencar Laser 19.8 -- VIEW-ONLY bridge from native Bezier v5 to the
// polyline-based offline planner. The original editable cubic data and IDs
// are NEVER replaced or modified. No GRBL, G-code, serial or TCP calls.
#include "editor_tools.hpp"
#include "bezier_native.hpp"
#include <algorithm>
#include <climits>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace alaser::sim198 {
struct NativePreview {
 EditorState state;
 std::size_t paths=0,closed=0,open=0,segments=0,missingLayers=0,unverifiedLayerParameters=0;
};
// Native paths are exact in the saved .alaser. The flattened copy exists
// ONLY for motion, screen preview and even/odd Fill calculations.
inline NativePreview withNativeCurves(const EditorState& source,
                                     const CurveDocument* native,
                                     double toleranceMm=0.08,
                                     std::size_t maxPreviewNodes=80000){
 if(!(toleranceMm>=0.01&&toleranceMm<=0.5)||maxPreviewNodes<2)
  throw std::invalid_argument("Invalid offline Bezier flatten tolerance");
 NativePreview out;out.state=source;
 if(!native||native->contours.empty())return out;
 validateCurve(*native);
 int nextObject=0;
 for(const auto& [id,meta]:source.objects){(void)meta;nextObject=std::max(nextObject,id);}
 for(const auto& c:source.doc.contours)nextObject=std::max(nextObject,c.key.object);
 int64_t nextNode=out.state.doc.nextId;
 std::size_t nodes=0;
 for(const auto& c:native->contours){
  if(nextObject>=INT_MAX-1)throw std::runtime_error("Native preview object ID capacity exceeded");
  std::vector<Pt> points;
  for(std::size_t edge=0;edge<edgeCount(c);++edge){
   const std::vector<Pt> segment=c.edgeKind[edge]?
      flatten(edgeAt(c,edge),toleranceMm):
      std::vector<Pt>{edgeAt(c,edge).p0,edgeAt(c,edge).p3};
   if(segment.size()<2)throw std::runtime_error("Native preview degenerate edge");
   if(nodes+points.size()+segment.size()>maxPreviewNodes)
    throw std::runtime_error("Native preview geometry exceeds node safety limit");
   // Do not duplicate adjacent anchors or close by repeating the first point:
   // the Contour.closed flag is the sole closing-edge authority.
   for(std::size_t j=points.empty()?0:1;j<segment.size();++j)
    if(points.empty()||dist(points.back(),segment[j])>1e-9)
     points.push_back(segment[j]);
  }
  if(c.closed&&points.size()>2&&dist(points.front(),points.back())<1e-8)
   points.pop_back();
  if(points.size()<2||(c.closed&&points.size()<3))
   throw std::runtime_error("Native preview invalid open/closed path");
  nodes+=points.size();
  Contour poly;poly.key={++nextObject,0};poly.closed=c.closed;
  poly.nodes.reserve(points.size());
  for(const Pt p:points)poly.nodes.push_back({nextNode++,p});
  out.state.doc.contours.push_back(std::move(poly));
  // Only the offline preview may substitute a missing layer; the editable
  // native curve remains on its original CutIndex. Unverified metadata is
  // counted separately, even after a missing layer has been repaired.
  int assigned=c.layer;
  const auto layer=source.layers.find(assigned);
  if(layer==source.layers.end()){
   ++out.missingLayers;assigned=source.layers.begin()->first;
  }else if(!layer->second.parametersVerified){
   ++out.unverifiedLayerParameters;
  }
  out.state.objects[nextObject]={assigned,0};
  ++out.paths;out.closed+=c.closed?1:0;out.open+=c.closed?0:1;
  out.segments+=points.size()-(c.closed?0:1);
 }
 out.state.doc.nextId=nextNode;
 validate(out.state);
 return out;
}
} // namespace alaser::sim198
