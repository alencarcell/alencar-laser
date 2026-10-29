#pragma once
// Alencar Laser TESTE_5 - experimental editable geometry, no machine commands.
#include "geometry.hpp"
#include "scissors_span.hpp"
#include <boost/json.hpp>
#include <set>
#include <map>
#include <functional>
#include <optional>
#include <tuple>
#include <string>
#include <unordered_set>
#include <numbers>

namespace alaser {
struct Layer {
 int id=0;std::string name;bool visible=true,locked=false;
 std::string mode="Line"; // modo editorial: Line ou Fill
 bool output=true,air=false; // controle editorial, nunca autorizacao de emissao
 double speedMmMin=3000.0,powerPercent=20.0; // valores virtuais, NUNCA hardware
 int passes=1;
 std::string powerMode="M4"; // M4=dinamica / M3=constante (somente metadados)
 bool optimizeTravel=true; // reordenacao VIRTUAL na previa apenas
};
struct Meta {int layer=0,group=0;};
struct EditableText {
 std::string text,fontFamily;
 double heightMm=12,spacingPercent=100;
 bool bold=false,italic=false;
 int align=0; // 0 left, 1 center, 2 right
 Pt anchor{},sourceBoundsMin{};
 std::vector<int> members;
};
struct EditorState {
 Document doc;std::map<int,Meta> objects;std::map<int,Layer> layers;
 int nextGroup=1;std::map<int,EditableText> editableTexts;
};
inline size_t countNodes(const Document& d){size_t n=0;for(auto& c:d.contours)n+=c.nodes.size();return n;}
inline void validate(const EditorState& s){
 std::set<Key> keys;std::set<int64_t> ids;std::set<int> objs;int64_t maxId=0;
 for(auto& c:s.doc.contours){
  if(!keys.insert(c.key).second)throw std::runtime_error("duplicate contour key");
  if(c.closed&&c.nodes.size()<3)throw std::runtime_error("invalid closed contour");
  objs.insert(c.key.object);
  for(auto& n:c.nodes){if(n.id<=0||!ids.insert(n.id).second||!std::isfinite(n.p.x)||!std::isfinite(n.p.y))throw std::runtime_error("invalid node");maxId=std::max(maxId,n.id);}
 }
 if(s.doc.nextId<=maxId||s.doc.nextId<1)throw std::runtime_error("invalid next node id");
 if(s.layers.empty())throw std::runtime_error("no layers");
 for(const auto& [id,layer]:s.layers)
  if(layer.mode!="Line"&&layer.mode!="Fill")throw std::runtime_error("invalid layer mode");
  else if(!std::isfinite(layer.speedMmMin)||layer.speedMmMin<10||layer.speedMmMin>100000||
          !std::isfinite(layer.powerPercent)||layer.powerPercent<0||layer.powerPercent>100||
          layer.passes<1||layer.passes>20||
          (layer.powerMode!="M3"&&layer.powerMode!="M4"))
   throw std::runtime_error("invalid layer simulation parameters");
 for(auto obj:objs){auto i=s.objects.find(obj);if(i==s.objects.end()||!s.layers.count(i->second.layer)||i->second.group<0)throw std::runtime_error("unregistered object/layer");}
 for(auto& [id,meta]:s.objects)if(!objs.count(id))throw std::runtime_error("metadata on missing object");
 for(const auto& [root,t]:s.editableTexts){
  if(t.text.empty()||t.text.size()>600||t.fontFamily.empty()||
     t.fontFamily.size()>250||t.members.empty()||t.members.front()!=root||
     !std::isfinite(t.heightMm)||t.heightMm<1||t.heightMm>150||
     !std::isfinite(t.spacingPercent)||t.spacingPercent<60||t.spacingPercent>200||
     !std::isfinite(t.anchor.x)||!std::isfinite(t.anchor.y)||
     !std::isfinite(t.sourceBoundsMin.x)||!std::isfinite(t.sourceBoundsMin.y)||
     t.align<0||t.align>2)throw std::runtime_error("invalid editable text");
  std::set<int> members;
  for(int id:t.members)if(!members.insert(id).second||!objs.count(id))
   throw std::runtime_error("missing text member");
 }
}
inline EditorState fromDocument(Document d){
 EditorState s;s.doc=std::move(d);s.layers[0]={0,"Padrão",true,false};
 for(auto& c:s.doc.contours)s.objects.try_emplace(c.key.object,Meta{});
 validate(s);return s;
}
inline bool editable(const EditorState& s,int obj){
 auto it=s.objects.find(obj);if(it==s.objects.end())return false;
 auto li=s.layers.find(it->second.layer);return li!=s.layers.end()&&li->second.visible&&!li->second.locked;
}
inline Pt centroid(const Contour& c){Pt sum{};if(c.nodes.empty())return sum;for(auto& n:c.nodes)sum=sum+n.p;return sum*(1.0/c.nodes.size());}
inline Pt rotateAround(Pt p,Pt origin,double degrees){
 double r=degrees*std::numbers::pi/180.;double cs=std::cos(r),sn=std::sin(r);Pt v=p-origin;
 return {origin.x+v.x*cs-v.y*sn,origin.y+v.x*sn+v.y*cs};
}
inline std::pair<Pt,Pt> boundsOf(const Document& d,const std::vector<int>& objs){
 std::set<int> selected(objs.begin(),objs.end());Pt lo{std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity()};Pt hi{-lo.x,-lo.y};
 for(auto& c:d.contours)if(selected.count(c.key.object))for(auto& n:c.nodes){lo.x=std::min(lo.x,n.p.x);lo.y=std::min(lo.y,n.p.y);hi.x=std::max(hi.x,n.p.x);hi.y=std::max(hi.y,n.p.y);}
 if(!std::isfinite(lo.x))throw std::runtime_error("empty bounds");
 return {lo,hi};
}
// Endpoint snapping is pixel-scaled by the Qt view and converted to mm.
// Only editable, ungrouped, single-subpath open contours on the chosen layer
// are eligible. Never weld other layers, closed paths or group members.
struct PathEndpointSnap {
 Key key{};
 bool atStart=false;
 Pt point{};
 double distanceMm=0;
};
inline std::optional<PathEndpointSnap> nearestOpenEndpoint(
 const EditorState& state,Pt point,int layer,double toleranceMm,
 std::optional<Key> exclude=std::nullopt){
 if(!std::isfinite(point.x)||!std::isfinite(point.y)||
    !std::isfinite(toleranceMm)||toleranceMm<=0||toleranceMm>100)
  throw std::runtime_error("invalid endpoint snap request");
 std::optional<PathEndpointSnap> winner;
 double best=toleranceMm;
 bool ambiguous=false;
 std::map<int,int> pathCount;
 for(const auto& c:state.doc.contours)++pathCount[c.key.object];
 for(const auto& c:state.doc.contours){
  if(c.closed||c.nodes.size()<2||c.nodes.size()>50000||
     (exclude&&*exclude==c.key))continue;
  const auto it=state.objects.find(c.key.object);
  if(it==state.objects.end()||it->second.layer!=layer||
     it->second.group!=0||!editable(state,c.key.object))continue;
  // Objects with multiple subpaths are not welded, preserving identity.
  if(pathCount[c.key.object]!=1)continue;
  for(int endpoint=0;endpoint<2;++endpoint){
   const Pt candidate=endpoint?c.nodes.back().p:c.nodes.front().p;
   const double distance=dist(point,candidate);
   if(distance>toleranceMm)continue;
   // Two endpoints at virtually the same distance are ambiguous: no snap.
   if(winner&&std::abs(distance-best)<1e-6&&
      (winner->key!=c.key||winner->atStart!=(endpoint==0))){
    ambiguous=true;continue;
   }
   if(!winner||distance<best-1e-6){
    winner=PathEndpointSnap{c.key,endpoint==0,candidate,distance};
    best=distance;ambiguous=false;
   }
  }
 }
 return ambiguous?std::nullopt:winner;
}
class Editor{
 EditorState current;
 std::vector<EditorState> past,future;
 size_t maxHistory=40;
 template<class Fn> void change(Fn fn,bool changesGeometry=true){
  EditorState next=current;fn(next);
  for(auto it=next.editableTexts.begin();it!=next.editableTexts.end();){
   bool intact=true;
   for(int id:it->second.members)
    if(!next.objects.count(id)){intact=false;break;}
   if(!intact)it=next.editableTexts.erase(it);else ++it;
  }
  validate(next);
  past.push_back(current);if(past.size()>maxHistory)past.erase(past.begin());
  current=std::move(next);future.clear();++generation;if(changesGeometry)++geometryGeneration;
 }
 static int nextObjectId(const Document& d){int maxObj=0;for(auto& c:d.contours)maxObj=std::max(maxObj,c.key.object);if(maxObj>=INT32_MAX)throw std::runtime_error("object ID exhausted");return maxObj+1;}
 static void apply(EditorState& st,const std::vector<int>&objects,const std::function<Pt(Pt)>& transform){
  if(objects.empty())throw std::runtime_error("empty selection");
  std::set<int> ids(objects.begin(),objects.end());
  // Select one member of a group -> transform the whole group atomically.
  std::set<int> groups;
  for(int id:ids){auto it=st.objects.find(id);if(it==st.objects.end())throw std::runtime_error("missing selection");if(it->second.group>0)groups.insert(it->second.group);}
  for(const auto& [id,meta]:st.objects)if(meta.group>0&&groups.count(meta.group))ids.insert(id);
  for(int id:ids)if(!editable(st,id))throw std::runtime_error("object not editable");
  bool found=false;
  for(auto& c:st.doc.contours)if(ids.count(c.key.object)){found=true;for(auto& n:c.nodes)n.p=transform(n.p);}
  if(!found)throw std::runtime_error("objects missing");
 }
public:
 uint64_t generation=0;
 // Identidade da geometria: alterações exclusivas de metadados não invalidam o índice.
 uint64_t geometryGeneration=0;
 explicit Editor(Document d):current(fromDocument(std::move(d))){}
 explicit Editor(EditorState s):current(std::move(s)){validate(current);}
 const EditorState& state()const{return current;}
 size_t undoCount()const{return past.size();}
 size_t redoCount()const{return future.size();}
 bool undo(){if(past.empty())return false;future.push_back(current);current=std::move(past.back());past.pop_back();++generation;++geometryGeneration;return true;}
 bool redo(){if(future.empty())return false;past.push_back(current);current=std::move(future.back());future.pop_back();++generation;++geometryGeneration;return true;}
 int addContour(std::vector<Pt> points,bool closed,int layerId=0){
  if(points.size()<(closed?3U:2U))throw std::runtime_error("not enough nodes");
  int obj=nextObjectId(current.doc);
  change([&](EditorState& s){
   auto li=s.layers.find(layerId);if(li==s.layers.end()||!li->second.visible||li->second.locked)throw std::runtime_error("invalid/locked layer");
   Contour c;c.key={obj,0};c.closed=closed;
   for(Pt p:points){if(!std::isfinite(p.x)||!std::isfinite(p.y))throw std::runtime_error("invalid point");c.nodes.push_back({s.doc.nextId++,p});}
   s.doc.contours.push_back(std::move(c));s.objects[obj]={layerId,0};
  });return obj;
 }
 // Insercao vetorial atomica: todas as letras/contornos contam como UMA acao
 // de desfazer, sem criar historico para cada glifo separadamente.
 std::vector<int> addContours(std::vector<std::vector<Pt>> contours,bool closed,int layerId=0,bool groupResult=false){
  if(contours.empty()||contours.size()>1000)throw std::runtime_error("contornos em lote fora do limite");
  size_t nodes=0;
  for(const auto& c:contours){
   if(c.size()<(closed?3U:2U))throw std::runtime_error("contorno vazio no lote");
   nodes+=c.size();if(nodes>50000)throw std::runtime_error("contornos em lote excedem 50 mil nos");
  }
  const int start=nextObjectId(current.doc);
  if(start>INT32_MAX-int(contours.size()))throw std::runtime_error("IDs de objetos esgotados");
  std::vector<int> ids;ids.reserve(contours.size());
  change([&](EditorState& state){
   auto li=state.layers.find(layerId);
   if(li==state.layers.end()||!li->second.visible||li->second.locked)
    throw std::runtime_error("camada de texto bloqueada ou invisivel");
   int next=start;
   const int groupId=groupResult&&contours.size()>1?state.nextGroup++:0;
   for(const auto& polygon:contours){
    Contour contour;contour.key={next,0};contour.closed=closed;
    for(Pt point:polygon){
     if(!std::isfinite(point.x)||!std::isfinite(point.y))
      throw std::runtime_error("coordenada invalida");
     contour.nodes.push_back({state.doc.nextId++,point});
    }
    state.doc.contours.push_back(std::move(contour));
    state.objects[next]={layerId,groupId};
    ids.push_back(next++);
   }
  });return ids;
 }
 std::optional<std::pair<int,EditableText>> findEditableText(int objectId)const{
  for(const auto& [id,t]:current.editableTexts)
   if(std::find(t.members.begin(),t.members.end(),objectId)!=t.members.end())
    return std::make_pair(id,t);
  return std::nullopt;
 }
 int addEditableText(std::vector<std::vector<Pt>> polygons,EditableText text,int layer){
  if(polygons.empty()||polygons.size()>1000||text.text.empty())
   throw std::runtime_error("invalid text polygons");
  std::size_t n=0;
  for(const auto& poly:polygons){if(poly.size()<3)throw std::runtime_error("text path invalid");n+=poly.size();}
  if(n>50000)throw std::runtime_error("text exceeds node limit");
  const int root=nextObjectId(current.doc);
  if(root>INT32_MAX-int(polygons.size()))throw std::runtime_error("text IDs exhausted");
  change([&](EditorState& state){
   if(!state.layers.count(layer)||!state.layers.at(layer).visible||
      state.layers.at(layer).locked)throw std::runtime_error("text layer not editable");
   text.members.clear();
   int id=root;
   const int group=polygons.size()>1?state.nextGroup++:0;
   for(const auto& poly:polygons){
    Contour c;c.key={id,0};c.closed=true;
    for(Pt p:poly)c.nodes.push_back({state.doc.nextId++,p});
    state.doc.contours.push_back(std::move(c));
    state.objects[id]={layer,group};text.members.push_back(id++);
   }
   text.sourceBoundsMin=boundsOf(state.doc,text.members).first;
   state.editableTexts[root]=text;
  });
  return root;
 }
 void replaceEditableText(int root,std::vector<std::vector<Pt>> polygons,
                          EditableText updated){
  if(polygons.empty()||polygons.size()>1000||updated.text.empty())
   throw std::runtime_error("invalid text replacement");
  size_t count=0;
  for(const auto& poly:polygons){if(poly.size()<3)throw std::runtime_error("invalid glyph");count+=poly.size();}
  if(count>50000)throw std::runtime_error("text exceeds node limit");
  change([&](EditorState& state){
   auto old=state.editableTexts.find(root);
   if(old==state.editableTexts.end())throw std::runtime_error("no editable text");
   auto previous=old->second;
   for(int id:previous.members)if(!editable(state,id))
    throw std::runtime_error("text member locked");
   const Meta meta=state.objects.at(root);
   std::set<int> remove(previous.members.begin(),previous.members.end());
   state.doc.contours.erase(std::remove_if(state.doc.contours.begin(),state.doc.contours.end(),
      [&](const Contour& c){return remove.count(c.key.object)>0;}),state.doc.contours.end());
   for(int id:remove)state.objects.erase(id);
   int next=std::max(root+1,nextObjectId(state.doc));
   updated.members.clear();
   for(std::size_t i=0;i<polygons.size();++i){
    const int id=i==0?root:next++;
    Contour c;c.key={id,0};c.closed=true;
    for(Pt p:polygons[i])c.nodes.push_back({state.doc.nextId++,p});
    state.doc.contours.push_back(std::move(c));
    state.objects[id]=meta;updated.members.push_back(id);
   }
   updated.sourceBoundsMin=boundsOf(state.doc,updated.members).first;
   state.editableTexts[root]=updated;
  });
 }
 // Explicit Alt+J action for TWO selected open contours; closest pair of
 // free endpoints must be within tolerance. Only one history transaction.
 int joinTwoOpenContours(int firstObject,int secondObject,double toleranceMm=0.5){
  if(firstObject==secondObject||toleranceMm<=0||toleranceMm>5||
     !std::isfinite(toleranceMm))
   throw std::runtime_error("select two different open contours to join");
  const Contour* a=nullptr;const Contour* b=nullptr;
  int countA=0,countB=0;
  for(const auto& c:current.doc.contours){
   if(c.key.object==firstObject){a=&c;++countA;}
   if(c.key.object==secondObject){b=&c;++countB;}
  }
  if(!a||!b||countA!=1||countB!=1||a->closed||b->closed||
     a->nodes.size()<2||b->nodes.size()<2)
   throw std::runtime_error("join requires two simple open contours");
  const auto ma=current.objects.find(firstObject),mb=current.objects.find(secondObject);
  if(ma==current.objects.end()||mb==current.objects.end()||
     ma->second.layer!=mb->second.layer||ma->second.group!=0||
     mb->second.group!=0||!editable(current,firstObject)||
     !editable(current,secondObject))
   throw std::runtime_error("join requires same editable ungrouped layer");
  double best=std::numeric_limits<double>::infinity();
  bool aStarts=false,bEnds=false;
  for(int i=0;i<2;++i)for(int j=0;j<2;++j){
   const double d=dist(i?a->nodes.front().p:a->nodes.back().p,
                       j?b->nodes.back().p:b->nodes.front().p);
   if(d<best){best=d;aStarts=(i==1);bEnds=(j==1);}
  }
  if(best>toleranceMm)
   throw std::runtime_error("selected endpoints are too far apart");
  const Key firstKey=a->key,secondKey=b->key;
  change([&](EditorState& state){
   Contour* head=nullptr;Contour* tail=nullptr;
   for(auto& c:state.doc.contours){
    if(c.key==firstKey)head=&c;
    if(c.key==secondKey)tail=&c;
   }
   if(!head||!tail)throw std::runtime_error("joined contour missing");
   std::vector<Node> points=head->nodes,other=tail->nodes;
   if(aStarts)std::reverse(points.begin(),points.end());
   if(bEnds)std::reverse(other.begin(),other.end());
   const bool coincide=dist(points.back().p,other.front().p)<1e-9;
   points.insert(points.end(),other.begin()+(coincide?1:0),other.end());
   state.doc.contours.erase(std::remove_if(
      state.doc.contours.begin(),state.doc.contours.end(),
      [&](const Contour& c){return c.key==secondKey;}),state.doc.contours.end());
   state.objects.erase(secondObject);
   for(auto& c:state.doc.contours)if(c.key==firstKey){
    c.nodes=std::move(points);break;
   }
  });
  return firstObject;
 }
 // Commit one drawn path, optionally stitching matching OPEN endpoints.
 // One undo item. Never modifies a closed shape, a locked layer, another
 // layer, grouped objects, or ambiguous endpoints. Source IDs are preserved.
 int commitJoinedPolyline(std::vector<Pt> draft,bool closed,int layer,
                          double toleranceMm=0.05){
  if(draft.size()<2||draft.size()>20000||(closed&&draft.size()<3))
   throw std::runtime_error("invalid path drawing");
  for(const Pt p:draft)
   if(!std::isfinite(p.x)||!std::isfinite(p.y))
    throw std::runtime_error("path contains invalid coordinates");
  if(!current.layers.count(layer)||!current.layers.at(layer).visible||
     current.layers.at(layer).locked)throw std::runtime_error("layer not editable");
  // Only preserve explicitly snapped coordinates within controlled tolerance.
  const auto first=closed?std::optional<PathEndpointSnap>{}:
    nearestOpenEndpoint(current,draft.front(),layer,toleranceMm);
  const auto last=closed?std::optional<PathEndpointSnap>{}:
    nearestOpenEndpoint(current,draft.back(),layer,toleranceMm);
  if(first)draft.front()=first->point;
  if(last)draft.back()=last->point;
  if(first&&last&&first->key==last->key&&first->atStart==last->atStart)
   throw std::runtime_error("cannot join same open endpoint twice");
  int resultingObject=0;
  change([&](EditorState& state){
   auto byKey=[&](Key k)->Contour*{
    for(auto& c:state.doc.contours)if(c.key==k)return &c;
    throw std::runtime_error("snap target disappeared");
   };
   auto appendPoints=[&](std::vector<Node>& to,const std::vector<Pt>& points,
                         std::size_t begin,std::size_t end){
    for(std::size_t i=begin;i<end;++i)
     if(to.empty()||dist(to.back().p,points[i])>1e-9)
      to.push_back({state.doc.nextId++,points[i]});
   };
   if(first&&last&&first->key==last->key){
    if(first->atStart==last->atStart)
     throw std::runtime_error("invalid self join");
    Contour* target=byKey(first->key);
    if(first->atStart){
     // Draft goes source start -> source end: reverse its inside points.
     std::reverse(draft.begin(),draft.end());
    }
    appendPoints(target->nodes,draft,1,draft.size()-1);
    if(target->nodes.size()<3)throw std::runtime_error("closed contour too small");
    target->closed=true;
    resultingObject=target->key.object;
   }else if(first||last){
    std::vector<Node> path;
    if(first){
     Contour* head=byKey(first->key);
     path=head->nodes;
     if(first->atStart)std::reverse(path.begin(),path.end());
     appendPoints(path,draft,1,draft.size());
     resultingObject=head->key.object;
    }else{
     appendPoints(path,draft,0,draft.size()-1);
     Contour* tail=byKey(last->key);
     std::vector<Node> tailPoints=tail->nodes;
     if(!last->atStart)std::reverse(tailPoints.begin(),tailPoints.end());
     path.insert(path.end(),tailPoints.begin(),tailPoints.end());
     resultingObject=tail->key.object;
    }
    if(first&&last){
     // Prevent an orphaned subpath: second target must be a distinct
     // ungrouped one-contour object in the SAME layer (pre-filtered above).
     Contour* tail=byKey(last->key);
     std::vector<Node> tailPoints=tail->nodes;
     if(!last->atStart)std::reverse(tailPoints.begin(),tailPoints.end());
     path.insert(path.end(),tailPoints.begin()+1,tailPoints.end());
     const Key second=last->key;
     state.doc.contours.erase(std::remove_if(state.doc.contours.begin(),
        state.doc.contours.end(),[&](const Contour& c){return c.key==second;}),
        state.doc.contours.end());
     state.objects.erase(second.object);
    }
    byKey(first?first->key:last->key)->nodes=std::move(path);
   }else{
    resultingObject=nextObjectId(state.doc);
    Contour c;c.key={resultingObject,0};c.closed=closed;
    appendPoints(c.nodes,draft,0,draft.size());
    state.doc.contours.push_back(std::move(c));
    state.objects[resultingObject]={layer,0};
   }
  });
  return resultingObject;
 }
 int line(Pt a,Pt b,int layer=0){if(dist(a,b)<1e-12)throw std::runtime_error("zero-length line");return addContour({a,b},false,layer);}
 int rectangle(Pt a,double w,double h,int layer=0){if(!std::isfinite(w)||!std::isfinite(h)||w<=0||h<=0)throw std::runtime_error("invalid rectangle");return addContour({a,{a.x+w,a.y},{a.x+w,a.y+h},{a.x,a.y+h}},true,layer);}
 int ellipse(Pt center,double rx,double ry,int steps=64,int layer=0){
  if(!std::isfinite(rx)||!std::isfinite(ry)||rx<=0||ry<=0||steps<12||steps>4096)throw std::runtime_error("invalid ellipse");
  std::vector<Pt> p;p.reserve(steps);for(int i=0;i<steps;i++){double theta=2*std::numbers::pi*i/steps;p.push_back({center.x+rx*std::cos(theta),center.y+ry*std::sin(theta)});}return addContour(std::move(p),true,layer);
 }
 int regularPolygon(Pt center,double radius,int sides,int layer=0){
  if(!std::isfinite(radius)||radius<=0||sides<3||sides>4096)throw std::runtime_error("invalid polygon");
  std::vector<Pt> p;for(int i=0;i<sides;i++){double a=2*std::numbers::pi*i/sides;p.push_back({center.x+radius*std::cos(a),center.y+radius*std::sin(a)});}return addContour(std::move(p),true,layer);
 }
 void moveObjects(std::vector<int> ids,Pt delta){change([&](EditorState& s){apply(s,ids,[&](Pt p){return p+delta;});});}
 // Exclusão atômica de formas inteiras, com expansão dos agrupamentos.
 // Camadas bloqueadas/invisíveis nunca são alteradas por uma seleção parcial.
 void deleteObjects(std::vector<int> ids){
  if(ids.empty())throw std::runtime_error("select objects before deleting");
  change([&](EditorState& s){
   std::set<int> expanded(ids.begin(),ids.end()),groups;
   for(int id:expanded){
    const auto it=s.objects.find(id);
    if(it==s.objects.end())throw std::runtime_error("missing object to delete");
    if(it->second.group>0)groups.insert(it->second.group);
   }
   for(const auto& [id,m]:s.objects)if(m.group>0&&groups.count(m.group))expanded.insert(id);
   for(int id:expanded)if(!editable(s,id))throw std::runtime_error("object locked or hidden");
   s.doc.contours.erase(std::remove_if(s.doc.contours.begin(),s.doc.contours.end(),
    [&](const Contour& c){return expanded.count(c.key.object)>0;}),s.doc.contours.end());
   for(int id:expanded)s.objects.erase(id);
  });
 }

 // Tesoura (TESTE_19.7): secciona um segmento de polilinha no ponto projetado.
 // Caminho ABERTO: cria duas formas, sem apagar nenhuma aresta.
 // Caminho FECHADO: abre no ponto do clique (inicio/fim coincidindo), sem eliminar geometria.
 // Nao altera outras subtrilhas do objeto, nao modifica curvas Bezier nativas nem grupos.
 // Uma unica transacao e um unico passo de desfazer/refazer.
 int cutPolylineEdge(Key key,int edge,Pt point){
  const Contour* source=get(current.doc,key);
  if(!source)throw std::runtime_error("contorno inexistente");
  if(!editable(current,key.object))throw std::runtime_error("camada invisivel ou bloqueada");
  if(current.objects.at(key.object).group>0)throw std::runtime_error("desagrupe antes de usar tesoura");
  const int n=int(source->nodes.size());
  if(n<2||edge<0||edge>=n-(source->closed?0:1))throw std::runtime_error("segmento invalido");
  const Pt a=source->nodes[size_t(edge)].p,b=source->nodes[size_t((edge+1)%n)].p;
  const Pt ab=b-a;double length2=norm2(ab);
  if(!std::isfinite(point.x)||!std::isfinite(point.y)||length2<1e-16)
   throw std::runtime_error("segmento degenerado");
  const double t=((point.x-a.x)*ab.x+(point.y-a.y)*ab.y)/length2;
  const Pt exact=a+ab*t;
  if(t<=0.0001||t>=0.9999||dist(exact,point)>1e-6)
   throw std::runtime_error("clique no interior da linha, afastado dos nos");
  const int newObject=source->closed?key.object:nextObjectId(current.doc);
  change([&](EditorState& state){
   Contour* original=get(state.doc,key);
   if(!original)throw std::runtime_error("contorno removido");
   const auto nodes=original->nodes;
   if(original->closed){
    std::vector<Node> opened;opened.reserve(size_t(n)+2);
    opened.push_back({state.doc.nextId++,exact});
    for(int j=1;j<=n;j++)opened.push_back(nodes[size_t((edge+j)%n)]);
    opened.push_back({state.doc.nextId++,exact});
    original->nodes=std::move(opened);original->closed=false;
   }else{
    std::vector<Node> first(nodes.begin(),nodes.begin()+edge+1);
    first.push_back({state.doc.nextId++,exact});
    std::vector<Node> second;
    second.reserve(size_t(n-edge));
    second.push_back({state.doc.nextId++,exact});
    second.insert(second.end(),nodes.begin()+edge+1,nodes.end());
    original->nodes=std::move(first);
    Contour other{{newObject,0},std::move(second),false};
    state.doc.contours.push_back(std::move(other));
    auto meta=state.objects.at(key.object);meta.group=0;
    state.objects.emplace(newObject,meta);
   }
  });
  return newObject;
 }
 // Tesoura de remocao: apaga a aresta clicada em vez de apenas abrir sua costura.
 // Um contorno fechado vira caminho aberto sem a aresta selecionada.
 // Um caminho aberto separa os trechos restantes, descartando subcaminhos
 // com menos de dois nos. Uma transacao/undo, respeitando camada e grupos.
 // Retorna 0 se o objeto foi completamente removido.
 int removePolylineEdge(Key key,int edge){
  const Contour* source=get(current.doc,key);
  if(!source)throw std::runtime_error("contorno inexistente");
  if(!editable(current,key.object))throw std::runtime_error("camada invisivel ou bloqueada");
  if(current.objects.at(key.object).group>0)throw std::runtime_error("desagrupe antes de usar tesoura");
  const int n=int(source->nodes.size());
  if(n<2||edge<0||edge>=n-(source->closed?0:1))throw std::runtime_error("segmento invalido");
  const int newObject=nextObjectId(current.doc);
  int result=key.object;
  change([&](EditorState& state){
   Contour* original=get(state.doc,key);
   if(!original)throw std::runtime_error("contorno removido");
   const auto nodes=original->nodes;
   if(original->closed){
    std::vector<Node> opened;
    opened.reserve(size_t(n));
    for(int i=0;i<n;i++)opened.push_back(nodes[size_t((edge+1+i)%n)]);
    original->nodes=std::move(opened);
    original->closed=false;
    return;
   }
   std::vector<Node> left(nodes.begin(),nodes.begin()+edge+1);
   std::vector<Node> right(nodes.begin()+edge+1,nodes.end());
   const bool leftValid=left.size()>=2,rightValid=right.size()>=2;
   if(leftValid&&rightValid){
    original->nodes=std::move(left);
    state.doc.contours.push_back({{newObject,0},std::move(right),false});
    auto meta=state.objects.at(key.object);meta.group=0;
    state.objects.emplace(newObject,meta);
    result=newObject;
   }else if(leftValid){
    original->nodes=std::move(left);
   }else if(rightValid){
    original->nodes=std::move(right);
   }else{
    state.doc.contours.erase(std::remove_if(state.doc.contours.begin(),state.doc.contours.end(),
     [&](const Contour& c){return c.key==key;}),state.doc.contours.end());
    bool remains=false;
    for(const auto& c:state.doc.contours)if(c.key.object==key.object){remains=true;break;}
    if(!remains)state.objects.erase(key.object);
    result=0;
   }
  });
  return result;
 }
 // Corte por INTERVALO CONTINUO: o preview escolhido e a geometria removida
 // usam os mesmos parametros. Os demais contornos nao sao modificados.
 // Uma unica transacao cria no maximo dois restos independentes.
 int removeScissorSpan(const ScissorSpan& span){
  const Contour* source=get(current.doc,span.key);
  if(!source)throw std::runtime_error("contorno de tesoura ausente");
  const bool logicalClosed=source->closed||hasExactDuplicateClosure(*source);
  if(logicalClosed!=span.closed)throw std::runtime_error("geometria de tesoura mudou");
  if(!editable(current,span.key.object))throw std::runtime_error("camada bloqueada ou invisivel");
  if(current.objects.at(span.key.object).group>0)throw std::runtime_error("desagrupe antes de usar tesoura");
  const int m=int(source->nodes.size())-(source->closed?0:1);
  if(m<=0||!std::isfinite(span.from)||!std::isfinite(span.to)||
     span.to<=span.from+1e-7||span.to-span.from>m+1e-7||
     (!logicalClosed&&(span.from<0||span.to>m))||
     (logicalClosed&&(span.from< -m||span.from>=m||span.to>2.*m)))
    throw std::runtime_error("intervalo de tesoura invalido");
  const int nextObject=nextObjectId(current.doc);
  int selectedResult=span.key.object;
  change([&](EditorState& state){
   Contour* original=get(state.doc,span.key);
   if(!original)throw std::runtime_error("contorno removido");
   // O fecho virtual fica persistente APENAS no contorno editado, no
   // instante em que a pessoa confirma o corte. Undo restaura o original.
   const Contour before=scissorLogicalContour(*original);
   const int n=int(before.nodes.size());
   auto nodeAt=[&](double u)->Node{
    const double value=before.closed?wrapScissorParam(u,m):u;
    const double rounded=std::round(value);
    if(std::abs(value-rounded)<1e-8){
     int j=int(rounded);
     if(before.closed)j%=n;
     return before.nodes.at(size_t(j));
    }
    return Node{state.doc.nextId++,pointAt(before,value)};
   };
   auto piece=[&](double a,double b)->std::vector<Node>{
    if(b-a<=1e-8)return {};
    std::vector<Node> nodes{nodeAt(a)};
    for(int j=int(std::floor(a))+1;double(j)<b-1e-8;j++){
     int index=before.closed?((j%n+n)%n):j;
     nodes.push_back(before.nodes.at(size_t(index)));
    }
    nodes.push_back(nodeAt(b));
    nodes.erase(std::unique(nodes.begin(),nodes.end(),[](const Node& x,const Node& y){
     return dist(x.p,y.p)<1e-10;
    }),nodes.end());
    if(nodes.size()<2)return {};
    return nodes;
   };
   if(before.closed){
    auto remainder=piece(span.to,span.from+m);
    if(remainder.size()>=2){
     original->nodes=std::move(remainder);original->closed=false;
    }else{
     state.doc.contours.erase(std::remove_if(state.doc.contours.begin(),state.doc.contours.end(),
      [&](const Contour& c){return c.key==span.key;}),state.doc.contours.end());
     bool others=false;
     for(const auto& c:state.doc.contours)if(c.key.object==span.key.object){others=true;break;}
     if(!others)state.objects.erase(span.key.object);
     selectedResult=0;
    }
   }else{
    auto left=piece(0,span.from),right=piece(span.to,double(m));
    if(left.size()>=2&&right.size()>=2){
     original->nodes=std::move(left);original->closed=false;
     state.doc.contours.push_back({{nextObject,0},std::move(right),false});
     Meta metadata=state.objects.at(span.key.object);metadata.group=0;
     state.objects.emplace(nextObject,metadata);selectedResult=nextObject;
    }else if(left.size()>=2){
     original->nodes=std::move(left);original->closed=false;
    }else if(right.size()>=2){
     original->nodes=std::move(right);original->closed=false;
    }else{
     state.doc.contours.erase(std::remove_if(state.doc.contours.begin(),state.doc.contours.end(),
      [&](const Contour& c){return c.key==span.key;}),state.doc.contours.end());
     bool others=false;
     for(const auto& c:state.doc.contours)if(c.key.object==span.key.object){others=true;break;}
     if(!others)state.objects.erase(span.key.object);
     selectedResult=0;
    }
   }
  });
  return selectedResult;
 }
 void rotateObjects(std::vector<int> ids,Pt pivot,double angle){change([&](EditorState& s){apply(s,ids,[&](Pt p){return rotateAround(p,pivot,angle);});});}
 void scaleObjects(std::vector<int> ids,Pt pivot,double sx,double sy){if(!std::isfinite(sx)||!std::isfinite(sy)||sx==0||sy==0)throw std::runtime_error("invalid scale");change([&](EditorState& s){apply(s,ids,[&](Pt p){return Pt{pivot.x+(p.x-pivot.x)*sx,pivot.y+(p.y-pivot.y)*sy};});});}
 void mirrorX(std::vector<int> ids,double axisX){change([&](EditorState& s){apply(s,ids,[&](Pt p){return Pt{2*axisX-p.x,p.y};});});}
 void mirrorY(std::vector<int> ids,double axisY){change([&](EditorState& s){apply(s,ids,[&](Pt p){return Pt{p.x,2*axisY-p.y};});});}
 int duplicate(int original,Pt offset){int obj=nextObjectId(current.doc);
  change([&](EditorState& s){if(!editable(s,original))throw std::runtime_error("cannot duplicate locked object");
   bool found=false;std::vector<Contour> additions;
   for(const auto& c:s.doc.contours)if(c.key.object==original){Contour n=c;n.key.object=obj;
    for(auto& node:n.nodes){node.id=s.doc.nextId++;node.p=node.p+offset;}additions.push_back(std::move(n));found=true;}
   if(!found)throw std::runtime_error("missing duplicate source");
   for(auto& c:additions)s.doc.contours.push_back(std::move(c));
   auto meta=s.objects.at(original);meta.group=0;s.objects[obj]=meta;
  });return obj;
 }
 void alignLeft(std::vector<int> ids){
  if(ids.size()<2)throw std::runtime_error("align requires 2 objects");
  auto [lo,hi]=boundsOf(current.doc,ids);(void)hi;
  change([&](EditorState& s){std::map<int,double> minXs;for(auto& c:s.doc.contours)if(std::find(ids.begin(),ids.end(),c.key.object)!=ids.end())for(auto& n:c.nodes){auto& v=minXs[c.key.object];if(!std::isfinite(v)||v==0){} if(!minXs.count(c.key.object))v=n.p.x;}
   // Recompute accurately (including positions at x=0).
   minXs.clear();for(auto& c:s.doc.contours)if(std::find(ids.begin(),ids.end(),c.key.object)!=ids.end())for(auto& n:c.nodes){auto it=minXs.find(c.key.object);if(it==minXs.end())minXs.emplace(c.key.object,n.p.x);else it->second=std::min(it->second,n.p.x);}
   for(int obj:ids)if(!editable(s,obj)||!minXs.count(obj))throw std::runtime_error("align invalid object");
   for(auto& c:s.doc.contours)if(minXs.count(c.key.object)){double diff=lo.x-minXs.at(c.key.object);for(auto& n:c.nodes)n.p.x+=diff;}
  });
 }
 int group(std::vector<int> ids){if(ids.size()<2)throw std::runtime_error("group requires two");int gid=current.nextGroup;
  change([&](EditorState& s){for(int i:ids)if(!editable(s,i))throw std::runtime_error("cannot group inaccessible object");for(int i:ids)s.objects.at(i).group=gid;s.nextGroup++;},false);return gid;
 }
 void ungroup(int groupId){change([&](EditorState& s){bool had=false;for(auto& [k,m]:s.objects)if(m.group==groupId){m.group=0;had=true;}if(!had)throw std::runtime_error("unknown group");},false);}
 int ungroupSelection(const std::vector<int>& ids){
  if(ids.empty())throw std::runtime_error("select a group to ungroup");
  std::set<int> groups;
  for(int id:ids){
   auto it=current.objects.find(id);
   if(it==current.objects.end()||!editable(current,id))
    throw std::runtime_error("ungroup selection missing, locked or hidden");
   if(it->second.group>0)groups.insert(it->second.group);
  }
  if(groups.empty())throw std::runtime_error("selection contains no group");
  change([&](EditorState& s){
   for(auto& [id,meta]:s.objects)if(groups.count(meta.group)){
    if(!editable(s,id))throw std::runtime_error("group has locked or hidden member");
    meta.group=0;
   }
  },false);
  return int(groups.size());
 }
 void addLayer(int id,std::string name){change([&](EditorState& s){if(id<0||s.layers.count(id)||name.empty())throw std::runtime_error("invalid new layer");s.layers[id]={id,name,true,false,"Line",!(id==1001||id==1002),false};},false);}
 void setLayerFlags(int id,bool visible,bool locked){change([&](EditorState& s){if(!s.layers.count(id))throw std::runtime_error("missing layer");s.layers.at(id).visible=visible;s.layers.at(id).locked=locked;},false);}
 void setLayerPreviewFlags(int id,bool output,bool air){
  if(!current.layers.count(id))throw std::runtime_error("missing layer");
  if(id==1001||id==1002)output=false;
  if(current.layers.at(id).output==output&&current.layers.at(id).air==air)return;
  change([&](EditorState& st){st.layers.at(id).output=output;st.layers.at(id).air=air;},false);
 }
 void setLayerProcess(int id,double speedMmMin,double powerPercent,int passes,const std::string& powerMode){
  if(!current.layers.count(id))throw std::runtime_error("missing layer");
  if(!std::isfinite(speedMmMin)||speedMmMin<10||speedMmMin>100000||
     !std::isfinite(powerPercent)||powerPercent<0||powerPercent>100||
     passes<1||passes>20||(powerMode!="M3"&&powerMode!="M4"))
   throw std::runtime_error("invalid virtual process settings");
  if(id==1001||id==1002)throw std::runtime_error("tool layers cannot carry laser process");
  const auto& l=current.layers.at(id);
  if(l.speedMmMin==speedMmMin&&l.powerPercent==powerPercent&&
     l.passes==passes&&l.powerMode==powerMode)return;
  change([&](EditorState& state){
   auto& layer=state.layers.at(id);
   layer.speedMmMin=speedMmMin;layer.powerPercent=powerPercent;
   layer.passes=passes;layer.powerMode=powerMode;
  },false);
 }
 // Single undo transaction for the layer settings dialog. Never sends machine data.
 void configureLayer(int id,const Layer& desired){
  if(!current.layers.count(id)||id==1001||id==1002)
   throw std::runtime_error("invalid layer configuration target");
  if(desired.name.empty()||desired.name.size()>200||
     (desired.mode!="Line"&&desired.mode!="Fill")||
     !std::isfinite(desired.speedMmMin)||desired.speedMmMin<10||desired.speedMmMin>100000||
     !std::isfinite(desired.powerPercent)||desired.powerPercent<0||desired.powerPercent>100||
     desired.passes<1||desired.passes>20||
     (desired.powerMode!="M3"&&desired.powerMode!="M4"))
   throw std::runtime_error("invalid virtual layer configuration");
  const auto& present=current.layers.at(id);
  if(present.name==desired.name&&present.mode==desired.mode&&
     present.speedMmMin==desired.speedMmMin&&
     present.powerPercent==desired.powerPercent&&present.passes==desired.passes&&
     present.powerMode==desired.powerMode&&present.optimizeTravel==desired.optimizeTravel)return;
  change([&](EditorState& state){
   auto& item=state.layers.at(id);
   item.name=desired.name;item.mode=desired.mode;
   item.speedMmMin=desired.speedMmMin;item.powerPercent=desired.powerPercent;
   item.passes=desired.passes;item.powerMode=desired.powerMode;
   item.optimizeTravel=desired.optimizeTravel;
  },false);
 }
 void setLayerMode(int id,const std::string& mode){
  if(mode!="Line"&&mode!="Fill")throw std::runtime_error("invalid layer mode");
  auto it=current.layers.find(id);if(it==current.layers.end())throw std::runtime_error("missing layer");
  if(it->second.mode==mode)return;
  change([&](EditorState& s){s.layers.at(id).mode=mode;},false);
 }
 void moveToLayer(int object,int layer){change([&](EditorState& s){if(!editable(s,object)||!s.layers.count(layer)||s.layers.at(layer).locked)throw std::runtime_error("invalid layer move");s.objects.at(object).layer=layer;},false);}
 void moveSelectionToLayer(const std::vector<int>& ids,int layer){
  change([&](EditorState& s){if(ids.empty()||!s.layers.count(layer)||s.layers.at(layer).locked||!s.layers.at(layer).visible)throw std::runtime_error("invalid target layer");
   for(int id:ids)if(!editable(s,id))throw std::runtime_error("inaccessible source object");
   for(int id:ids)s.objects.at(id).layer=layer;},false);
 }
 void editNode(Key key,int64_t nodeId,Pt dest){change([&](EditorState& s){if(!editable(s,key.object)||!moveNode(s.doc,key,nodeId,dest))throw std::runtime_error("cannot edit node");});}
 void insert(Key key,size_t index,Pt p){change([&](EditorState& s){if(!editable(s,key.object))throw std::runtime_error("cannot insert node");insertNode(s.doc,key,index,p);});}
 void erase(Key key,int64_t id){change([&](EditorState& s){if(!editable(s,key.object))throw std::runtime_error("cannot erase node");eraseNode(s.doc,key,id);if(!s.doc.contours.empty()){
    bool remaining=false;for(auto& c:s.doc.contours)if(c.key.object==key.object){remaining=true;break;}if(!remaining)s.objects.erase(key.object);
   }else s.objects.clear();});}
 std::vector<Hit> selectableHits(Pt scenePoint,double toleranceScene)const{
  auto all=segments(current.doc);std::vector<int> eligible;eligible.reserve(all.size());for(size_t i=0;i<all.size();i++)if(editable(current,all[i].key.object))eligible.push_back(int(i));return evaluate(all,eligible,scenePoint,toleranceScene);
 }
};
// Cache unico de selecao para GUI e testes de regressao.
// Um evento de metadados nao modifica geometria. Undo/redo invalidam por seguranca.
// Filtragem das camadas ocorre a cada consulta para nunca selecionar objetos ocultos.
class CachedHits {
 std::vector<Segment> segs_;
 SegmentBVH bvh_;
 uint64_t version_=uint64_t(-1);
 size_t rebuilds_=0, direct_=0, indexed_=0;
public:
 void invalidate(){version_=uint64_t(-1);}
 size_t rebuilds()const{return rebuilds_;}
 size_t directQueries()const{return direct_;}
 size_t indexedQueries()const{return indexed_;}
 size_t segmentCount()const{return segs_.size();}
 std::vector<Hit> query(const Editor& editor,Pt position,double pixelRadius,double pxPerMillimeter,double dpr=1.){
  if(!(pixelRadius>0&&pxPerMillimeter>0&&dpr>0)||!std::isfinite(pixelRadius)||!std::isfinite(pxPerMillimeter)||!std::isfinite(dpr))throw std::runtime_error("invalid viewport scale/tolerance");
  if(version_!=editor.geometryGeneration){
   segs_=segments(editor.state().doc);
   bvh_.reset(segs_);version_=editor.geometryGeneration;++rebuilds_;
  }
  const double physicalScale=pxPerMillimeter*dpr;
  const double tol=pixelRadius/physicalScale;
  std::vector<Hit> hits;
  if(physicalScale<0.5){++direct_;hits=bruteHitsNoAlloc(segs_,position,tol);}
  else {++indexed_;hits=bvh_.hits(position,tol);}
  hits.erase(std::remove_if(hits.begin(),hits.end(),[&](const Hit& h){return !editable(editor.state(),h.key.object);}),hits.end());
  return hits;
 }
};
inline boost::json::object metadata(const EditorState& s){
 using namespace boost::json;object m;m["schema"]="editor-meta-1";m["nextGroup"]=s.nextGroup;array layers;
 for(auto&[id,l]:s.layers)layers.push_back(object{{"id",id},{"name",l.name},{"visible",l.visible},{"locked",l.locked},{"mode",l.mode},{"output",l.output},{"air",l.air},
   {"speedMmMin",l.speedMmMin},{"powerPercent",l.powerPercent},
   {"passes",l.passes},{"powerMode",l.powerMode},{"optimizeTravel",l.optimizeTravel}});
 m["layers"]=std::move(layers);array objects;
 for(auto&[id,o]:s.objects)objects.push_back(object{{"id",id},{"layer",o.layer},{"group",o.group}});
 m["objects"]=std::move(objects);
 array texts;
 for(const auto& [root,t]:s.editableTexts){
  array members;for(int id:t.members)members.push_back(id);
  texts.push_back(object{{"root",root},{"text",t.text},{"family",t.fontFamily},
   {"heightMm",t.heightMm},{"spacingPercent",t.spacingPercent},
   {"bold",t.bold},{"italic",t.italic},{"align",t.align},
   {"anchorX",t.anchor.x},{"anchorY",t.anchor.y},
   {"boundsX",t.sourceBoundsMin.x},{"boundsY",t.sourceBoundsMin.y},
   {"members",std::move(members)}});
 }
 m["editableTexts"]=std::move(texts);return m;
}
inline EditorState fromMetadata(Document d,const boost::json::value& v){
 EditorState s;s.doc=std::move(d);const auto& m=v.as_object();if(m.at("schema").as_string()!="editor-meta-1")throw std::runtime_error("metadata schema unsupported");s.nextGroup=int(m.at("nextGroup").as_int64());
 for(const auto& item:m.at("layers").as_array()){const auto& x=item.as_object();int id=int(x.at("id").as_int64());if(s.layers.count(id))throw std::runtime_error("duplicate layer");const auto* mode=x.if_contains("mode");
   const std::string layerMode=(mode&&mode->is_string())?
    std::string(mode->as_string()):"Line";
   s.layers[id]={id,std::string(x.at("name").as_string()),
    x.at("visible").as_bool(),x.at("locked").as_bool(),layerMode,
    id==1001||id==1002?false:(x.if_contains("output")?x.at("output").as_bool():true),
    x.if_contains("air")?x.at("air").as_bool():false,
    x.if_contains("speedMmMin")?x.at("speedMmMin").to_number<double>():3000.0,
    x.if_contains("powerPercent")?x.at("powerPercent").to_number<double>():20.0,
    x.if_contains("passes")?int(x.at("passes").as_int64()):1,
    x.if_contains("powerMode")?std::string(x.at("powerMode").as_string()):"M4",
    x.if_contains("optimizeTravel")?x.at("optimizeTravel").as_bool():true};}
 for(const auto& item:m.at("objects").as_array()){const auto& x=item.as_object();int id=int(x.at("id").as_int64());if(s.objects.count(id))throw std::runtime_error("duplicate object meta");s.objects[id]={int(x.at("layer").as_int64()),int(x.at("group").as_int64())};}
 if(const auto* texts=m.if_contains("editableTexts")){
  for(const auto& item:texts->as_array()){
   const auto& x=item.as_object();
   EditableText t;
   const int root=int(x.at("root").as_int64());
   t.text=std::string(x.at("text").as_string());
   t.fontFamily=std::string(x.at("family").as_string());
   t.heightMm=x.at("heightMm").to_number<double>();
   t.spacingPercent=x.at("spacingPercent").to_number<double>();
   t.bold=x.at("bold").as_bool();t.italic=x.at("italic").as_bool();
   t.align=int(x.at("align").as_int64());
   t.anchor={x.at("anchorX").to_number<double>(),x.at("anchorY").to_number<double>()};
   t.sourceBoundsMin={x.at("boundsX").to_number<double>(),x.at("boundsY").to_number<double>()};
   for(const auto& v:x.at("members").as_array())t.members.push_back(int(v.as_int64()));
   if(!s.editableTexts.emplace(root,std::move(t)).second)
    throw std::runtime_error("duplicate text root");
  }
 }
 validate(s);return s;
}
} // namespace alaser
