#pragma once
// TESTE 19.5 - Windows serial observer. Explicit port selection is required.
// Opening a USB serial port MAY reset certain controller boards. Never automatic.
#include "grbl_observer.hpp"
#include <string>
#include <vector>
#include <stdexcept>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
namespace alaser::grbl {
class WindowsSerialObserver {
 HANDLE handle_=INVALID_HANDLE_VALUE;
public:
 WindowsSerialObserver()=default;
 ~WindowsSerialObserver(){close();}
 WindowsSerialObserver(const WindowsSerialObserver&)=delete;
 WindowsSerialObserver& operator=(const WindowsSerialObserver&)=delete;
 static std::vector<std::string> ports(){
  std::vector<std::string> found;
  for(int i=1;i<=64;i++){
   const std::wstring name=L"COM"+std::to_wstring(i);
   wchar_t target[4096]{};
   if(QueryDosDeviceW(name.c_str(),target,4096))found.push_back("COM"+std::to_string(i));
  }
  return found;
 }
 bool connected()const{return handle_!=INVALID_HANDLE_VALUE;}
 void close(){if(connected()){CloseHandle(handle_);handle_=INVALID_HANDLE_VALUE;}}
 void open(const std::string& name){
  close();
  const auto portsFound=ports();
  bool valid=false;for(const auto& p:portsFound)if(p==name)valid=true;
  if(!valid)throw std::runtime_error("serial port unavailable");
  std::wstring port=L"\\\\.\\";
  for(char c:name)port+=static_cast<wchar_t>(c);
  HANDLE h=CreateFileW(port.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_EXISTING,0,nullptr);
  if(h==INVALID_HANDLE_VALUE)throw std::runtime_error("cannot open selected port (used by another application?)");
  DCB dcb{};dcb.DCBlength=sizeof(dcb);
  if(!GetCommState(h,&dcb)){CloseHandle(h);throw std::runtime_error("serial configuration not readable");}
  dcb.BaudRate=CBR_115200;dcb.ByteSize=8;dcb.Parity=NOPARITY;dcb.StopBits=ONESTOPBIT;
  dcb.fOutxCtsFlow=FALSE;dcb.fOutxDsrFlow=FALSE;
  dcb.fDtrControl=DTR_CONTROL_DISABLE;dcb.fRtsControl=RTS_CONTROL_DISABLE;
  if(!SetCommState(h,&dcb)){CloseHandle(h);throw std::runtime_error("cannot set 115200/8N1");}
  COMMTIMEOUTS timeouts{};
  timeouts.ReadIntervalTimeout=MAXDWORD;
  timeouts.ReadTotalTimeoutMultiplier=0;timeouts.ReadTotalTimeoutConstant=0;
  timeouts.WriteTotalTimeoutMultiplier=0;timeouts.WriteTotalTimeoutConstant=200;
  if(!SetCommTimeouts(h,&timeouts)){CloseHandle(h);throw std::runtime_error("cannot configure nonblocking reads");}
  handle_=h;
 }
 std::string readAvailable(){
  if(!connected())return {};
  COMSTAT stat{};DWORD error=0;
  if(!ClearCommError(handle_,&error,&stat))throw std::runtime_error("serial read status failed");
  if(stat.cbInQue==0)return {};
  char buf[2048];DWORD got=0;
  const DWORD wanted=stat.cbInQue<sizeof(buf)?stat.cbInQue:sizeof(buf);
  if(!ReadFile(handle_,buf,wanted,&got,nullptr))throw std::runtime_error("serial read failed");
  return std::string(buf,buf+got);
 }
 void request(Probe which,const Observation& obs){
  if(!connected())throw std::runtime_error("not connected");
  const std::string bytes=probeBytes(which,obs.grblBanner,obs.state);
  DWORD written=0;
  if(!WriteFile(handle_,bytes.data(),DWORD(bytes.size()),&written,nullptr)||written!=bytes.size())
   throw std::runtime_error("diagnostic query failed");
 }
};
}
#else
namespace alaser::grbl {
class WindowsSerialObserver {
public:
 static std::vector<std::string> ports(){return {};}
 bool connected()const{return false;}
 void close(){}
 void open(const std::string&){throw std::runtime_error("Windows-only serial observer");}
 std::string readAvailable(){return {};}
 void request(Probe,const Observation&){throw std::runtime_error("Windows-only serial observer");}
};
}
#endif
