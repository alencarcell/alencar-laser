// Alencar Laser phase20m - independent C++ atomic bulk duplicate gate.
#include "editor_tools.hpp"
#include <iostream>
#include <set>
#include <cmath>
using namespace alaser;
static void require(bool ok,const char*message){if(!ok)throw std::runtime_error(message);}
int main(int argc,char**argv){
 try{
  require(argc==2,"usage: test_bulk_duplicate_20m fixture.csv");
  Document original=readCsv(argv[1]);
  require(original.contours.size()==575,"fixture must contain 575 contours");
  Editor e(original);
  std::set<int> objectSet;
  for(const auto& c:e.state().doc.contours)objectSet.insert(c.key.object);
  require(objectSet.size()==575,"fixture must have 575 distinct objects");
  const std::vector<int> all(objectSet.begin(),objectSet.end());
  const auto baseline=e.state().doc.contours.size();
  const auto originalNodes=countNodes(e.state().doc);
  const auto undoStart=e.undoCount();
  const auto duplicated=e.duplicateObjects(all,{3,3});
  require(duplicated.size()==all.size(),"duplicate count mismatch");
  require(e.state().doc.contours.size()==2*baseline,"contour count incorrect");
  require(countNodes(e.state().doc)==2*originalNodes,"node count incorrect");
  require(e.undoCount()==undoStart+1,"bulk duplication not atomic");
  require(std::set<int>(duplicated.begin(),duplicated.end()).size()==all.size(),"IDs not unique");
  for(size_t i=0;i<all.size();++i){
   const auto* source=get(original,{all[i],0});
   const auto* clone=get(e.state().doc,{duplicated[i],0});
   require(source&&clone,"source/clone missing");
   require(source->nodes.size()==clone->nodes.size(),"different node count");
   require(e.state().objects.at(all[i]).layer==e.state().objects.at(duplicated[i]).layer,"layer lost");
   for(size_t j=0;j<source->nodes.size();++j){
    require(std::abs((clone->nodes[j].p.x-source->nodes[j].p.x)-3.)<1e-8 &&
      std::abs((clone->nodes[j].p.y-source->nodes[j].p.y)-3.)<1e-8,
      "geometry offset mismatch");
   }
  }
  require(e.undo(),"undo failed");
  require(e.state().doc.contours.size()==baseline,"undo did not restore state");
  require(e.redo(),"redo failed");
  require(e.state().doc.contours.size()==2*baseline,"redo did not restore clones");

  Editor grouping(original);
  const int a=all.at(0),b=all.at(1);
  const int oldGroup=grouping.group({a,b});
  const auto grouped=grouping.duplicateObjects({a},{3,3});
  require(grouped.size()==2,"group members not expanded");
  const int newGroup=grouping.state().objects.at(grouped[0]).group;
  require(newGroup>0&&newGroup!=oldGroup,"group identity collision");
  require(grouping.state().objects.at(grouped[1]).group==newGroup,"group not preserved");

  Editor blocked(original);
  blocked.setLayerFlags(0,true,true);
  const auto before=blocked.state().doc.contours.size();
  const auto beforeHistory=blocked.undoCount();
  bool rejected=false;
  try{blocked.duplicateObjects({a,b},{3,3});}
  catch(const std::exception&){rejected=true;}
  require(rejected,"locked layer was duplicated");
  require(blocked.state().doc.contours.size()==before &&
          blocked.undoCount()==beforeHistory,"rejected duplication modified state");

  Editor text(original);
  EditableText metadata;metadata.text="abc";metadata.fontFamily="Arial";
  metadata.anchor={4,5};metadata.sourceBoundsMin={0,0};
  int root=text.addEditableText({{{0,0},{2,0},{2,2}},{{3,0},{5,0},{5,2}}},metadata,0);
  const auto textClones=text.duplicateObjects({root},{3,3});
  require(textClones.size()==2,"editable text was not expanded");
  const auto info=text.findEditableText(textClones[0]);
  require(info.has_value()&&info->second.members.size()==2,"editable text metadata missing");
  require(std::abs(info->second.anchor.x-7.)<1e-8,"text anchor incorrect");
  // Stress regression: repeated practical batches must be rejected BEFORE
  // a copy of an already huge EditorState, and without changing history.
  Editor stress(original);
  auto firstBatch=stress.duplicateObjects(all,{3,3});
  require(firstBatch.size()==575,"stress first duplication failed");
  std::vector<int> selected1150;
  for(const auto& [id,meta]:stress.state().objects)selected1150.push_back(id);
  require(selected1150.size()==1150,"unexpected first stress total");
  stress.duplicateObjects(selected1150,{3,3});
  require(stress.state().doc.contours.size()==2300,"unexpected second stress total");
  std::vector<int> selected2000;
  for(const auto& [id,meta]:stress.state().objects){
   if(selected2000.size()==2000)break;
   selected2000.push_back(id);
  }
  stress.duplicateObjects(selected2000,{3,3});
  require(stress.state().doc.contours.size()==4300,"unexpected third stress total");
  std::vector<int> selected1800(selected2000.begin(),selected2000.begin()+1800);
  const auto stableHistory=stress.undoCount();
  const auto stableContours=stress.state().doc.contours.size();
  const auto stableNodes=countNodes(stress.state().doc);
  bool blockedByGuard=false;
  try{stress.duplicateObjects(selected1800,{3,3});}
  catch(const std::exception& exc){
   blockedByGuard=std::string(exc.what()).find("bloqueada por seguranca")!=std::string::npos;
  }
  require(blockedByGuard,"excessive Ctrl+D must fail with safety message");
  require(stress.undoCount()==stableHistory&&
          stress.state().doc.contours.size()==stableContours&&
          countNodes(stress.state().doc)==stableNodes,
          "rejected bulk duplication changed state/history");
  std::cout<<"PASS BULK_DUPLICATE_20M: 575 objects, atomic Undo/Redo, geometry, group, locked layer, editable text; extreme load blocked safely\n";
  return 0;
 }catch(const std::exception&err){
  std::cerr<<"FAIL BULK_DUPLICATE_20M: "<<err.what()<<"\n";return 1;
 }
}
