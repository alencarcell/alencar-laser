#pragma once
// TESTE 19.5 - hard gate for read-only GRBL diagnostic probes.
// No generic serial-write method, G-code, reset, homing or realtime motion controls.
#include <array>
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
namespace alaser::grbl {
enum class Probe { Status, Firmware, Settings };
inline std::string probeBytes(Probe probe, bool bannerConfirmed, std::string_view state) {
 switch(probe) {
  case Probe::Status: return "?"; // GRBL realtime status; no newline
  case Probe::Firmware:
   if(bannerConfirmed && state=="Idle") return "$I\n";
   break;
  case Probe::Settings:
   if(bannerConfirmed && state=="Idle") return "$$\n";
   break;
 }
 throw std::runtime_error("diagnostic probe blocked: controller not confirmed idle");
}
struct Observation {
 bool grblBanner=false;
 std::string state="Unknown";
 size_t received=0;
 size_t ignored=0;
 std::vector<std::string> recent;
 void accept(std::string_view line) {
  // Strip controls and cap every line. Keep only 80 lines, in RAM.
  if(line.size()>600) { ++ignored; return; }
  std::string clean; clean.reserve(line.size());
  for(unsigned char c:line) if(c>=32 && c<127) clean+=char(c);
  if(clean.empty())return;
  ++received;
  if(clean.rfind("Grbl ",0)==0 || clean.rfind("grbl ",0)==0)grblBanner=true;
  if(clean[0]=='<' && clean.back()=='>') {
   const auto divider=clean.find('|');
   std::string candidate=clean.substr(1, divider==std::string::npos?clean.size()-2:divider-1);
   // Parse conservative recognized states only (case sensitive, family variants unknown).
   static constexpr std::array<std::string_view,9> states={"Idle","Run","Hold","Jog","Alarm","Door","Check","Home","Sleep"};
   bool recognized=false;
   for(auto key:states) if(candidate==key || (candidate.size()>key.size() && candidate.substr(0,key.size())==key && candidate[key.size()]==':')) {
    state=std::string(key);recognized=true;break;
   }
   if(!recognized)state="Unknown";
  } else if(clean.rfind("ALARM:",0)==0)state="Alarm";
  recent.push_back(std::move(clean));
  if(recent.size()>80)recent.erase(recent.begin());
 }
};
}
