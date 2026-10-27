#pragma once
// TESTE 19.6: restrict Wi-Fi diagnostics to manual IPv4 private LAN addresses.
// Port 23 is the documented AlgoLaser/LightBurn starting default. No network discovery.
#include <cstdint>
#include <string_view>
#include <charconv>
namespace alaser::tcp196 {
inline bool privateIpv4(std::string_view s){
 unsigned a[4]{};size_t start=0;
 for(int i=0;i<4;++i){
  size_t end=i==3?s.size():s.find('.',start);
  if(end==std::string_view::npos||end<=start||end-start>3)return false;
  auto chunk=s.substr(start,end-start);unsigned v=0;
  auto result=std::from_chars(chunk.data(),chunk.data()+chunk.size(),v);
  if(result.ec!=std::errc{}||result.ptr!=chunk.data()+chunk.size()||v>255)return false;
  a[i]=v;start=end+1;
 }
 return (a[0]==10)||(a[0]==192&&a[1]==168)||(a[0]==172&&a[1]>=16&&a[1]<=31);
}
inline bool validPort(unsigned port){return port>0&&port<=65535;}
}
