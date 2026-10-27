// Leitor de validacao separado para arquivos alaser experimentais v4/v5.
#include "bezier_package.hpp"
#include <iostream>
using namespace alaser;
int main(int argc,char** argv){
 if(argc!=2){std::cerr<<"usage: verify13 file.alaser\n";return 2;}
 try{auto p=readIntegratedProject(argv[1]);validateCurve(p.curves);std::cout<<"VALID v"<<p.version<<" poly="<<p.legacy.doc.contours.size()<<" curve="<<p.curves.contours.size()<<"\n";return 0;}
 catch(const std::exception& e){std::cerr<<"REJECTED: "<<e.what()<<"\n";return 1;}
}
