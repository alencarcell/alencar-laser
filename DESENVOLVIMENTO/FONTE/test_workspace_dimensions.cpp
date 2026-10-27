#include "workspace_dimensions.hpp"
#include <cassert>
#include <limits>
#include <iostream>
using namespace alaser;
int main(){
 WorkspaceConfig c; c.validate();assert(c.drawingWidth==400&&c.machineHeight==400);
 c.drawingWidth=400;c.drawingHeight=800;c.validate();
 c.drawingWidth=800;c.drawingHeight=800;c.validate();
 c.drawingWidth=1000;c.drawingHeight=2000;c.machineWidth=400;c.machineHeight=400;c.validate();
 c.drawingWidth=100000;c.drawingHeight=150000;c.validate();
 for(double bad:{0.0,-1.0,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),1000001.0}){
  c.drawingWidth=bad;bool failed=false;try{c.validate();}catch(const std::invalid_argument&){failed=true;}assert(failed);
 }
 c.drawingWidth=400;c.profile.clear();bool failed=false;try{c.validate();}catch(const std::invalid_argument&){failed=true;}assert(failed);
 std::cout<<"TESTE_19.2 workspace dimensions PASS\n";
}
