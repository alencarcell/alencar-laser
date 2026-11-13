#pragma once
// Alencar Laser TESTE 19.5: coletor de eventos operacionais, opt-in e SOMENTE LOCAL.
// Sem caminhos de arquivo, dados pessoais, coordenadas ou telemetria de hardware.
#include <cstdint>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace alaser {
struct DiagnosticEvent {
 std::string type;
 int objects=0;
 std::int64_t elapsedMs=0;
};
class DiagnosticsCollector {
 using Clock=std::chrono::steady_clock;
 Clock::time_point origin_=Clock::now();
 std::chrono::milliseconds accumulated_{0};
 bool active_=false;
 std::vector<DiagnosticEvent> records_;
 std::map<std::string,int> counters_;
 size_t dropped_=0;
 static constexpr size_t limit_=2000;
public:
 bool active()const{return active_;}
 size_t limit()const{return limit_;}
 size_t dropped()const{return dropped_;}
 const std::vector<DiagnosticEvent>& records()const{return records_;}
 const std::map<std::string,int>& counters()const{return counters_;}
 std::int64_t elapsedMs()const{
  return (accumulated_+(active_?std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now()-origin_):
    std::chrono::milliseconds{0})).count();
 }
 void start(){if(!active_){active_=true;origin_=Clock::now();}}
 void stop(){if(active_){accumulated_+=std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now()-origin_);active_=false;}}
 void clear(){records_.clear();counters_.clear();dropped_=0;accumulated_=std::chrono::milliseconds{0};origin_=Clock::now();}
 bool record(const std::string& event,int objects=0){
  if(!active_)return false;
  // Only known action names; avoid logging arbitrary text or sensitive content.
  static const std::vector<std::string> allow={
   "shapes_moved","shapes_deleted","shape_created","bezier_contour_moved",
   "bezier_contour_deleted","bezier_node_deleted","bezier_node_moved",
   "undo","redo","project_saved","project_opened","command_error",
   "grbl_connected","grbl_disconnected","grbl_probe","grbl_received",
   "grbl_tcp_connect_requested","grbl_tcp_connected","grbl_tcp_disconnected",
   "grbl_tcp_error","objects_layer_changed",
   // Eventos reais usados pelo Qt 19.8: antes eram rejeitados silenciosamente.
   "scissor_no_hit","scissor_no_safe_span","scissor_continuous_span_removed",
    "bezier_edge_between_nodes_cut",
   "bezier_intersection_detected","bezier_intersection_cut",
   "bezier_scissor_no_intersection","bezier_scissor_rejected",
   "vector_text_created","groups_ungrouped","layer_mode_visual_changed",
   "lightburn_geometry_imported","marquee_selected","pencil_stroke_created",
   "tool_scissors_activated","tool_edge_trim_activated","tool_bezier_activated","bezier_curve_created","image_trace_started",
   "image_trace_created","image_trace_cancelled",
   // Native Bezier v5 edits use the same local opt-in diagnostics contract.
   // Keep the allowlist strict: these names never include path coordinates.
   "bezier_contour_scissor_cut","bezier_undo","bezier_redo",
   "native_draw_lines_created","bezier_node_inserted",
   "bezier_contour_scaled","bezier_contour_rotated",
   "polyline_node_deleted","polyline_edge_deleted"};
  if(std::find(allow.begin(),allow.end(),event)==allow.end())throw std::invalid_argument("unknown diagnostic event");
  if(objects<0)throw std::invalid_argument("negative item count");
  ++counters_[event];
  // Frequent passive GRBL traffic is counted, not stored as individual events.
  // Preserve edit actions and errors in the bounded event history.
  if(event=="grbl_received")return false;
  if(records_.size()==limit_){records_.erase(records_.begin());++dropped_;}
  records_.push_back({event,objects,elapsedMs()});return true;
 }
};
} // namespace alaser
