// Alencar Laser TESTE 19.8 — source-editable text and alignment, offline C++20.
#include "../src/editor_tools.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace alaser;
static bool near(double a,double b){return std::abs(a-b)<1e-7;}
int main(){
 Editor editor(Document{});
 EditableText source;source.text="ALENCAR";source.fontFamily="Arial";
 source.heightMm=12;source.spacingPercent=100;source.bold=true;
 source.align=1;source.anchor={10,20};
 const int root=editor.addEditableText(
  {{{10,20},{20,20},{20,30},{10,30}},
   {{25,20},{30,20},{30,30},{25,30}}},source,0);
 assert(editor.state().editableTexts.at(root).members.size()==2);
 assert(editor.state().objects.at(root).group>0);
 const auto loaded=fromMetadata(editor.state().doc,metadata(editor.state()));
 assert(loaded.editableTexts.at(root).text=="ALENCAR");
 assert(loaded.editableTexts.at(root).fontFamily=="Arial");
 assert(loaded.editableTexts.at(root).bold);
 const auto before=editor.undoCount();
 source.text="LASER";source.italic=true;
 editor.replaceEditableText(root,
  {{{10,20},{35,20},{35,35},{10,35}}},source);
 assert(editor.undoCount()==before+1);
 assert(editor.state().editableTexts.at(root).text=="LASER");
 assert(editor.state().objects.count(root));
 assert(editor.undo()&&editor.state().editableTexts.at(root).text=="ALENCAR");
 assert(editor.redo()&&editor.state().editableTexts.at(root).text=="LASER");
 const auto again=fromMetadata(editor.state().doc,metadata(editor.state()));
 assert(again.editableTexts.at(root).text=="LASER");
 // Existing legacy metadata without editableTexts remains readable.
 auto legacy=metadata(editor.state());legacy.erase("editableTexts");
 const auto old=fromMetadata(editor.state().doc,legacy);
 assert(old.editableTexts.empty());
 // Converting editable text must not alter geometry, IDs, or layers and
 // must be reversible with a single Undo / Redo.
 Editor flattened(editor.state());
 const int firstTextMember=flattened.state().editableTexts.at(root).members.back();
 const auto textNodeCount=countNodes(flattened.state().doc);
 const auto originalLayer=flattened.state().objects.at(root).layer;
 const auto changesBefore=flattened.undoCount();
 assert(flattened.convertEditableTextToPaths(firstTextMember));
 assert(flattened.undoCount()==changesBefore+1);
 assert(flattened.state().editableTexts.empty()&&
        countNodes(flattened.state().doc)==textNodeCount);
 assert(flattened.state().objects.at(root).layer==originalLayer&&
        flattened.state().objects.at(root).group==0);
 assert(flattened.undo()&&flattened.state().editableTexts.at(root).text=="LASER");
 assert(flattened.redo()&&flattened.state().editableTexts.empty());
 assert(!flattened.convertEditableTextToPaths(root));
 // Alignment: X center, Y center, left and right, all using a reference shape.
 Editor align(Document{});
 const int a=align.rectangle({0,0},10,10);
 const int b=align.rectangle({30,40},20,5);
 align.alignObjects({a,b},4,a);
 auto ba=boundsOf(align.state().doc,{a}),bb=boundsOf(align.state().doc,{b});
 assert(near(ba.first.x+ba.second.x,bb.first.x+bb.second.x));
 align.alignObjects({a,b},5,a);
 ba=boundsOf(align.state().doc,{a});bb=boundsOf(align.state().doc,{b});
 assert(near(ba.first.y+ba.second.y,bb.first.y+bb.second.y));
 align.alignObjects({a,b},0,a);
 ba=boundsOf(align.state().doc,{a});bb=boundsOf(align.state().doc,{b});
 assert(near(ba.first.x,bb.first.x));
 align.alignObjects({a,b},1,a);
 ba=boundsOf(align.state().doc,{a});bb=boundsOf(align.state().doc,{b});
 assert(near(ba.second.x,bb.second.x));
 // A group is aligned as one unit without changing relative geometry.
 Editor grouped(Document{});
 const int g1=grouped.rectangle({10,10},5,5);
 const int g2=grouped.rectangle({25,10},5,5);
 const int target=grouped.rectangle({100,40},12,12);
 grouped.group({g1,g2});
 const auto beforeG1=boundsOf(grouped.state().doc,{g1});
 const auto beforeG2=boundsOf(grouped.state().doc,{g2});
 const auto history=grouped.undoCount();
 grouped.alignObjects({g1,target},0,target);
 const auto afterG1=boundsOf(grouped.state().doc,{g1});
 const auto afterG2=boundsOf(grouped.state().doc,{g2});
 assert(near(afterG1.first.x,100.));
 assert(near(afterG2.first.x-afterG1.first.x,beforeG2.first.x-beforeG1.first.x));
 assert(grouped.undoCount()==history+1);
 assert(grouped.undo()&&near(boundsOf(grouped.state().doc,{g1}).first.x,
                             beforeG1.first.x));
 const auto snapshot=metadata(align.state());
 assert(align.undo());
 assert(metadata(align.state())==snapshot); // geometry changes, metadata stays unchanged
 std::cout<<"PASS: editable text metadata/redo/legacy, 4 align modes; no hardware\n";
}
