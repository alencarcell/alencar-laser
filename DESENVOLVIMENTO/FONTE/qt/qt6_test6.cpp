// Alencar Laser TESTE_15 — Qt6 integration laboratory (Bézier native + v5), not production software.
// GRBL 19.5: diagnostic observer only; no G-code, motion or laser operations.
#include "editor_tools.hpp"
#include "workspace_dimensions.hpp"
#include "lightburn_reference_inspector.hpp"
#include "lightburn_geometry_import.hpp"
#include "diagnostics_collector.hpp"
#include "persistent_session_store.hpp"
#include "recovery_policy.hpp"
#include "tool_glyphs.hpp"
#include "serial_observer_win.hpp"
#include "zoom_precision_196.hpp"
#include "grbl_tcp_rules_196.hpp"
#include "persistence.hpp"
#include "bezier_package.hpp"
#include "integrated_project_edit.hpp"
#include "bezier_interaction.hpp"
#include "raster_trace_19_8.hpp"
#include "render_chunks.hpp"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QSlider>
#include <QContextMenuEvent>
#include <array>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPen>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QWheelEvent>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSettings>
#include <QStandardPaths>
#include <QCloseEvent>
#include <QIcon>
#include <QPixmap>
#include <QPolygonF>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QTabWidget>
#include <QTcpSocket>
#include <QMenuBar>
#include <QHostAddress>
#include <QSpinBox>
#include <QTimer>
#include <QMessageBox>
#include <QInputDialog>
#include <QFontDialog>
#include <QFont>
#include <QToolButton>
#include <QCursor>
#include <QMenu>
#include <QScrollArea>
#include <QGridLayout>
#include <QGroupBox>
#include <QStyle>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QElapsedTimer>
#include <QColor>
#include <QDebug>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QSplitter>
#include <QSizePolicy>
#include <QPushButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QLineEdit>
#include <QFontMetrics>
#include <QEvent>
#include <QCheckBox>
#include <QLabel>
#include <QShortcut>
#include <QSaveFile>
#include <QPlainTextEdit>
#include <QDateTime>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include <QFileDialog>
#include <QSysInfo>
#include <QLineF>
#include <numbers>
#include <algorithm>
#include <iterator>
#include <QUuid>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#endif
#include <QPaintEvent>
#include <map>
#include <optional>
#include <cmath>
#include <stdexcept>
#include <filesystem>
#include <sstream>
static const QString processInstance=QUuid::createUuid().toString(QUuid::WithoutBraces);
using namespace alaser;
namespace fs=std::filesystem;
static void check(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
static QPointF toQt(Pt p){return{p.x,p.y};}
static Pt fromQt(QPointF p){return{p.x(),p.y()};}
static QPainterPath pathOf(const Contour& c){
 QPainterPath p;if(c.nodes.size()<2)return p;
 p.moveTo(toQt(c.nodes[0].p));for(size_t j=1;j<c.nodes.size();j++)p.lineTo(toQt(c.nodes[j].p));
 if(c.closed&&c.nodes.size()>=3)p.closeSubpath();
 return p;
}
// Curvas nativas renderizadas diretamente a partir de CurveDocument, sem polilinha intermediaria.
static QPainterPath nativeCurvePath(const CurveContour& contour){
 QPainterPath path;
 if(contour.nodes.empty())return path;
 path.moveTo(toQt(contour.nodes.front().anchor));
 for(size_t i=0;i<edgeCount(contour);++i){
  auto cubic=edgeAt(contour,i);
  if(contour.edgeKind.at(i)==1)path.cubicTo(toQt(cubic.p1),toQt(cubic.p2),toQt(cubic.p3));
  else path.lineTo(toQt(cubic.p3));
 }
 if(contour.closed)path.closeSubpath();
 return path;
}
static const CurveContour* findNativeCurve(const CurveDocument& doc,Key key){
 for(const auto& curve:doc.contours)if(curve.key==key)return &curve;
 return nullptr;
}
static QPainterPath pathFromChunk(const EdgeChunk& chunk){
 QPainterPath path;
 if(chunk.edges.empty())return path;
 path.moveTo(toQt(chunk.edges.front().a));
 for(const auto& edge:chunk.edges)path.lineTo(toQt(edge.b));
 return path;
}
static QPainterPath contourChunkPath(const Contour& c,size_t start,size_t n){
 QPainterPath path;
 if(c.nodes.empty()||n==0)return path;
 path.moveTo(toQt(c.nodes[start].p));
 for(size_t j=start;j<start+n;++j)path.lineTo(toQt(c.nodes[(j+1)%c.nodes.size()].p));
 return path;
}
// Previa O(N) sem clonar o contorno: uma unica sequencia de QPainterPath
// preserva exatamente joins e caps internos; somente um objeto e atualizado.
static QPainterPath previewNode(const Document& d,Key key,int64_t id,Pt pos){
 const auto*c=get(d,key);if(!c)throw std::runtime_error("preview contour absent");
 QPainterPath path;bool found=false;
 for(size_t i=0;i<c->nodes.size();++i){
  const auto&n=c->nodes[i];Pt p=n.p;
  if(n.id==id){p=pos;found=true;}
  if(i==0)path.moveTo(toQt(p));else path.lineTo(toQt(p));
 }
 if(!found)throw std::runtime_error("preview node absent");
 if(c->closed&&c->nodes.size()>=3)path.closeSubpath();
 return path;
}

class QtEditorLab final:public QGraphicsView{
 QGraphicsScene scene_;
 WorkspaceConfig workspace_{};
 bool workspaceGuides_=false;
 bool workspaceDark_=true;
 std::map<Key,QGraphicsPathItem*> objects_;
 std::map<int,std::vector<QGraphicsPathItem*>> itemsByObject_;
 std::set<int> styledObjects_;
 // O modo de curvas compartilha a MESMA viewport do editor integrado.
 // A sessao e propriedade de LabWindow; QtEditorLab nunca a libera.
 CurveDragSession* nativeSession_=nullptr;
 bool nativeMode_=false;
 bool fillPreview_=false; // Editor LightBurn mostra linhas por padrao.
 QImage rasterSource_; // preview temporario fora do arquivo .alaser
 QRectF rasterRect_;
 QGraphicsPixmapItem* rasterOverlay_=nullptr;
 std::map<Key,QGraphicsPathItem*> nativeItems_;
 std::optional<CurveHit> nativeSelectedPart_;
 std::optional<Key> nativeStyledKey_;
 bool nativePressing_=false;
 bool middlePanning_=false;
 QPointF middlePanLast_{};

 qint64 styleSetterCalls_=0;
 CachedHits picker_;
 std::optional<Key> active_;
 std::optional<int64_t> draggingNode_;
 Pt dragOriginal_{};QPointF dragStart_{};
 // Prévia de movimento: usa QGraphicsItem::pos, sem alterar geometria canônica.
 bool shapeDragging_=false;
 QPointF shapeDragStart_{};
 std::vector<QGraphicsPathItem*> shapePreviewItems_;
 // Alcas independentes dos dados; commit unico no mouseRelease.
 bool transformDragging_=false;
 int transformHandle_=-1; // 0..7 tamanho; 8..11 rotacao; 12 move pelo centro
 QRectF transformBounds_;
 QPointF transformAnchor_{};
 double transformSx_=1.,transformSy_=1.,transformAngle_=0.;
 QPointF transformMoveDelta_{};
 std::vector<int> transformIds_;
 std::vector<QGraphicsPathItem*> transformItems_;
 bool nativeContourPressing_=false;
 bool strokeDrawing_=false;QPointF drawStart_,drawCurrent_;
 std::vector<Pt> pencilPoints_;
 bool polylineDrawing_=false;
 bool polylineStartedOnPress_=false;
 QPointF polylineDown_{};
 std::vector<Pt> polylinePoints_;
 QPointF polylineCursor_{};
 QGraphicsPathItem* polylinePreview_=nullptr;
 bool marqueeActive_=false;
 QPointF marqueeStart_{};
 QGraphicsRectItem* marqueeItem_=nullptr;
 QGraphicsPathItem* pendingPreview_=nullptr;
 ScissorSpanPlanner scissorPlanner_;
 uint64_t scissorIndexVersion_=std::numeric_limits<uint64_t>::max();
 std::optional<ScissorSpan> scissorHover_;
 QGraphicsPathItem* scissorPreview_=nullptr;
 uint64_t scissorHoverEvents_=0;
 std::optional<QPoint> scissorLastHoverPosition_;
 // TESTE_12: caminho continuo e o modo seguro padrao.
 // Blocos sao EXCLUSIVAMENTE experimentais (ALENCAR_LAB_CHUNKS_EXPERIMENTAL=1).
 bool continuousPreview_=false;
 bool experimentalChunks_=qEnvironmentVariableIsSet("ALENCAR_LAB_CHUNKS_EXPERIMENTAL") &&
     qgetenv("ALENCAR_LAB_CHUNKS_EXPERIMENTAL")=="1";
 std::optional<ChunkedContourPreview> draggingChunks_;
 std::vector<QGraphicsPathItem*> dragChunkItems_;
 std::optional<Key> dragChunkKey_;
 int chunkPathsUpdated_=0,fullDragPathUpdates_=0,previewChunkCount_=0;
 void endChunkPreview(){
  for(auto* item:dragChunkItems_){scene_.removeItem(item);delete item;}
  dragChunkItems_.clear();draggingChunks_.reset();
  if(dragChunkKey_){
   auto it=objects_.find(*dragChunkKey_);
   if(it!=objects_.end()){
    // Cancela a previsualizacao sem jamais alterar o documento ou historico.
    if(continuousPreview_){
     if(const auto*c=get(editor.state().doc,*dragChunkKey_))it->second->setPath(pathOf(*c));
    }
    it->second->setVisible(true);
   }
  }
  continuousPreview_=false;dragChunkKey_.reset();
 }
 void startChunkPreview(Key key,int64_t nodeId){
  endChunkPreview();
  const auto*c=get(editor.state().doc,key);if(!c)throw std::runtime_error("drag contour absent");
  dragChunkKey_=key;
  if(!experimentalChunks_){
   // Um unico caminho e um unico QGraphicsPathItem: sem caps artificiais
   // no meio do contorno e sem dezenas de itens temporarios na cena.
   continuousPreview_=true;
   return;
  }
  const size_t edgeCount=c->nodes.size()-1+(c->closed?1:0);
  const size_t adaptiveChunkEdges=std::max<size_t>(256,(edgeCount+127)/128);
  draggingChunks_.emplace(*c,nodeId,adaptiveChunkEdges);
  auto* original=pathItem(key);
  for(const auto&ch:draggingChunks_->chunks()){
   auto* item=scene_.addPath(pathFromChunk(ch),original->pen());
   item->setZValue(original->zValue());dragChunkItems_.push_back(item);
  }
  previewChunkCount_=std::max(previewChunkCount_,int(dragChunkItems_.size()));
  original->setVisible(false);
 }

 int lastCreated_=0;
 Pt snapPolyline(Pt at,bool shift)const{
  if(!shift||polylinePoints_.empty())return at;
  const Pt last=polylinePoints_.back();
  const double dx=at.x-last.x,dy=at.y-last.y,length=std::hypot(dx,dy);
  if(length<1e-9)return at;
  const double step=std::numbers::pi/4.;
  const double angle=std::round(std::atan2(dy,dx)/step)*step;
  return {last.x+length*std::cos(angle),last.y+length*std::sin(angle)};
 }
 void redrawPolyline(){
  if(!polylinePreview_||polylinePoints_.empty())return;
  QPainterPath path;path.moveTo(toQt(polylinePoints_.front()));
  for(size_t i=1;i<polylinePoints_.size();++i)path.lineTo(toQt(polylinePoints_[i]));
  path.lineTo(polylineCursor_);polylinePreview_->setPath(path);
 }
 void clearMarquee(){
  marqueeActive_=false;
  if(marqueeItem_){scene_.removeItem(marqueeItem_);delete marqueeItem_;marqueeItem_=nullptr;}
 }
 void showMarquee(QPointF pointer){
  if(!marqueeActive_||QLineF(pointer,marqueeStart_).length()<3.)return;
  QRectF area(mapToScene(marqueeStart_.toPoint()),mapToScene(pointer.toPoint()));
  area=area.normalized();
  const QColor color=pointer.x()<marqueeStart_.x()?QColor("#38BC70"):QColor("#E65B5B");
  QPen pen(color,1.5,Qt::DashLine);pen.setCosmetic(true);
  if(!marqueeItem_){
   marqueeItem_=scene_.addRect(area,pen,QBrush(QColor(color.red(),color.green(),color.blue(),38)));
   marqueeItem_->setZValue(49);marqueeItem_->setAcceptedMouseButtons(Qt::NoButton);
  }else{
   marqueeItem_->setRect(area);marqueeItem_->setPen(pen);
   marqueeItem_->setBrush(QColor(color.red(),color.green(),color.blue(),38));
  }
 }
 void finishMarquee(QPointF pointer,Qt::KeyboardModifiers mods){
  if(!marqueeActive_)return;
  const QPointF start=marqueeStart_;
  clearMarquee();
  if(QLineF(pointer,start).length()<3.){
   if(!(mods&(Qt::ShiftModifier|Qt::ControlModifier)))clearSelection();
   if(stateChanged)stateChanged();return;
  }
  const bool crossing=pointer.x()<start.x();
  const QRectF area=QRectF(mapToScene(start.toPoint()),mapToScene(pointer.toPoint())).normalized();
  std::set<int> found;
  std::map<int,std::set<int>> groupMembers;
  std::map<int,bool> groupInside,groupCross;
  const auto& state=editor.state();
  for(const auto& [id,meta]:state.objects){
   if(!editable(state,id))continue;
   auto items=itemsByObject_.find(id);
   if(items==itemsByObject_.end())continue;
   bool inside=true,cross=false,has=false;
   for(auto* item:items->second){
    if(!item->isVisible())continue;
    has=true;
    const QRectF bounds=item->sceneBoundingRect();
    inside=inside&&area.contains(bounds);
    if(area.intersects(bounds)){
     const QPainterPath world=item->mapToScene(item->path());
     if(world.intersects(area)||area.contains(bounds))cross=true;
    }
   }
   if(!has)continue;
   if(meta.group>0){
    groupMembers[meta.group].insert(id);
    if(!groupInside.count(meta.group))groupInside[meta.group]=true;
    groupInside[meta.group]=groupInside[meta.group]&&inside;
    groupCross[meta.group]=groupCross[meta.group]||cross;
   }else if(crossing?cross:inside)found.insert(id);
  }
  for(const auto& [group,ids]:groupMembers)
   if(crossing?groupCross[group]:groupInside[group])
    found.insert(ids.begin(),ids.end());
  const bool shift=mods.testFlag(Qt::ShiftModifier);
  const bool ctrl=mods.testFlag(Qt::ControlModifier);
  if(!shift&&!ctrl)selected=std::move(found);
  else for(int id:found){
   if(shift&&ctrl)selected.erase(id);
   else if(ctrl){if(!selected.erase(id))selected.insert(id);}
   else selected.insert(id);
  }
  active_.reset();applySelectionStyles();viewport()->update();
  if(diagnosticEvent)diagnosticEvent("marquee_selected",int(selected.size()));
  if(stateChanged)stateChanged();
 }
 void clearPolyline(){
  polylineDrawing_=false;polylineStartedOnPress_=false;polylinePoints_.clear();
  if(polylinePreview_){scene_.removeItem(polylinePreview_);delete polylinePreview_;polylinePreview_=nullptr;}
  viewport()->update();
 }
 void finishPolyline(bool closed){
  if(!polylineDrawing_)return;
  auto points=std::move(polylinePoints_);clearPolyline();
  if(points.size()<2||(closed&&points.size()<3))return;
  try{
   lastCreated_=editor.addContour(std::move(points),closed,targetLayer());
   ++toolCreates;rebuild();activate({lastCreated_,0});
   if(diagnosticEvent)diagnosticEvent(closed?"polyline_closed":"polyline_created",1);
   if(stateChanged)stateChanged();
  }catch(const std::exception& ex){if(toolWarning)toolWarning(QString::fromUtf8(ex.what()));}
 }

 QElapsedTimer clock_;qint64 pendingInput_=-1;
 double nativePxPerMm()const{
  auto m=viewportTransform();return std::max(1e-9,std::min(
   std::hypot(m.m11(),m.m12()),std::hypot(m.m21(),m.m22())));
 }
 void updateNativePaths(bool all){
  if(!nativeSession_)return;
  auto selectedKey=nativeSession_->selected();
  // Durante drag, apenas o item ativo recebe setPath; selecao troca estilo
  // somente do item anterior/novo, nunca dos 575 outros contornos.
  bool selectionChanged=nativeStyledKey_!=selectedKey;
  for(const auto& c:nativeSession_->visible().contours){
   auto it=nativeItems_.find(c.key);if(it==nativeItems_.end())continue;
   const bool selected=selectedKey&&*selectedKey==c.key;
   const bool wasSelected=nativeStyledKey_&&*nativeStyledKey_==c.key;
   if(all||selected)it->second->setPath(nativeCurvePath(c));
   if(all||(selectionChanged&&(selected||wasSelected))){
    QPen pen(selected?QColor(239,109,25):QColor(71,91,105));
    pen.setCosmetic(true);pen.setWidthF(selected?2.0:1.2);
    pen.setCapStyle(Qt::RoundCap);pen.setJoinStyle(Qt::RoundJoin);
    it->second->setPen(pen);it->second->setZValue(selected?30:3);
   }
  }
  nativeStyledKey_=selectedKey;
  if(all||selectionChanged)viewport()->update();
 }

 std::vector<int> expandedSelection()const{
  std::set<int> ids=selected,groups;
  const auto& st=editor.state();
  for(int id:ids){
   auto it=st.objects.find(id);
   if(it!=st.objects.end()&&it->second.group>0)groups.insert(it->second.group);
  }
  for(const auto& [id,meta]:st.objects)
   if(meta.group>0&&groups.count(meta.group))ids.insert(id);
  return {ids.begin(),ids.end()};
 }
 std::optional<QRectF> selectedBounds()const{
  if(selected.empty()||tool!=Tool::Select||nativeMode_)return {};
  const auto ids=expandedSelection();
  const std::set<int> idsSet(ids.begin(),ids.end());
  double minX=0.,minY=0.,maxX=0.,maxY=0.;bool has=false;
  for(const auto& c:editor.state().doc.contours){
   if(!idsSet.count(c.key.object))continue;
   if(!editable(editor.state(),c.key.object))continue;
   for(const auto& node:c.nodes){
    const Pt p=node.p;
    if(!has){minX=maxX=p.x;minY=maxY=p.y;has=true;}
    else{minX=std::min(minX,p.x);minY=std::min(minY,p.y);
         maxX=std::max(maxX,p.x);maxY=std::max(maxY,p.y);}
   }
  }
  if(!has)return {};
  return QRectF(QPointF(minX,minY),QPointF(maxX,maxY));
 }
 std::array<QPointF,13> selectionHandles(const QRectF& bounds)const{
  const QPointF tl=viewportTransform().map(bounds.topLeft());
  const QPointF br=viewportTransform().map(bounds.bottomRight());
  const double x0=tl.x(),x1=br.x(),y0=tl.y(),y1=br.y();
  const double cx=(x0+x1)*.5,cy=(y0+y1)*.5;
  return {QPointF(x0,y0),QPointF(cx,y0),QPointF(x1,y0),
   QPointF(x1,cy),QPointF(x1,y1),QPointF(cx,y1),
   QPointF(x0,y1),QPointF(x0,cy),
   QPointF(x0-17,y0-17),QPointF(x1+17,y0-17),
   QPointF(x1+17,y1+17),QPointF(x0-17,y1+17),
   QPointF(cx,cy)};
 }
 void paintSelectionHandles(QPainter* p)const{
  if(tool!=Tool::Select||selected.empty()||nativeMode_)return;
  auto bounds=selectedBounds();if(!bounds)return;
  const QPointF topLeft=viewportTransform().map(bounds->topLeft());
  const QPointF bottomRight=viewportTransform().map(bounds->bottomRight());
  const QRectF pixel(topLeft,bottomRight);
  const auto handles=selectionHandles(*bounds);
  p->save();p->resetTransform();p->setBrush(Qt::NoBrush);
  QPen edge(QColor(238,154,60),1,Qt::DashLine);edge.setCosmetic(true);p->setPen(edge);
  p->drawRect(pixel.normalized());
  for(int h=0;h<13;++h){
   const QPointF point=handles[size_t(h)];
   p->setPen(QPen(QColor("#182633"),1));
   p->setBrush(h==12?QColor("#FFAE45"):(h>=8?QColor("#49BCF4"):QColor("#F3F5F7")));
   if(h==12){
    p->drawRect(QRectF(point-QPointF(5.,5.),QSizeF(10.,10.)));
    p->drawLine(point+QPointF(-3,0),point+QPointF(3,0));
    p->drawLine(point+QPointF(0,-3),point+QPointF(0,3));
   }else if(h>=8)p->drawEllipse(point,4.,4.);
   else p->drawRect(QRectF(point-QPointF(4.,4.),QSizeF(8.,8.)));
  }
  p->restore();
 }
 void cancelSelectionTransform(){
  for(auto* item:transformItems_)item->setTransform(QTransform());
  transformItems_.clear();transformIds_.clear();
  transformDragging_=false;transformHandle_=-1;
  transformSx_=transformSy_=1.;transformAngle_=0.;transformMoveDelta_={};
  viewport()->update();
 }
 bool beginSelectionTransform(QPointF position){
  if(tool!=Tool::Select||nativeMode_||selected.empty()||busy())return false;
  const auto bounds=selectedBounds();if(!bounds)return false;
  const auto handles=selectionHandles(*bounds);
  int h=-1;double distance=11.;
  // Preferir o ponto central quando a selecao e grande suficiente.
  if(QLineF(position,handles[12]).length()<=7.){h=12;distance=7.;}
  if(h<0)for(int i=0;i<12;++i){
   const double d=QLineF(position,handles[size_t(i)]).length();
   if(d<distance){h=i;distance=d;}
  }
  if(h<0)return false;
  transformIds_=expandedSelection();
  for(int id:transformIds_){
   if(!editable(editor.state(),id)){cancelSelectionTransform();return false;}
   auto found=itemsByObject_.find(id);
   if(found!=itemsByObject_.end())
    transformItems_.insert(transformItems_.end(),found->second.begin(),found->second.end());
  }
  if(transformItems_.empty()){cancelSelectionTransform();return false;}
  transformDragging_=true;transformHandle_=h;transformBounds_=*bounds;
  transformAnchor_=h>=8?bounds->center():QPointF{};
  transformSx_=transformSy_=1.;transformAngle_=0.;
  return true;
 }
 void updateSelectionTransform(QPointF pointer,Qt::KeyboardModifiers mods){
  if(!transformDragging_)return;
  QTransform t;
  if(transformHandle_==12){
   const QPointF origin=viewportTransform().map(transformBounds_.center());
   QPointF delta=mapToScene(pointer.toPoint())-transformBounds_.center();
   // LightBurn: Shift limita deslocamento em 0/45/90 graus.
   if(mods.testFlag(Qt::ShiftModifier)){
    const double angle=std::atan2(delta.y(),delta.x());
    const double snapped=std::round(angle/(std::numbers::pi/4.))*(std::numbers::pi/4.);
    const double length=std::hypot(delta.x(),delta.y());
    delta=QPointF(length*std::cos(snapped),length*std::sin(snapped));
   }
   transformMoveDelta_=delta;
   t.translate(delta.x(),delta.y());(void)origin;
  }else if(transformHandle_>=8){
   const QPointF pivot=transformBounds_.center();
   const auto handles=selectionHandles(transformBounds_);
   const QPointF initial=handles[size_t(transformHandle_)];
   const QPointF center=viewportTransform().map(pivot);
   const auto start=initial-center;
   const auto now=pointer-center;
   const double angle=std::atan2(now.y(),now.x())-std::atan2(start.y(),start.x());
   transformAngle_=angle*180./std::numbers::pi;
   if(mods.testFlag(Qt::ShiftModifier))
    transformAngle_=std::round(transformAngle_/15.)*15.;
   t.translate(pivot.x(),pivot.y());t.rotate(transformAngle_);t.translate(-pivot.x(),-pivot.y());
  }else{
   const double x0=transformBounds_.left(),x1=transformBounds_.right();
   const double y0=transformBounds_.top(),y1=transformBounds_.bottom();
   const double mx=(x0+x1)*.5,my=(y0+y1)*.5;
   const bool center=mods.testFlag(Qt::ControlModifier);
   const auto handles=selectionHandles(transformBounds_);
   const QPointF oldPoint=mapToScene(handles[size_t(transformHandle_)].toPoint());
   const QPointF moving=mapToScene(pointer.toPoint());
   QPointF pivot;
   switch(transformHandle_){
    case 0:pivot={x1,y1};break;case 1:pivot={mx,y1};break;
    case 2:pivot={x0,y1};break;case 3:pivot={x0,my};break;
    case 4:pivot={x0,y0};break;case 5:pivot={mx,y0};break;
    case 6:pivot={x1,y0};break;default:pivot={x1,my};break;
   }
   if(center)pivot=transformBounds_.center();
   transformAnchor_=pivot;
   const bool horizontal=transformHandle_!=1&&transformHandle_!=5;
   const bool vertical=transformHandle_!=3&&transformHandle_!=7;
   double sx=1.,sy=1.;
   if(horizontal&&std::abs(oldPoint.x()-pivot.x())>1e-9)
     sx=(moving.x()-pivot.x())/(oldPoint.x()-pivot.x());
   if(vertical&&std::abs(oldPoint.y()-pivot.y())>1e-9)
     sy=(moving.y()-pivot.y())/(oldPoint.y()-pivot.y());
   sx=std::clamp(sx,.02,100.);sy=std::clamp(sy,.02,100.);
   const bool corner=transformHandle_%2==0;
   if(corner&&!mods.testFlag(Qt::ShiftModifier)){
    const double uniform=std::abs(sx-1.)>=std::abs(sy-1.)?sx:sy;
    sx=uniform;sy=uniform;
   }
   transformSx_=sx;transformSy_=sy;
   t.translate(pivot.x(),pivot.y());t.scale(sx,sy);t.translate(-pivot.x(),-pivot.y());
  }
  for(auto* item:transformItems_)item->setTransform(t);
  viewport()->update();
 }
 void finishSelectionTransform(){
  if(!transformDragging_)return;
  const int h=transformHandle_;
  const auto ids=transformIds_;
  const QPointF pivot=h>=8?transformBounds_.center():transformAnchor_;
  const double sx=transformSx_,sy=transformSy_,angle=transformAngle_;
  const QPointF move=transformMoveDelta_;
  cancelSelectionTransform();
  if(h==12){
   if(QLineF(QPointF{},move).length()<1e-8)return;
   editor.moveObjects(ids,fromQt(move));
  }else if(h>=8){
   if(std::abs(angle)<.01)return;
   editor.rotateObjects(ids,fromQt(pivot),angle);
  }else{
   if(std::abs(sx-1.)<1e-5&&std::abs(sy-1.)<1e-5)return;
   editor.scaleObjects(ids,fromQt(pivot),sx,sy);
  }
  rebuild();
  if(diagnosticEvent)diagnosticEvent(h==12?"shapes_moved":h>=8?"selection_rotated":"selection_scaled",int(ids.size()));
  if(stateChanged)stateChanged();
 }
 void clearShapePreview(){
  for(auto* item:shapePreviewItems_)item->setPos(0,0);
  shapePreviewItems_.clear();shapeDragging_=false;
 }
 void beginShapePreview(QPointF origin){
  clearShapePreview();
  std::set<int> ids=selected,groups;
  const auto& state=editor.state();
  for(int id:ids){const auto it=state.objects.find(id);
   if(it!=state.objects.end()&&it->second.group>0)groups.insert(it->second.group);
  }
  for(const auto& [id,meta]:state.objects)if(meta.group>0&&groups.count(meta.group))ids.insert(id);
  for(int id:ids){
   if(!editable(state,id)){clearShapePreview();return;}
   const auto it=itemsByObject_.find(id);
   if(it!=itemsByObject_.end())shapePreviewItems_.insert(shapePreviewItems_.end(),it->second.begin(),it->second.end());
  }
  if(shapePreviewItems_.empty())return;
  shapeDragStart_=origin;shapeDragging_=true;
 }
 void drawRasterOverlay(){
  rasterOverlay_=nullptr;
  if(rasterSource_.isNull())return;
  rasterOverlay_=scene_.addPixmap(QPixmap::fromImage(rasterSource_));
  rasterOverlay_->setAcceptedMouseButtons(Qt::NoButton);
  rasterOverlay_->setZValue(-20);
  rasterOverlay_->setPos(rasterRect_.topLeft());
  rasterOverlay_->setTransform(QTransform::fromScale(
    rasterRect_.width()/rasterSource_.width(),rasterRect_.height()/rasterSource_.height()));
  rasterOverlay_->setOpacity(.72);
 }
 void setRasterImage(const QImage& image){
  if(image.isNull())throw std::runtime_error("Imagem vazia");
  if(image.width()>10000||image.height()>10000)
   throw std::runtime_error("Imagem excede 10000 pixels");
  rasterSource_=image;
  rasterRect_=QRectF(20,20,120.,120.*image.height()/image.width());
  rebuild();viewport()->update();
 }
 const QImage& rasterImage()const{return rasterSource_;}
 QRectF rasterBounds()const{return rasterRect_;}
 bool hasRasterImage()const{return !rasterSource_.isNull();}
 void clearRasterImage(){
  rasterSource_=QImage();rasterRect_=QRectF();
  if(rasterOverlay_){scene_.removeItem(rasterOverlay_);delete rasterOverlay_;rasterOverlay_=nullptr;}
  viewport()->update();
 }
 void clearNativeItems(){
  nativeContourPressing_=false;
  for(auto& entry:nativeItems_){scene_.removeItem(entry.second);delete entry.second;}
  nativeItems_.clear();nativeSelectedPart_.reset();nativeStyledKey_.reset();nativePressing_=false;
 }
 void createNativeItems(){
  clearNativeItems();
  if(!nativeSession_)return;
  for(const auto& c:nativeSession_->committed().contours){
   auto* item=scene_.addPath(nativeCurvePath(c));
   item->setAcceptedMouseButtons(Qt::NoButton);
   nativeItems_[c.key]=item;
  }
  updateNativePaths(true);
 }
 void nativeMousePress(QMouseEvent* e){
  if(!nativeSession_){e->accept();return;}
  const QPointF pos=mapToScene(e->position().toPoint());Pt click=fromQt(pos);
  const double zoom=nativePxPerMm(),dpr=viewport()->devicePixelRatioF();
  const bool alt=e->modifiers().testFlag(Qt::AltModifier);
  const bool insert=e->modifiers().testFlag(Qt::ShiftModifier);
  // Ctrl+arraste: deslocar o contorno Bézier completo sem mexer em alças isoladas.
  if(e->modifiers().testFlag(Qt::ControlModifier)){
   const auto hits=nativeSession_->pickEdges(click,zoom,dpr,6);
   if(!hits.empty()&&nativeSession_->startContourMove(hits.front().key,click)){
    nativeSelectedPart_.reset();nativeContourPressing_=true;
    updateNativePaths(true);if(stateChanged)stateChanged();
   }
   e->accept();return;
  }
  if(!alt&&!insert&&nativeSession_->selected()){
   auto hit=pickCurveNode(nativeSession_->committed(),*nativeSession_->selected(),click,zoom,dpr,7);
   if(hit){
    nativeSelectedPart_=*hit;
    nativePressing_=nativeSession_->start(*hit);
    updateNativePaths(false);viewport()->update();e->accept();return;
   }
  }
  const auto hits=nativeSession_->pickEdges(click,zoom,dpr,6);
  if(hits.empty()){
   nativeSelectedPart_.reset();viewport()->update();e->accept();return;
  }
  size_t i=0;
  if(alt&&nativeSession_->selected()){
   for(size_t n=0;n<hits.size();++n)if(hits[n].key==*nativeSession_->selected()){
    i=(n+1)%hits.size();break;
   }
  }
  // Insertion follows the selected subpath, including coincident contours.
  if(insert&&nativeSession_->selected()){
   for(size_t n=0;n<hits.size();++n)if(hits[n].key==*nativeSession_->selected()){i=n;break;}
  }
  const CurveHit hit=hits[i];
  if(insert){
   // Primeira acao escolhe o contorno correto; nunca insere em objeto diferente.
   if(!nativeSession_->selected()||*nativeSession_->selected()!=hit.key){
    nativeSession_->select(hit.key);nativeSelectedPart_.reset();
   }else{
    // Evita criar pontos quase coincidentes com as ancoras de extremidade.
    if(hit.t>=0.015&&hit.t<=0.985){
     const int64_t node=nativeSession_->insertAnchor(hit,hit.t);
     nativeSelectedPart_=CurveHit{hit.key,-1,node,CurvePart::Anchor,0,0,hit.point};
    }
   }
  }else{
   nativeSession_->select(hit.key);nativeSelectedPart_.reset();
  }
  updateNativePaths(true);if(stateChanged)stateChanged();e->accept();
 }

public:
 enum class Tool{Select,Nodes,Scissors,Rectangle,Line,Ellipse,Polygon,Pencil,Text};
 Tool tool=Tool::Select;
 Editor editor;
 std::set<int> selected;
 std::vector<double> paintMs,inputToPaintMs,hitTestMs,styleApplyMs,wheelMs;
 static void sample(std::vector<double>& measurements,double duration){
  if(measurements.size()>=10000)measurements.erase(measurements.begin(),measurements.begin()+5000);
  measurements.push_back(duration);
 }
 qint64 markerCandidates=0,markerDraws=0;
 std::function<void()> stateChanged;
 std::function<void(const QString&)> toolWarning;
 std::function<void(Pt)> requestTextAt;
 std::function<void()> requestTraceImage;
 std::function<void()> requestSelectTool;
 std::function<void(Tool)> requestToolChange;
 std::function<void(const QString&,int)> diagnosticEvent;
 std::function<void(double)> zoomChanged;
 double currentZoom()const{return std::hypot(transform().m11(),transform().m21());}
 void reportZoom(){if(zoomChanged)zoomChanged(currentZoom());}
 bool fillPreview()const{return fillPreview_;}
 void setFillPreview(bool enabled){
  if(fillPreview_==enabled)return;
  fillPreview_=enabled;rebuild();
 }
 std::function<int()> drawingLayer;
 int targetLayer()const{return drawingLayer?drawingLayer():0;}
 bool busy()const{return draggingNode_.has_value()||shapeDragging_||transformDragging_||strokeDrawing_||polylineDrawing_||marqueeActive_||nativePressing_||nativeContourPressing_;}
 bool nativeMode()const{return nativeMode_;}
 bool hasNativeDocument()const{return nativeSession_&&!nativeSession_->committed().contours.empty();}
 std::optional<Key> nativeSelected()const{return nativeSession_?nativeSession_->selected():std::nullopt;}
 std::optional<int64_t> nativeSelectedNode()const{
  if(!nativeSelectedPart_||nativeSelectedPart_->part==CurvePart::Edge)return {};
  return nativeSelectedPart_->nodeId;
 }
 size_t nativeItemCount()const{return nativeItems_.size();}
 QPainterPath nativeItemPath(Key key)const{return nativeItems_.at(key)->path();}
 // A sessao deve estar viva durante todo o periodo em que estiver anexada.
 void attachNative(CurveDragSession* session){
  if((nativePressing_||nativeContourPressing_)&&nativeSession_)nativeSession_->cancel();
  nativePressing_=false;nativeContourPressing_=false;nativeMode_=false;
  clearNativeItems();nativeSession_=session;
  createNativeItems();
 }
 void enableNative(bool enabled){
  if(busy())return;
  nativeMode_=enabled&&hasNativeDocument();
  if(nativeMode_){tool=Tool::Select;viewport()->setCursor(Qt::ArrowCursor);}
  updateNativePaths(false);
 }
 void nativeHistory(bool redo){
  if(!nativeSession_||nativePressing_||nativeContourPressing_)return;
  bool changed=redo?nativeSession_->redo():nativeSession_->undo();
  if(changed){nativeSelectedPart_.reset();updateNativePaths(true);if(stateChanged)stateChanged();}
 }
 bool nativeDeleteSelected(){
  if(!nativeSession_||nativePressing_||nativeContourPressing_||!nativeSelectedPart_||!nativeSession_->selected())return false;
  const Key key=*nativeSession_->selected();
  if(key!=nativeSelectedPart_->key||nativeSelectedPart_->part==CurvePart::Edge)return false;
  try{
   if(!nativeSession_->deleteAnchor(nativeSelectedPart_->nodeId))return false;
  }catch(const std::exception& e){
   // Operacao invalida (por exemplo, topologia minima): sem excecao atraves do Qt event loop.
   qWarning().noquote()<<"Bézier: exclusão recusada:"<<e.what();return false;
  }
  nativeSelectedPart_.reset();updateNativePaths(true);if(stateChanged)stateChanged();
  if(diagnosticEvent)diagnosticEvent("bezier_node_deleted",1);
  return true;
 }
 bool nativeDeleteContour(){
  if(!nativeSession_||nativePressing_||nativeContourPressing_||!nativeSession_->selected())return false;
  if(!nativeSession_->deleteSelectedContour())return false;
  nativeSelectedPart_.reset();createNativeItems();
  if(diagnosticEvent)diagnosticEvent("bezier_contour_deleted",1);
  if(stateChanged)stateChanged();return true;
 }
 bool deleteSelectedShapes(){
  if(nativeMode_||busy()||selected.empty())return false;
  const int count=int(selected.size());
  editor.deleteObjects({selected.begin(),selected.end()});
  selected.clear();active_.reset();rebuild();
  if(diagnosticEvent)diagnosticEvent("shapes_deleted",count);
  return true;
 }

 int frameUpdates=0,fullRebuilds=0,toolCreates=0;
 int localizedNodeRepaints=0; qint64 localizedNodeAreaPx=0;
 int chunkPathsUpdated()const{return chunkPathsUpdated_;}
 int fullDragPathUpdates()const{return fullDragPathUpdates_;}
 int previewChunkCount()const{return previewChunkCount_;}
 bool previewMatches(const Document& expected)const{
  if(!dragChunkKey_)return false;
  const auto* c=get(expected,*dragChunkKey_);if(!c)return false;
  auto it=objects_.find(*dragChunkKey_);if(it==objects_.end())return false;
  if(continuousPreview_){
   return dragChunkItems_.empty() && it->second->isVisible() &&
      it->second->path()==pathOf(*c);
  }
  if(!draggingChunks_||dragChunkItems_.size()!=draggingChunks_->chunkCount())return false;
  for(size_t i=0;i<dragChunkItems_.size();++i){
   const auto&ch=draggingChunks_->chunk(i);
   if(dragChunkItems_[i]->path()!=contourChunkPath(*c,ch.first,ch.edges.size()))return false;
   if(!dragChunkItems_[i]->isVisible())return false;
  }
  return !it->second->isVisible();
 }
 bool hasChunkPreview()const{return continuousPreview_||draggingChunks_.has_value();}
 bool usingContinuousPreview()const{return !experimentalChunks_;}
 QString previewModeName()const{return experimentalChunks_?"EXPERIMENTAL_CHUNKS":"CONTINUOUS_SAFE";}

 // Representação visual: não inclui elementos no documento, histórico ou exportação.
 void setWorkspaceTheme(bool dark){workspaceDark_=dark;viewport()->update();}
 void setWorkspace(const WorkspaceConfig& cfg, bool showGuides){
  cfg.validate(); workspace_=cfg; workspaceGuides_=showGuides;
  const auto bounds=QRectF(-80,-80,std::max(cfg.drawingWidth,cfg.machineWidth)+160,
                        std::max(cfg.drawingHeight,cfg.machineHeight)+160);
  scene_.setSceneRect(scene_.sceneRect().united(bounds));
  viewport()->update();
 }
 const WorkspaceConfig& workspace() const{return workspace_;}
 void fitMachineBed(){
  const QRectF r(0,0,workspace_.machineWidth,workspace_.machineHeight);
  if(!r.isEmpty())fitInView(r.adjusted(-12,-12,12,12),Qt::KeepAspectRatio);
  reportZoom();
 }
protected:
 void drawBackground(QPainter* painter,const QRectF& exposed) override{
  QGraphicsView::drawBackground(painter,exposed);
  if(!workspaceGuides_)return;
  painter->save();
  const QRectF drawing(0,0,workspace_.drawingWidth,workspace_.drawingHeight);
  const QRectF bed(0,0,workspace_.machineWidth,workspace_.machineHeight);
  const QRectF clipped=exposed.intersected(workspace_.gridToBed?bed:drawing);
  painter->fillRect(bed,workspaceDark_?QColor("#273745"):QColor("#D9DEE3"));
  if(workspace_.showGrid&&!clipped.isEmpty()){
   const double z=std::max(1e-9,currentZoom());
   double step=10.0;
   while(step*z<18 && step<100000.0)step*=5;
   const double firstX=std::max(0.,std::ceil(clipped.left()/step)*step);
   const double firstY=std::max(0.,std::ceil(clipped.top()/step)*step);
   painter->setPen(QPen(workspaceDark_?QColor(155,177,191,55):QColor(93,113,129,60),0));
   for(double x=firstX;x<=clipped.right()&&x<=firstX+step*500;x+=step)
    painter->drawLine(QPointF(x,clipped.top()),QPointF(x,clipped.bottom()));
   for(double y=firstY;y<=clipped.bottom()&&y<=firstY+step*500;y+=step)
    painter->drawLine(QPointF(clipped.left(),y),QPointF(clipped.right(),y));
  }
  painter->setPen(QPen(QColor(90,135,165,190),0));
  if(workspace_.showDrawingFrame)painter->drawRect(drawing);
  if(workspace_.showMachineFrame){
   QPen edge(QColor(242,145,47,210),0);edge.setStyle(Qt::DashLine);
   painter->setPen(edge);painter->drawRect(bed);
  }
  if(workspace_.showOrigin){
   painter->setPen(QPen(QColor(242,145,47),0));
   const double arm=std::clamp(11.0/std::max(1e-9,currentZoom()),0.1,40.0);
   painter->drawLine(QPointF(-arm,0),QPointF(arm,0));
   painter->drawLine(QPointF(0,-arm),QPointF(0,arm));
  }
  painter->restore();
 }
public:
 QtEditorLab(Document d):QGraphicsView(),editor(std::move(d)){
  setScene(&scene_);setSceneRect(-2000,-2000,5000,5000);setRenderHints(QPainter::Antialiasing);
  setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);setTransformationAnchor(QGraphicsView::NoAnchor);
  // QAbstractScrollArea entrega os movimentos atraves do QWidget viewport.
  // Garantir rastreamento explicitamente no filho que recebe os eventos.
  setMouseTracking(true);viewport()->setMouseTracking(true);
  viewport()->setAttribute(Qt::WA_Hover,true);
  setFocusPolicy(Qt::StrongFocus);resize(780,560);
  clock_.start();rebuild();
 }
 // Apenas objetos que trocam de estado visual devem ser modificados.
 // Evita milhares de chamadas setPen/setZValue em cada clique.
  static QColor layerColor(int id){
  static const QColor colors[]={
   QColor("#788998"),QColor("#3A86FF"),QColor("#E63946"),QColor("#06D6A0"),
   QColor("#FFD166"),QColor("#F77F00"),QColor("#9B5DE5"),QColor("#00BBF9"),
   QColor("#A7C957"),QColor("#FF006E"),QColor("#748CAB"),QColor("#219EBC"),
   QColor("#B5179E"),QColor("#38B000"),QColor("#BC6C25"),QColor("#8ECAE6")
  };
  return colors[(id%16+16)%16];
 }
 void applySelectionStyles(){
  QElapsedTimer profiler;profiler.start();
  std::vector<int> changed;
  std::set_symmetric_difference(styledObjects_.begin(),styledObjects_.end(),
    selected.begin(),selected.end(),std::back_inserter(changed));
  for(int objectId:changed){
   auto it=itemsByObject_.find(objectId);
   if(it==itemsByObject_.end())continue;
   bool isSelected=selected.count(objectId)>0;
   for(auto* item:it->second){
     const auto& meta=editor.state().objects.at(objectId);
     item->setPen(QPen(isSelected?QColor(239,109,25):layerColor(meta.layer),0));
     // O pincel ja foi criado no rebuild pela geometria canonica (closed).
     item->setZValue(isSelected?9:1);
    styleSetterCalls_+=2;
   }
  }
  styledObjects_=selected;
  styleApplyMs.push_back(profiler.nsecsElapsed()/1e6);
 }
 qint64 styleSetterCalls()const{return styleSetterCalls_;}
 size_t indexBuilds()const{return picker_.rebuilds();}
 size_t indexedQueries()const{return picker_.indexedQueries();}
 size_t directQueries()const{return picker_.directQueries();}
 void invalidateSelectionCache(){picker_.invalidate();}
 void rebuild(){
  clearScissorHover();clearShapePreview();cancelSelectionTransform();endChunkPreview();
  clearPolyline();clearMarquee();scene_.clear();rasterOverlay_=nullptr;objects_.clear();nativeItems_.clear();itemsByObject_.clear();styledObjects_.clear();pendingPreview_=nullptr;
  for(auto& c:editor.state().doc.contours){
    const auto meta=editor.state().objects.at(c.key.object);
    const auto layer=editor.state().layers.at(meta.layer);
    auto* o=scene_.addPath(pathOf(c),QPen(layerColor(meta.layer),0));o->setZValue(1);
    if(fillPreview_&&layer.mode=="Fill"&&(c.closed||hasExactDuplicateClosure(c))){
     QColor translucent=layerColor(meta.layer);translucent.setAlpha(115);
     o->setBrush(QBrush(translucent));
    }
    o->setVisible(layer.visible);objects_[c.key]=o;itemsByObject_[c.key.object].push_back(o);
  }
  for(auto it=selected.begin();it!=selected.end();)if(!editable(editor.state(),*it))it=selected.erase(it);else ++it;
  if(active_&&(!get(editor.state().doc,*active_)||!editable(editor.state(),active_->object)))active_.reset();
  if(active_)activate(*active_,false);else applySelectionStyles();
  createNativeItems();
  drawRasterOverlay();
  // O índice só deve ser invalidado por uma alteração real na geometria.
  // Mudanças em camadas, agrupamentos ou destaque preservam o cache.
  fullRebuilds++;viewport()->update();
  if(stateChanged)stateChanged();
 }
 void activate(Key key,bool replace=true){
  bool changedActive=!active_.has_value()||*active_!=key;
  active_=key;if(replace){selected.clear();selected.insert(key.object);}
  applySelectionStyles();
  // Foreground node markers depend on the active contour. A repeated click
  // on the same contour needs no full-viewport invalidation.
  if(changedActive)viewport()->update();
 }
 // O caminho QGraphicsPathItem::setPath() invalida seus limites antigos/novos.
 // Os marcadores desenhados em drawForeground nao sao QGraphicsItems;
 // para eles solicitamos apenas a area antiga + nova, nunca toda a viewport.
 void updateNodeMarkers(QPointF oldPosition,QPointF newPosition){
  auto a=oldPosition.toPoint();auto b=newPosition.toPoint();
  QRect dirty=QRect(a,b).normalized().adjusted(-9,-9,9,9);
  viewport()->update(dirty);
  ++localizedNodeRepaints;localizedNodeAreaPx+=qint64(dirty.width())*dirty.height();
 }
 QGraphicsPathItem* pathItem(Key k){return objects_.at(k);}
 std::optional<Key> active()const{return active_;}
 std::optional<ScissorSpan> scissorPreview()const{return scissorHover_;}
 uint64_t scissorHoverEventCount()const{return scissorHoverEvents_;}
 bool scissorHoverReceivedAt(QPoint expected)const{
  return scissorLastHoverPosition_&&*scissorLastHoverPosition_==expected;
 }
 std::string scissorHoverDiagnostics(QPoint at){
  const auto scenePt=mapToScene(at);
  std::ostringstream out;
  out<<"hover_events="<<scissorHoverEvents_
     <<" mode="<<int(tool)<<" native="<<nativeMode_<<" busy="<<busy()
     <<" point=("<<at.x()<<","<<at.y()<<")"
     <<" viewport=("<<viewport()->width()<<","<<viewport()->height()<<")"
     <<" tracking="<<viewport()->hasMouseTracking()
     <<" hover_attr="<<viewport()->testAttribute(Qt::WA_Hover)
     <<" last_hover="<<(scissorLastHoverPosition_?
       ("("+std::to_string(scissorLastHoverPosition_->x())+","+
             std::to_string(scissorLastHoverPosition_->y())+")"):"none")
     <<" scene=("<<scenePt.x()<<","<<scenePt.y()<<")";
  auto hits=clickHits(at);
  out<<" hits="<<hits.size();
  ScissorSpanPlanner checker;
  checker.reset(editor.state().doc,[&](Key k){
   auto it=editor.state().objects.find(k.object);
   if(it==editor.state().objects.end())return false;
   auto li=editor.state().layers.find(it->second.layer);
   return li!=editor.state().layers.end()&&li->second.visible;
  });
  for(size_t i=0;i<std::min<size_t>(hits.size(),3);++i){
   auto candidate=checker.candidate(editor.state().doc,hits[i]);
   out<<" hit"<<i<<"=(object:"<<hits[i].key.object
      <<",edge:"<<hits[i].segment<<",candidate:"<<(bool(candidate))
      <<",pts:"<<(candidate?candidate->preview.size():0)<<")";
  }
  out<<" preview="<<(bool(scissorHover_))
     <<" overlay="<<(scissorPreview_!=nullptr);
  return out.str();
 }
 void clearScissorHover(){
  scissorHover_.reset();
  if(scissorPreview_){scene_.removeItem(scissorPreview_);delete scissorPreview_;scissorPreview_=nullptr;}
  viewport()->setCursor(Qt::ArrowCursor);
 }
 void updateScissorHover(QPoint where){
  ++scissorHoverEvents_;
  scissorLastHoverPosition_=where;
  if(tool!=Tool::Scissors||nativeMode_||busy()){clearScissorHover();return;}
  if(scissorIndexVersion_!=editor.generation){
   clearScissorHover();
   const auto& state=editor.state();
   scissorPlanner_.reset(state.doc,[&](Key key){
    const auto meta=state.objects.find(key.object);
    if(meta==state.objects.end())return false;
    const auto li=state.layers.find(meta->second.layer);
    return li!=state.layers.end()&&li->second.visible;
   });
   scissorIndexVersion_=editor.generation;
  }
  std::optional<ScissorSpan> next;
  const auto& state=editor.state();
  for(const Hit& hit:clickHits(where)){
   const auto meta=state.objects.find(hit.key.object);
   if(meta==state.objects.end()||meta->second.group>0||!editable(state,hit.key.object))continue;
   next=scissorPlanner_.candidate(state.doc,hit);
   if(next)break;
  }
  if(!next){if(scissorHover_)clearScissorHover();return;}
  if(scissorHover_&&scissorHover_->key==next->key&&
     std::abs(scissorHover_->from-next->from)<1e-8&&
     std::abs(scissorHover_->to-next->to)<1e-8)return;
  clearScissorHover();
  scissorHover_=std::move(next);
  QPainterPath colored;
  colored.moveTo(toQt(scissorHover_->preview.front()));
  for(size_t i=1;i<scissorHover_->preview.size();i++)
   colored.lineTo(toQt(scissorHover_->preview[i]));
  QPen highlight(QColor(255,160,37),3);
  highlight.setCosmetic(true);
  highlight.setCapStyle(Qt::RoundCap);
  highlight.setJoinStyle(Qt::RoundJoin);
  scissorPreview_=scene_.addPath(colored,highlight);
  scissorPreview_->setZValue(45);
  scissorPreview_->setAcceptedMouseButtons(Qt::NoButton);
  viewport()->setCursor(QCursor(alaser::toolGlyph("scissors").pixmap(24,24),4,4));
 }
 void setTool(Tool t){
  if(polylineDrawing_&&t!=Tool::Line)finishPolyline(false);
  if(!busy()){
   clearScissorHover();nativeMode_=false;tool=t;
   viewport()->setCursor(Qt::ArrowCursor);viewport()->update();
  }
 }
 QPointF constrainedCorner(QPointF first,QPointF last,bool shift)const{
  if(!shift||(tool!=Tool::Rectangle&&tool!=Tool::Ellipse))return last;
  const double dx=last.x()-first.x(),dy=last.y()-first.y();
  const double size=std::min(std::abs(dx),std::abs(dy));
  return {first.x()+std::copysign(size,dx),first.y()+std::copysign(size,dy)};
 }
 void cancelCurrentGesture(){
  if(nativeContourPressing_&&nativeSession_){nativeSession_->cancel();nativeContourPressing_=false;updateNativePaths(true);}
  if(nativePressing_&&nativeSession_){nativeSession_->cancel();nativePressing_=false;updateNativePaths(false);}
  if(shapeDragging_)clearShapePreview();
  if(transformDragging_)cancelSelectionTransform();
  if(draggingNode_){endChunkPreview();draggingNode_.reset();viewport()->update();}
  if(strokeDrawing_){strokeDrawing_=false;pencilPoints_.clear();if(pendingPreview_){scene_.removeItem(pendingPreview_);delete pendingPreview_;pendingPreview_=nullptr;}viewport()->update();}
  if(polylineDrawing_)clearPolyline();
  if(marqueeActive_)clearMarquee();
 }
 void history(bool redo){if(busy())return;if(redo?editor.redo():editor.undo()){
  active_.reset();selected.clear();rebuild();if(diagnosticEvent)diagnosticEvent(redo?"redo":"undo",1);
 }}
 void clearSelection(){bool hadActive=active_.has_value();active_.reset();selected.clear();applySelectionStyles();if(hadActive)viewport()->update();}
 void selectAllVisible(){
  if(nativeMode_||busy())return;
  selected.clear();active_.reset();
  for(const auto& [id,meta]:editor.state().objects)
   if(editable(editor.state(),id))selected.insert(id);
  applySelectionStyles();viewport()->update();
  if(stateChanged)stateChanged();
 }
 void invertSelection(){
  if(nativeMode_||busy())return;
  std::set<int> inverted;
  for(const auto& [id,meta]:editor.state().objects)
   if(editable(editor.state(),id)&&!selected.count(id))inverted.insert(id);
  selected=std::move(inverted);active_.reset();applySelectionStyles();viewport()->update();
  if(stateChanged)stateChanged();
 }
 bool isMarqueeActive()const{return marqueeActive_;}
 bool isPolylineActive()const{return polylineDrawing_;}
 size_t polylineVertexCount()const{return polylinePoints_.size();}

 std::vector<Hit> clickHits(QPoint viewportPoint){
  QElapsedTimer profiler;profiler.start();
  auto m=viewportTransform();double sx=std::hypot(m.m11(),m.m12()),sy=std::hypot(m.m21(),m.m22());
  // Qt fornece unidades da viewport em pixels logicos; passamos o DPR separadamente.
  const double pxPerMm=std::max(1e-8,std::min(sx,sy));
  // Tolerancia maior apenas para localizar linhas finas com a Tesoura.
  const double tolerancePx=(tool==Tool::Scissors?9.:6.);
  auto hs=picker_.query(editor,fromQt(mapToScene(viewportPoint)),
                        tolerancePx,pxPerMm,viewport()->devicePixelRatioF());
  sample(hitTestMs,profiler.nsecsElapsed()/1e6);return hs;
 }
 int lastCreated()const{return lastCreated_;}
protected:
 // TESTE_19.1: zoom ancorado no cursor, independente da ferramenta ativa.
 // Nao altera pontos, IDs, historico ou o documento canonico.
 void wheelEvent(QWheelEvent* e)override{
  QElapsedTimer wheelTimer;wheelTimer.start();
  const QPoint angle=e->angleDelta(),pixel=e->pixelDelta();
  const double steps=angle.y()!=0 ? double(angle.y())/120.0 : double(pixel.y())/100.0;
  if(steps==0.0){QGraphicsView::wheelEvent(e);return;}
  if(busy()){e->accept();return;}
  const double oldZoom=currentZoom();
  if(!std::isfinite(oldZoom)||oldZoom<=0.0){e->accept();return;}
  const double newZoom=alaser::zoom196::applyWheel(oldZoom,steps);
  if(std::abs(newZoom-oldZoom)<1e-12){e->accept();return;}
  const QPoint cursor=e->position().toPoint();
  const QPointF pinnedScene=mapToScene(cursor);
  scale(newZoom/oldZoom,newZoom/oldZoom);
  // Reposiciona as barras de rolagem para manter o ponto sob o mouse.
  // O limite de scroll da cena pode restringir a ancoragem nas extremidades.
  const QPoint mapped=mapFromScene(pinnedScene);
  horizontalScrollBar()->setValue(horizontalScrollBar()->value()+mapped.x()-cursor.x());
  verticalScrollBar()->setValue(verticalScrollBar()->value()+mapped.y()-cursor.y());
  reportZoom();viewport()->update();sample(wheelMs,wheelTimer.nsecsElapsed()/1e6);e->accept();
 }
 void paintEvent(QPaintEvent* e)override{
  QElapsedTimer timer;timer.start();QGraphicsView::paintEvent(e);sample(paintMs,timer.nsecsElapsed()/1e6);
  if(pendingInput_>=0){sample(inputToPaintMs,(clock_.nsecsElapsed()-pendingInput_)/1e6);pendingInput_=-1;}
 }
 void drawForeground(QPainter* p,const QRectF& exposed)override{
  // QGraphicsPathItem normalizes a move-only path to empty. Draw isolated
  // anchors separately while keeping canonical path equality exact.
  p->save();p->resetTransform();
  for(const auto& c:editor.state().doc.contours)if(c.nodes.size()==1){
   const auto& layer=editor.state().layers.at(editor.state().objects.at(c.key.object).layer);
   if(!layer.visible)continue;
   p->setPen(Qt::NoPen);p->setBrush(selected.count(c.key.object)?QColor(239,109,25):layerColor(editor.state().objects.at(c.key.object).layer));
   p->drawEllipse(viewportTransform().map(toQt(c.nodes.front().p)),2,2);
  }
   p->restore();
   paintSelectionHandles(p);
   if(nativeMode_&&nativeSession_&&nativeSession_->selected()){
   const auto* c=findNativeCurve(nativeSession_->visible(),*nativeSession_->selected());
   if(!c)return;
   const QTransform transform=viewportTransform();
   p->save();p->resetTransform();
   for(const auto& n:c->nodes){
    const QPointF anchor=transform.map(toQt(n.anchor));
    if(!QRectF(QPointF(0,0),QSizeF(viewport()->size())).adjusted(-10,-10,10,10).contains(anchor))continue;
    const QPointF in=transform.map(toQt(n.handleIn));
    const QPointF out=transform.map(toQt(n.handleOut));
    // Guias por no, em pixels logicos (nao mm), evitando marcadores gigantes.
    p->setPen(QPen(QColor(87,144,178,145),1));
    p->drawLine(anchor,in);p->drawLine(anchor,out);
    p->setPen(QPen(QColor(46,91,115),1));p->setBrush(QColor(215,237,253));
    p->drawRect(QRectF(in-QPointF(2.5,2.5),QSizeF(5,5)));
    p->drawRect(QRectF(out-QPointF(2.5,2.5),QSizeF(5,5)));
    bool activeNode=nativeSelectedPart_&&nativeSelectedPart_->nodeId==n.id;
    p->setBrush(activeNode?QColor(255,186,84):Qt::white);
    p->setPen(QPen(QColor(32,52,74),1));
    p->drawEllipse(anchor,4,4);
   }
   p->restore();return;
  }
  if(!active_||tool!=Tool::Nodes)return;
  auto*c=get(editor.state().doc,*active_);if(!c)return;
  const auto t=viewportTransform();
  double pxPerScene=std::max(1e-9,std::min(std::hypot(t.m11(),t.m12()),std::hypot(t.m21(),t.m22())));
  const auto visible=exposed.adjusted(-7./pxPerScene,-7./pxPerScene,7./pxPerScene,7./pxPerScene);
  const bool fullMarkers=qEnvironmentVariableIsSet("ALENCAR_FULL_MARKERS");
  const double markerScale=fullMarkers?1.:viewport()->devicePixelRatioF();
  const double cell=4./markerScale;
  std::set<std::pair<int,int>> occupied;
  std::optional<QPointF> draggedMarker;
  p->save();p->resetTransform();p->setPen(QPen(QColor(34,54,75),1./markerScale));p->setBrush(Qt::white);
  for(auto n:c->nodes){QPointF position=toQt(n.p);
   if(draggingNode_&&*draggingNode_==n.id){auto inverse=viewportTransform().inverted();
    position=position+inverse.map(drawCurrent_)-inverse.map(dragStart_);
   }
   if(!visible.contains(position))continue;
   auto display=viewportTransform().map(position);++markerCandidates;
   if(!fullMarkers){
    if(draggingNode_&&*draggingNode_==n.id){draggedMarker=display;continue;}
    if(!occupied.emplace(int(std::floor(display.x()/cell)),int(std::floor(display.y()/cell))).second)continue;
   }
   p->drawEllipse(display,3./markerScale,3./markerScale);++markerDraws;
  }
  // Never suppress the actively dragged node; render it last above neighbours.
  if(draggedMarker){p->setBrush(QColor(255,186,84));p->drawEllipse(*draggedMarker,4./markerScale,4./markerScale);++markerDraws;}
  p->restore();
 }
void mousePressEvent(QMouseEvent* e)override{
 if(e->button()==Qt::MiddleButton){
  if(!busy()){
   middlePanning_=true;middlePanLast_=e->position();
   viewport()->setCursor(Qt::ClosedHandCursor);
  }
  e->accept();return;
 }
 if(e->button()==Qt::RightButton&&tool==Tool::Line){
  if(polylineDrawing_)finishPolyline(false);
  e->accept();return;
 }
 if(e->button()!=Qt::LeftButton){QGraphicsView::mousePressEvent(e);return;}
  if(nativeMode_){nativeMousePress(e);return;}
   if(tool==Tool::Select){
    if(beginSelectionTransform(e->position())){e->accept();return;}
    auto hits=clickHits(e->position().toPoint());
   if(hits.empty()){
    marqueeActive_=true;marqueeStart_=e->position();
   }
   else {Key chosen=hits.front().key;
    if(e->modifiers().testFlag(Qt::AltModifier)&&active_){auto it=std::find_if(hits.begin(),hits.end(),[&](const auto& h){return h.key==*active_;});if(it!=hits.end())chosen=hits[(size_t(it-hits.begin())+1)%hits.size()].key;}
    if(e->modifiers().testFlag(Qt::ShiftModifier)){if(selected.count(chosen.object))selected.erase(chosen.object);else selected.insert(chosen.object);activate(chosen,false);}
    else {
     // Não colapsar seleção múltipla ao arrastar um dos objetos selecionados.
     if(!selected.count(chosen.object)||selected.size()<2)activate(chosen);
     else {active_=chosen;applySelectionStyles();}
     if(e->modifiers()==Qt::NoModifier)beginShapePreview(e->position());
    }
   }
   if(stateChanged)stateChanged();
   e->accept();return;
  }
  if(tool==Tool::Scissors){
   updateScissorHover(e->position().toPoint());
   if(!scissorHover_){
    // Diagnostico localizado: nao remove fragmentos quando nao existe intervalo seguro.
    const auto hits=clickHits(e->position().toPoint());
    QString reason;
    if(hits.empty()){
     reason="Tesoura: nenhuma linha proxima (9 px). Amplie e posicione sobre o contorno.";
     if(diagnosticEvent)diagnosticEvent("scissor_no_hit",1);
    }else{
     bool grouped=false,locked=false,hidden=false;
     for(const auto& hit:hits){
      auto it=editor.state().objects.find(hit.key.object);
      if(it==editor.state().objects.end())continue;
      grouped|=it->second.group>0;
      auto layer=editor.state().layers.find(it->second.layer);
      if(layer!=editor.state().layers.end()){
       locked|=layer->second.locked;hidden|=!layer->second.visible;
      }
     }
     if(grouped)reason="Tesoura: forma agrupada; desagrupe para recortar.";
     else if(locked||hidden)reason="Tesoura: objeto em camada bloqueada ou oculta.";
     else reason="Tesoura: contorno encontrado, mas sem trecho seguro; verifique vertice, tangencia, sobreposicao ou geometria composta.";
     if(diagnosticEvent)diagnosticEvent("scissor_no_safe_span",1);
    }
    if(toolWarning)toolWarning(reason);
    e->accept();return;
   }
   try{
    const ScissorSpan cutting=*scissorHover_;
    clearScissorHover();
    const int obj=editor.removeScissorSpan(cutting);
    selected.clear();rebuild();if(obj>0)activate({obj,0});
    if(diagnosticEvent)diagnosticEvent("scissor_continuous_span_removed",1);
    if(stateChanged)stateChanged();
   }catch(const std::exception& ex){
    clearScissorHover();
    if(toolWarning)toolWarning(QString::fromUtf8(ex.what()));
   }
   e->accept();return;
  }
  if(tool==Tool::Nodes&&active_&&editable(editor.state(),active_->object)){
   auto* c=get(editor.state().doc,*active_);double ratio=viewport()->devicePixelRatioF();double best=6.0;
   if(c)for(const auto& n:c->nodes){auto q=viewportTransform().map(toQt(n.p));double px=std::hypot((q.x()-e->position().x())*ratio,(q.y()-e->position().y())*ratio);
    if(px<best||(!draggingNode_&&px<=best)){best=px;draggingNode_=n.id;dragOriginal_=n.p;}
   }
   if(draggingNode_){dragStart_=e->position();drawCurrent_=e->position();startChunkPreview(*active_,*draggingNode_);}
   e->accept();return;
  }
  if(tool==Tool::Text){
   if(requestTextAt)requestTextAt(fromQt(mapToScene(e->position().toPoint())));
   e->accept();return;
  }
  if(tool==Tool::Line){
   const auto& layer=editor.state().layers.at(targetLayer());
   if(!layer.visible||layer.locked){e->accept();return;}
   Pt at=snapPolyline(fromQt(mapToScene(e->position().toPoint())),
                      e->modifiers().testFlag(Qt::ShiftModifier));
   if(!polylineDrawing_){
    polylineDrawing_=true;polylineStartedOnPress_=true;polylineDown_=e->position();
    polylinePoints_={at};polylineCursor_=toQt(at);
    QPen pen(QColor(25,156,225),1.5,Qt::DashLine);pen.setCosmetic(true);
    polylinePreview_=scene_.addPath(QPainterPath(),pen);
    polylinePreview_->setZValue(30);polylinePreview_->setAcceptedMouseButtons(Qt::NoButton);
   }else{
    polylineStartedOnPress_=false;
    const QPointF firstPx=viewportTransform().map(toQt(polylinePoints_.front()));
    const bool close=polylinePoints_.size()>=3&&QLineF(firstPx,e->position()).length()<=10.;
    if(close){finishPolyline(true);e->accept();return;}
    if(polylinePoints_.size()<20000&&dist(polylinePoints_.back(),at)>1e-6)
     polylinePoints_.push_back(at);
    polylineCursor_=toQt(at);
   }
   redrawPolyline();e->accept();return;
  }
  if(tool==Tool::Pencil){
   const auto& layer=editor.state().layers.at(targetLayer());
   if(!layer.visible||layer.locked){e->accept();return;}
   strokeDrawing_=true;drawStart_=e->position();drawCurrent_=drawStart_;
   pencilPoints_.clear();pencilPoints_.push_back(fromQt(mapToScene(e->position().toPoint())));
   if(!pendingPreview_){
    QPen previewPen(QColor(255,159,35),1.4);previewPen.setCosmetic(true);
    pendingPreview_=scene_.addPath(QPainterPath(),previewPen);
    pendingPreview_->setZValue(20);
   }
   e->accept();return;
  }
  if(tool==Tool::Rectangle||tool==Tool::Ellipse||tool==Tool::Line||tool==Tool::Polygon){
   const auto& layer=editor.state().layers.at(targetLayer());if(!layer.visible||layer.locked){e->accept();return;}
   strokeDrawing_=true;drawStart_=e->position();drawCurrent_=drawStart_;
   if(!pendingPreview_){pendingPreview_=scene_.addPath(QPainterPath(),QPen(QColor(25,156,225),0,Qt::DashLine));pendingPreview_->setZValue(20);}
   e->accept();return;
  }
  QGraphicsView::mousePressEvent(e);
 }
 QPainterPath temporaryShape()const{
  QPainterPath p;QPointF a=mapToScene(drawStart_.toPoint()),b=mapToScene(drawCurrent_.toPoint());
  b=constrainedCorner(a,b,QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier));
  if(tool==Tool::Line){p.moveTo(a);p.lineTo(b);}
  else if(tool==Tool::Rectangle){p.addRect(QRectF(a,b).normalized());}
  else if(tool==Tool::Ellipse){p.addEllipse(QRectF(a,b).normalized());}
  else if(tool==Tool::Polygon){QPointF c=(a+b)/2;double r=std::min(std::abs(b.x()-a.x()),std::abs(b.y()-a.y()))/2;
   if(r>0){p.moveTo(c+QPointF(r,0));for(int i=1;i<6;i++){double t=2*std::numbers::pi*i/6;p.lineTo(c+QPointF(r*std::cos(t),r*std::sin(t)));}p.closeSubpath();}
  }return p;
 }
void mouseMoveEvent(QMouseEvent* e)override{
 if(middlePanning_){
  const QPointF delta=e->position()-middlePanLast_;
  middlePanLast_=e->position();
  horizontalScrollBar()->setValue(horizontalScrollBar()->value()-std::lround(delta.x()));
  verticalScrollBar()->setValue(verticalScrollBar()->value()-std::lround(delta.y()));
  e->accept();return;
 }
 if(nativeMode_&&nativeContourPressing_&&nativeSession_){
  pendingInput_=clock_.nsecsElapsed();
  nativeSession_->updateContourMove(fromQt(mapToScene(e->position().toPoint())));
  updateNativePaths(false);++frameUpdates;e->accept();return;
 }
 if(nativeMode_&&nativePressing_&&nativeSession_){
   pendingInput_=clock_.nsecsElapsed();
   const QPointF pt=mapToScene(e->position().toPoint());
   nativeSession_->update(fromQt(pt));updateNativePaths(false);
   if(nativeSelectedPart_){
    auto& part=*nativeSelectedPart_;
    const QPointF oldPix=viewportTransform().map(toQt(part.point));
    const QPointF newPix=viewportTransform().map(pt);
    viewport()->update(QRect(oldPix.toPoint(),newPix.toPoint()).normalized().adjusted(-20,-20,20,20));
    part.point=fromQt(pt);
   }
   ++frameUpdates;
   e->accept();return;
  }
  if(busy())pendingInput_=clock_.nsecsElapsed();
   if(transformDragging_){updateSelectionTransform(e->position(),e->modifiers());e->accept();return;}
   if(marqueeActive_){showMarquee(e->position());e->accept();return;}
  if(shapeDragging_){
   const QPointF delta=mapToScene(e->position().toPoint())-mapToScene(shapeDragStart_.toPoint());
   for(auto* item:shapePreviewItems_)item->setPos(delta);
   ++frameUpdates;e->accept();return;
  }
  if(draggingNode_&&active_){
   const auto previousDisplay=viewportTransform().map(toQt(dragOriginal_))+(drawCurrent_-dragStart_);
   drawCurrent_=e->position();auto inverse=viewportTransform().inverted();
   QPointF sceneDiff=inverse.map(drawCurrent_)-inverse.map(dragStart_);
   Pt to=dragOriginal_+fromQt(sceneDiff);
   if(continuousPreview_){
    // Apenas o objeto ativo e reconstruido; objetos de fundo ficam intactos.
    // Qt invalida automaticamente os limites antigos/novos do path item.
    pathItem(*active_)->setPath(previewNode(editor.state().doc,*active_,*draggingNode_,to));
    ++fullDragPathUpdates_;
   }else if(draggingChunks_){
    auto dirty=draggingChunks_->update(to);
    for(auto id:dirty.dirtyChunkIds){dragChunkItems_.at(id)->setPath(pathFromChunk(draggingChunks_->chunk(id)));++chunkPathsUpdated_;}
   }else throw std::runtime_error("drag preview not initialized");
   const auto nextDisplay=viewportTransform().map(toQt(to));
   updateNodeMarkers(previousDisplay,nextDisplay);
   frameUpdates++;e->accept();return;
  }
  if(polylineDrawing_&&tool==Tool::Line){
   Pt at=snapPolyline(fromQt(mapToScene(e->position().toPoint())),
                      e->modifiers().testFlag(Qt::ShiftModifier));
   polylineCursor_=toQt(at);redrawPolyline();++frameUpdates;
   e->accept();return;
  }
  if(strokeDrawing_&&tool==Tool::Pencil){
   const Pt here=fromQt(mapToScene(e->position().toPoint()));
   const double step=std::max(.005,1.4/std::max(currentZoom(),.001));
   if(pencilPoints_.size()<20000&&dist(pencilPoints_.back(),here)>=step){
    pencilPoints_.push_back(here);
    QPainterPath stroke;stroke.moveTo(toQt(pencilPoints_.front()));
    for(size_t i=1;i<pencilPoints_.size();++i)stroke.lineTo(toQt(pencilPoints_[i]));
    pendingPreview_->setPath(stroke);++frameUpdates;
   }
   e->accept();return;
  }
  if(strokeDrawing_){drawCurrent_=e->position();pendingPreview_->setPath(temporaryShape());frameUpdates++;e->accept();return;}
  if(tool==Tool::Scissors){updateScissorHover(e->position().toPoint());e->accept();return;}
  QGraphicsView::mouseMoveEvent(e);
 }
void mouseReleaseEvent(QMouseEvent* e)override{
 if(e->button()==Qt::MiddleButton){
  if(middlePanning_){middlePanning_=false;viewport()->unsetCursor();}
  e->accept();return;
 }
 if(e->button()!=Qt::LeftButton){QGraphicsView::mouseReleaseEvent(e);return;}
  if(nativeMode_){
   if(nativeContourPressing_&&nativeSession_){
    // O ponto inicial está preservado no CurveDragSession.
    const bool moved=nativeSession_->commitContourMove(fromQt(mapToScene(e->position().toPoint())));
    nativeContourPressing_=false;updateNativePaths(true);
    if(moved&&diagnosticEvent)diagnosticEvent("bezier_contour_moved",1);
    if(stateChanged)stateChanged();
   }
   if(nativePressing_&&nativeSession_){
    const auto pt=mapToScene(e->position().toPoint());
    nativeSession_->update(fromQt(pt));const bool changed=nativeSession_->commit();nativePressing_=false;
    updateNativePaths(false);if(changed&&diagnosticEvent)diagnosticEvent("bezier_node_moved",1);
    if(stateChanged)stateChanged();
   }
   e->accept();return;
  }
   if(transformDragging_){updateSelectionTransform(e->position(),e->modifiers());finishSelectionTransform();e->accept();return;}
   if(marqueeActive_){finishMarquee(e->position(),e->modifiers());e->accept();return;}
  if(tool==Tool::Line&&polylineDrawing_){
   // Mantem gesto legada: arrastar logo no primeiro segmento cria uma linha.
   if(polylineStartedOnPress_&&QLineF(polylineDown_,e->position()).length()>4.){
    const Pt at=snapPolyline(fromQt(mapToScene(e->position().toPoint())),
                             e->modifiers().testFlag(Qt::ShiftModifier));
    if(dist(polylinePoints_.front(),at)>1e-6){
     polylinePoints_.push_back(at);
     finishPolyline(false);
    }
   }
   polylineStartedOnPress_=false;e->accept();return;
  }
  if(shapeDragging_){
   const QPointF delta=mapToScene(e->position().toPoint())-mapToScene(shapeDragStart_.toPoint());
   clearShapePreview();
   if(QLineF(QPointF{},delta).length()>1e-8){
    const int changed=int(selected.size());
    editor.moveObjects({selected.begin(),selected.end()},fromQt(delta));
    rebuild();
    if(diagnosticEvent)diagnosticEvent("shapes_moved",changed);
   }
   if(stateChanged)stateChanged();e->accept();return;
  }
  if(draggingNode_&&active_){
   auto inverse=viewportTransform().inverted();QPointF delta=inverse.map(e->position())-inverse.map(dragStart_);
   auto key=*active_;
   auto node=*draggingNode_;
   endChunkPreview();
   editor.editNode(key,node,dragOriginal_+fromQt(delta));
   if(diagnosticEvent)diagnosticEvent("bezier_node_moved",1);
   draggingNode_.reset();pathItem(key)->setPath(pathOf(*get(editor.state().doc,key)));
   // editor.editNode incrementou geometryGeneration: reconstrução automática no próximo clique.
   updateNodeMarkers(viewportTransform().map(toQt(dragOriginal_))+(drawCurrent_-dragStart_),
                     viewportTransform().map(toQt(dragOriginal_))+ (e->position()-dragStart_));
   e->accept();return;
  }
  if(strokeDrawing_&&tool==Tool::Pencil){
   const Pt final=fromQt(mapToScene(e->position().toPoint()));
   if(pencilPoints_.size()<20000&&dist(pencilPoints_.back(),final)>1e-8)
    pencilPoints_.push_back(final);
   if(pencilPoints_.size()>=2){
    lastCreated_=editor.addContour(std::move(pencilPoints_),false,targetLayer());
    ++toolCreates;rebuild();activate({lastCreated_,0});
    if(diagnosticEvent)diagnosticEvent("pencil_stroke_created",1);
   }
   pencilPoints_.clear();strokeDrawing_=false;
   if(pendingPreview_){scene_.removeItem(pendingPreview_);delete pendingPreview_;pendingPreview_=nullptr;}
   if(stateChanged)stateChanged();e->accept();return;
  }
  if(strokeDrawing_){
   drawCurrent_=e->position();auto first=mapToScene(drawStart_.toPoint());auto last=mapToScene(drawCurrent_.toPoint());
   last=constrainedCorner(first,last,e->modifiers().testFlag(Qt::ShiftModifier));
   QPointF a{std::min(first.x(),last.x()),std::min(first.y(),last.y())}, b{std::max(first.x(),last.x()),std::max(first.y(),last.y())};
   if(QLineF(first,last).length()>1e-5&&(tool==Tool::Line||(b.x()>a.x()&&b.y()>a.y()))){
    if(tool==Tool::Rectangle)lastCreated_=editor.rectangle(fromQt(a),b.x()-a.x(),b.y()-a.y(),targetLayer());
    else if(tool==Tool::Line)lastCreated_=editor.line(fromQt(first),fromQt(last),targetLayer());
    else if(tool==Tool::Ellipse)lastCreated_=editor.ellipse(fromQt((a+b)*.5),(b.x()-a.x())*.5,(b.y()-a.y())*.5,512,targetLayer());
    else if(tool==Tool::Polygon){double r=std::min(b.x()-a.x(),b.y()-a.y())*.5;if(r>1e-5)lastCreated_=editor.regularPolygon(fromQt((a+b)*.5),r,6,targetLayer());}
    ++toolCreates;rebuild();activate({lastCreated_,0});
    if(diagnosticEvent)diagnosticEvent("shape_created",1);
   }
   strokeDrawing_=false;
   if(pendingPreview_){scene_.removeItem(pendingPreview_);delete pendingPreview_;pendingPreview_=nullptr;}
   viewport()->update();if(stateChanged)stateChanged();e->accept();return;
  }
  QGraphicsView::mouseReleaseEvent(e);
 }
 void contextMenuEvent(QContextMenuEvent* e)override{
  if(tool==Tool::Line||nativeMode_||busy()){e->accept();return;}
  QMenu menu(this);
  QAction* all=menu.addAction("Selecionar tudo (Ctrl+A)");
  QAction* invert=menu.addAction("Inverter seleção (Ctrl+Shift+I)");
  QAction* clear=menu.addAction("Limpar seleção");
  menu.addSeparator();
  QAction* duplicate=menu.addAction("Duplicar forma (+3 mm)");
  QAction* group=menu.addAction("Agrupar seleção");
  QAction* ungroup=menu.addAction("Desagrupar seleção (Ctrl+U)");
  QAction* ungroupScissors=menu.addAction("Desagrupar e ativar Tesoura");
  QAction* remove=menu.addAction("Excluir selecionados (Delete)");
  duplicate->setEnabled(selected.size()==1);
  group->setEnabled(selected.size()>=2);
  bool grouped=false;
  for(int id:selected){
   auto it=editor.state().objects.find(id);
   if(it!=editor.state().objects.end()&&it->second.group>0)grouped=true;
  }
  ungroup->setEnabled(grouped);
  ungroupScissors->setEnabled(grouped);
  remove->setEnabled(!selected.empty());
  menu.addSeparator();
  QAction* undo=menu.addAction("Desfazer (Ctrl+Z)");
  QAction* redo=menu.addAction("Refazer (Ctrl+Y)");
  menu.addSeparator();
  auto* tools=menu.addMenu("Ferramentas");
  QAction* select=tools->addAction("Selecionar (V)");
  QAction* line=tools->addAction("Linha contínua");
  QAction* scissors=tools->addAction("Tesoura (K)");
  QAction* nodes=tools->addAction("Editar nós");
  menu.addSeparator();
  QAction* traceBitmap=menu.addAction("Traçar imagem (Alt+T) — P&B");
  QAction* clearBitmap=menu.addAction("Remover imagem de referência");
  traceBitmap->setEnabled(hasRasterImage());
  clearBitmap->setEnabled(hasRasterImage());
  QAction* picked=menu.exec(e->globalPos());
  if(!picked){e->accept();return;}
  try{
   if(picked==traceBitmap&&requestTraceImage)requestTraceImage();
   else if(picked==clearBitmap)clearRasterImage();
   else if(picked==all)selectAllVisible();
   else if(picked==invert)invertSelection();
   else if(picked==clear)clearSelection();
   else if(picked==remove)deleteSelectedShapes();
   else if(picked==undo)history(false);
   else if(picked==redo)history(true);
   else if(picked==duplicate&&selected.size()==1){
    const int id=editor.duplicate(*selected.begin(),{3,3});
    selected.clear();active_.reset();rebuild();activate({id,0});
   }else if(picked==group&&selected.size()>=2){
    editor.group(std::vector<int>(selected.begin(),selected.end()));rebuild();
   }else if((picked==ungroup||picked==ungroupScissors)&&grouped){
    const int count=editor.ungroupSelection({selected.begin(),selected.end()});
    rebuild();
    if(diagnosticEvent)diagnosticEvent("groups_ungrouped",count);
    if(picked==ungroupScissors&&requestToolChange)requestToolChange(Tool::Scissors);
   }else if(requestToolChange){
    if(picked==select)requestToolChange(Tool::Select);
    else if(picked==line)requestToolChange(Tool::Line);
    else if(picked==scissors)requestToolChange(Tool::Scissors);
    else if(picked==nodes)requestToolChange(Tool::Nodes);
   }
   if(stateChanged)stateChanged();
  }catch(const std::exception& ex){if(toolWarning)toolWarning(QString::fromUtf8(ex.what()));}
  e->accept();
 }
 void keyReleaseEvent(QKeyEvent* e)override{
  if(e->key()==Qt::Key_Shift&&strokeDrawing_&&pendingPreview_){
   pendingPreview_->setPath(temporaryShape());viewport()->update();
  }
  QGraphicsView::keyReleaseEvent(e);
 }
 void keyPressEvent(QKeyEvent* e)override{
  if(nativeMode_&&nativeSession_){
   if(e->key()==Qt::Key_Escape&&nativeContourPressing_){
    nativeSession_->cancel();nativeContourPressing_=false;updateNativePaths(true);e->accept();return;
   }
   if(e->key()==Qt::Key_Escape&&nativePressing_){
    nativeSession_->cancel();nativePressing_=false;updateNativePaths(false);e->accept();return;
   }
   if(e->key()==Qt::Key_Delete&&!nativePressing_&&!nativeContourPressing_){
    if(e->modifiers().testFlag(Qt::ControlModifier))nativeDeleteContour();
    else nativeDeleteSelected();
    e->accept();return;
   }
   if(e->modifiers().testFlag(Qt::ControlModifier)&&!nativePressing_&&
      (e->key()==Qt::Key_Z||e->key()==Qt::Key_Y)){
    nativeHistory(e->key()==Qt::Key_Y||e->modifiers().testFlag(Qt::ShiftModifier));
    e->accept();return;
   }
  }
  if(e->modifiers() & Qt::ControlModifier){
   if(e->key()==Qt::Key_Z){history(e->modifiers().testFlag(Qt::ShiftModifier));e->accept();return;}
   if(e->key()==Qt::Key_Y){history(true);e->accept();return;}
  }
  if(e->modifiers()==Qt::ControlModifier&&e->key()==Qt::Key_A&&tool==Tool::Select&&!nativeMode_){
   selectAllVisible();e->accept();return;
  }
  if(e->modifiers()==(Qt::ControlModifier|Qt::ShiftModifier)&&e->key()==Qt::Key_I&&tool==Tool::Select&&!nativeMode_){
   invertSelection();e->accept();return;
  }
  if(e->key()==Qt::Key_Delete&&tool==Tool::Select&&!nativeMode_){
   deleteSelectedShapes();e->accept();return;
  }
  if(e->key()==Qt::Key_Shift&&strokeDrawing_&&pendingPreview_){
   pendingPreview_->setPath(temporaryShape());viewport()->update();
  }
   if(e->key()==Qt::Key_Escape){
    if(transformDragging_){cancelSelectionTransform();e->accept();return;}
    if(polylineDrawing_){clearPolyline();e->accept();return;}
   if(marqueeActive_){clearMarquee();e->accept();return;}
   if(!busy()){if(requestSelectTool)requestSelectTool();e->accept();return;}
   if(shapeDragging_){clearShapePreview();e->accept();return;}
   if(draggingNode_&&active_){
    const auto oldMarker=viewportTransform().map(toQt(dragOriginal_))+(drawCurrent_-dragStart_);
    const auto originalMarker=viewportTransform().map(toQt(dragOriginal_));
    endChunkPreview();
    draggingNode_.reset();updateNodeMarkers(oldMarker,originalMarker);
    e->accept();return;
   }
   if(strokeDrawing_){strokeDrawing_=false;pencilPoints_.clear();if(pendingPreview_){scene_.removeItem(pendingPreview_);delete pendingPreview_;pendingPreview_=nullptr;}viewport()->update();e->accept();return;}
  }
  QGraphicsView::keyPressEvent(e);
 }
};
static std::string stateText(const EditorState& s){return serialize(encode(s.doc))+"\n"+serialize(metadata(s));}
static QByteArray fingerprint(const EditorState& s){auto text=stateText(s);return QCryptographicHash::hash(QByteArray(text.data(),qsizetype(text.size())),QCryptographicHash::Sha256).toHex();}
// Persistência v5 (curvas nativas + geometria poligonal). Mantém leitura v4 legada.
static IntegratedProject saveNative(const QString& file,const EditorState& state,
                                    const CurveDocument& curves=CurveDocument{},
                                    const IntegratedProject* previous=nullptr){
 QTemporaryDir temp;check(temp.isValid(),"temporary directory failed");
 auto zipped=temp.path()+"/project.alaser";
 IntegratedProject project=buildEditedIntegratedProject(state,curves,previous);
 writeIntegratedProject(zipped.toStdString(),project);
 QFile source(zipped);check(source.open(QIODevice::ReadOnly),"cannot read temporary package");
 QSaveFile target(file);target.setDirectWriteFallback(false);check(target.open(QIODevice::WriteOnly),"cannot open native destination");
 auto bytes=source.readAll();check(target.write(bytes)==bytes.size()&&target.commit(),"native atomic commit failed");
 return project;
}
static IntegratedProject openNative(const QString& file){
 return readIntegratedProject(file.toStdString());
}
static double percentile(std::vector<double> values,double q){if(values.empty())return 0;std::sort(values.begin(),values.end());return values[size_t((values.size()-1)*q)];}
// Vetorizador unico usado pelo botao Texto e pela regressao Qt; nao envia G-code.
// Tesselação adaptativa de Bézier cúbica de fonte: erro geométrico em mm.
// QPainterPath::toSubpathPolygons usa tolerância visual no espaço do glifo e
// cria facetas visíveis quando o usuário aproxima letras (ex.: R, O, S).
static double cubicFlatness(Pt a,Pt b,Pt c,Pt d){
 const double dx=d.x-a.x,dy=d.y-a.y,length=std::hypot(dx,dy);
 if(length<1e-9)return std::max(dist(a,b),dist(a,c));
 return std::max(std::abs((b.x-a.x)*dy-(b.y-a.y)*dx),
                 std::abs((c.x-a.x)*dy-(c.y-a.y)*dx))/length;
}
static void appendAdaptiveCubic(std::vector<Pt>& out,Pt a,Pt b,Pt c,Pt d,
                                double tolerance,int depth=0){
 if(out.size()>=80000)throw std::runtime_error("Texto excedeu limite seguro de nos");
 if(depth>=15||cubicFlatness(a,b,c,d)<=tolerance){
  if(out.empty()||dist(out.back(),d)>1e-8)out.push_back(d);
  return;
 }
 const Pt ab=(a+b)*.5,bc=(b+c)*.5,cd=(c+d)*.5;
 const Pt abc=(ab+bc)*.5,bcd=(bc+cd)*.5,m=(abc+bcd)*.5;
 appendAdaptiveCubic(out,a,ab,abc,m,tolerance,depth+1);
 appendAdaptiveCubic(out,m,bcd,cd,d,tolerance,depth+1);
}
static std::vector<std::vector<Pt>> vectorTextOutlines(const QString& value,QFont font,
                                                        double height,Pt origin){
 if(value.trimmed().isEmpty()||value.size()>100||!std::isfinite(height)||height<1||height>150)
  throw std::runtime_error("Texto ou altura fora do limite");
 font.setPixelSize(1024); // escala interna alta; coordenadas finais continuam em mm
 QPainterPath outline;outline.addText(QPointF(0,0),font,value);
 if(outline.isEmpty())throw std::runtime_error("A fonte nao produziu contornos");
 const QRectF bounds=outline.boundingRect();
 if(bounds.height()<1e-6)throw std::runtime_error("Fonte sem altura util");
 QTransform t;
 t.translate(origin.x,origin.y);
 const double scale=height/bounds.height();t.scale(scale,scale);
 t.translate(-bounds.left(),-bounds.top());
 auto transformed=[&](const QPainterPath::Element& e)->Pt {
  const QPointF p=t.map(QPointF(e.x,e.y));return {p.x(),p.y()};
 };
 std::vector<std::vector<Pt>> shapes;
 std::vector<Pt> current;
 auto complete=[&]{
  if(current.size()>1&&dist(current.front(),current.back())<1e-7)current.pop_back();
  if(current.size()>=3)shapes.push_back(std::move(current));
  current.clear();
 };
 constexpr double toleranceMm=.016; // erro máximo local de curva, não arredonda cantos
 for(int i=0;i<outline.elementCount();++i){
  const auto e=outline.elementAt(i);
  if(e.type==QPainterPath::MoveToElement){
   complete();current.push_back(transformed(e));continue;
  }
  if(e.type==QPainterPath::LineToElement){
   Pt p=transformed(e);
   if(current.empty()||dist(current.back(),p)>1e-8)current.push_back(p);
   continue;
  }
  if(e.type==QPainterPath::CurveToElement){
   if(i+2>=outline.elementCount()||current.empty())
    throw std::runtime_error("Fonte com curva incompleta");
   const Pt b=transformed(e),c=transformed(outline.elementAt(i+1)),
            d=transformed(outline.elementAt(i+2));
   appendAdaptiveCubic(current,current.back(),b,c,d,toleranceMm);
   i+=2;
   continue;
  }
  if(e.type==QPainterPath::CurveToDataElement)
   throw std::runtime_error("Controle Bezier inesperado no texto");
 }
 complete();
 size_t total=0;
 for(const auto& polygon:shapes)total+=polygon.size();
 if(shapes.empty()||total>80000)throw std::runtime_error("Texto sem contornos validos ou muito complexo");
 return shapes;
}

class LabWindow final:public QWidget{
public:
 QtEditorLab view;
 // Sessão Bézier nativa (v5). Quando vazia, o editor opera só em polilinhas (compatível TESTE_6).
 std::unique_ptr<CurveDragSession> curves;
 std::map<QString,QPushButton*> buttons;
  QComboBox* layers;QLineEdit* layerName;QCheckBox* visible;QCheckBox* locked;QLabel* status;
 QString projectPath,lastError;bool refreshing=false;
 std::optional<IntegratedProject> loadedProject_; // operações, origem e metadados adicionais preservados
 bool bezierToolActive_=false;
 QLineEdit* inlineTextEntry_=nullptr;
 Pt inlineTextAnchor_{};
 double inlineTextHeightMm_=12.;
 QFont inlineTextFont_{"Arial",12};
 bool darkTheme_=true;
 // Persistent local preferences and crash recovery; no automatic machine connection.
 alaser::RecoveryPolicy recoveryPolicy_;
 QSplitter* persistentSplitter_=nullptr;
 QTimer* recoveryTimer_=nullptr;
 bool restoringSnapshot_=false;
 bool uiPersistenceEnabled_=false;
 WorkspaceConfig workspaceCfg_{};
 QLabel* workspaceSummary_=nullptr;
 QLabel* workspaceTopMeta_=nullptr;
 QComboBox* machineProfileSelector_=nullptr;
 DiagnosticsCollector collector_;
 alaser::grbl::WindowsSerialObserver serial_;
 alaser::grbl::Observation controller_;
 std::string pendingSerial_;
 QLabel* machineLiveStatus_=nullptr;
 QPlainTextEdit* machineConsole_=nullptr;
 QTcpSocket* wifi_=nullptr; // Qt event loop: nonblocking; no auto reconnect or discovery
 QLabel* transportHint_=nullptr;
  std::vector<QPushButton*> layerPalette_;
  QComboBox* layerMode_=nullptr;
  std::vector<QShortcut*> editingShortcuts_;
 bool machineConnected()const{return serial_.connected()||(wifi_&&wifi_->state()==QAbstractSocket::ConnectedState);}
 bool wifiConnected()const{return wifi_&&wifi_->state()==QAbstractSocket::ConnectedState;}
 size_t machineUpdates_=0;
 size_t machineProbes_=0;
 QLabel* machineHeaderStatus_=nullptr;
 QLabel* collectorSummary_=nullptr;
 QPlainTextEdit* collectorLog_=nullptr;
 // Diagnostics must NEVER let an exception cross a Qt event-loop callback.
 void captureEvent(const QString& code,int n=1) noexcept{
  try{
   if(!collector_.record(code.toStdString(),n))return;
   if(collectorLog_)collectorLog_->appendPlainText(
    QString("+%1 s  %2  (%3)").arg(collector_.elapsedMs()/1000).arg(code).arg(n));
   updateCaptureSummary();
  }catch(const std::exception&){
   // Never terminate an editing session for a telemetry error.
  }catch(...){
   // Protect the GUI even if an unexpected telemetry exception occurs.
  }
 }
 // Save a separate .alaser v5 snapshot, NEVER the user's original document.
 void snapshotForRecovery(){
  if(!uiPersistenceEnabled_||!recoveryPolicy_.needsSnapshot()||view.busy()||(curves&&curves->dragging()))return;
  try{
   const auto cd=curves?curves->committed():CurveDocument{};
   const QString folder=alaser::PersistentSessionStore::folder();
   if(folder.isEmpty()||!QDir().mkpath(folder))throw std::runtime_error("recovery folder unavailable");
   const QString target=alaser::PersistentSessionStore::snapshotPath();
   saveNative(target,view.editor.state(),cd,loadedProject_?&*loadedProject_:nullptr);
   if(!alaser::PersistentSessionStore::writeMarker(projectPath))
    throw std::runtime_error("could not commit recovery manifest");
   recoveryPolicy_.snapshotted();
  }catch(...){
   alaser::PersistentSessionStore::recordErrorCode("autosave_failed");
   // Recovery is best-effort and must never close or freeze the editor.
  }
 }
 void recoverAtStartup(){
  if(!uiPersistenceEnabled_||!alaser::PersistentSessionStore::hasSnapshot())return;
  const auto decision=QMessageBox::question(this,"Recuperação de trabalho — Alencar Laser",
   "Foi encontrada uma cópia de recuperação local.\n\nDeseja abrir essa cópia SEM substituir o projeto original?",
   QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
  if(decision!=QMessageBox::Yes)return;
  try{
   restoringSnapshot_=true;
   projectPath=alaser::PersistentSessionStore::snapshotPath();
   open();
   // Recovery is intentionally unsaved; Ctrl+S asks for a NEW destination.
   projectPath.clear();
   recoveryPolicy_.edited();
   restoringSnapshot_=false;
   status->setText("Cópia de recuperação aberta. Use Salvar como para criar um novo arquivo .alaser.");
  }catch(...){
   restoringSnapshot_=false;
   projectPath.clear();
   QMessageBox::warning(this,"Recuperação","Não foi possível abrir a cópia. Os arquivos de recuperação foram preservados.");
   alaser::PersistentSessionStore::recordErrorCode("recovery_open_failed");
  }
 }
 void closeEvent(QCloseEvent* event) override {
  if(uiPersistenceEnabled_&&recoveryPolicy_.dirty()){
   const auto choice=QMessageBox::warning(this,"Alterações não salvas",
    "Há alterações ainda não salvas no projeto. Deseja salvá-las antes de fechar?",
    QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel,QMessageBox::Save);
   if(choice==QMessageBox::Cancel){event->ignore();return;}
   if(choice==QMessageBox::Save){
    try{save();}catch(...){event->ignore();return;}
    if(recoveryPolicy_.dirty()){event->ignore();return;}
   }else{
    alaser::PersistentSessionStore::discard();
   }
  }
  if(uiPersistenceEnabled_){
   QSettings settings("AlencarLaser","DesktopBeta");
   settings.setValue("ui/windowGeometry",saveGeometry());
   if(persistentSplitter_)settings.setValue("ui/splitterState",persistentSplitter_->saveState());
   settings.sync();
  }
  QWidget::closeEvent(event);
 }
 void updateCaptureSummary(){
  if(!collectorSummary_)return;
  collectorSummary_->setText(QString("Estado: %1\nEventos: %2  •  limite: %3  •  descartados: %4\nTempo ativo da coleta: %5 s")
   .arg(collector_.active()?"GRAVANDO (memória local)":"PARADO")
   .arg(collector_.records().size()).arg(collector_.limit()).arg(collector_.dropped())
   .arg(collector_.elapsedMs()/1000));
 }
 QJsonObject captureDocument()const{
  QJsonObject counters;for(const auto& [name,n]:collector_.counters())counters[QString::fromStdString(name)]=n;
  QJsonArray events;for(const auto& e:collector_.records())events.append(QJsonObject{
   {"event",QString::fromStdString(e.type)},{"objects",e.objects},{"elapsed_ms",double(e.elapsedMs)}});
  return {{"schema","alencar-laser-diagnostics-3"},{"version","TESTE_19.8"},
   {"exported_utc",QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
   {"collection_active",collector_.active()},{"events_retained",int(collector_.records().size())},
   {"events_dropped",double(collector_.dropped())},{"elapsed_ms",double(collector_.elapsedMs())},
   {"events",events},{"counts",counters},{"project_summary",QJsonObject{
    {"polyline_contours",int(view.editor.state().doc.contours.size())},
    {"polyline_nodes",double(countNodes(view.editor.state().doc))},
    {"bezier_contours",int(curves?curves->committed().contours.size():0)},
    {"selected_polyline_objects",int(view.selected.size())},
    {"undo_available",int(view.editor.undoCount())},{"redo_available",int(view.editor.redoCount())}}},
   {"performance",QJsonObject{{"paint_samples",int(view.paintMs.size())},
      {"paint_p50_ms",percentile(view.paintMs,.5)},
      {"paint_p95_ms",percentile(view.paintMs,.95)},
      {"paint_p99_ms",percentile(view.paintMs,.99)},
      {"paint_max_ms",view.paintMs.empty()?0:*std::max_element(view.paintMs.begin(),view.paintMs.end())},
      {"input_to_paint_p95_ms",percentile(view.inputToPaintMs,.95)},
      {"hit_test_p95_ms",percentile(view.hitTestMs,.95)},
      {"wheel_p95_ms",percentile(view.wheelMs,.95)},
      {"wheel_samples",int(view.wheelMs.size())},
      {"rss_bytes",double(readResidentMemory())},
      {"rss_not_collected",readResidentMemory()==0}}},
   {"environment",QJsonObject{{"qt",qVersion()},{"platform",QGuiApplication::platformName()},
      {"os",QSysInfo::prettyProductName()},{"machine_connected",machineConnected()},
      {"machine_transport",wifiConnected()?"TCP Wi-Fi":(serial_.connected()?"USB":"none")}}},
   {"machine_diagnostics",QJsonObject{{"observed_lines",double(machineUpdates_)},
       {"diagnostic_probes_sent",double(machineProbes_)},
       {"tcp_connected",wifiConnected()},
       {"controller_confirmed",controller_.grblBanner},
       {"state",QString::fromStdString(controller_.state)},
       {"raw_responses_exported",false}}},
   {"privacy","Opt-in, memory only, manual JSON export; no paths, device identifiers, raw machine messages or design coordinates."}};
 }
 void exportCapture(){
  const QString path=QFileDialog::getSaveFileName(this,"Exportar diagnóstico local", 
   "Alencar_Laser_Diagnostico_TESTE_19_7.json","JSON (*.json)");
  if(path.isEmpty())return;
  QSaveFile file(path);
  if(!file.open(QIODevice::WriteOnly))throw std::runtime_error("cannot open diagnostics destination");
  const QByteArray content=QJsonDocument(captureDocument()).toJson(QJsonDocument::Indented);
  if(file.write(content)!=content.size()||!file.commit())throw std::runtime_error("cannot save diagnostics file");
  status->setText("Diagnóstico JSON exportado localmente. Nenhum dado enviado pela rede.");
 }
 static quint64 readResidentMemory(){
#ifdef Q_OS_WIN
  PROCESS_MEMORY_COUNTERS_EX pmc{};
  if(GetProcessMemoryInfo(GetCurrentProcess(),reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),sizeof(pmc)))return quint64(pmc.WorkingSetSize);
#endif
  return 0;
 }
 void observeSerialData(const std::string& bytes){
  pendingSerial_+=bytes;
  // Drop untrusted incomplete messages before they grow without bound.
  if(pendingSerial_.size()>8192)pendingSerial_.clear();
  size_t pos=0;
  while((pos=pendingSerial_.find_first_of("\r\n"))!=std::string::npos){
   std::string line=pendingSerial_.substr(0,pos);
   pendingSerial_.erase(0,pos+1);
   if(line.empty())continue;
   controller_.accept(line);
   ++machineUpdates_;
   // No raw firmware serial numbers or coordinates enter console/JSON.
   if(machineConsole_){
    QString type="Resposta recebida";
    if(line.rfind("Grbl ",0)==0 || line.rfind("grbl ",0)==0)type="Banner GRBL identificado";
    else if(line.rfind("<",0)==0)type="Estado: "+QString::fromStdString(controller_.state);
    else if(line.rfind("ALARM:",0)==0)type="Alarme informado pela controladora";
    else if(line.rfind("error:",0)==0)type="Erro retornado pela controladora";
    else if(line=="ok")type="Consulta confirmada";
    machineConsole_->appendPlainText(type);
   }
   if(collector_.active())captureEvent("grbl_received");
  }
  refreshMachineStatus();
 }
 void refreshMachineStatus(){
  if(!machineLiveStatus_)return;
  if(machineHeaderStatus_){
   machineHeaderStatus_->setText(machineConnected()?"● DIAGNÓSTICO CONECTADO | EMISSÃO BLOQUEADA":"● MÁQUINA DESCONECTADA | EMISSÃO BLOQUEADA");
   machineHeaderStatus_->setToolTip("TESTE 19.7: USB ou Wi-Fi/TCP somente leitura; sem comandos de movimento ou laser.");
  }
  machineLiveStatus_->setText(QString("Transporte: %1\nGRBL identificado: %2\nEstado reportado: %3\nConsultas realizadas: %4\nEmissão/movimento: BLOQUEADOS")
    .arg(wifiConnected()?"Wi-Fi/TCP (leitura)":(serial_.connected()?"USB (leitura)":"DESCONECTADO"))
    .arg(controller_.grblBanner?"sim":"não confirmado")
    .arg(QString::fromStdString(controller_.state)).arg(qulonglong(machineProbes_)));
 }
 static bool isInteractive(){
  const auto args=QCoreApplication::arguments();
  return args.size()==1||args.value(1)=="--interactive";
 }
 void loadWorkspaceSettings(){
  QSettings settings("AlencarLaser","DesktopBeta");
  auto read=[&](const char* key,double fallback){
   const double n=settings.value(key,fallback).toDouble();
   return WorkspaceConfig::goodDimension(n)?n:fallback;
  };
  workspaceCfg_.drawingWidth=read("workspace/drawingWidth",400);
  workspaceCfg_.drawingHeight=read("workspace/drawingHeight",400);
  workspaceCfg_.machineWidth=read("workspace/machineWidth",400);
  workspaceCfg_.machineHeight=read("workspace/machineHeight",400);
  workspaceCfg_.showGrid=settings.value("workspace/grid",true).toBool();
  workspaceCfg_.gridToBed=settings.value("workspace/gridToBed",true).toBool();
  workspaceCfg_.showDrawingFrame=settings.value("workspace/drawingFrame",true).toBool();
  workspaceCfg_.showMachineFrame=settings.value("workspace/machineFrame",true).toBool();
  workspaceCfg_.showOrigin=settings.value("workspace/origin",true).toBool();
  const QString profile=settings.value("workspace/profile","AlgoLaser Alpha Mk2 20W").toString();
  workspaceCfg_.profile=(profile=="Personalizado")?"Personalizado":"AlgoLaser Alpha Mk2 20W";
  workspaceCfg_.validate();
 }
 void persistWorkspaceSettings(){
  QSettings settings("AlencarLaser","DesktopBeta");
  settings.setValue("workspace/drawingWidth",workspaceCfg_.drawingWidth);
  settings.setValue("workspace/drawingHeight",workspaceCfg_.drawingHeight);
  settings.setValue("workspace/machineWidth",workspaceCfg_.machineWidth);
  settings.setValue("workspace/machineHeight",workspaceCfg_.machineHeight);
  settings.setValue("workspace/grid",workspaceCfg_.showGrid);
  settings.setValue("workspace/gridToBed",workspaceCfg_.gridToBed);
  settings.setValue("workspace/drawingFrame",workspaceCfg_.showDrawingFrame);
  settings.setValue("workspace/machineFrame",workspaceCfg_.showMachineFrame);
  settings.setValue("workspace/origin",workspaceCfg_.showOrigin);
  settings.setValue("workspace/profile",QString::fromStdString(workspaceCfg_.profile));
  settings.sync();
 }
 void refreshWorkspaceLabels(){
  const QString draw=QString("%1 × %2 mm").arg(workspaceCfg_.drawingWidth,0,'f',0).arg(workspaceCfg_.drawingHeight,0,'f',0);
  const QString machine=QString("%1 × %2 mm").arg(workspaceCfg_.machineWidth,0,'f',0).arg(workspaceCfg_.machineHeight,0,'f',0);
  if(workspaceSummary_)workspaceSummary_->setText("Área de desenho: "+draw+"\nMesa configurada: "+machine);
  if(workspaceTopMeta_)workspaceTopMeta_->setText("Desenho "+draw+"  ·  Mesa "+machine);
  if(machineProfileSelector_){
   QSignalBlocker block(machineProfileSelector_);
   machineProfileSelector_->setCurrentIndex(workspaceCfg_.profile=="Personalizado"?1:0);
  }
 }
 void configureWorkspace(){
  QDialog dialog(this);dialog.setWindowTitle("Área de desenho e mesa — Alencar Laser");
  dialog.setMinimumWidth(420);
  auto* vertical=new QVBoxLayout(&dialog);
  auto* note=new QLabel("Dimensões apenas visuais. NÃO alteram limites GRBL nem autorizam movimentação.");
  note->setWordWrap(true);vertical->addWidget(note);
  auto* presets=new QComboBox;
  presets->addItems({"Dimensões atuais / personalizadas","400 × 400 mm","400 × 800 mm","800 × 800 mm","1000 × 1000 mm"});
  vertical->addWidget(presets);
  auto* form=new QFormLayout;
  auto spin=[&](double n){auto* box=new QDoubleSpinBox;box->setRange(1,WorkspaceConfig::MaxEditableDimension);
   box->setDecimals(2);box->setSingleStep(50);box->setSuffix(" mm");box->setValue(n);return box;};
  auto* drawingW=spin(workspaceCfg_.drawingWidth),*drawingH=spin(workspaceCfg_.drawingHeight);
  auto* machineW=spin(workspaceCfg_.machineWidth),*machineH=spin(workspaceCfg_.machineHeight);
  form->addRow("Desenho — largura:",drawingW);form->addRow("Desenho — altura:",drawingH);
  form->addRow("Mesa — largura:",machineW);form->addRow("Mesa — altura:",machineH);
  vertical->addLayout(form);
  connect(presets,QOverload<int>::of(&QComboBox::currentIndexChanged),&dialog,
    [=](int index){
      if(index==0)return;
      static const double dims[4][2]={{400,400},{400,800},{800,800},{1000,1000}};
      machineW->setValue(dims[index-1][0]);machineH->setValue(dims[index-1][1]);
    });
  auto* grid=new QCheckBox("Mostrar grade");grid->setChecked(workspaceCfg_.showGrid);
  auto* gridToBed=new QCheckBox("Grade segue a mesa configurada (em vez do desenho)");
  gridToBed->setChecked(workspaceCfg_.gridToBed);
  auto* frame=new QCheckBox("Mostrar contorno da mesa");frame->setChecked(workspaceCfg_.showMachineFrame);
  auto* drawFrame=new QCheckBox("Mostrar limite da area de desenho");drawFrame->setChecked(workspaceCfg_.showDrawingFrame);
  auto* origin=new QCheckBox("Mostrar origem (0,0)");origin->setChecked(workspaceCfg_.showOrigin);
  vertical->addWidget(grid);vertical->addWidget(gridToBed);vertical->addWidget(drawFrame);vertical->addWidget(frame);vertical->addWidget(origin);
  auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
  connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);vertical->addWidget(buttons);
  if(dialog.exec()!=QDialog::Accepted)return;
  WorkspaceConfig next=workspaceCfg_;
  next.drawingWidth=drawingW->value();next.drawingHeight=drawingH->value();
  next.machineWidth=machineW->value();next.machineHeight=machineH->value();
  next.showGrid=grid->isChecked();next.gridToBed=gridToBed->isChecked();
  next.showMachineFrame=frame->isChecked();next.showDrawingFrame=drawFrame->isChecked();next.showOrigin=origin->isChecked();
  next.validate();workspaceCfg_=next;
  view.setWorkspace(workspaceCfg_,isInteractive());persistWorkspaceSettings();refreshWorkspaceLabels();
  view.setFocus();
 }

 // A interface nao envia comandos a laser; o painel exibe somente estado local.
 void applyTheme(){
  if(darkTheme_){
   setStyleSheet(R"QSS(
    QWidget#alencarRoot { background:#121519; color:#E6ECF3; font-family:'Segoe UI'; font-size:12px; }
    QFrame#topHeader { background:#161B21; border-bottom:1px solid #303740; }
    QFrame#quickActions { background:#1A2027; border-bottom:1px solid #2D343D; }
    QFrame#quickActions QPushButton { padding:4px 7px; min-height:22px; font-size:11px; }
    QFrame#leftPanel,QFrame#rightPanel { background:#192027; border:0; }
    QFrame#canvasPanel { background:#101419; border:0; }
    QFrame#bottomStatus,QFrame#layerPaletteBar { background:#1A2027; border-top:1px solid #333B45; }
    QMenuBar#webInspiredMenu { background:#111820; color:#D7E1ED; padding:3px 10px; }
    QMenuBar#webInspiredMenu::item:selected { background:#313F4E; border-radius:4px; }
    QMenu { background:#1C2833; color:#ECF1F8; border:1px solid #526273; }
    QMenu::item:selected { background:#A96323; }
    QLabel { color:#DDE5EB; background:transparent; }
    QLabel#brandName { color:#FA932C; font-weight:800; font-size:17px; }
    QLabel#brandSub { color:#9CA9B6; font-size:11px; }
    QLabel#sectionTitle { color:#FF9B3C; font-size:11px; font-weight:bold; padding:4px 1px; }
    QLabel#machineState { color:#F0BB80; background:#342515; border:1px solid #624323; border-radius:6px; padding:6px 10px; }
    QLabel#canvasMeta { color:#B7C3CD; font-size:11px; }
    QLabel#status { color:#CBD5DF; padding:4px 10px; }
    QPushButton { background:#303944; color:#F1F5F8; border:1px solid #4B5662; border-radius:6px; padding:7px 11px; font-weight:500; }
    QPushButton:hover { background:#3C4854; border-color:#EF8A2C; }
    QPushButton:disabled { background:#222930; border-color:#333B43; color:#77838E; }
    QPushButton[compact="true"] { padding:3px 2px; min-height:25px; font-size:10px; }
    QTabWidget::pane { border:0; }
    QTabBar::tab { background:#262F38; padding:6px; color:#C4D0D9; }
    QTabBar::tab:selected { background:#3B4752; color:#FFA247; }
    QScrollArea { background:transparent; border:0; }
    QPushButton:pressed { background:#614122; }
    QPushButton:checked { color:#171717; background:#F18A2C; border:1px solid #FFA856; font-weight:bold; }
    QPushButton#save { background:#A65D21; color:#FFF6EB; border-color:#E68B32; font-weight:bold; }
    QPushButton#save:hover { background:#B86B29; }
    QLineEdit,QComboBox { background:#252D35; border:1px solid #495460; border-radius:5px; color:#F0F5F9; padding:6px; min-height:20px; }
    QComboBox QAbstractItemView { background:#252D35; color:#F2F5F9; selection-background-color:#9A581D; }
    QCheckBox { color:#D7E0E8; padding:5px 0; spacing:8px; }
    QCheckBox::indicator { width:15px; height:15px; }
    QGraphicsView#drawingCanvas { border:1px solid #3B4650; border-radius:4px; }
    QSplitter::handle { background:#313B45; width:3px; }
    QToolTip { color:#FDFDFD; background:#242B32; border:1px solid #ED8B30; }
   )QSS");
   view.setBackgroundBrush(QColor("#111922"));view.setWorkspaceTheme(true);
  } else {
   setStyleSheet(R"QSS(
    QWidget#alencarRoot { background:#DFE6EB; color:#253342; font-family:'Segoe UI'; font-size:12px; }
    QFrame#topHeader { background:#D6E0E6; border-bottom:1px solid #C2CDD5; }
    QFrame#quickActions { background:#E3E9ED; border-bottom:1px solid #CED6DE; }
    QFrame#quickActions QPushButton { padding:4px 7px; min-height:22px; font-size:11px; }
    QFrame#leftPanel,QFrame#rightPanel { background:#CED7DE; border:0; }
    QFrame#canvasPanel { background:#B6C1CA; border:0; }
    QFrame#bottomStatus,QFrame#layerPaletteBar { background:#F6F8FA; border-top:1px solid #D0D8DF; }
    QMenuBar#webInspiredMenu { background:#DFE7EE; color:#23394F; padding:3px 10px; }
    QMenuBar#webInspiredMenu::item:selected { background:#FFDFC1; }
    QLabel { color:#253342; background:transparent; }
    QLabel#brandName { color:#CC6A15; font-weight:800; font-size:17px; }
    QLabel#brandSub { color:#67798B; font-size:11px; }
    QLabel#sectionTitle { color:#AB5B17; font-size:11px; font-weight:bold; padding:4px 1px; }
    QLabel#machineState { color:#985000; background:#FFF0D9; border:1px solid #E7BE84; border-radius:6px; padding:6px 10px; }
    QLabel#canvasMeta { color:#5B6A78; font-size:11px; }
    QLabel#status { color:#354658; padding:4px 10px; }
    QPushButton { background:#E9EFF3; color:#253442; border:1px solid #C7D1DC; border-radius:6px; padding:7px 11px; font-weight:500; }
    QPushButton:hover { background:#FFF0DF; border-color:#E7832C; }
    QPushButton:disabled { background:#EBEFF2; border-color:#D9DFE5; color:#8B99A5; }
    QPushButton[compact="true"] { padding:3px 2px; min-height:25px; font-size:10px; }
    QTabWidget::pane { border:0; }
    QTabBar::tab { background:#E0E6EA; padding:6px; color:#445666; }
    QTabBar::tab:selected { background:#DFE6EA; color:#BA611A; }
    QScrollArea { background:transparent; border:0; }
    QPushButton:pressed { background:#F7D6B0; }
    QPushButton:checked { color:#FFFFFF; background:#CB711D; border:1px solid #A85B15; font-weight:bold; }
    QPushButton#save { background:#CC741E; color:white; border-color:#AA5E13; font-weight:bold; }
    QLineEdit,QComboBox { background:#ECF0F3; border:1px solid #C5D0DA; border-radius:5px; color:#253442; padding:6px; min-height:20px; }
    QCheckBox { color:#324354; padding:5px 0; spacing:8px; }
    QGraphicsView#drawingCanvas { border:1px solid #CDD5DE; border-radius:4px; }
    QSplitter::handle { background:#C9D4DF; width:3px; }
   )QSS");
   view.setBackgroundBrush(QColor("#9FADB9"));view.setWorkspaceTheme(false);
  }
  view.viewport()->update();
 }
 LabWindow(Document doc,CurveDocument curveDoc=CurveDocument{}):view(std::move(doc)){
  setAcceptDrops(true);
  view.setAcceptDrops(false);view.viewport()->setAcceptDrops(false);
  if(!curveDoc.contours.empty())curves=std::make_unique<CurveDragSession>(std::move(curveDoc));
  view.attachNative(curves.get());
  darkTheme_=QSettings("AlencarLaser","DesktopBeta").value("workspace/darkTheme",true).toBool();
  loadWorkspaceSettings();view.setWorkspace(workspaceCfg_,isInteractive());
  setObjectName("alencarRoot");
  setWindowTitle("Alencar Laser | Estúdio Qt 6 — TESTE_19.8 (candidata)");
  resize(1390,870);setMinimumSize(950,620);
  auto* layout=new QVBoxLayout(this);layout->setContentsMargins(0,0,0,0);layout->setSpacing(0);
  auto* header=new QFrame;header->setObjectName("topHeader");
  auto* headerRow=new QHBoxLayout(header);headerRow->setContentsMargins(11,4,11,4);
  auto* identity=new QVBoxLayout;identity->setSpacing(1);
  auto* brand=new QLabel("ALENCAR LASER");brand->setObjectName("brandName");
  // Identidade oficial: usa a arte ORIGINAL fornecida pelo proprietario,
  // nunca redesenha nem cria um substituto. Fallback textual sem arquivo.
  const QStringList logoCandidates={
   QDir::current().absoluteFilePath("DADOS/LOGO/logo_alencar_laser.png"),
   QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("logo_alencar_laser.png"),
   QStringLiteral("C:/ProjetoAlencarLaser/DADOS/LOGO/logo_alencar_laser.png")
  };
  for(const QString& logoPath:logoCandidates){
   QPixmap original(logoPath);
   if(original.isNull())continue;
   brand->setPixmap(original.scaled(162,42,Qt::KeepAspectRatio,Qt::SmoothTransformation));
   brand->setToolTip("Logo oficial Alencar Laser: "+logoPath);
   brand->setFixedHeight(44);
   break;
  }
  if(brand->pixmap().isNull())
   brand->setToolTip("Logo oficial ainda nao encontrada. Instale em DADOS/LOGO/logo_alencar_laser.png.");
  identity->addWidget(brand);
  auto* subtitle=new QLabel("TESTE_19.8 • Candidata");subtitle->setObjectName("brandSub");subtitle->hide();identity->addWidget(subtitle);
  headerRow->addLayout(identity);headerRow->addStretch();
  auto* safety=new QLabel("● MÁQUINA DESCONECTADA   |   EMISSÃO BLOQUEADA");
  safety->setObjectName("machineState");safety->setToolTip("Leitura GRBL opcional; sem comandos de movimento ou laser.");
  machineHeaderStatus_=safety;
  headerRow->addWidget(safety);
  layout->addWidget(header);
  auto add=[&](QHBoxLayout* row,const QString& id,const QString& title,std::function<void()> action){
   auto* b=new QPushButton(title);b->setObjectName(id);buttons[id]=b;
   b->setCursor(Qt::PointingHandCursor);b->setFocusPolicy(Qt::NoFocus);row->addWidget(b);
   connect(b,&QPushButton::clicked,this,[this,action]{run(action);});return b;
  };
  auto addVertical=[&](QVBoxLayout* column,const QString& id,const QString& title,std::function<void()> action){
   auto* row=new QHBoxLayout;row->setContentsMargins(0,0,0,0);
   auto* b=add(row,id,title,action);b->setMinimumHeight(34);column->addLayout(row);return b;
  };
  auto titleLabel=[&](const QString& label){auto* l=new QLabel(label);l->setObjectName("sectionTitle");return l;};
  auto* quick=new QFrame;quick->setObjectName("quickActions");
  auto* quickRow=new QHBoxLayout(quick);quickRow->setContentsMargins(7,3,7,3);quickRow->setSpacing(5);
  add(quickRow,"open","Abrir",[this]{open();});
  add(quickRow,"open_other","Abrir outro...",[this]{openOther();});
  add(quickRow,"save","Salvar",[this]{save();});
  add(quickRow,"save_as","Salvar como...",[this]{saveAs();});
  quickRow->addSpacing(12);
  add(quickRow,"undo","↶ Desfazer",[this]{
   if(bezierToolActive_&&curves){if(curves->dragging())return;view.nativeHistory(false);status->setText("Bézier: desfeito");}
   else view.history(false);
  });
  add(quickRow,"redo","↷ Refazer",[this]{
   if(bezierToolActive_&&curves){if(curves->dragging())return;view.nativeHistory(true);status->setText("Bézier: refeito");}
   else view.history(true);
  });
  quickRow->addStretch();
  // Referências em SOMENTE LEITURA: sem mutação no Editor, sem movimento e sem G-code.
  auto* referenceMenuButton=new QToolButton;
  referenceMenuButton->setObjectName("references_menu");
  referenceMenuButton->setText("Referências ▾");
  referenceMenuButton->setToolTip("Inspecionar arquivos LightBurn / GRBL sem importação para desenho");
  referenceMenuButton->setPopupMode(QToolButton::InstantPopup);
  auto* referenceMenu=new QMenu(referenceMenuButton);
  auto* xmlAction=referenceMenu->addAction("Analisar projeto LightBurn (.lbrn2)...");
  auto* logAction=referenceMenu->addAction("Analisar log LightBurn / GRBL (.txt)...");
  referenceMenuButton->setMenu(referenceMenu);
  connect(xmlAction,&QAction::triggered,this,[this]{run([this]{
   const auto file=QFileDialog::getOpenFileName(this,"Analisar projeto LightBurn (somente leitura)",{},"Projeto LightBurn (*.lbrn2)");
   if(!file.isEmpty())reference::inspectLightBurn(this,file);
  });});
  connect(logAction,&QAction::triggered,this,[this]{run([this]{
   const auto file=QFileDialog::getOpenFileName(this,"Analisar log GRBL (somente leitura)",{},"Log de texto (*.txt *.log)");
   if(!file.isEmpty())reference::inspectGrblLog(this,file);
  });});
  quickRow->addWidget(referenceMenuButton);
  auto* hideTools=new QPushButton("‹ Ferramentas");hideTools->setFocusPolicy(Qt::NoFocus);
  hideTools->setToolTip("Recolher ou mostrar barra de ferramentas");quickRow->addWidget(hideTools);
  auto* hidePanels=new QPushButton("Painéis ›");hidePanels->setFocusPolicy(Qt::NoFocus);
  hidePanels->setToolTip("Recolher ou mostrar os painéis à direita");quickRow->addWidget(hidePanels);
  auto* themeButton=new QPushButton("☼ Tema");themeButton->setObjectName("theme_toggle");
  themeButton->setToolTip("Alternar tema escuro ou claro");themeButton->setFocusPolicy(Qt::NoFocus);quickRow->addWidget(themeButton);
  connect(themeButton,&QPushButton::clicked,this,[this]{darkTheme_=!darkTheme_;
   QSettings("AlencarLaser","DesktopBeta").setValue("workspace/darkTheme",darkTheme_);
   applyTheme();view.setFocus();});
  layout->addWidget(quick);

  auto* splitter=new QSplitter(Qt::Horizontal);splitter->setObjectName("workspaceSplitter");
  persistentSplitter_=splitter;
  splitter->setChildrenCollapsible(false);splitter->setHandleWidth(3);
  auto* left=new QFrame;left->setObjectName("leftPanel");
  left->setMinimumWidth(52);left->setMaximumWidth(64);
  auto* leftOuter=new QVBoxLayout(left);leftOuter->setContentsMargins(4,5,4,5);
  auto* toolScroll=new QScrollArea;toolScroll->setWidgetResizable(true);
  toolScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  auto* leftContent=new QWidget;
  auto* tools=new QVBoxLayout(leftContent);tools->setContentsMargins(4,4,4,4);tools->setSpacing(5);
  auto* toolGroup=new QButtonGroup(this);toolGroup->setExclusive(true);
  auto compact=[&](QGridLayout* grid,int row,int col,const QString& id,const QString& title,
                   const QString& tooltip,std::function<void()> action){
    auto* b=new QPushButton(title);b->setObjectName(id);b->setProperty("compact",true);
    // Compact icon-first toolbar, with full accessible name and shortcut in tooltip.
    b->setAccessibleName(title);
    b->setIcon(alaser::toolGlyph(id));
    b->setText(QString());
    b->setIconSize(QSize(20,20));
    b->setFixedSize(QSize(38,36));
    b->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    b->setToolTip(title+" — "+tooltip);b->setFocusPolicy(Qt::NoFocus);
    buttons[id]=b;grid->addWidget(b,row,col);
    b->setCheckable(true);toolGroup->addButton(b);
    connect(b,&QPushButton::clicked,this,[this,action]{
     // Mudanca de ferramenta nao pode deixar campo de texto vazio sobre a mesa.
     if(inlineTextEntry_)
      finishInlineText(!inlineTextEntry_->text().trimmed().isEmpty());
     run(action);
    });return b;
  };
  auto newGrid=[&](){auto* g=new QGridLayout;g->setSpacing(3);g->setAlignment(Qt::AlignHCenter);tools->addLayout(g);return g;};
  tools->addSpacing(4); // Barra compacta: ferramentas de edicao
  auto* editorGrid=newGrid();
  compact(editorGrid,0,0,"select","↖ Seleção","Selecionar (Alt alterna contornos sobrepostos)",
    [this]{bezierToolActive_=false;view.enableNative(false);view.setTool(QtEditorLab::Tool::Select);});
  compact(editorGrid,1,0,"nodes","✎ Nós","Editar nós das polilinhas",
    [this]{bezierToolActive_=false;view.enableNative(false);view.setTool(QtEditorLab::Tool::Nodes);});
  [[maybe_unused]] auto* bezier=compact(editorGrid,2,0,"bezier","⌁ Bézier","Editar âncoras e alças Bézier nativas",
   [this]{if(!curves){status->setText("Abra um projeto com curvas v5.");return;}
     bezierToolActive_=true;view.enableNative(true);
     status->setText("Bézier: arraste âncoras/alças | Alt alterna | Shift insere | Delete exclui");});
  compact(editorGrid,3,0,"scissors","✂ Tesoura","Ao aproximar, marca em laranja o trecho entre cruzamentos; clique remove tudo (K). Bézier nativa indisponivel.",
    [this]{bezierToolActive_=false;view.enableNative(false);view.setTool(QtEditorLab::Tool::Scissors);
     status->setText("Tesoura: passe o mouse sobre um trecho; destaque laranja = corte completo. Ctrl+Z desfaz.");});
  tools->addSpacing(5); // Ferramentas de desenho
  auto* shapeGrid=newGrid();
  compact(shapeGrid,0,0,"line","╱ Linha contínua","Clique cria nós, clique na origem fecha, botão direito termina, Esc cancela, Shift restringe a 45 graus",
    [this]{bezierToolActive_=false;view.enableNative(false);view.setTool(QtEditorLab::Tool::Line);});
  compact(shapeGrid,1,0,"rectangle","▭ Retâng.","Desenhar retângulo",
    [this]{bezierToolActive_=false;view.enableNative(false);view.setTool(QtEditorLab::Tool::Rectangle);});
  compact(shapeGrid,2,0,"ellipse","◯ Elipse","Desenhar elipse",
    [this]{bezierToolActive_=false;view.enableNative(false);view.setTool(QtEditorLab::Tool::Ellipse);});
  compact(shapeGrid,3,0,"polygon","⬡ Políg.","Desenhar polígono",
    [this]{bezierToolActive_=false;view.enableNative(false);view.setTool(QtEditorLab::Tool::Polygon);});
  compact(shapeGrid,4,0,"pencil","✏ Lápis","Traço livre (P), arraste o mouse; uma acao de desfazer",
    [this]{bezierToolActive_=false;view.enableNative(false);view.setTool(QtEditorLab::Tool::Pencil);});
  compact(shapeGrid,5,0,"text","T Texto","Texto transparente direto na mesa (T): Enter/Esc conclui",
    [this]{bezierToolActive_=false;view.enableNative(false);view.setTool(QtEditorLab::Tool::Text);});
  buttons.at("select")->setChecked(true);
  auto future=[&](const QString& name,const QString& tip){
    auto* b=new QPushButton(name.left(1));b->setProperty("compact",true);
    b->setFixedSize(QSize(38,32));b->setAccessibleName(name);
    b->setEnabled(false);b->setToolTip(tip+" — em desenvolvimento; função desativada");
    b->setFocusPolicy(Qt::NoFocus);return b;
  };
  tools->addSpacing(6); // Ferramentas futuras desativadas
  auto* vectorGrid=newGrid();
  for(const auto& data:std::vector<std::tuple<QString,QString>>{
   {"◎ Offset","Contorno interno/externo"},
   {"⊕ União","Soldar vetores"},{"⊖ Subtr.","Subtrair formas"},
   {"⊞ Alinhar","Alinhamento e distribuição"},{"⌗ Medir","Ferramenta de medição"}}){
    const int index=vectorGrid->count();vectorGrid->addWidget(future(std::get<0>(data),std::get<1>(data)),index,0);
  }
  tools->addSpacing(5); // Importadores futuros desativados
  auto* fileGrid=newGrid();
  for(const auto& data:std::vector<QString>{"SVG", "DXF", "Imagem", "LightBurn"}){
   const int index=fileGrid->count();fileGrid->addWidget(future(data,"Importação de "+data),index,0);
  }
  tools->addStretch();
  auto* tip=new QLabel("Arraste: mover forma\nDelete: excluir forma\nCtrl+arraste: mover curva Bézier\nScroll: zoom no cursor\nBotão central: mover tela\nCtrl+Z/Y: histórico");
  tip->setObjectName("brandSub");tip->setWordWrap(true);tip->hide();tools->addWidget(tip);
  toolScroll->setWidget(leftContent);leftOuter->addWidget(toolScroll);
  splitter->addWidget(left);

  auto* center=new QFrame;center->setObjectName("canvasPanel");
  auto* canvasLayout=new QVBoxLayout(center);canvasLayout->setContentsMargins(4,2,4,3);canvasLayout->setSpacing(2);
  auto* canvasBar=new QHBoxLayout;
  auto* canvasMeta=new QLabel("Desenho 400 × 400 · Mesa 400 × 400 mm");canvasMeta->setObjectName("canvasMeta");
  workspaceTopMeta_=canvasMeta;
  canvasBar->addWidget(canvasMeta);canvasBar->addStretch();
  auto* configureBed=new QPushButton("Área…");configureBed->setObjectName("configure_workspace");
  configureBed->setToolTip("Configurar separadamente a área de desenho e a mesa da máquina");
  configureBed->setFocusPolicy(Qt::NoFocus);canvasBar->addWidget(configureBed);
  connect(configureBed,&QPushButton::clicked,this,[this]{run([this]{configureWorkspace();});});
  auto* fitBed=new QPushButton("Mesa");fitBed->setFocusPolicy(Qt::NoFocus);
  connect(fitBed,&QPushButton::clicked,this,[this]{view.fitMachineBed();view.setFocus();});
  canvasBar->addWidget(fitBed);
  auto* fit=new QPushButton("Desenho");fit->setObjectName("fit_drawing");fit->setFocusPolicy(Qt::NoFocus);
  canvasBar->addWidget(fit);connect(fit,&QPushButton::clicked,this,[this]{
    auto r=view.scene()->itemsBoundingRect();if(!r.isEmpty())view.fitInView(r.adjusted(-10,-10,10,10),Qt::KeepAspectRatio);
    view.reportZoom();view.setFocus();
  });
  auto* zoom=new QComboBox;zoom->setObjectName("zoom_scale");zoom->setMinimumWidth(76);
  for(int scale:{1,2,4,6,8,12,24,48,120,240,500,1000,2000,4000})zoom->addItem(QString::number(scale)+"×",scale);
  zoom->addItem("Personalizado",-1.0);
  zoom->setCurrentIndex(3);
  canvasBar->addWidget(zoom);
  for(auto* button:{configureBed,fitBed,fit}){
   button->setProperty("compact",true);
   button->setStyleSheet("QPushButton {padding:2px 5px; min-height:18px; font-size:10px;}");
  }
  zoom->setStyleSheet("QComboBox {padding:2px 4px; min-height:19px; font-size:10px;}");
  configureBed->setToolTip("Configurar área de desenho e mesa");
  fitBed->setToolTip("Enquadrar a mesa configurada");
  fit->setToolTip("Ajustar desenho à área disponível");
  view.zoomChanged=[zoom](double level){
   QSignalBlocker block(zoom);
   int ix=-1;
   for(int i=0;i<zoom->count()-1;++i)
    if(std::abs(zoom->itemData(i).toDouble()-level)<0.001){ix=i;break;}
   const int custom=zoom->count()-1;
   zoom->setItemText(custom,QString::number(level,'f',2)+"×");
   zoom->setCurrentIndex(ix>=0?ix:custom);
  };
  connect(zoom,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this,zoom](int index){
   if(index<0||zoom->itemData(index).toDouble()<=0)return;
   QPointF focus=view.mapToScene(view.viewport()->rect().center());
   const double scale=alaser::zoom196::clampScale(zoom->itemData(index).toDouble());
   view.setTransform(QTransform::fromScale(scale,scale));view.centerOn(focus);view.reportZoom();view.setFocus();
  });
  canvasLayout->addLayout(canvasBar);
  view.setObjectName("drawingCanvas");view.setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  canvasLayout->addWidget(&view,1);
  splitter->addWidget(center);

  auto* right=new QFrame;right->setObjectName("rightPanel");right->setMinimumWidth(260);right->setMaximumWidth(420);
  auto* rightMain=new QVBoxLayout(right);rightMain->setContentsMargins(4,5,4,5);
  auto* inspectorTabs=new QTabWidget;inspectorTabs->setObjectName("inspectorTabs");rightMain->addWidget(inspectorTabs);
  auto* layersTab=new QWidget;
  auto* inspector=new QVBoxLayout(layersTab);inspector->setContentsMargins(8,8,8,8);inspector->setSpacing(6);
  inspector->addWidget(titleLabel("CAMADAS E VISIBILIDADE"));
  layers=new QComboBox;layers->setObjectName("layers");layers->setMinimumWidth(160);inspector->addWidget(layers);
  visible=new QCheckBox("Visível");locked=new QCheckBox("Bloqueada");
  auto* flagRow=new QHBoxLayout;flagRow->addWidget(visible);flagRow->addWidget(locked);inspector->addLayout(flagRow);
   layerMode_=new QComboBox;
   layerMode_->setObjectName("layer_mode");
   layerMode_->addItem("Linha — contornos",QString("Line"));
   layerMode_->addItem("Preenchimento — formas fechadas",QString("Fill"));
   layerMode_->setToolTip("Apenas visualizacao editorial. Sem G-code, velocidade ou potencia.");
   inspector->addWidget(titleLabel("MODO DA CAMADA ATIVA (PREVISUALIZACAO)"));
   inspector->addWidget(layerMode_);
   auto* showFill=new QCheckBox("Prévia de preenchimento (apenas visual)");
   showFill->setObjectName("preview_fill");
   showFill->setChecked(false);
   showFill->setToolTip("Desligado por padrão como no LightBurn; não gera hatch de gravação.");
   inspector->addWidget(showFill);
   connect(showFill,&QCheckBox::toggled,this,[this](bool checked){view.setFillPreview(checked);});
   auto* fillNote=new QLabel("Linha/Fill são configurações editoriais. A mesa mostra contornos por padrão; sem G-code.");
   fillNote->setObjectName("brandSub");fillNote->setWordWrap(true);inspector->addWidget(fillNote);
   layerName=new QLineEdit;layerName->setObjectName("layerName");layerName->setPlaceholderText("Nome da nova camada");
  inspector->addWidget(layerName);
  auto* layerActions=new QHBoxLayout;layerActions->setSpacing(4);
  add(layerActions,"add_layer","Nova camada",[this]{
   int id=view.editor.state().layers.rbegin()->first+1;
    const QString name=layerName->text().trimmed();
    view.editor.addLayer(id,(name.isEmpty()?QString("Camada %1").arg(id):name).toStdString());
    refresh();layers->setCurrentIndex(layers->findData(id));
  });inspector->addLayout(layerActions);
  addVertical(inspector,"move_layer","→ Enviar seleção para camada ativa",[this]{
   check(!bezierToolActive_,"Transferencia de camada: selecione polilinhas.");
   const auto ids=selection();check(!ids.empty(),"Selecione formas e depois a camada de destino.");
   view.editor.moveSelectionToLayer(ids,layers->currentData().toInt());view.rebuild();
   captureEvent("objects_layer_changed",int(ids.size()));
  });
  inspector->addSpacing(12);inspector->addWidget(titleLabel("ORGANIZAÇÃO"));
  auto* groupRow=new QHBoxLayout;groupRow->setSpacing(5);
  add(groupRow,"group","Agrupar",[this]{view.editor.group(selection());view.rebuild();});
   add(groupRow,"ungroup","Desagrupar (Ctrl+U)",[this]{
    check(!bezierToolActive_,"Desagrupar: selecione formas poligonais.");
    const int groups=view.editor.ungroupSelection(selection());
    view.rebuild();captureEvent("groups_ungrouped",groups);
    status->setText(QString("%1 grupo(s) desagrupado(s). Formas prontas para Tesoura (K).").arg(groups));
   });inspector->addLayout(groupRow);
  addVertical(inspector,"ungroup_scissors","Desagrupar → Tesoura (K)",[this]{
   check(!bezierToolActive_,"Somente formas poligonais podem ser desagrupadas.");
   const int count=view.editor.ungroupSelection(selection());
   view.rebuild();captureEvent("groups_ungrouped",count);
   QTimer::singleShot(0,this,[this]{if(buttons.count("scissors"))buttons.at("scissors")->click();});
  });
  addVertical(inspector,"move","Mover +1 mm",[this]{view.editor.moveObjects(selection(),{1,0});view.rebuild();captureEvent("shapes_moved",int(selection().size()));});
  addVertical(inspector,"delete_shapes","Excluir formas selecionadas",[this]{
   check(!bezierToolActive_,"Para excluir uma curva nativa, use Ctrl+Delete no modo Bézier.");
   check(view.deleteSelectedShapes(),"Selecione uma ou mais formas editáveis.");
  });
  addVertical(inspector,"delete_curve","Excluir curva Bézier selecionada",[this]{
   check(bezierToolActive_,"Ative o modo Bézier e selecione uma curva.");
   check(view.nativeDeleteContour(),"Selecione um contorno Bézier.");
  });
inspector->addSpacing(10);inspector->addWidget(titleLabel("TRANSFORMAÇÕES — POLILINHAS"));
addVertical(inspector,"duplicate_obj","Duplicar (+3 mm)",[this]{
 if(bezierToolActive_)throw std::runtime_error("Duplicar: selecione antes a ferramenta Selecionar (polilinhas).");
 const auto ids=selection();check(ids.size()==1,"Duplicar: selecione um objeto por vez.");
 int newId=view.editor.duplicate(ids.front(),{3,3});
 view.selected.clear();view.rebuild();view.activate({newId,0});
});
addVertical(inspector,"mirror_horiz","Espelhar horizontal ↔",[this]{
 if(bezierToolActive_)throw std::runtime_error("Espelhar: somente polilinhas nesta beta.");
 const auto ids=selection();check(!ids.empty(),"Espelhar: selecione um objeto.");
 const auto [lo,hi]=boundsOf(view.editor.state().doc,ids);
 view.editor.mirrorX(ids,(lo.x+hi.x)/2.0);view.rebuild();
});
addVertical(inspector,"rotate_90","Girar 90°",[this]{
 if(bezierToolActive_)throw std::runtime_error("Girar: somente polilinhas nesta beta.");
 const auto ids=selection();check(!ids.empty(),"Girar: selecione um objeto.");
 const auto [lo,hi]=boundsOf(view.editor.state().doc,ids);
 view.editor.rotateObjects(ids,{(lo.x+hi.x)/2.0,(lo.y+hi.y)/2.0},90.0);
 view.rebuild();
});
inspector->addStretch();
auto* lockedNote=new QLabel("Máquina: sem conexão\nCAM / emissão: indisponíveis nesta beta");
  lockedNote->setObjectName("brandSub");lockedNote->setWordWrap(true);inspector->addWidget(lockedNote);
  auto* layersScroll=new QScrollArea;layersScroll->setWidgetResizable(true);
  layersScroll->setWidget(layersTab);inspectorTabs->addTab(layersScroll,"Camadas");

  auto* propsTab=new QWidget;auto* properties=new QVBoxLayout(propsTab);
  properties->setContentsMargins(9,9,9,9);properties->setSpacing(8);
  properties->addWidget(titleLabel("ÁREA DE DESENHO / MESA"));
  workspaceSummary_=new QLabel;workspaceSummary_->setWordWrap(true);properties->addWidget(workspaceSummary_);
  auto* editWorkspace=new QPushButton("Configurar dimensões...");
  connect(editWorkspace,&QPushButton::clicked,this,[this]{run([this]{configureWorkspace();});});
  properties->addWidget(editWorkspace);
  properties->addWidget(titleLabel("PROPRIEDADES DO OBJETO"));
  properties->addWidget(new QLabel("Seleção e transformação em mm"));
  auto* exactAngle=new QPushButton("Girar seleção por ângulo...");
  exactAngle->setObjectName("rotate_exact_195");
  connect(exactAngle,&QPushButton::clicked,this,[this]{run([this]{
   check(!bezierToolActive_,"Girar exige modo Seleção de polilinhas.");
   const auto ids=selection();check(!ids.empty(),"Selecione formas editáveis.");
   bool ok=false;const double angle=QInputDialog::getDouble(this,"Rotação","Ângulo em graus:",0,-360,360,2,&ok);
   if(!ok)return;
   const auto [lo,hi]=boundsOf(view.editor.state().doc,ids);
   view.editor.rotateObjects(ids,{(lo.x+hi.x)/2.,(lo.y+hi.y)/2.},angle);view.rebuild();
  });});properties->addWidget(exactAngle);
  auto* scaleExact=new QPushButton("Escalar seleção proporcionalmente...");
  scaleExact->setObjectName("scale_exact_195");
  connect(scaleExact,&QPushButton::clicked,this,[this]{run([this]{
   check(!bezierToolActive_,"Escalar exige modo Seleção de polilinhas.");
   const auto ids=selection();check(!ids.empty(),"Selecione formas editáveis.");
   bool ok=false;const double factor=QInputDialog::getDouble(this,"Escala","Escala (%):",100,1,1000,1,&ok);
   if(!ok)return;
   const auto [lo,hi]=boundsOf(view.editor.state().doc,ids);
   view.editor.scaleObjects(ids,{(lo.x+hi.x)/2.,(lo.y+hi.y)/2.},factor/100.,factor/100.);view.rebuild();
  });});properties->addWidget(scaleExact);
  auto* alignLeftBtn=new QPushButton("Alinhar à esquerda (2+ objetos)");
  alignLeftBtn->setObjectName("align_left_195");
  connect(alignLeftBtn,&QPushButton::clicked,this,[this]{run([this]{
   check(!bezierToolActive_,"Alinhar exige modo Seleção de polilinhas.");
   const auto ids=selection();
   for(const int id:ids)if(view.editor.state().objects.at(id).group!=0)
    throw std::runtime_error("Alinhamento de grupos exige desagrupar primeiro.");
   view.editor.alignLeft(ids);view.rebuild();
  });});properties->addWidget(alignLeftBtn);
  properties->addStretch();inspectorTabs->addTab(propsTab,"Objetos");

  auto* referenceTab=new QWidget;
  auto* referenceLayout=new QVBoxLayout(referenceTab);referenceLayout->setContentsMargins(9,9,9,9);
  referenceLayout->addWidget(titleLabel("ARQUIVOS REAIS / INSPEÇÃO"));
  auto* referenceNote=new QLabel("Abra um projeto .lbrn2 ou log GRBL para consultar geometria, camadas e parâmetros. "
     "O documento atual NÃO é modificado; conversão/importação ainda indisponíveis.");
  referenceNote->setWordWrap(true);referenceLayout->addWidget(referenceNote);
  auto* refXml=new QPushButton("Examinar projeto .lbrn2...");refXml->setObjectName("reference_lightburn");
  connect(refXml,&QPushButton::clicked,this,[this,xmlAction]{xmlAction->trigger();});
  referenceLayout->addWidget(refXml);
  auto* refLog=new QPushButton("Examinar log GRBL...");refLog->setObjectName("reference_grbl_log");
  connect(refLog,&QPushButton::clicked,this,[this,logAction]{logAction->trigger();});
  referenceLayout->addWidget(refLog);
  auto* refSafety=new QLabel("● Apenas leitura local\n● Sem emissão\n● Sem importação automática\n● Sem mudança de parâmetros");
  refSafety->setObjectName("brandSub");refSafety->setWordWrap(true);referenceLayout->addWidget(refSafety);
  referenceLayout->addStretch();inspectorTabs->addTab(referenceTab,"Referências");

  auto* collectionTab=new QWidget;
  auto* collectionLayout=new QVBoxLayout(collectionTab);
  collectionLayout->setContentsMargins(9,9,9,9);
  collectionLayout->addWidget(titleLabel("COLETOR LOCAL DE DIAGNÓSTICOS"));
  auto* explanation=new QLabel("Opcional e desligado inicialmente. Registra eventos de edição e métricas agregadas na memória; "
   "não captura projetos, nomes de arquivos, coordenadas ou conteúdo do desenho. "
   "Nada é enviado pela internet.");
  explanation->setWordWrap(true);collectionLayout->addWidget(explanation);
  auto* enableCapture=new QCheckBox("Ativar coleta nesta sessão");
  enableCapture->setObjectName("diagnostics_enable");
  collectionLayout->addWidget(enableCapture);
  collectorSummary_=new QLabel;collectorSummary_->setWordWrap(true);collectionLayout->addWidget(collectorSummary_);
  collectorLog_=new QPlainTextEdit;collectorLog_->setReadOnly(true);collectorLog_->setMaximumBlockCount(110);
  collectorLog_->setObjectName("diagnostics_events");collectorLog_->setPlaceholderText("Eventos aparecerão aqui após ativar a coleta.");
  collectionLayout->addWidget(collectorLog_,1);
  connect(enableCapture,&QCheckBox::toggled,this,[this](bool active){
   if(active)collector_.start();else collector_.stop();updateCaptureSummary();
  });
  auto* exportBtn=new QPushButton("Exportar relatório JSON...");
  exportBtn->setObjectName("diagnostics_export");
  connect(exportBtn,&QPushButton::clicked,this,[this]{run([this]{exportCapture();});});
  collectionLayout->addWidget(exportBtn);
  auto* clearBtn=new QPushButton("Limpar dados coletados");
  clearBtn->setObjectName("diagnostics_clear");
  connect(clearBtn,&QPushButton::clicked,this,[this]{
   collector_.clear();collectorLog_->clear();updateCaptureSummary();
  });collectionLayout->addWidget(clearBtn);
  inspectorTabs->addTab(collectionTab,"Diagnósticos");updateCaptureSummary();

  auto* wifiLauncher=new QPushButton("Wi-Fi / TCP · Diagnóstico");
  wifiLauncher->setObjectName("wifi_launcher_196");wifiLauncher->setToolTip("Abrir o painel Máquina. Nenhuma conexão é iniciada automaticamente.");
  wifiLauncher->setFocusPolicy(Qt::NoFocus);quickRow->insertWidget(quickRow->count()-1,wifiLauncher);
  connect(wifiLauncher,&QPushButton::clicked,this,[inspectorTabs]{inspectorTabs->setCurrentIndex(inspectorTabs->indexOf(inspectorTabs->findChild<QScrollArea*>("machineScroll_196")));});
  auto* machineTab=new QWidget;machineTab->setObjectName("machineTab196");auto* machineLayout=new QVBoxLayout(machineTab);
  machineLayout->setContentsMargins(9,9,9,9);machineLayout->setSpacing(8);
  machineLayout->addWidget(titleLabel("PERFIL VISUAL DA MÁQUINA"));
  machineProfileSelector_=new QComboBox;
  machineProfileSelector_->addItems({"AlgoLaser Alpha Mk2 20W", "Personalizado"});
  // Tipos futuros aparecem na interface, sem conectar ou alegar compatibilidade.
  for(const QString profile:{"CO₂ / GRBL — futuro","CO₂ / DSP — futuro","Fibra / Galvo — futuro","UV / Galvo — futuro"}){
   machineProfileSelector_->addItem(profile);
   const int row=machineProfileSelector_->count()-1;
   if(auto* model=qobject_cast<QStandardItemModel*>(machineProfileSelector_->model()))
    model->item(row)->setEnabled(false);
  }
  machineLayout->addWidget(machineProfileSelector_);
  connect(machineProfileSelector_,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this](int idx){
   if(idx<0||idx>1)return;
   workspaceCfg_.profile=idx==0?"AlgoLaser Alpha Mk2 20W":"Personalizado";
   persistWorkspaceSettings();refreshWorkspaceLabels();
  });
  machineLiveStatus_=new QLabel;machineLiveStatus_->setWordWrap(true);
  machineLiveStatus_->setObjectName("machineLiveStatus");machineLayout->addWidget(machineLiveStatus_);
  auto* warning=new QLabel("TESTE 19.6 — diagnóstico USB / Wi-Fi supervisionado. Abrir a porta pode reiniciar algumas controladoras. "
   "Feche o LightBurn, mantenha a máquina parada, sem execução e sob supervisão. Nenhum comando de movimento é oferecido.");
  warning->setWordWrap(true);machineLayout->addWidget(warning);
  auto* portChoice=new QComboBox;portChoice->setObjectName("machine_com_195");machineLayout->addWidget(portChoice);
  auto* portRefresh=new QPushButton("Atualizar portas COM");machineLayout->addWidget(portRefresh);
  auto refreshPorts=[portChoice]{
   const QString chosen=portChoice->currentText();portChoice->clear();
   for(const auto& p:alaser::grbl::WindowsSerialObserver::ports())portChoice->addItem(QString::fromStdString(p));
   if(int index=portChoice->findText(chosen);index>=0)portChoice->setCurrentIndex(index);
  };
  connect(portRefresh,&QPushButton::clicked,this,refreshPorts);refreshPorts();
  auto* connectDiag=new QPushButton("Conectar apenas para diagnóstico");
  connectDiag->setObjectName("machine_connect_195");machineLayout->addWidget(connectDiag);
  connectDiag->setEnabled(false);
  connectDiag->setToolTip("Bloqueado na 19.8 enquanto a estabilidade de conexao e auditada.");
  auto* disconnectDiag=new QPushButton("Desconectar USB");
  disconnectDiag->setObjectName("machine_disconnect_195");machineLayout->addWidget(disconnectDiag);
  auto* probes=new QHBoxLayout;
  auto* statusProbe=new QPushButton("Consultar estado (?)");
  auto* firmwareProbe=new QPushButton("Consultar firmware ($I)");
  auto* settingsProbe=new QPushButton("Ler parâmetros ($$)");
  probes->addWidget(statusProbe);machineLayout->addLayout(probes);
  machineLayout->addWidget(firmwareProbe);machineLayout->addWidget(settingsProbe);
  machineConsole_=new QPlainTextEdit;machineConsole_->setReadOnly(true);
  machineConsole_->setMaximumBlockCount(80);machineConsole_->setPlaceholderText("Respostas resumidas, sem números de série ou coordenadas.");
  machineLayout->addWidget(machineConsole_,1);
  connect(connectDiag,&QPushButton::clicked,this,[this,portChoice]{
   // Physical diagnostics disabled in this release; blocking also applies to programmatic click signals.
   return;
   if(machineConnected()||(wifi_&&wifi_->state()!=QAbstractSocket::UnconnectedState)){
     QMessageBox::information(this,"Diagnóstico","Desconecte o transporte atual antes de conectar USB.");return;}
   if(portChoice->currentText().isEmpty()){QMessageBox::information(this,"Porta COM","Nenhuma porta COM selecionada.");return;}
   if(QMessageBox::warning(this,"Conexão de diagnóstico",
     "Confirma máquina parada e LightBurn fechado?\nAbrir a porta USB pode causar reinício da controladora.\n"
     "Não haverá comandos de movimento, homing, reset ou laser.",
     QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes)return;
   try{controller_={};pendingSerial_.clear();serial_.open(portChoice->currentText().toStdString());
    machineConsole_->appendPlainText("Porta aberta: aguardando mensagens passivas.");
    captureEvent("grbl_connected");refreshMachineStatus();}
   catch(const std::exception& e){QMessageBox::warning(this,"Conexão recusada",QString::fromUtf8(e.what()));}
  });
  connect(disconnectDiag,&QPushButton::clicked,this,[this]{
   serial_.close();captureEvent("grbl_disconnected");refreshMachineStatus();
   machineConsole_->appendPlainText("Porta fechada.");
  });
  // TCP/Wi-Fi: IP local digitado manualmente, sem busca, sem reconexão, sem credenciais.
  machineLayout->addWidget(titleLabel("WI-FI / TCP  ·  DIAGNÓSTICO SOMENTE LEITURA"));
  auto* wifiRow=new QHBoxLayout;
  auto* ip=new QLineEdit;ip->setObjectName("wifi_ip_196");ip->setPlaceholderText("IP local da máquina (ex.: 192.168.1.50)");
  ip->setToolTip("IP IPv4 privado exibido na AlgoLaser/LightBurn; nunca é enviado no relatório.");
  auto* tcpPort=new QSpinBox;tcpPort->setObjectName("wifi_port_196");tcpPort->setRange(1,65535);tcpPort->setValue(23);tcpPort->setPrefix("TCP ");
  {
   QSettings saved("AlencarLaser","DesktopBeta");
   ip->setText(saved.value("connection/wifiIp",QString()).toString());
   tcpPort->setValue(saved.value("connection/wifiPort",23).toInt());
   connect(ip,&QLineEdit::editingFinished,this,[ip]{
    QSettings("AlencarLaser","DesktopBeta").setValue("connection/wifiIp",ip->text().trimmed());
   });
   connect(tcpPort,QOverload<int>::of(&QSpinBox::valueChanged),this,[](int port){
    QSettings("AlencarLaser","DesktopBeta").setValue("connection/wifiPort",port);
   });
  }
  wifiRow->addWidget(ip,1);wifiRow->addWidget(tcpPort);machineLayout->addLayout(wifiRow);
  auto* wifiRowActions=new QHBoxLayout;
  auto* connectWifi=new QPushButton("Conectar Wi-Fi (leitura)");connectWifi->setObjectName("wifi_connect_196");
  auto* disconnectWifi=new QPushButton("Desconectar Wi-Fi");disconnectWifi->setObjectName("wifi_disconnect_196");
  wifiRowActions->addWidget(connectWifi);wifiRowActions->addWidget(disconnectWifi);machineLayout->addLayout(wifiRowActions);
  connectWifi->setEnabled(false);
  connectWifi->setToolTip("Bloqueado na 19.8 enquanto a estabilidade de conexao e auditada.");
  transportHint_=new QLabel("Somente rede local privada. Sem descoberta, gravação, movimento, G-code ou reconexão automática.");
  transportHint_->setWordWrap(true);machineLayout->addWidget(transportHint_);
  wifi_=new QTcpSocket(this);
  connect(wifi_,&QTcpSocket::connected,this,[this]{
    machineConsole_->appendPlainText("TCP conectado; aguardando mensagens GRBL passivas.");
    captureEvent("grbl_tcp_connected");refreshMachineStatus();
  });
  connect(wifi_,&QTcpSocket::readyRead,this,[this]{
    observeSerialData(wifi_->readAll().toStdString());
  });
  connect(wifi_,&QTcpSocket::disconnected,this,[this]{
    machineConsole_->appendPlainText("Conexão Wi-Fi encerrada.");
    captureEvent("grbl_tcp_disconnected");refreshMachineStatus();
  });
  connect(wifi_,&QTcpSocket::errorOccurred,this,[this](QAbstractSocket::SocketError){
    if(machineConsole_)machineConsole_->appendPlainText("Falha de rede; verifique IP, porta e se a máquina está disponível.");
    captureEvent("grbl_tcp_error");
    alaser::PersistentSessionStore::recordErrorCode("tcp_error");
    refreshMachineStatus();
  });
  connect(connectWifi,&QPushButton::clicked,this,[this,ip,tcpPort]{
    // Physical diagnostics disabled in this release; blocking also applies to programmatic click signals.
    return;
    if(machineConnected()||wifi_->state()!=QAbstractSocket::UnconnectedState){
      QMessageBox::information(this,"Transporte já ativo","Desconecte USB ou Wi-Fi antes de outra conexão.");return;
    }
    const QString address=ip->text().trimmed();
    if(!alaser::tcp196::privateIpv4(address.toStdString())||!alaser::tcp196::validPort(unsigned(tcpPort->value()))){
      QMessageBox::warning(this,"Endereço recusado","Informe somente IPv4 de rede privada local (10.x, 172.16-31.x ou 192.168.x). ");return;
    }
    if(QMessageBox::warning(this,"Wi-Fi somente leitura",
      "Confirma máquina parada, monitorada presencialmente e LightBurn desconectado?\n"
      "Esta versão NÃO envia G-code, homing, reset, laser ou movimento.\n"
      "A conexão TCP é manual e não faz descoberta da rede.",
      QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes)return;
    controller_={};pendingSerial_.clear();wifi_->connectToHost(QHostAddress(address),quint16(tcpPort->value()));
    captureEvent("grbl_tcp_connect_requested");
    alaser::PersistentSessionStore::recordErrorCode("tcp_connection_requested");
    refreshMachineStatus();
  });
  connect(disconnectWifi,&QPushButton::clicked,this,[this]{
    if(!wifi_)return;
    wifi_->abort();refreshMachineStatus();
  });
  auto query=[this](alaser::grbl::Probe p){
   try{
    if(serial_.connected())serial_.request(p,controller_);
    else if(wifiConnected()){
      const std::string bytes=alaser::grbl::probeBytes(p,controller_.grblBanner,controller_.state);
      if(wifi_->write(bytes.data(),qint64(bytes.size()))!=qint64(bytes.size()))throw std::runtime_error("TCP diagnostic query failed");
    } else throw std::runtime_error("Conecte USB ou Wi-Fi manualmente primeiro.");
    ++machineProbes_;captureEvent("grbl_probe");refreshMachineStatus();
   }catch(const std::exception& e){QMessageBox::information(this,"Consulta bloqueada",QString::fromUtf8(e.what()));}
  };
  connect(statusProbe,&QPushButton::clicked,this,[query]{query(alaser::grbl::Probe::Status);});
  connect(firmwareProbe,&QPushButton::clicked,this,[query]{query(alaser::grbl::Probe::Firmware);});
  connect(settingsProbe,&QPushButton::clicked,this,[query]{query(alaser::grbl::Probe::Settings);});
  auto* serialTimer=new QTimer(this);serialTimer->setInterval(250);
  connect(serialTimer,&QTimer::timeout,this,[this]{
   if(!serial_.connected())return;
   try{observeSerialData(serial_.readAvailable());}
   catch(const std::exception&){serial_.close();refreshMachineStatus();
     if(machineConsole_)machineConsole_->appendPlainText("Falha de leitura; porta fechada.");}
  });serialTimer->start();refreshMachineStatus();
  auto* safetyInfo=new QLabel("Perfil e dimensões não alteram limites físicos. Sem transmissão de G-code.");
  safetyInfo->setWordWrap(true);machineLayout->addWidget(safetyInfo);
  machineLayout->addStretch();
  auto* machineScroll=new QScrollArea;machineScroll->setObjectName("machineScroll_196");
  machineScroll->setWidgetResizable(true);machineScroll->setWidget(machineTab);
  inspectorTabs->addTab(machineScroll,"Máquina");

  auto* camTab=new QWidget;auto* camLayout=new QVBoxLayout(camTab);
  camLayout->setContentsMargins(9,9,9,9);camLayout->setSpacing(7);
  camLayout->addWidget(titleLabel("CAM / OPERAÇÕES  ·  FUTURO"));
  for(const QString name:{"Corte de linha","Gravação de linha","Preenchimento raster", "Potência (%)",
         "Velocidade (mm/min)","Número de passes","Biblioteca de materiais","Pré-visualizar percurso","Enviar trabalho ao laser"}){
   auto* b=new QPushButton(name);b->setEnabled(false);
   b->setToolTip("Aguardando desenvolvimento e homologação; não executa operações");camLayout->addWidget(b);
  }
  camLayout->addStretch();inspectorTabs->addTab(camTab,"CAM");
  refreshWorkspaceLabels();
  connect(hideTools,&QPushButton::clicked,this,[left,hideTools]{
   left->setVisible(!left->isVisible());hideTools->setText(left->isVisible()?"‹ Ferramentas":"Ferramentas ›");
  });
  connect(hidePanels,&QPushButton::clicked,this,[right,hidePanels]{
   right->setVisible(!right->isVisible());hidePanels->setText(right->isVisible()?"Painéis ›":"‹ Painéis");
  });
  splitter->addWidget(right);
  splitter->setStretchFactor(0,0);splitter->setStretchFactor(1,1);splitter->setStretchFactor(2,0);
  splitter->setSizes({168,950,275});
  if(isInteractive()){
   QSettings settings("AlencarLaser","DesktopBeta");
   if(settings.contains("ui/splitterState"))splitter->restoreState(settings.value("ui/splitterState").toByteArray());
  }
  layout->addWidget(splitter,1);
  // Menus no mesmo arranjo do Alencar Laser Web; ações reutilizam operações Qt existentes.
  auto* mainMenu=new QMenuBar(this);mainMenu->setObjectName("webInspiredMenu");
  auto command=[this](QMenu* menu,const QString& text,const QString& id){
    auto* action=menu->addAction(text);
    connect(action,&QAction::triggered,this,[this,id]{if(buttons.count(id))buttons.at(id)->click();});
  };
  auto* fileMenu=mainMenu->addMenu("Arquivo");
  command(fileMenu,"Abrir projeto...","open");
  connect(fileMenu->addAction("Abrir último projeto (manual)"),&QAction::triggered,this,[this]{
   run([this]{
    const QString path=QSettings("AlencarLaser","DesktopBeta").value("files/lastProject").toString();
    check(!path.isEmpty()&&QFile::exists(path),"Último projeto indisponível.");
    const QString before=projectPath;
    projectPath=path;
    try{open();}catch(...){projectPath=before;throw;}
   });
  });
  command(fileMenu,"Salvar","save");command(fileMenu,"Salvar como...","save_as");
  auto* editMenu=mainMenu->addMenu("Editar");
  command(editMenu,"Desfazer","undo");command(editMenu,"Refazer","redo");command(editMenu,"Duplicar seleção","duplicate_obj");
  command(editMenu,"Excluir formas selecionadas","delete_shapes");
  command(editMenu,"Selecionar (Esc / V)","select");command(editMenu,"Tesoura (K)","scissors");
  auto* arrangeMenu=mainMenu->addMenu("Organizar");
  command(arrangeMenu,"Agrupar","group");command(arrangeMenu,"Desagrupar","ungroup");
  command(arrangeMenu,"Enviar selecao para camada ativa (Ctrl+L)","move_layer");
  command(arrangeMenu,"Girar 90°","rotate_90");command(arrangeMenu,"Alinhar à esquerda","align_left_195");
  auto* laserMenu=mainMenu->addMenu("Laser");
  auto* wifiEntry=laserMenu->addAction("USB / Wi-Fi — Diagnóstico apenas");
  connect(wifiEntry,&QAction::triggered,this,[wifiLauncher]{wifiLauncher->click();});
  auto* transmit=laserMenu->addAction("Iniciar trabalho / emissão — BLOQUEADO");transmit->setEnabled(false);
  auto* viewMenu=mainMenu->addMenu("Visualizar");
  connect(viewMenu->addAction("Ajustar desenho"),&QAction::triggered,this,[fit]{fit->click();});
  connect(viewMenu->addAction("Enquadrar mesa"),&QAction::triggered,this,[fitBed]{fitBed->click();});
  auto* windowMenu=mainMenu->addMenu("Janela");
  connect(windowMenu->addAction("Exibir / ocultar ferramentas"),&QAction::triggered,this,[hideTools]{hideTools->click();});
  connect(windowMenu->addAction("Exibir / ocultar painéis"),&QAction::triggered,this,[hidePanels]{hidePanels->click();});
  auto* helpMenu=mainMenu->addMenu("Ajuda");
  connect(helpMenu->addAction("Sobre a candidata 19.7"),&QAction::triggered,this,[this]{
    QMessageBox::information(this,"Alencar Laser 19.7",
      "Editor Qt 6 com layout inspirado na versão Web 0.10.58.\n"
      "Zoom até 4096× experimental. GRBL USB/TCP somente leitura.\n"
      "Sem execução de trabalhos, G-code, movimentos ou emissão.\n"
      "Versão de desenvolvimento: não substitui a versão aprovada.");
  });
  layout->insertWidget(1,mainMenu);
  auto* paletteFrame=new QFrame;paletteFrame->setObjectName("layerPaletteBar");
  auto* paletteOuter=new QHBoxLayout(paletteFrame);paletteOuter->setContentsMargins(8,3,8,3);paletteOuter->setSpacing(6);
  auto* palLabel=new QLabel("Camadas");palLabel->setObjectName("canvasMeta");paletteOuter->addWidget(palLabel);
  auto* palScroll=new QScrollArea;palScroll->setWidgetResizable(true);
  palScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  palScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);palScroll->setFixedHeight(38);
  auto* palBody=new QWidget;auto* swatches=new QHBoxLayout(palBody);swatches->setContentsMargins(0,0,0,0);swatches->setSpacing(3);
  const QStringList shades={"#222831","#3A86FF","#E63946","#06D6A0","#FFD166","#F77F00","#9B5DE5","#00BBF9","#A7C957","#FF006E","#748CAB","#219EBC","#B5179E","#38B000","#BC6C25","#8ECAE6"};
  for(int id=0;id<32;++id){
    auto* b=new QPushButton(QString::number(id));b->setObjectName(QString("layer_swatch_%1").arg(id));
    b->setCheckable(true);b->setFixedSize(29,25);b->setFocusPolicy(Qt::NoFocus);
    const QString color=shades.at(id%shades.size());
    b->setStyleSheet(QString("QPushButton { background:%1; color:white; border:1px solid #69717B; border-radius:4px; padding:1px; font-size:10px;} "
      "QPushButton:checked {border:3px solid #FFFFFF;} QPushButton:disabled {background:#41454D; color:#797D82;}").arg(color));
    b->setToolTip(QString("Clique: move seleção para camada %1; sem seleção, ativa. Shift+clique seleciona objetos desta camada.").arg(id));
    connect(b,&QPushButton::clicked,this,[this,id]{
     if(layers->findData(id)<0){
      run([this,id]{view.editor.addLayer(id,"Camada "+std::to_string(id));});
     }
     const int ix=layers->findData(id);
     if(ix<0)return;
     if(QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)){
      if(bezierToolActive_){status->setText("Selecione polilinhas no modo Seleção.");return;}
      view.clearSelection();
      for(const auto& [obj,meta]:view.editor.state().objects)
       if(meta.layer==id&&editable(view.editor.state(),obj))view.selected.insert(obj);
      view.applySelectionStyles();view.viewport()->update();
      layers->setCurrentIndex(ix);
     }else if(!selection().empty()){
      run([this,id]{
       check(!bezierToolActive_,"Mover camadas: polilinhas no modo Seleção.");
       const auto ids=selection();
       view.editor.moveSelectionToLayer(ids,id);view.rebuild();
       captureEvent("objects_layer_changed",int(ids.size()));
       layers->setCurrentIndex(layers->findData(id));
      });
     }else layers->setCurrentIndex(ix);
    });
    layerPalette_.push_back(b);swatches->addWidget(b);
  }
  swatches->addStretch();palScroll->setWidget(palBody);paletteOuter->addWidget(palScroll,1);
  auto* sendToLayer=new QPushButton("→ Enviar seleção");sendToLayer->setObjectName("quick_move_layer");
  sendToLayer->setToolTip("Selecionar uma ou mais formas, clicar na camada e enviar (Ctrl+L)");
  sendToLayer->setFocusPolicy(Qt::NoFocus);paletteOuter->addWidget(sendToLayer);
  connect(sendToLayer,&QPushButton::clicked,this,[this]{buttons.at("move_layer")->click();});
  layout->insertWidget(layout->count()-1,paletteFrame);
  connect(layers,&QComboBox::currentIndexChanged,this,[this]{
    if(refreshing)return;
    for(size_t i=0;i<layerPalette_.size();++i)layerPalette_[i]->setChecked(int(i)==layers->currentData().toInt());
  });

  auto* statusFrame=new QFrame;statusFrame->setObjectName("bottomStatus");
  auto* statusRow=new QHBoxLayout(statusFrame);statusRow->setContentsMargins(12,4,12,4);
  status=new QLabel;status->setObjectName("status");status->setMinimumHeight(24);
  statusRow->addWidget(status,1);layout->addWidget(statusFrame);
  connect(layers,&QComboBox::currentIndexChanged,this,[this]{if(!refreshing)refreshFlags();});
  auto flags=[this]{if(!refreshing)run([this]{view.editor.setLayerFlags(layers->currentData().toInt(),visible->isChecked(),locked->isChecked());view.rebuild();});};
   connect(visible,&QCheckBox::clicked,this,flags);connect(locked,&QCheckBox::clicked,this,flags);
   connect(layerMode_,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this](int index){
    if(refreshing||index<0||layers->currentIndex()<0)return;
    const int id=layers->currentData().toInt();
    const QString mode=layerMode_->itemData(index).toString();
    run([this,id,mode]{
     view.editor.setLayerMode(id,mode.toStdString());
     view.rebuild();captureEvent("layer_mode_visual_changed");
    });
   });
  view.stateChanged=[this]{if(uiPersistenceEnabled_)recoveryPolicy_.edited();refresh();};
  view.diagnosticEvent=[this](const QString& code,int count){captureEvent(code,count);};
  view.drawingLayer=[this]{return layers->currentData().toInt();};
   view.requestTextAt=[this](Pt at){insertVectorText(at);};
   connect(qApp,&QApplication::focusChanged,this,[this](QWidget* previous,QWidget* next){
    if(!inlineTextEntry_||previous!=inlineTextEntry_||next==inlineTextEntry_)return;
    QTimer::singleShot(0,this,[this]{
     if(inlineTextEntry_&&QApplication::focusWidget()!=inlineTextEntry_)
      finishInlineText(!inlineTextEntry_->text().trimmed().isEmpty());
    });
   });
   auto shortcut=[this](const QString& sequence,std::function<void()> fn){
    auto* sc=new QShortcut(QKeySequence(sequence),this);
    editingShortcuts_.push_back(sc);
    connect(sc,&QShortcut::activated,this,[this,fn]{run(fn);});
   };
  shortcut("Ctrl+Z",[this]{if(bezierToolActive_&&curves){if(!curves->dragging())view.nativeHistory(false);}else view.history(false);});
  shortcut("Ctrl+Y",[this]{if(bezierToolActive_&&curves){if(!curves->dragging())view.nativeHistory(true);}else view.history(true);});
  shortcut("Ctrl+Shift+Z",[this]{if(bezierToolActive_&&curves){if(!curves->dragging())view.nativeHistory(true);}else view.history(true);});
  shortcut("Ctrl+S",[this]{save();});shortcut("Ctrl+O",[this]{open();});
  shortcut("Ctrl+Shift+S",[this]{saveAs();});shortcut("Ctrl+Shift+O",[this]{openOther();});
  shortcut("Delete",[this]{
   if(bezierToolActive_)view.nativeDeleteSelected();
   else if(view.tool==QtEditorLab::Tool::Select)view.deleteSelectedShapes();
  });
  shortcut("Ctrl+Delete",[this]{if(bezierToolActive_)view.nativeDeleteContour();});
  view.requestSelectTool=[this]{if(buttons.count("select"))buttons.at("select")->click();};
  view.requestToolChange=[this](QtEditorLab::Tool t){
   QString id;
   switch(t){
    case QtEditorLab::Tool::Select:id="select";break;
    case QtEditorLab::Tool::Line:id="line";break;
    case QtEditorLab::Tool::Scissors:id="scissors";break;
    case QtEditorLab::Tool::Nodes:id="nodes";break;
    default:break;
   }
   if(buttons.count(id))buttons.at(id)->click();
  };
  view.toolWarning=[this](const QString& message){status->setText(message);};
  // Atalho Esc nao pode depender de run(): run() rejeita comandos durante gestos ativos.
  auto* cancelGesture=new QShortcut(QKeySequence("Esc"),this);
  connect(cancelGesture,&QShortcut::activated,this,[this]{
   if(inlineTextEntry_){finishInlineText(true);return;}
   view.cancelCurrentGesture();
   if(buttons.count("select"))buttons.at("select")->click();
  });
  shortcut("V",[this]{buttons.at("select")->click();});
  shortcut("K",[this]{buttons.at("scissors")->click();});
  shortcut("Ctrl+K",[this]{buttons.at("scissors")->click();});
  shortcut("Ctrl+R",[this]{buttons.at("rectangle")->click();});
  shortcut("P",[this]{buttons.at("pencil")->click();});
  shortcut("T",[this]{buttons.at("text")->click();});
  shortcut("Ctrl+E",[this]{buttons.at("ellipse")->click();});
  shortcut("Ctrl+L",[this]{buttons.at("move_layer")->click();});
   shortcut("Ctrl+D",[this]{buttons.at("duplicate_obj")->click();});
   shortcut("Ctrl+U",[this]{buttons.at("ungroup")->click();});
   shortcut("Ctrl+G",[this]{buttons.at("group")->click();});
  shortcut("Ctrl+0",[fit]{fit->click();});
  applyTheme();refresh();view.reportZoom();
  uiPersistenceEnabled_=isInteractive();
  if(uiPersistenceEnabled_){
   QSettings settings("AlencarLaser","DesktopBeta");
   if(settings.contains("ui/windowGeometry"))restoreGeometry(settings.value("ui/windowGeometry").toByteArray());
   recoveryTimer_=new QTimer(this);
   recoveryTimer_->setInterval(75000);
   connect(recoveryTimer_,&QTimer::timeout,this,[this]{snapshotForRecovery();});
   recoveryTimer_->start();
   QTimer::singleShot(0,this,[this]{recoverAtStartup();});
  }
 }
 void dragEnterEvent(QDragEnterEvent* e)override{
  const auto* mime=e->mimeData();
  if(!mime||!mime->hasUrls()||mime->urls().size()!=1){e->ignore();return;}
  const QUrl url=mime->urls().first();
  const QString suffix=QFileInfo(url.toLocalFile()).suffix().toLower();
  if(url.isLocalFile()&&(suffix=="alaser"||suffix=="lbrn2"))
   e->acceptProposedAction();
  else e->ignore();
 }
 void dropEvent(QDropEvent* e)override{
  const auto* mime=e->mimeData();
  if(!mime||!mime->hasUrls()||mime->urls().size()!=1){e->ignore();return;}
  const QUrl url=mime->urls().first();
  const QString suffix=QFileInfo(url.toLocalFile()).suffix().toLower();
  if(!url.isLocalFile()||(suffix!="alaser"&&suffix!="lbrn2")){e->ignore();return;}
  e->acceptProposedAction();
  try{openFilePath(url.toLocalFile());}
  catch(const std::exception& error){
   QMessageBox::warning(this,"Arquivo nao importado",QString::fromUtf8(error.what())+
      "\n\nO desenho anterior foi preservado. Use Referencias para inspecionar o .lbrn2.");
  }
 }
 std::vector<int> selection()const{return {view.selected.begin(),view.selected.end()};}
  void run(const std::function<void()>& action){
   if(inlineTextEntry_)
    finishInlineText(!inlineTextEntry_->text().trimmed().isEmpty());
   if(view.busy()||(curves&&curves->dragging())){status->setText("Conclua ou cancele o arraste antes de outro comando.");return;}
  const auto generationBefore=view.editor.generation;
  try{action();lastError.clear();
   if(uiPersistenceEnabled_&&view.editor.generation!=generationBefore)recoveryPolicy_.edited();
   refresh();
  }catch(const std::exception& e){
   lastError=QString::fromUtf8(e.what());status->setText(lastError);
   captureEvent("command_error"); // não captura mensagem ou caminho
  }
  view.setFocus();
 }
  void refreshFlags(){
   if(layers->currentIndex()<0)return;
   refreshing=true;
   const auto l=view.editor.state().layers.at(layers->currentData().toInt());
   visible->setChecked(l.visible);locked->setChecked(l.locked);
   if(layerMode_){
    QSignalBlocker guard(layerMode_);
    int modeIndex=layerMode_->findData(QString::fromStdString(l.mode));
    layerMode_->setCurrentIndex(modeIndex<0?0:modeIndex);
   }
   refreshing=false;
  }
 void refresh(){
  int current=layers->currentData().toInt();refreshing=true;layers->clear();for(const auto& [id,l]:view.editor.state().layers)layers->addItem(QString::fromStdString(l.name),id);
  int ix=layers->findData(current);layers->setCurrentIndex(ix<0?0:ix);refreshing=false;refreshFlags();
  for(size_t i=0;i<layerPalette_.size();++i){
   auto* b=layerPalette_[i];b->setEnabled(true);
   b->setChecked(int(i)==layers->currentData().toInt());
  }
  const int curveCount=curves?int(curves->committed().contours.size()):0;
  status->setText(QString("%1 contornos | %2 nós | %3 selecionados | curvas v5: %4 | %5 | %6")
    .arg(view.editor.state().doc.contours.size()).arg(countNodes(view.editor.state().doc)).arg(view.selected.size())
    .arg(curveCount)
    .arg(bezierToolActive_?"modo Bézier":"modo polilinha")
    .arg(bezierToolActive_?"Ctrl+arraste move curva | Delete nó | Ctrl+Delete curva":"Arraste move formas | Esc selecionar | K tesoura | Ctrl+L camada")
    +QString(" | Desenho %1×%2 mm / mesa %3×%4 mm")
      .arg(workspaceCfg_.drawingWidth,0,'f',0).arg(workspaceCfg_.drawingHeight,0,'f',0)
      .arg(workspaceCfg_.machineWidth,0,'f',0).arg(workspaceCfg_.machineHeight,0,'f',0));
 }
 void save(){
  if(projectPath.isEmpty())projectPath=QFileDialog::getSaveFileName(this,"Salvar projeto",{},"Alencar Laser (*.alaser)");
  if(projectPath.isEmpty())return;
  CurveDocument cd=curves?curves->committed():CurveDocument{};
  loadedProject_=saveNative(projectPath,view.editor.state(),cd,
    loadedProject_?&*loadedProject_:nullptr);
  recoveryPolicy_.saved();
  alaser::PersistentSessionStore::discard();
  if(uiPersistenceEnabled_){
   QSettings settings("AlencarLaser","DesktopBeta");
   settings.setValue("files/lastProject",projectPath);
  }
  captureEvent("project_saved");
  status->setText(QString("Salvo v5: %1 (curvas nativas: %2)").arg(projectPath).arg(cd.contours.size()));
 }
 void saveAs(){
  const QString chosen=QFileDialog::getSaveFileName(this,"Salvar projeto como",projectPath,"Alencar Laser (*.alaser)");
  if(chosen.isEmpty())return;
  const QString previous=projectPath;projectPath=chosen;
  try{save();}catch(...){projectPath=previous;throw;}
 }

 bool eventFilter(QObject* object,QEvent* event)override{
  if(inlineTextEntry_&&object==inlineTextEntry_){
   if(event->type()==QEvent::ShortcutOverride){
    auto* key=static_cast<QKeyEvent*>(event);
    if(key->key()==Qt::Key_Escape){event->accept();return false;}
   }
   if(event->type()==QEvent::KeyPress){
    auto* key=static_cast<QKeyEvent*>(event);
    if(key->key()==Qt::Key_Escape){finishInlineText(true);return true;}
   }
  }
  return QWidget::eventFilter(object,event);
 }
  void finishInlineText(bool commit){
   if(!inlineTextEntry_)return;
  QLineEdit* field=inlineTextEntry_;
  const QString message=field->text();const Pt anchor=inlineTextAnchor_;
   inlineTextEntry_=nullptr;
   field->removeEventFilter(this);field->hide();field->deleteLater();
   for(QShortcut* shortcut:editingShortcuts_)shortcut->setEnabled(true);
  if(!commit||message.trimmed().isEmpty()){view.setFocus();return;}
  try{
   if(message.size()>100)throw std::runtime_error("Texto limitado a 100 caracteres");
   auto outlines=vectorTextOutlines(message,inlineTextFont_,inlineTextHeightMm_,anchor);
   const auto ids=view.editor.addContours(std::move(outlines),true,layers->currentData().toInt());
   view.clearSelection();view.invalidateSelectionCache();view.rebuild();
   view.activate({ids.front(),0});recoveryPolicy_.edited();
   captureEvent("vector_text_created");
   status->setText(QString("Texto inserido na mesa: %1 contornos; Ctrl+Z desfaz.").arg(ids.size()));
  }catch(const std::exception& ex){status->setText(QString::fromUtf8(ex.what()));}
  view.setFocus();
 }
 void insertVectorText(Pt at){
  if(inlineTextEntry_)finishInlineText(true);
  if(view.busy()||layers->currentIndex()<0)return;
  const auto layer=view.editor.state().layers.at(layers->currentData().toInt());
  if(!layer.visible||layer.locked){
   status->setText("Texto: camada deve estar visivel e desbloqueada.");return;
  }
  inlineTextAnchor_=at;
  auto* field=new QLineEdit(view.viewport());
  inlineTextEntry_=field;field->setObjectName("inlineTextEdit");
  field->setMaxLength(100);field->setPlaceholderText("Digite texto...");
  const int px=int(std::clamp(inlineTextHeightMm_*view.currentZoom(),12.,82.));
  QFont font=inlineTextFont_;font.setPixelSize(px);field->setFont(font);
  field->setFrame(false);
  field->setStyleSheet("QLineEdit#inlineTextEdit { background:transparent; border:none;"
     " color:#FFAA54; padding:0; selection-background-color:#996031; }");
  QPoint point=view.mapFromScene(QPointF(at.x,at.y));
  point.setX(std::clamp(point.x(),0,std::max(0,view.viewport()->width()-80)));
  point.setY(std::clamp(point.y(),0,std::max(0,view.viewport()->height()-px-12)));
   field->move(point);field->resize(100,px+12);
   connect(field,&QLineEdit::textChanged,this,[this,field](const QString& text){
    if(field!=inlineTextEntry_)return;
    const int textPixels=QFontMetrics(field->font()).horizontalAdvance(text);
    const int available=std::max(80,view.viewport()->width()-field->x()-8);
    field->setFixedWidth(std::clamp(textPixels+44,80,available));
   });
   for(QShortcut* shortcut:editingShortcuts_)shortcut->setEnabled(false);
   field->installEventFilter(this);
  connect(field,&QLineEdit::returnPressed,this,[this]{finishInlineText(true);});
  field->show();field->raise();field->setFocus(Qt::MouseFocusReason);
   status->setText("Texto direto: digite no desenho; Enter/Esc conclui; trocar ferramenta cancela campo vazio.");
 }

 void loadRasterFromFile(const QString& file){
  QImage image(file);
  if(image.isNull())throw std::runtime_error("Imagem nao suportada (PNG/JPG/BMP/WebP)");
  view.setRasterImage(image);
  status->setText("Imagem importada. Clique direito na mesa > Tracar imagem (Alt+T).");
 }
 void traceImageDialog(){
  if(!view.hasRasterImage()){status->setText("Importe uma imagem primeiro.");return;}
  if(view.busy()){status->setText("Conclua o gesto atual antes de rastrear.");return;}
  captureEvent("image_trace_started");
  QDialog dlg(this);dlg.setWindowTitle("Rastrear imagem — P&B / Alencar Laser 19.8");
  dlg.resize(550,580);auto* layout=new QVBoxLayout(&dlg);
  auto* form=new QFormLayout;layout->addLayout(form);
  auto* threshold=new QSlider(Qt::Horizontal);threshold->setRange(1,254);
  threshold->setValue(std::clamp(trace198::otsuThreshold(trace198::grayscale(
   view.rasterImage().scaled(960,960,Qt::KeepAspectRatio,Qt::SmoothTransformation))),1,254));
  auto* minPixels=new QSpinBox;minPixels->setRange(1,500);minPixels->setValue(5);
  auto* optimize=new QDoubleSpinBox;optimize->setRange(0,2);optimize->setDecimals(2);optimize->setSingleStep(.1);optimize->setValue(.45);
  auto* smooth=new QCheckBox("Arredondamento leve (pode deformar cantos)");
  auto* width=new QDoubleSpinBox;width->setRange(1,400);width->setSuffix(" mm");width->setValue(view.rasterBounds().width());
  form->addRow("Limiar P&B",threshold);form->addRow("Ignorar ilhas menores",minPixels);
  form->addRow("Otimizar (px)",optimize);form->addRow("Suavizar",smooth);
  form->addRow("Largura física",width);
  auto* preview=new QLabel("Clique em Previsualizar para calcular os vetores");
  preview->setObjectName("trace_preview");preview->setMinimumHeight(280);
  preview->setAlignment(Qt::AlignCenter);
  preview->setStyleSheet("background:#fff;color:#202020;border:1px solid #666;");
  layout->addWidget(preview,1);
  auto* information=new QLabel("Limite 80 mil nós. Arquivo de imagem original nunca é alterado.");
  information->setWordWrap(true);layout->addWidget(information);
  auto* previewButton=new QPushButton("Previsualizar vetores (roxo)");layout->addWidget(previewButton);
  auto* actions=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  auto* okay=actions->button(QDialogButtonBox::Ok);okay->setText("Criar traçado");okay->setEnabled(false);
  layout->addWidget(actions);
  std::optional<trace198::Result> prepared;
  auto stale=[&]{prepared.reset();okay->setEnabled(false);information->setText("Atualize a prévia após alterar parâmetros.");};
  connect(threshold,&QSlider::valueChanged,&dlg,[&](int){stale();});
  connect(minPixels,QOverload<int>::of(&QSpinBox::valueChanged),&dlg,[&](int){stale();});
  connect(optimize,QOverload<double>::of(&QDoubleSpinBox::valueChanged),&dlg,[&](double){stale();});
  connect(width,QOverload<double>::of(&QDoubleSpinBox::valueChanged),&dlg,[&](double){stale();});
  connect(smooth,&QCheckBox::toggled,&dlg,[&](bool){stale();});
  connect(previewButton,&QPushButton::clicked,&dlg,[&]{
   try{
    trace198::Options opts;opts.threshold=threshold->value();
    opts.minPixels=minPixels->value();opts.optimizePx=optimize->value();
    opts.smooth=smooth->isChecked();opts.widthMm=width->value();
    prepared=trace198::trace(view.rasterImage(),opts);
    const QSize target=prepared->binary.size().scaled(QSize(460,320),Qt::KeepAspectRatio);
    QImage image=prepared->binary.scaled(target,Qt::IgnoreAspectRatio,Qt::FastTransformation)
       .convertToFormat(QImage::Format_ARGB32);
    QPainter painter(&image);painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor("#A200E2"),1.2));painter.setBrush(Qt::NoBrush);
    const double scale=target.width()/opts.widthMm;
    size_t nodes=0;
    for(const auto& path:prepared->contours){
     if(path.empty())continue;nodes+=path.size();
     QPainterPath vector;vector.moveTo(path.front().x*scale,path.front().y*scale);
     for(size_t i=1;i<path.size();++i)
      vector.lineTo(path[i].x*scale,path[i].y*scale);
     vector.closeSubpath();painter.drawPath(vector);
    }
    painter.end();preview->setPixmap(QPixmap::fromImage(image));
    information->setText(QString("%1 contornos, %2 nós; %3 pequenos componentes removidos. Revisar texto pequeno.")
      .arg(prepared->contours.size()).arg(nodes).arg(prepared->discardedComponents));
    okay->setEnabled(true);
   }catch(const std::exception& e){
    prepared.reset();okay->setEnabled(false);
    information->setText(QString("Rastreio recusado: %1").arg(QString::fromUtf8(e.what())));
   }
  });
  connect(actions,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);
  connect(actions,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
  if(dlg.exec()!=QDialog::Accepted||!prepared){captureEvent("image_trace_cancelled");return;}
  const QRectF anchor=view.rasterBounds();
  auto contours=std::move(prepared->contours);
  for(auto& poly:contours)for(auto& p:poly){p.x+=anchor.left();p.y+=anchor.top();}
  const int count=int(contours.size());
  view.editor.addContours(std::move(contours),true,layers->currentData().toInt());
  view.setRasterWidthMm(width->value());
  view.clearSelection();view.invalidateSelectionCache();view.rebuild();
  recoveryPolicy_.edited();captureEvent("image_trace_created",count);
  refresh();
  status->setText(QString("%1 contornos adicionados. Imagem original somente como referencia.").arg(count));
 }
 void openOther(){
  const QString chosen=QFileDialog::getOpenFileName(this,"Abrir arquivo de projeto",
   projectPath,"Projetos de desenho (*.alaser *.lbrn2);;Alencar Laser (*.alaser);;LightBurn (*.lbrn2)");
  if(!chosen.isEmpty())openFilePath(chosen);
 }
 void ensureSafeToReplace(){
  if(!uiPersistenceEnabled_||!recoveryPolicy_.dirty())return;
  const auto answer=QMessageBox::warning(this,"Projeto com edicoes nao salvas",
     "O projeto atual tem alteracoes nao salvas. Salve antes de abrir outro arquivo.",
     QMessageBox::Save|QMessageBox::Cancel,QMessageBox::Save);
  if(answer!=QMessageBox::Save)throw std::runtime_error("Abertura cancelada: projeto atual preservado");
  save();
  if(recoveryPolicy_.dirty())throw std::runtime_error("Salvamento cancelado: projeto atual preservado");
 }
 void openFilePath(const QString& path){
  const QString suffix=QFileInfo(path).suffix().toLower();
  if(suffix=="alaser"){
   ensureSafeToReplace();
   const QString previous=projectPath;projectPath=path;
   try{open();}catch(...){projectPath=previous;throw;}
   return;
  }
  if(suffix!="lbrn2")throw std::runtime_error("Use .alaser ou .lbrn2");
  // Mensagem visivel quando um arquivo LBRN2 real contem formas ausentes
  // do importador. Nunca trocar o desenho atual numa importacao incompleta.
  lightburn::ImportResult imported=lightburn::ImportResult(fromDocument(Document{}),0);
  try{imported=lightburn::readGeometry(path);}
  catch(const std::exception& error){
   QMessageBox::warning(this,"LightBurn: importacao ainda nao compativel",
      QString("Nao foi possivel abrir este arquivo .lbrn2 por inteiro.\n\n"
       "Motivo tecnico: %1\n\n"
       "O projeto aberto no Alencar Laser NAO foi alterado. "
       "Nao sera feita uma importacao parcial sem sua autorizacao.\n\n"
       "Para ampliar a compatibilidade, envie uma copia deste .lbrn2 para analise.")
        .arg(QString::fromUtf8(error.what())));
   status->setText("Importacao LBRN2 nao realizada: formato ainda incompatível");
   return;
  }
  const auto response=QMessageBox::question(this,"Importar LightBurn (somente desenhos)",
    QString("Foram reconhecidas %1 formas vetoriais.\n\n"
            "Substituir o desenho atual por uma COPIA editavel?\n"
            "As configuracoes de corte e da maquina NAO serao importadas.\n"
            "O arquivo .lbrn2 original nao sera alterado.")
      .arg(imported.shapeCount),
    QMessageBox::Yes|QMessageBox::No,QMessageBox::No);
  if(response!=QMessageBox::Yes)return;
  ensureSafeToReplace();
  view.attachNative(nullptr);
  curves.reset();
  view.editor=Editor(imported.state);
  view.clearSelection();view.invalidateSelectionCache();view.rebuild();
  bezierToolActive_=false;
  loadedProject_.reset();
  projectPath.clear(); // Nunca salvar por cima de um .lbrn2!
  buttons.at("select")->click();
  recoveryPolicy_.edited();
  captureEvent("lightburn_geometry_imported");
  refresh();
  view.setFocus();
  status->setText(QString("Importadas %1 formas LightBurn. Salve como .alaser para preservar.")
     .arg(imported.shapeCount));
 }
 void open(){
  if(projectPath.isEmpty()){
   const QString chosen=QFileDialog::getOpenFileName(this,"Abrir projeto",{},
    "Projetos (*.alaser *.lbrn2);;Alencar Laser (*.alaser);;LightBurn (*.lbrn2)");
   if(chosen.isEmpty())return;
   if(QFileInfo(chosen).suffix().compare("lbrn2",Qt::CaseInsensitive)==0){
    openFilePath(chosen);return;
   }
   projectPath=chosen;
  }
  auto project=openNative(projectPath);
  auto state=editorStateForIntegratedProject(project);
  view.editor=Editor(std::move(state));
  view.invalidateSelectionCache();view.clearSelection();view.rebuild();
  view.attachNative(nullptr); // elimina itens antes de destruir sua sessao
  if(project.version>=5&&!project.curves.contours.empty())
   curves=std::make_unique<CurveDragSession>(std::move(project.curves));
  else
   curves.reset();
  bezierToolActive_=false;view.attachNative(curves.get());
  buttons.at("select")->setChecked(true);
  loadedProject_=std::move(project);
  recoveryPolicy_.opened();
  if(uiPersistenceEnabled_&&!restoringSnapshot_){
   QSettings settings("AlencarLaser","DesktopBeta");
   settings.setValue("files/lastProject",projectPath);
  }
  captureEvent("project_opened");
  status->setText(QString("Aberto schema v%1 | curvas nativas: %2")
    .arg(project.version).arg(curves?int(curves->committed().contours.size()):0));
 }
 // API usada pelos testes de integração: carrega curvas nativas sem arquivo.
 void setCurves(CurveDocument doc){
  loadedProject_.reset(); // origem anterior nao pertence ao novo documento de teste
  view.attachNative(nullptr);
  if(doc.contours.empty())curves.reset();
  else curves=std::make_unique<CurveDragSession>(std::move(doc));
  bezierToolActive_=false;view.attachNative(curves.get());
 }
 const CurveDocument* committedCurves()const{return curves?&curves->committed():nullptr;}
};
static void clickButton(LabWindow& w,const QString& id){QTest::mouseClick(w.buttons.at(id),Qt::LeftButton);QApplication::processEvents();check(w.lastError.isEmpty(),qPrintable(w.lastError));}
static void pressKey(LabWindow& w,Qt::Key key,Qt::KeyboardModifiers mods=Qt::NoModifier){w.view.setFocus();QTest::keyClick(&w.view,key,mods);QApplication::processEvents();check(w.lastError.isEmpty(),qPrintable(w.lastError));}
static void clickScene(LabWindow& w,QPointF point,Qt::KeyboardModifiers mods=Qt::NoModifier){QTest::mouseClick(w.view.viewport(),Qt::LeftButton,mods,w.view.mapFromScene(point));QApplication::processEvents();}
static void checkRendered(const LabWindow& w){
 auto& v=const_cast<QtEditorLab&>(w.view);for(const auto& c:v.editor.state().doc.contours){check(v.pathItem(c.key)->path()==pathOf(c),"rendered geometry differs from canonical state");check(v.pathItem(c.key)->isVisible()==v.editor.state().layers.at(v.editor.state().objects.at(c.key.object).layer).visible,"layer visibility mismatch");}
}
static QJsonObject memoryUsage(){
#ifdef Q_OS_WIN
 PROCESS_MEMORY_COUNTERS counters{};counters.cb=sizeof(counters);
 if(GetProcessMemoryInfo(GetCurrentProcess(),&counters,sizeof(counters)))return {{"available",true},{"rss_bytes",double(counters.WorkingSetSize)},{"peak_rss_bytes",double(counters.PeakWorkingSetSize)},{"source","GetProcessMemoryInfo"}};
#else
 QFile status("/proc/self/status");if(status.open(QIODevice::ReadOnly)){QJsonObject r{{"available",false},{"source","/proc/self/status"}};for(auto line:status.readAll().split('\n')){auto fields=line.simplified().split(' ');if(fields.size()>1&&(fields[0]=="VmRSS:"||fields[0]=="VmHWM:")){r[fields[0]=="VmRSS:"?"rss_bytes":"peak_rss_bytes"]=double(fields[1].toLongLong()*1024);r["available"]=true;}}return r;}
#endif
 return {{"available",false}};
}
static QJsonObject telemetry(const LabWindow& w){return {{"marker_candidates",double(w.view.markerCandidates)},{"markers_drawn",double(w.view.markerDraws)},{"full_markers",qEnvironmentVariableIsSet("ALENCAR_FULL_MARKERS")},{"memory",memoryUsage()},{"paint_count",int(w.view.paintMs.size())},{"paint_p50_ms",percentile(w.view.paintMs,.5)},{"paint_p95_ms",percentile(w.view.paintMs,.95)},{"input_to_paint_samples",int(w.view.inputToPaintMs.size())},{"input_to_paint_p50_ms",percentile(w.view.inputToPaintMs,.5)},{"input_to_paint_p95_ms",percentile(w.view.inputToPaintMs,.95)},{"platform",QGuiApplication::platformName()},{"os",QSysInfo::prettyProductName()},{"dpr",w.view.viewport()->devicePixelRatioF()},{"qt",qVersion()},{"hardware_connected",false},{"localized_node_updates",w.view.localizedNodeRepaints},{"localized_node_area_px",double(w.view.localizedNodeAreaPx)}};}
static QJsonObject integratedTest(const QString& csv,const QString& output){
 LabWindow w(readCsv(csv.toStdString()));auto& v=w.view;w.show();w.activateWindow();v.setTransform(QTransform::fromScale(6.,6.));QApplication::processEvents();
 check(v.editor.state().doc.contours.size()==575&&countNodes(v.editor.state().doc)==13415,"real fixture mismatch");
 QJsonArray stages;auto stage=[&](const char* label){checkRendered(w);stages.append(QJsonObject{{"name",label},{"status","PASS"},{"objects",int(v.editor.state().objects.size())},{"undo",int(v.editor.undoCount())},{"redo",int(v.editor.redoCount())}});};
 // Select actual fixture geometry solely by delivered mouse events.
 auto reference=v.editor.state().doc;auto seed=reference.contours[0].nodes[15];v.centerOn(toQt(seed.p));QApplication::processEvents();
 QPoint start=v.mapFromScene(toQt(seed.p));auto candidates=bruteHits(segments(reference),fromQt(v.mapToScene(start)),6./(6.*v.viewport()->devicePixelRatioF()));check(!candidates.empty(),"reference pick has no hit");
 Key targetKey=candidates.front().key;clickButton(w,"select");QTest::mouseClick(v.viewport(),Qt::LeftButton,Qt::NoModifier,start);QApplication::processEvents();
 check(v.active()&&*v.active()==targetKey,"mouse selected wrong real contour");clickButton(w,"nodes");
 const auto* contour=get(reference,targetKey);auto node=contour->nodes[0];double nearest=6.;bool found=false;
 for(const auto& candidate:contour->nodes){auto pixel=v.viewportTransform().map(toQt(candidate.p));double d=QLineF(pixel,start).length()*v.viewport()->devicePixelRatioF();if(d<nearest){nearest=d;node=candidate;found=true;}}
 check(found,"reference node missing");auto unchanged=stateText(v.editor.state());auto historyBefore=v.editor.undoCount();
 QTest::mousePress(v.viewport(),Qt::LeftButton,Qt::NoModifier,start);
 Document expected=reference;auto* expectedContour=get(expected,targetKey);QElapsedTimer timer;std::vector<double> eventMs;
 auto inv=v.viewportTransform().inverted();
 const int dragFrames=std::clamp(qEnvironmentVariableIntValue("ALENCAR_DRAG_FRAMES"),30,10000);
 for(int frame=1;frame<=dragFrames;++frame){int i=frame==dragFrames?30:1+(frame-1)%30;QPoint end=start+QPoint(i,-i/2);auto delta=inv.map(QPointF(end))-inv.map(QPointF(start));
  for(auto& n:expectedContour->nodes)if(n.id==node.id)n.p=node.p+fromQt(delta);
  timer.start();QTest::mouseMove(v.viewport(),end);QApplication::processEvents();eventMs.push_back(timer.nsecsElapsed()/1e6);
  check(stateText(v.editor.state())==unchanged&&v.editor.undoCount()==historyBefore,"preview modified model/history");
  check(v.previewMatches(expected),"TESTE_12: preview differs from canonical edited contour");
  for(const auto& c:reference.contours){
   if(v.active()&&c.key==*v.active()&&v.usingContinuousPreview())continue;
   check(v.pathItem(c.key)->path()==pathOf(c),"TESTE_12: unrelated canonical path changed during preview");
  }
 }
 w.grab().save(output+"/arraste_ao_vivo_TESTE_6.png");
 QTest::mouseRelease(v.viewport(),Qt::LeftButton,Qt::NoModifier,start+QPoint(30,-15));QApplication::processEvents();
 check(serialize(encode(v.editor.state().doc))==serialize(encode(expected))&&v.editor.undoCount()==historyBefore+1,"drag commit identity/history mismatch");
 check(v.localizedNodeRepaints>=30,"TESTE_9: drag did not invalidate node marker ROIs");
 if(v.usingContinuousPreview())
  check(v.fullDragPathUpdates()>=30&&v.chunkPathsUpdated()==0,"TESTE_12: continuous preview not used by default");
 else
  check(v.chunkPathsUpdated()>=30&&v.fullDragPathUpdates()==0,"TESTE_12: experimental chunk preview not used");
 stage("real_mouse_selection_and_30_live_frames");
 auto moved=stateText(v.editor.state());pressKey(w,Qt::Key_Z,Qt::ControlModifier);check(stateText(v.editor.state())==unchanged,"Ctrl+Z did not undo move");
 pressKey(w,Qt::Key_Y,Qt::ControlModifier);check(stateText(v.editor.state())==moved,"Ctrl+Y did not redo move");
 pressKey(w,Qt::Key_Z,Qt::ControlModifier);pressKey(w,Qt::Key_Z,Qt::ControlModifier|Qt::ShiftModifier);check(stateText(v.editor.state())==moved,"Ctrl+Shift+Z did not redo move");stage("keyboard_history_three_shortcuts");
 // Reselect via mouse after undo/redo, then cancel and release.
 clickButton(w,"select");QTest::mouseClick(v.viewport(),Qt::LeftButton,Qt::NoModifier,start+QPoint(30,-15));clickButton(w,"nodes");
 auto cancelGeneration=v.editor.generation;
 QTest::mousePress(v.viewport(),Qt::LeftButton,Qt::NoModifier,start+QPoint(30,-15));QTest::mouseMove(v.viewport(),start+QPoint(50,10));QApplication::processEvents();
 check(v.hasChunkPreview(),"TESTE_10: cancel drag did not start");
 // History while dragging is blocked rather than invalidating pointers into the scene.
 pressKey(w,Qt::Key_Z,Qt::ControlModifier);check(v.editor.generation==cancelGeneration,"history changed during drag");
 pressKey(w,Qt::Key_Escape);QTest::mouseRelease(v.viewport(),Qt::LeftButton,Qt::NoModifier,start+QPoint(50,10));QApplication::processEvents();
 check(stateText(v.editor.state())==moved&&v.editor.generation==cancelGeneration,"cancel mutated document");stage("escape_release_and_history_guard");
 auto draw=[&](const QString& tool,QPointF a,QPointF b,int nodes){
  clickButton(w,tool);v.centerOn((a+b)/2);QApplication::processEvents();auto pa=v.mapFromScene(a),pb=v.mapFromScene(b);
  QPointF aa=v.mapToScene(pa),bb=v.mapToScene(pb);auto before=stateText(v.editor.state());int count=int(v.editor.state().objects.size());
  QTest::mousePress(v.viewport(),Qt::LeftButton,Qt::NoModifier,pa);QTest::mouseMove(v.viewport(),pb);QApplication::processEvents();
  check(stateText(v.editor.state())==before,"shape preview changed document");w.grab().save(output+"/previa_"+tool+"_TESTE_6.png");
  QTest::mouseRelease(v.viewport(),Qt::LeftButton,Qt::NoModifier,pb);QApplication::processEvents();int id=v.lastCreated();
  check(int(v.editor.state().objects.size())==count+1,"shape missing");auto* c=get(v.editor.state().doc,{id,0});check(c&&int(c->nodes.size())==nodes,"shape topology mismatch");
  QPointF lo(std::min(aa.x(),bb.x()),std::min(aa.y(),bb.y())),hi(std::max(aa.x(),bb.x()),std::max(aa.y(),bb.y()));
  for(int i=0;i<nodes;++i){QPointF wanted;
   if(tool=="line")wanted=i==0?aa:bb;
   else if(tool=="rectangle"){std::vector<QPointF> pts{lo,{hi.x(),lo.y()},hi,{lo.x(),hi.y()}};wanted=pts[size_t(i)];}
   else {double theta=2*std::numbers::pi*i/nodes;double rx=(hi.x()-lo.x())/2,ry=(hi.y()-lo.y())/2;if(tool=="polygon")rx=ry=std::min(rx,ry);wanted=(lo+hi)/2+QPointF(rx*std::cos(theta),ry*std::sin(theta));}
   check(QLineF(toQt(c->nodes[size_t(i)].p),wanted).length()<1e-9,"shape coordinate oracle mismatch");
  }
  auto created=stateText(v.editor.state());pressKey(w,Qt::Key_Z,Qt::ControlModifier);check(stateText(v.editor.state())==before,"shape keyboard undo mismatch");
  pressKey(w,Qt::Key_Z,Qt::ControlModifier|Qt::ShiftModifier);check(stateText(v.editor.state())==created,"shape keyboard redo mismatch");stage(qPrintable("mouse_shape_"+tool));return id;
 };
 int a=draw("rectangle",{-920,-920},{-890,-900},4),b=draw("rectangle",{-920,-920},{-890,-900},4);
 clickButton(w,"select");v.centerOn(-905,-910);QApplication::processEvents();QPointF overlap(-905,-920);
 clickScene(w,overlap);check(v.active()&&v.active()->object==a,"first overlap identity wrong");
 clickScene(w,overlap,Qt::AltModifier);check(v.active()&&v.active()->object==b,"Alt click did not reach second identity");
 clickScene(w,overlap,Qt::AltModifier);check(v.active()&&v.active()->object==a,"overlap cycle did not wrap");
 auto nearPixel=v.mapFromScene(overlap);int outside=int(std::ceil(8/v.viewport()->devicePixelRatioF()));QTest::mouseClick(v.viewport(),Qt::LeftButton,Qt::NoModifier,nearPixel-QPoint(0,outside));QApplication::processEvents();check(!v.active(),"outside pixel radius selected a contour");stage("overlap_exact_identity_and_cycle");
 int ellipse=draw("ellipse",{-820,-920},{-790,-900},512),polygon=draw("polygon",{-770,-920},{-740,-890},6);
 draw("line",{-720,-920},{-690,-900},2);
 // GUI multi-selection, group, transform, layer controls, and metadata-aware history.
 clickButton(w,"select");v.centerOn(-780,-905);QApplication::processEvents();auto ellipsePoint=toQt(get(v.editor.state().doc,{ellipse,0})->nodes[0].p);auto polygonPoint=toQt(get(v.editor.state().doc,{polygon,0})->nodes[0].p);
 clickScene(w,ellipsePoint);clickScene(w,polygonPoint,Qt::ShiftModifier);check(v.selected==std::set<int>{ellipse,polygon},"Shift selection mismatch");
 auto beforeGroup=stateText(v.editor.state());clickButton(w,"group");int group=v.editor.state().objects.at(ellipse).group;check(group>0&&v.editor.state().objects.at(polygon).group==group,"UI grouping failed");
 auto grouped=stateText(v.editor.state());pressKey(w,Qt::Key_Z,Qt::ControlModifier);check(stateText(v.editor.state())==beforeGroup,"group undo metadata mismatch");pressKey(w,Qt::Key_Y,Qt::ControlModifier);check(stateText(v.editor.state())==grouped,"group redo metadata mismatch");
 clickScene(w,ellipsePoint);auto beforeMove=v.editor.state().doc;clickButton(w,"move");auto wanted=beforeMove;
 for(auto& c:wanted.contours)if(c.key.object==ellipse||c.key.object==polygon)for(auto& n:c.nodes)n.p.x+=1;
 check(serialize(encode(v.editor.state().doc))==serialize(encode(wanted)),"group translation touched wrong geometry");
 clickButton(w,"ungroup");check(v.editor.state().objects.at(ellipse).group==0&&v.editor.state().objects.at(polygon).group==0,"UI ungroup failed");pressKey(w,Qt::Key_Z,Qt::ControlModifier);check(v.editor.state().objects.at(ellipse).group==group,"undo ungroup failed");
 ellipsePoint+=QPointF(1,0);polygonPoint+=QPointF(1,0);clickScene(w,ellipsePoint);clickScene(w,polygonPoint,Qt::ShiftModifier);
 QTest::mouseClick(w.layerName,Qt::LeftButton);QTest::keyClicks(w.layerName,"Camada TESTE_6");clickButton(w,"add_layer");int layer=w.layers->currentData().toInt();check(layer>0,"new layer UI failed");
 clickButton(w,"move_layer");check(v.editor.state().objects.at(ellipse).layer==layer&&v.editor.state().objects.at(polygon).layer==layer,"move selected layer failed");
 auto layerState=stateText(v.editor.state());QTest::mouseClick(w.visible,Qt::LeftButton);QApplication::processEvents();check(!v.pathItem({ellipse,0})->isVisible(),"hidden layer still rendered");
 clickScene(w,ellipsePoint);check(!v.active(),"hidden object selected");auto hidden=stateText(v.editor.state());
 pressKey(w,Qt::Key_Z,Qt::ControlModifier);check(stateText(v.editor.state())==layerState,"undo visibility metadata mismatch");pressKey(w,Qt::Key_Y,Qt::ControlModifier);check(stateText(v.editor.state())==hidden,"redo visibility metadata mismatch");
 QTest::mouseClick(w.visible,Qt::LeftButton);QTest::mouseClick(w.locked,Qt::LeftButton);QApplication::processEvents();clickScene(w,ellipsePoint);check(!v.active(),"locked object selected");
 check(v.pathItem({ellipse,0})->isVisible(),"locked visible layer disappeared");
 QTest::mouseClick(w.locked,Qt::LeftButton);QApplication::processEvents();clickScene(w,ellipsePoint);check(v.active()&&v.active()->object==ellipse,"unlocked layer not selectable");
 QTest::mouseClick(w.locked,Qt::LeftButton);QApplication::processEvents();stage("gui_group_ungroup_move_layers_visibility_lock");
 // Drawing respects the selected layer, including its lock.
 auto lockedState=stateText(v.editor.state());clickButton(w,"line");v.centerOn(-650,-800);QApplication::processEvents();
 auto lp=v.mapFromScene(QPointF(-660,-810)),lq=v.mapFromScene(QPointF(-640,-790));
 QTest::mousePress(v.viewport(),Qt::LeftButton,Qt::NoModifier,lp);QTest::mouseMove(v.viewport(),lq);QTest::mouseRelease(v.viewport(),Qt::LeftButton,Qt::NoModifier,lq);QApplication::processEvents();
 check(stateText(v.editor.state())==lockedState,"drawing in locked layer changed document");
 QTest::mouseClick(w.locked,Qt::LeftButton);QApplication::processEvents();
 int layered=draw("line",{-660,-810},{-640,-790},2);check(v.editor.state().objects.at(layered).layer==layer,"new shape ignored chosen layer");
 QTest::mouseClick(w.locked,Qt::LeftButton);QApplication::processEvents();stage("drawing_target_layer_and_lock");
 w.grab().save(output+"/laboratorio_integrado_TESTE_6.png");
 // Actual mouse selection latency over the real project, not only calls to the index.
 clickButton(w,"select");QJsonArray timings;
 for(double zoom:{.05,.75,6.,80.,1200.}){
  v.setTransform(QTransform::fromScale(zoom,zoom));v.centerOn(toQt(seed.p));QApplication::processEvents();std::vector<double> latencies;
  QPoint origin=v.mapFromScene(toQt(seed.p));
  for(int i=0;i<40;++i){QPoint p=origin+QPoint(i%9-4,i%7-3);timer.start();QTest::mouseClick(v.viewport(),Qt::LeftButton,Qt::NoModifier,p);QApplication::processEvents();latencies.push_back(timer.nsecsElapsed()/1e6);
   auto expectedHits=bruteHits(segments(v.editor.state().doc),fromQt(v.mapToScene(p)),6./(zoom*v.viewport()->devicePixelRatioF()));
   expectedHits.erase(std::remove_if(expectedHits.begin(),expectedHits.end(),[&](auto h){return !editable(v.editor.state(),h.key.object);}),expectedHits.end());
   check(expectedHits.empty()?!v.active():(v.active()&&*v.active()==expectedHits.front().key),"event benchmark selected incorrect identity");
  }
  timings.append(QJsonObject{{"zoom",zoom},{"events",40},{"click_and_events_p50_ms",percentile(latencies,.5)},{"click_and_events_p95_ms",percentile(latencies,.95)}});
 }
 stage("200_real_mouse_clicks_identity_and_latency");
 // Instrument the exact Qt scene property calls to validate the optimization.
 // This count covers this writer session; separate from CPU/paint timing.
 const qint64 sceneStyleSetters=v.styleSetterCalls();
 check(sceneStyleSetters<50000,"TESTE_7: Qt selection styling is invalidating too many items");

 w.projectPath=output+"/projeto_integrado_TESTE_6.alaser";pressKey(w,Qt::Key_S,Qt::ControlModifier);check(QFile::exists(w.projectPath),"Ctrl+S did not save");
 auto hash=fingerprint(v.editor.state());QFile expectedFile(output+"/estado_esperado_TESTE_6.json");check(expectedFile.open(QIODevice::WriteOnly),"cannot write expected state");
 expectedFile.write(QJsonDocument(QJsonObject{{"fingerprint",QString(hash)},{"writer_pid",QCoreApplication::applicationPid()},{"writer_instance",processInstance},{"objects",int(v.editor.state().objects.size())},{"nodes",int(countNodes(v.editor.state().doc))},{"next_id",double(v.editor.state().doc.nextId)}}).toJson());
 auto result=telemetry(w);result["status"]="PASS";result["stages"]=stages;result["selection_timings"]=timings;result["drag_frames"]=dragFrames;result["drag_event_and_paint_p95_ms"]=percentile(eventMs,.95);result["scene_style_property_setter_calls"]=double(sceneStyleSetters);
 result["geometry_index_builds"]=double(v.indexBuilds());
 result["TESTE_12_preview_mode"]=v.previewModeName();
 result["TESTE_12_default_safe_continuous"]=v.usingContinuousPreview();
 result["TESTE_10_chunk_path_updates"]=v.chunkPathsUpdated();
 result["TESTE_10_full_drag_path_updates"]=v.fullDragPathUpdates();
 result["TESTE_10_max_preview_chunk_count"]=v.previewChunkCount();
 result["spatial_index_queries"]=double(v.indexedQueries());
 result["direct_queries"]=double(v.directQueries());
 result["hit_test_p50_ms"]=percentile(v.hitTestMs,.5);
 result["hit_test_p95_ms"]=percentile(v.hitTestMs,.95);
 result["style_application_p95_ms"]=percentile(v.styleApplyMs,.95);result["writer_pid"]=QCoreApplication::applicationPid();result["fingerprint"]=QString(hash);return result;
}
static QJsonObject reopenTest(const QString& output){
 QFile expectedFile(output+"/estado_esperado_TESTE_6.json");check(expectedFile.open(QIODevice::ReadOnly),"expected state missing");auto expected=QJsonDocument::fromJson(expectedFile.readAll()).object();
 check(expected["writer_instance"].toString()!=processInstance,"reader must be independent process");
 Document empty;LabWindow w(empty);w.show();w.activateWindow();QApplication::processEvents();w.projectPath=output+"/projeto_integrado_TESTE_6.alaser";
 pressKey(w,Qt::Key_O,Qt::ControlModifier);check(QString(fingerprint(w.view.editor.state()))==expected["fingerprint"].toString(),"cross-process geometry/IDs/metadata mismatch");
 check(w.view.editor.undoCount()==0&&w.view.editor.redoCount()==0&&!w.view.active(),"transient state survived restart");checkRendered(w);
 // Verify post-restart editability and allocator through the same GUI.
 auto restored=stateText(w.view.editor.state());int64_t next=w.view.editor.state().doc.nextId;
 clickButton(w,"line");w.view.setTransform(QTransform::fromScale(6.,6.));w.view.centerOn(-500,-500);QApplication::processEvents();
 QTest::mousePress(w.view.viewport(),Qt::LeftButton,Qt::NoModifier,w.view.mapFromScene(QPointF(-510,-510)));QTest::mouseMove(w.view.viewport(),w.view.mapFromScene(QPointF(-490,-490)));QTest::mouseRelease(w.view.viewport(),Qt::LeftButton,Qt::NoModifier,w.view.mapFromScene(QPointF(-490,-490)));QApplication::processEvents();
 auto* c=get(w.view.editor.state().doc,{w.view.lastCreated(),0});check(c&&c->nodes.size()==2&&c->nodes[0].id==next&&c->nodes[1].id==next+1,"allocator after restart mismatch");
 auto edited=stateText(w.view.editor.state());pressKey(w,Qt::Key_Z,Qt::ControlModifier);check(stateText(w.view.editor.state())==restored,"post-restart undo failed");pressKey(w,Qt::Key_Y,Qt::ControlModifier);check(stateText(w.view.editor.state())==edited,"post-restart redo failed");pressKey(w,Qt::Key_Z,Qt::ControlModifier);
 pressKey(w,Qt::Key_S,Qt::ControlModifier);pressKey(w,Qt::Key_O,Qt::ControlModifier);check(stateText(w.view.editor.state())==restored,"overwrite/reload changed state");
 w.view.fitInView(w.view.scene()->itemsBoundingRect().adjusted(-5,-5,5,5),Qt::KeepAspectRatio);QApplication::processEvents();
 w.grab().save(output+"/reaberto_outro_processo_TESTE_6.png");
 return {{"status","PASS"},{"reader_pid",QCoreApplication::applicationPid()},{"reader_instance",processInstance},{"writer_instance",expected["writer_instance"]},{"writer_pid",expected["writer_pid"]},{"fingerprint",QString(fingerprint(w.view.editor.state()))},{"geometry_ids_layers_groups_exact",true},{"keyboard_open_save",true},{"edit_after_restart",true},{"history_shortcuts_after_restart",true},{"native_overwrite",true}};
}
// TESTE_19.1: ensaio automatizado Qt; realiza eventos reais na viewport.
static QJsonObject navigationTest(const QString& csv,const QString& output){
 LabWindow w(readCsv(csv.toStdString()));w.show();QApplication::processEvents();
 auto& v=w.view;v.setTransform(QTransform::fromScale(6.,6.));
 v.centerOn(toQt(v.editor.state().doc.contours.at(0).nodes.at(15).p));QApplication::processEvents();
 const QPoint cursor=v.viewport()->rect().center()+QPoint(32,19);
 const QPointF pinned=v.mapToScene(cursor);
 const double scaleBefore=v.currentZoom();
 QWheelEvent wheelIn(QPointF(cursor),QPointF(v.viewport()->mapToGlobal(cursor)),
  QPoint(0,0),QPoint(0,120),Qt::NoButton,Qt::NoModifier,Qt::NoScrollPhase,false);
 QApplication::sendEvent(v.viewport(),&wheelIn);QApplication::processEvents();
 check(wheelIn.isAccepted(),"mouse wheel was ignored");
 check(v.currentZoom()>scaleBefore,"wheel in did not zoom");
 check(QLineF(pinned,v.mapToScene(cursor)).length()<0.8,"cursor drift during wheel zoom");
 QWheelEvent wheelOut(QPointF(cursor),QPointF(v.viewport()->mapToGlobal(cursor)),
  QPoint(0,0),QPoint(0,-120),Qt::NoButton,Qt::NoModifier,Qt::NoScrollPhase,false);
 QApplication::sendEvent(v.viewport(),&wheelOut);QApplication::processEvents();
 check(std::abs(v.currentZoom()-scaleBefore)<0.01,"wheel out failed to restore scale");
 // TESTE 19.6: verify newly exposed precision zoom above legacy 240x cap.
 v.setTransform(QTransform::fromScale(240.,240.));v.centerOn(toQt(v.editor.state().doc.contours.at(0).nodes.at(15).p));
 QApplication::processEvents();
 QWheelEvent wheelPrecision(QPointF(cursor),QPointF(v.viewport()->mapToGlobal(cursor)),
  QPoint(0,0),QPoint(0,120),Qt::NoButton,Qt::NoModifier,Qt::NoScrollPhase,false);
 QApplication::sendEvent(v.viewport(),&wheelPrecision);QApplication::processEvents();
 check(v.currentZoom()>240.,"precision wheel still capped at 240x");
 v.setTransform(QTransform::fromScale(4096.,4096.));QApplication::processEvents();
 check(std::abs(v.currentZoom()-4096.)<0.01,"4096x precision transformation failed");
 v.setTransform(QTransform::fromScale(scaleBefore,scaleBefore));
 v.centerOn(toQt(v.editor.state().doc.contours.at(0).nodes.at(15).p));QApplication::processEvents();
 const QPointF beforePan=v.mapToScene(cursor);
 QTest::mousePress(v.viewport(),Qt::MiddleButton,Qt::NoModifier,cursor);
 QTest::mouseMove(v.viewport(),cursor+QPoint(30,15));QApplication::processEvents();
 QTest::mouseRelease(v.viewport(),Qt::MiddleButton,Qt::NoModifier,cursor+QPoint(30,15));
 check(QLineF(beforePan,v.mapToScene(cursor)).length()>0.5,"middle-button pan ineffective");
 // Os tres novos comandos devem atuar sobre os objetos legados e manter histórico.
 const int original=v.editor.rectangle({25,25},12,8);v.rebuild();v.activate({original,0});
 const auto startCount=v.editor.state().doc.contours.size();
 clickButton(w,"duplicate_obj");
 check(v.editor.state().doc.contours.size()==startCount+1,"duplicate missing");
 check(v.selected.size()==1,"duplicate did not select new object");
 const int duplicateId=*v.selected.begin();
 const auto* duplicateContour=get(v.editor.state().doc,{duplicateId,0});
 check(duplicateContour&&duplicateContour->nodes.size()==4,"duplicate topology invalid");
 const Pt initial=duplicateContour->nodes.at(0).p;
 clickButton(w,"mirror_horiz");
 const Pt mirrored=get(v.editor.state().doc,{duplicateId,0})->nodes.at(0).p;
 check(std::abs(initial.x-mirrored.x)>1.0,"mirror had no effect");
 clickButton(w,"rotate_90");
 const Pt rotated=get(v.editor.state().doc,{duplicateId,0})->nodes.at(0).p;
 check(std::hypot(rotated.x-mirrored.x,rotated.y-mirrored.y)>1.0,"rotate had no effect");
 pressKey(w,Qt::Key_Z,Qt::ControlModifier);
 check(std::abs(get(v.editor.state().doc,{duplicateId,0})->nodes.at(0).p.x-mirrored.x)<1e-9,
  "rotate undo failed");
 checkRendered(w);
 QDir().mkpath(output);
 w.grab().save(output+"/zoom_e_transformacoes_TESTE_19_1.png");
 return {{"status","PASS"},{"wheel_zoom_at_cursor",true},
  {"middle_mouse_pan",true},{"duplicate",true},{"mirror_x",true},
  {"rotate_90",true},{"undo_after_rotate",true},
  {"fabricationEnabled",false}};
}

// TESTE_19.3: inspeção real de arquivos de referência. Não altera sessão ou a controladora.
static QJsonObject referenceTest(const QString& project,const QString& log){
 const auto r=reference::readLightBurn(project);
 check(r.valid,"LightBurn reference parse failed");
 check(r.count==186,"wrong LightBurn figure count");
 check(r.types.value("Path")==178&&r.types.value("Ellipse")==6&&
       r.types.value("Rect")==1&&r.types.value("Bitmap")==1,"LightBurn figure types mismatch");
 check(r.layers.value("0")==19&&r.layers.value("1")==167,"LightBurn layers mismatch");
 check(r.withoutPrimitives==87,"missing PrimList paths mismatch");
 check(!r.embeddedPreview.isEmpty(),"embedded thumbnail missing");
 const auto l=reference::readGrblLog(log);
 check(l.valid,"GRBL log parse failed");
 check(l.lines==263843,"GRBL log line count mismatch");
 check(l.outputs==44169,"GRBL log packet count mismatch");
 check(l.settings.value("$30")=="1000.000"&&l.settings.value("$32")=="1",
       "GRBL settings not parsed");
 return {{"status","PASS"},{"shapes",r.count},{"pathsWithoutPrimList",r.withoutPrimitives},
         {"logLines",double(l.lines)},{"outgoingPackets",double(l.outputs)},
         {"readOnly",true},{"fabricationEnabled",false}};
}

// TESTE_19.2: verificação Qt da estrutura visual, sem hardware e sem alterar v5.
static QJsonObject workspaceLayoutTest(const QString& csv,const QString& output){
 LabWindow w(readCsv(csv.toStdString()));w.show();QApplication::processEvents();
 const auto before=fingerprint(w.view.editor.state());
 const int originalObjects=int(w.view.editor.state().doc.contours.size());
 WorkspaceConfig cfg=w.workspaceCfg_;
 cfg.drawingWidth=1200;cfg.drawingHeight=900;
 cfg.machineWidth=800;cfg.machineHeight=400;cfg.showGrid=true;cfg.showMachineFrame=true;
 cfg.validate();w.view.setWorkspace(cfg,true);
 w.workspaceCfg_=cfg;w.refreshWorkspaceLabels();
 w.view.fitMachineBed();QApplication::processEvents();
 check(w.view.workspace().drawingWidth==1200,"workspace drawing width not applied");
 check(w.view.workspace().machineHeight==400,"workspace physical height not applied");
 check(w.view.sceneRect().contains(QPointF(1200,900)),"workspace scene rectangle too small");
 check(fingerprint(w.view.editor.state())==before,"workspace settings modified canonical geometry");
 check(int(w.view.editor.state().doc.contours.size())==originalObjects,"workspace changed objects");
 for(const QString id:{"select","nodes","bezier","line","rectangle","ellipse","polygon",
      "open","open_other","save","save_as","undo","redo","duplicate_obj","rotate_90","mirror_horiz"})
  check(w.buttons.count(id)>0,"editor button ID missing after layout");
 const auto futureButtons=w.findChildren<QPushButton*>();int disabled=0;
 for(auto* b:futureButtons)if(!b->isEnabled())++disabled;
 check(disabled>=15,"missing blocked future functions");
 check(w.machineProfileSelector_&&w.machineProfileSelector_->count()>=6,"machine profile visual list incomplete");
 check(w.workspaceSummary_&&w.workspaceSummary_->text().contains("1200"),"area indicator not updated");
 QDir().mkpath(output);check(w.grab().save(output+"/LAYOUT_TESTE_19_2_WINDOWS.png"),"workspace screenshot failed");
 return {{"status","PASS"},{"area_desenho_mm","1200x900"},{"mesa_visual_mm","800x400"},
  {"objects_unchanged",true},{"native_io_changed",false},{"disabled_future_buttons",disabled},
  {"fabricationEnabled",false}};
}

// TESTE_19.4: eventos reais de mouse, histórico e coleta opcional, sem máquina.
static QJsonObject shapeOperationsTest(const QString& directory){
 QDir().mkpath(directory);
 LabWindow w(Document{});w.show();QApplication::processEvents();
 auto& v=w.view;v.setTransform(QTransform::fromScale(8.,8.));
 int a=v.editor.rectangle({100,100},20,15),b=v.editor.line({150,100},{180,100});
 v.rebuild();v.centerOn(QPointF(110,107));QApplication::processEvents();
 // Diagnóstico fica desligado até o usuário ativá-lo.
 check(w.captureDocument()["events_retained"].toInt()==0,"collector started automatically");
 check(w.captureDocument()["version"].toString()=="TESTE_19.7","diagnostic schema version");
 check(w.findChild<QLineEdit*>("wifi_ip_196")!=nullptr,"manual wifi IP missing");
 check(w.findChild<QSpinBox*>("wifi_port_196")!=nullptr,"manual TCP port missing");
 check(w.findChild<QMenuBar*>("webInspiredMenu")!=nullptr,"Web-style native menu missing");
 check(w.findChild<QPushButton*>("layer_swatch_0")!=nullptr,"layer palette missing");
 check(alaser::zoom196::clampScale(4000)==4000,"precision zoom missing");
 check(w.findChild<QPushButton*>("machine_connect_195")!=nullptr,"manual COM button missing");
 check(w.findChild<QPushButton*>("rotate_exact_195")!=nullptr,"angle rotation button missing");
 check(w.findChild<QPushButton*>("scale_exact_195")!=nullptr,"scaling button missing");
 check(w.findChild<QPushButton*>("align_left_195")!=nullptr,"align button missing");
 check(w.captureDocument()["environment"].toObject()["machine_connected"].toBool()==false,"must start transport disconnected");
 auto* enable=w.findChild<QCheckBox*>("diagnostics_enable");check(enable!=nullptr,"diagnostics toggle missing");
 enable->setChecked(true);
 QPoint from=v.mapFromScene(QPointF(100,108));
 QPoint to=v.mapFromScene(QPointF(110,113));
 const auto old=stateText(v.editor.state());
 const auto before=v.editor.undoCount();
 QTest::mousePress(v.viewport(),Qt::LeftButton,Qt::NoModifier,from);
 QTest::mouseMove(v.viewport(),to);QApplication::processEvents();
 check(stateText(v.editor.state())==old&&v.editor.undoCount()==before,
  "drag preview touched document or history");
 QTest::mouseRelease(v.viewport(),Qt::LeftButton,Qt::NoModifier,to);
 QApplication::processEvents();
 check(v.editor.undoCount()==before+1,"drag created wrong history count");
 check(std::abs(get(v.editor.state().doc,{a,0})->nodes[0].p.x-110)<1e-6,"object move wrong x");
 auto moved=stateText(v.editor.state());
 pressKey(w,Qt::Key_Z,Qt::ControlModifier);
 check(stateText(v.editor.state())==old,"undo drag mismatch");
 pressKey(w,Qt::Key_Y,Qt::ControlModifier);
 check(stateText(v.editor.state())==moved,"redo drag mismatch");
 // Excluir um objeto inteiro sem apagar seus nós isoladamente.
 v.activate({a,0});const auto beforeDelete=v.editor.undoCount();
 pressKey(w,Qt::Key_Delete);
 check(v.editor.state().objects.count(a)==0&&v.editor.state().objects.count(b)==1,
  "delete did not remove complete selected object");
 check(v.editor.undoCount()==beforeDelete+1,"delete history not atomic");
 pressKey(w,Qt::Key_Z,Qt::ControlModifier);
 check(v.editor.state().objects.count(a)==1,"undo delete did not restore object");
 auto counts=w.captureDocument()["counts"].toObject();
 check(counts["shapes_moved"].toInt()==1&&counts["shapes_deleted"].toInt()==1,
  "collector did not capture actions");
 enable->setChecked(false);const int total=w.captureDocument()["events_retained"].toInt();
 w.captureEvent("shapes_moved");check(w.captureDocument()["events_retained"].toInt()==total,
  "collector did not stop");
 return {{"status","PASS"},{"drag_preview_safe",true},{"single_history_move",true},
  {"delete_and_undo",true},{"opt_in_collection",true},{"stopped_collection",true},
  {"events",total}};
}
// TESTE_19.7: teclado, corte via clique e envio para camada no Qt real.
static QJsonObject editTools197Test(const QString& directory){
 QDir().mkpath(directory);
 LabWindow w(Document{});w.show();QApplication::processEvents();
 auto& v=w.view;v.setTransform(QTransform::fromScale(8.,8.));
 int rectangle=v.editor.rectangle({100,100},20,20);
 int line=v.editor.line({150,110},{190,110});
 int crossing=v.editor.line({90,110},{130,110});v.rebuild();
 v.centerOn(QPointF(135,110));QApplication::processEvents();
 clickButton(w,"nodes");check(v.tool==QtEditorLab::Tool::Nodes,"nodes tool not activated");
 pressKey(w,Qt::Key_Escape);check(v.tool==QtEditorLab::Tool::Select,"Esc failed to restore selection");
 clickButton(w,"line");pressKey(w,Qt::Key_Escape);
 check(v.tool==QtEditorLab::Tool::Select,"Esc failed from drawing mode");
 pressKey(w,Qt::Key_K);check(v.tool==QtEditorLab::Tool::Scissors,"K scissor shortcut missing");
 check(v.viewport()->cursor().shape()==Qt::ArrowCursor,"tesoura fora do objeto deveria ser seta");
 const QPoint hoverPoint=v.mapFromScene({110,100});
 check(v.viewport()->hasMouseTracking(),"viewport sem mouse tracking");
 const QPoint clearPoint=v.mapFromScene({138,135});
 bool usedViewportEventFallback=false;
 // QTest::mouseMove pode nao gerar hover SEM botao no ambiente de testes
 // Windows/Qt. A segunda via envia um QMouseEvent real ao QWidget viewport,
 // atravessando QAbstractScrollArea/viewportEvent -> QtEditorLab::mouseMoveEvent.
 // Nao chama updateScissorHover diretamente, nem altera documento ou preview.
 auto moveHover=[&](QPoint at){
  const uint64_t before=v.scissorHoverEventCount();
  QTest::mouseMove(v.viewport(),at);QApplication::processEvents();
  // QTest pode entregar um evento atrasado com coordenadas anteriores.
  // Receber ALGUM evento nao demonstra que o cursor saiu do contorno.
  if(v.scissorHoverEventCount()!=before&&v.scissorHoverReceivedAt(at))return;
  usedViewportEventFallback=true;
  const QPointF local(at),global(v.viewport()->mapToGlobal(at));
  QMouseEvent mouse(QEvent::MouseMove,local,global,
                    Qt::NoButton,Qt::NoButton,Qt::NoModifier);
  QApplication::sendEvent(v.viewport(),&mouse);
  QApplication::processEvents();
  if(v.scissorHoverEventCount()==before||!v.scissorHoverReceivedAt(at))
   throw std::runtime_error("MouseMove nao chegou na coordenada esperada pela viewport: "+v.scissorHoverDiagnostics(at));
 };
 moveHover(clearPoint);
 moveHover(hoverPoint);
 auto preview=v.scissorPreview();
 if(!preview||preview->preview.size()<4){
  w.grab().save(directory+"/FALHA_HOVER_TESOURA.png");
  throw std::runtime_error("hover nao iluminou arco completo: "+v.scissorHoverDiagnostics(hoverPoint));
 }
 check(v.viewport()->cursor().shape()!=Qt::ArrowCursor,"cursor nao mudou sobre trecho valido");
 moveHover(clearPoint);
 if(v.scissorPreview()||v.viewport()->cursor().shape()!=Qt::ArrowCursor){
  w.grab().save(directory+"/FALHA_SAIDA_TESOURA.png");
  throw std::runtime_error("Tesoura permaneceu ativa fora do trecho: "+
                           v.scissorHoverDiagnostics(clearPoint));
 }
 const size_t old=v.editor.undoCount();
 clickScene(w,{110,100});
 check(v.editor.undoCount()==old+1,"scissor mouse click did not create history");
 check(!get(v.editor.state().doc,{rectangle,0})->closed,"scissor failed to open closed contour");
 check(get(v.editor.state().doc,{rectangle,0})->nodes.size()==4,"Tesoura removeu arco incompleto ou deixou arestas");
 check(get(v.editor.state().doc,{crossing,0})&&get(v.editor.state().doc,{crossing,0})->nodes.size()==2,
       "Tesoura alterou contorno cruzador");
 checkRendered(w);
 pressKey(w,Qt::Key_Z,Qt::ControlModifier);
 check(get(v.editor.state().doc,{rectangle,0})->closed,"undo cut failed");
 pressKey(w,Qt::Key_V);check(v.tool==QtEditorLab::Tool::Select,"V selection shortcut missing");
 v.editor.addLayer(1,"Camada destino");w.refresh();
 v.activate({line,0});w.layers->setCurrentIndex(w.layers->findData(1));
 pressKey(w,Qt::Key_L,Qt::ControlModifier);
 check(v.editor.state().objects.at(line).layer==1,"Ctrl+L failed to move object to layer");
 pressKey(w,Qt::Key_Z,Qt::ControlModifier);
 check(v.editor.state().objects.at(line).layer==0,"undo layer transfer failed");
 const auto before=stateText(v.editor.state());
 v.editor.setLayerFlags(1,true,true);w.refresh();
 v.activate({line,0});w.layers->setCurrentIndex(w.layers->findData(1));
 pressKey(w,Qt::Key_L,Qt::ControlModifier);
 check(v.editor.state().objects.at(line).layer==0,"locked layer transfer accepted");
 // Regressao visual 19.8: Ctrl+R, Ctrl+E e Shift preservam proporcoes.
 w.layers->setCurrentIndex(w.layers->findData(0));
 auto shiftedShape=[&](Qt::Key shortcut,QtEditorLab::Tool expected,QPointF first,QPointF second){
  pressKey(w,shortcut,Qt::ControlModifier);
  check(v.tool==expected,"Ctrl+R/E nao ativou ferramenta");
  const auto p1=v.mapFromScene(first),p2=v.mapFromScene(second);
  QTest::mousePress(v.viewport(),Qt::LeftButton,Qt::ShiftModifier,p1);
  QTest::mouseMove(v.viewport(),p2);QApplication::processEvents();
  QTest::mouseRelease(v.viewport(),Qt::LeftButton,Qt::ShiftModifier,p2);
  QApplication::processEvents();
  const int id=v.lastCreated();
  check(id>0&&v.editor.state().objects.count(id),"forma proporcional nao criada");
  auto [lo,hi]=boundsOf(v.editor.state().doc,{id});
  check(std::abs((hi.x-lo.x)-(hi.y-lo.y))<1e-6,"Shift nao preservou dimensoes iguais");
 };
 shiftedShape(Qt::Key_R,QtEditorLab::Tool::Rectangle,{180,112},{201,123});
 shiftedShape(Qt::Key_E,QtEditorLab::Tool::Ellipse,{180,143},{208,155});
 auto* layer6=w.findChild<QPushButton*>("layer_swatch_6");
 check(layer6!=nullptr,"paleta de camadas incompleta");
 layer6->click();
 check(w.layers->findData(6)>=0&&w.layers->currentData().toInt()==6,"camada nao criada por clique");
 w.layers->setCurrentIndex(w.layers->findData(0));
 // Tesoura igual ao Trim Shapes: formas novas isoladas ficam 100% laranja,
 // e desaparecem inteiras ao clicar, sem exigir cruzamento.
 LabWindow shapes(Document{});shapes.show();QApplication::processEvents();
 auto& sv=shapes.view;
 sv.setTransform(QTransform::fromScale(8.,8.));
 const int circleId=sv.editor.ellipse({120,100},10,10,64);
 const int rectId=sv.editor.rectangle({145,90},18,18);
 sv.rebuild();sv.centerOn(QPointF(135,100));QApplication::processEvents();
 clickButton(shapes,"scissors");
 // Evita aceitar eventos Qt atrasados do hover anterior.
 // Confirma a coordenada solicitada e usa a viewport para entregar o evento
 // quando QTest::mouseMove nao a atualizou. Nao chama o planner diretamente.
 auto pointHover=[&](QPointF p){
  const QPoint at=sv.mapFromScene(p);
  const uint64_t count=sv.scissorHoverEventCount();
  QTest::mouseMove(sv.viewport(),at);QApplication::processEvents();
  if(!sv.scissorHoverReceivedAt(at)||sv.scissorHoverEventCount()==count){
   const QPointF local(at),global(sv.viewport()->mapToGlobal(at));
   QMouseEvent move(QEvent::MouseMove,local,global,
                    Qt::NoButton,Qt::NoButton,Qt::NoModifier);
   QApplication::sendEvent(sv.viewport(),&move);QApplication::processEvents();
  }
  if(sv.scissorHoverEventCount()==count||!sv.scissorHoverReceivedAt(at))
   throw std::runtime_error("MouseMove nao chegou ao contorno isolado: "+
                            sv.scissorHoverDiagnostics(at));
 };
 pointHover({130,100});
 auto circleSpan=sv.scissorPreview();
 check(bool(circleSpan)&&circleSpan->key.object==circleId&&circleSpan->preview.size()>=64,
       "Tesoura nao destacou circulo novo completo");
 const int beforeIsolated=int(sv.editor.state().doc.contours.size());
 clickScene(shapes,{130,100});
 check(sv.editor.state().doc.contours.size()==size_t(beforeIsolated-1)&&
       sv.editor.state().objects.count(circleId)==0,"Circulo isolado nao excluido");
 pointHover({154,90});
 auto rectSpan=sv.scissorPreview();
 if(!rectSpan||rectSpan->key.object!=rectId||rectSpan->preview.size()!=5){
  shapes.grab().save(directory+"/FALHA_RETANGULO_TESOURA.png");
  throw std::runtime_error("Tesoura nao destacou retangulo novo completo: "+
                           sv.scissorHoverDiagnostics(sv.mapFromScene(QPointF(154,90))));
 }
 clickScene(shapes,{154,90});
 check(sv.editor.state().doc.contours.empty()&&sv.editor.state().objects.count(rectId)==0,
       "Retangulo isolado nao excluido");
 w.grab().save(directory+"/TESTE_19_7_FERRAMENTAS.png");
 return {{"status","PASS"},{"selection_esc",true},{"selection_v",true},
   {"scissors_k",true},{"scissors_mouse",true},{"scissors_continuous_span_preview",true},{"scissors_cursor_conditional",true},{"scissors_isolated_circle",true},{"scissors_isolated_rectangle",true},{"undo_scissors",true},
   {"hover_delivery",usedViewportEventFallback?"viewport_synthetic_mouse_move":"qtest_mouse_move"},
   {"hardware_hover_verified",false},
   {"ctrl_r_rectangle",true},{"ctrl_e_ellipse",true},{"shift_square_circle",true},{"create_layer_by_palette",true},
   {"move_layer_ctrl_l",true},{"layer_lock_rejected",true},
   {"document_before_lock_attempt",QString::number(before.size())},
   {"hardware_commands_sent",false}};
}
// Integrated Qt test: v5 recovery snapshots never overwrite the user's file.
static QJsonObject recovery198Test(const QString& directory){
 QStandardPaths::setTestModeEnabled(true);
 // Em cada rodada, arquivos originais sintéticos vivem num diretorio exclusivo.
 // Nunca apagar resultados anteriores nem reaproveitar um arquivo ja salvo.
 check(QDir().mkpath(directory),"cannot prepare test report directory");
 QTemporaryDir isolated(QDir(directory).filePath("recovery_case_XXXXXX"));
 check(isolated.isValid(),"cannot create isolated recovery test directory");
 alaser::PersistentSessionStore::discard();
 LabWindow w(Document{});
 w.uiPersistenceEnabled_=true; // test explicitly drives recovery without timer/UI prompts
 const int id=w.view.editor.rectangle({20,30},50,40);
 w.view.rebuild();w.recoveryPolicy_.edited();
 check(w.recoveryPolicy_.needsSnapshot(),"recovery did not observe edit");
 const QString original=isolated.filePath("original_untouched.alaser");
 check(!QFile::exists(original),"isolated test target not empty before snapshot");
 w.projectPath=original;
 w.snapshotForRecovery();
 check(alaser::PersistentSessionStore::hasSnapshot(),"no recovery copy");
 check(!QFile::exists(original),"autosave overwrote original path");
 const auto snapshot=openNative(alaser::PersistentSessionStore::snapshotPath());
 const auto restored=editorStateForIntegratedProject(snapshot);
 check(restored.objects.count(id)==1,"recovery lost object");
 check(!w.recoveryPolicy_.needsSnapshot()&&w.recoveryPolicy_.dirty(),"snapshot incorrectly marked saved");
 w.save();
 check(QFile::exists(original),"explicit save did not create project");
 check(!alaser::PersistentSessionStore::hasSnapshot(),"recovery not cleared after successful save");
 check(!w.recoveryPolicy_.dirty(),"explicit save kept dirty state");
 w.uiPersistenceEnabled_=false;
 return {{"status","PASS"},{"snapshot_v5",true},{"original_untouched_until_save",true},
         {"successful_save_discarded_snapshot",true},{"no_hardware_access",true}};
}
// TESTE_19.8 importacao: XMLs sinteticos representam formas garantidas.
// Testes sem abrir dialogos, sem salvar ou conectar a maquina.
static QJsonObject newDrawingTools198Test(const QString& directory){
 // O mesmo vetorizador do botão Texto e exercitado sem dialogos.
 const auto letters=vectorTextOutlines("LASER",QFont("Sans Serif",12),12.,{10,20});
 check(!letters.empty(),"Fonte instalada nao gerou letra vetorial");
 size_t letterNodes=0;for(const auto& contour:letters)letterNodes+=contour.size();
 check(letterNodes>=12&&letterNodes<50000,"Texto vetorial inesperado");
 Editor glyphs(Document{});
 const auto allGlyphs=glyphs.addContours(letters,true,0);
 check(allGlyphs.size()==letters.size()&&glyphs.undoCount()==1,
       "Texto real nao entrou numa unica transacao");
 check(glyphs.undo()&&glyphs.state().doc.contours.empty(),"Undo de fonte real falhou");
 Editor editor(Document{});
 const auto start=editor.undoCount();
 const std::vector<std::vector<Pt>> outlines={
  {{1,1},{4,1},{4,5},{1,5}},
  {{8,1},{12,1},{12,5},{8,5}}
 };
 const auto ids=editor.addContours(outlines,true,0);
 check(ids.size()==2&&editor.undoCount()==start+1,"texto nao gerou contornos atomicos");
 const Document textDoc=editor.state().doc;
 check(editor.undo()&&editor.state().doc.contours.empty(),"desfazer texto nao retirou todos os glifos");
 check(editor.redo()&&sameDoc(textDoc,editor.state().doc),"refazer texto corrompeu contornos");
 LabWindow w(Document{});w.show();QApplication::processEvents();
 clickButton(w,"pencil");
 check(w.view.tool==QtEditorLab::Tool::Pencil,"botao Lapis nao foi ativado");
 auto& v=w.view;
 const QPoint a=v.mapFromScene(QPointF(20,20));
 const QPoint b=v.mapFromScene(QPointF(30,25));
 const QPoint c=v.mapFromScene(QPointF(40,30));
 QTest::mousePress(v.viewport(),Qt::LeftButton,Qt::NoModifier,a);
 QApplication::processEvents();
 auto moved=[&](const QPoint& pt){
  const QPointF local(pt),global(v.viewport()->mapToGlobal(pt));
  QMouseEvent ev(QEvent::MouseMove,local,global,Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
  QApplication::sendEvent(v.viewport(),&ev);
  QApplication::processEvents();
 };
 moved(b);moved(c);
 QTest::mouseRelease(v.viewport(),Qt::LeftButton,Qt::NoModifier,c);
 QApplication::processEvents();
 check(v.editor.state().doc.contours.size()==1,"Lapis nao gerou forma editavel");
 check(v.editor.state().doc.contours.front().nodes.size()>=2,
       "Lapis perdeu pontos do tracado");
 check(!v.editor.state().doc.contours.front().closed,"Lapis gerou forma fechada por engano");
 check(v.editor.undoCount()==1,"Lapis gerou multiplos passos de undo");
 check(v.editor.undo()&&v.editor.state().doc.contours.empty(),
       "Undo do Lapis nao voltou ao documento vazio");
 clickButton(w,"text");
 check(v.tool==QtEditorLab::Tool::Text,"botao Texto nao foi ativado");
 w.grab().save(directory+"/TESTE_19_8_TEXTO_LAPIS.png");
 return {{"status","PASS"},{"pencil_drawn",true},{"pencil_undo",true},
         {"vector_text_atomic",true},{"vector_text_undo_redo",true},
         {"text_tool_enabled",true},{"hardware_commands_sent",false}};
}

// Qt GUI: texto direto na mesa, proporcao, escala livre e rotacao.
// Qt 19.8: camadas persistentes, modo visual, grupos multiplos e Ctrl+U.
// 19.8: topologia real do arquivo TESTE TESOURA.alaser (sem copiar sua arte).
// O arquivo do cliente tem 571 anéis com ultimo no repetido e closed=false.
static QJsonObject scissorClosures198Test(const QString& directory){
 QDir().mkpath(directory);
 Document d;d.nextId=6;
 d.contours.push_back({{1,0},
  {{1,{0,0}},{2,{20,0}},{3,{20,20}},{4,{0,20}},{5,{0,0}}},false});
 check(hasExactDuplicateClosure(d.contours.front()),"Fecho duplicado nao identificado");
 const auto original=serialize(encode(d));
 ScissorSpanPlanner planner;planner.reset(d);
 auto candidate=planner.candidate(d,Hit{{1,0},0,0,{10,0}});
 check(candidate.has_value()&&candidate->closed,"Tesoura ignorou anel virtual");
 check(serialize(encode(d))==original,"Hover da Tesoura alterou arquivo");
 Editor isolated(fromDocument(d));
 isolated.removeScissorSpan(*candidate);
 check(isolated.state().doc.contours.empty(),"Tesoura nao removeu anel isolado");
 check(isolated.undo(),"Undo do corte falhou");
 check(serialize(encode(isolated.state().doc))==original,"Undo nao restaurou fecho original");
 Document crosses=d;
 crosses.contours.push_back({{2,0},{{6,{5,-10}},{7,{5,30}}},false});
 crosses.contours.push_back({{3,0},{{8,{15,-10}},{9,{15,30}}},false});
 crosses.nextId=10;
 ScissorSpanPlanner crossPlanner;crossPlanner.reset(crosses);
 auto crossSpan=crossPlanner.candidate(crosses,Hit{{1,0},0,0,{10,0}});
 check(crossSpan.has_value()&&crossSpan->closed,
  "Tesoura nao encontrou segmento entre cruzamentos com fecho duplicado");
 const auto crossingBefore=serialize(encode(crosses));
 Editor crossed(fromDocument(crosses));
 const auto crossUndo=crossed.undoCount();
 crossed.removeScissorSpan(*crossSpan);
 check(crossed.undoCount()==crossUndo+1,"Corte nao registrou undo unico");
 check(crossed.state().doc.contours.size()==3,"Corte alterou outras formas");
 const auto* trimmed=get(crossed.state().doc,{1,0});
 check(trimmed&&(!trimmed->closed),"Corte nao abriu restante do anel");
 check(sameDoc(Document{{crosses.contours[1],crosses.contours[2]},10},
                   Document{{crossed.state().doc.contours[1],crossed.state().doc.contours[2]},10}),
       "Corte alterou outros objetos");
 check(crossed.undo()&&serialize(encode(crossed.state().doc))==crossingBefore,
       "Undo nao restaurou contornos cruzados");
 // Alca central move o conjunto com uma unica acao.
 LabWindow w(Document{});w.show();QApplication::processEvents();
 auto& v=w.view;v.setTransform(QTransform::fromScale(5.,5.));
 v.centerOn(QPointF(40,40));QApplication::processEvents();
 const int id=v.editor.rectangle({30,30},20,20);v.rebuild();
 clickButton(w,"select");v.activate({id,0});
 const auto start=v.mapFromScene({40,40}),end=v.mapFromScene({50,40});
 const auto nUndo=v.editor.undoCount();
 QTest::mousePress(v.viewport(),Qt::LeftButton,Qt::NoModifier,start);
 QMouseEvent move(QEvent::MouseMove,QPointF(end),
   QPointF(v.viewport()->mapToGlobal(end)),Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
 QApplication::sendEvent(v.viewport(),&move);
 QTest::mouseRelease(v.viewport(),Qt::LeftButton,Qt::NoModifier,end);
 QApplication::processEvents();
 check(v.editor.undoCount()==nUndo+1,"Alca central nao gerou um unico undo");
 const auto* moved=get(v.editor.state().doc,{id,0});
 check(moved&&std::abs(moved->nodes.front().p.x-40.)<.5,
       "Alca central nao moveu o objeto em 10 mm");
 check(v.editor.undo(),"Alca central nao pode ser desfeita");
 check(std::abs(get(v.editor.state().doc,{id,0})->nodes.front().p.x-30.)<.1,
       "Undo da alca central falhou");
 // Paleta: clique simples move selecao para camada.
 v.activate({id,0});
 auto* swatch=w.findChild<QPushButton*>("layer_swatch_3");
 check(swatch,"Swatch nao encontrado");
 QTest::mouseClick(swatch,Qt::LeftButton);QApplication::processEvents();
 check(v.editor.state().objects.at(id).layer==3,
       "Clique simples na paleta nao moveu a selecao para camada 3");
 // XML LightBurn com raio em Rect e elipse.
 const QString path=directory+"/LBRN2_RAIO_19_8.lbrn2";
 QFile xml(path);check(xml.open(QIODevice::WriteOnly),"Nao criou fixture LBRN2");
 const QByteArray fixture=R"XML(<?xml version="1.0" encoding="utf-8"?>
<LightBurnProject>
<Shape Type="Rect" W="20" H="10" Cr="3" CutIndex="2">
 <XForm>1 0 0 1 25 30</XForm>
</Shape>
<Shape Type="Ellipse" W="16" H="16" CutIndex="3">
 <XForm>1 0 0 1 50 50</XForm>
</Shape>
</LightBurnProject>)XML";
 xml.write(fixture);xml.close();
 auto parsed=lightburn::readGeometry(path);
 check(parsed.shapeCount==2&&parsed.state.doc.contours.size()==2,
       "Importador rejeitou retangulo arredondado e elipse");
 check(parsed.state.doc.contours.front().nodes.size()>70,
       "Cr foi ignorado e retangulo virou cantos retos");
 check(parsed.state.doc.contours[1].nodes.size()==384,
       "Elipse LBRN2 nao manteve suavizacao");
 w.grab().save(directory+"/ALCA_CENTRAL_19_8.png");
 return {{"status","PASS"},{"client_topology_reproduced",true},
   {"virtual_closed_isolated",true},{"virtual_closed_crossings",true},
   {"original_and_undo_preserved",true},{"center_move",true},
   {"palette_simple_click",true},{"rounded_rect_lbrn2",true},
   {"laser_commands_sent",false}};
}
static QJsonObject layersUngroup198Test(const QString& directory){
 QDir().mkpath(directory);
 LabWindow w(Document{});w.show();QApplication::processEvents();
 auto& v=w.view;
 const int a=v.editor.rectangle({20,20},15,15);
 const int b=v.editor.rectangle({40,20},15,15);
 const int c=v.editor.rectangle({70,20},15,15);
 const int d=v.editor.rectangle({90,20},15,15);
 v.editor.addLayer(2,"Gravacao");
 w.refresh();w.layers->setCurrentIndex(w.layers->findData(2));
 v.selected.insert(a);v.selected.insert(b);v.applySelectionStyles();
 clickButton(w,"move_layer");
 check(v.editor.state().objects.at(a).layer==2&&
       v.editor.state().objects.at(b).layer==2,"Botao enviar selecao nao moveu camadas");
 const auto geometryBefore=serialize(encode(v.editor.state().doc));
 check(w.layerMode_!=nullptr,"Seletor de modo de camada nao foi criado");
 w.layerMode_->setCurrentIndex(w.layerMode_->findData(QString("Fill")));
 QApplication::processEvents();
 v.rebuild();
 check(v.editor.state().layers.at(2).mode=="Fill","Modo Fill nao persistiu no Editor");
 check(v.pathItem({a,0})->brush().style()==Qt::NoBrush,
  "Editor deve mostrar contornos no padrao LightBurn");
 v.setFillPreview(true);
 check(v.pathItem({a,0})->brush().style()!=Qt::NoBrush,"Previa opcional nao preencheu forma fechada");
 check(v.pathItem({c,0})->brush().style()==Qt::NoBrush,"Modo Linha indevidamente preenchido");
 const int ga=v.editor.group({a,b});
 const int gb=v.editor.group({c,d});
 check(ga!=gb,"Os dois grupos receberam mesmo ID");
 const auto beforeUngroup=v.editor.undoCount();
 const int changed=v.editor.ungroupSelection({a,c});
 check(changed==2&&v.editor.undoCount()==beforeUngroup+1,
  "Desagrupar selecao deveria consumir uma operacao unica");
 for(const int id:{a,b,c,d})
  check(v.editor.state().objects.at(id).group==0,"Membro ficou agrupado");
 check(serialize(encode(v.editor.state().doc))==geometryBefore,
       "Desagrupar alterou geometria do desenho");
 check(v.editor.state().objects.at(a).layer==2&&
       v.editor.state().objects.at(c).layer==0,"Desagrupar alterou camadas");
 v.rebuild();
 check(v.editor.undo(),"Nao foi possivel desfazer desagrupamento");
 check(v.editor.state().objects.at(a).group==ga&&
       v.editor.state().objects.at(c).group==gb,"Desfazer nao restaurou agrupamento");
 check(v.editor.redo(),"Nao foi possivel refazer desagrupamento");
 check(v.editor.state().objects.at(a).group==0,"Refazer nao separou objetos");
 // Gesto real da interface: Ctrl+U deve desagrupar novamente pela selecao.
 const int g3=v.editor.group({a,b});
 v.rebuild();v.clearSelection();v.selected.insert(a);
 clickButton(w,"select");
 const auto undoBeforeShortcut=v.editor.undoCount();
 pressKey(w,Qt::Key_U,Qt::ControlModifier);
 check(v.editor.state().objects.at(a).group==0&&
       v.editor.state().objects.at(b).group==0,
       "Ctrl+U nao desagrupou o objeto selecionado");
 check(v.editor.undoCount()==undoBeforeShortcut+1,
       "Ctrl+U nao foi atomico");
 (void)g3;
 auto actual=metadata(v.editor.state());
 for(auto& layer:actual.at("layers").as_array()){
  auto& row=layer.as_object();row.erase("mode");
 }
 const auto legacy=fromMetadata(v.editor.state().doc,actual);
 check(legacy.layers.at(2).mode=="Line","Metadata antiga sem modo deixou de abrir");
 QTemporaryDir temp(QDir(directory).filePath("camadas_XXXXXX"));
 check(temp.isValid(),"Nao foi possivel criar diretorio de teste");
 const QString file=temp.filePath("roundtrip.alaser");
 (void)saveNative(file,v.editor.state());
 const auto restored=editorStateForIntegratedProject(openNative(file));
 check(restored.layers.at(2).mode=="Fill","Modo nao sobreviveu ao salvar/reabrir .alaser");
 check(restored.objects.at(a).layer==2,"Transferencia de camada se perdeu no roundtrip");
 w.grab().save(directory+"/CAMADAS_19_8.png");
 return {{"status","PASS"},{"ungroup_two_groups_atomic",true},
  {"ungroup_undo_redo",true},{"layer_transfer",true},
  {"fill_preview_closed",true},{"legacy_mode_default",true},
  {"alaser_roundtrip",true},{"hardware_commands_sent",false}};
}
static QJsonObject inlineTransforms198Test(const QString& directory){
 QDir().mkpath(directory);
 LabWindow textWindow(Document{});textWindow.show();QApplication::processEvents();
 auto& tv=textWindow.view;tv.setTransform(QTransform::fromScale(4.,4.));
 tv.centerOn(QPointF(80,65));QApplication::processEvents();
 clickButton(textWindow,"text");clickScene(textWindow,{80,65});
 QLineEdit* empty=textWindow.inlineTextEntry_;
 check(empty&&empty->isVisible(),"Campo de texto vazio nao foi criado");
 const auto beforeEmpty=stateText(tv.editor.state());
 clickButton(textWindow,"select");QApplication::processEvents();
 check(textWindow.inlineTextEntry_==nullptr,"Trocar ferramenta nao cancelou campo vazio");
 check(stateText(tv.editor.state())==beforeEmpty,"Campo vazio alterou o desenho");
 clickButton(textWindow,"text");clickScene(textWindow,{80,65});
 QLineEdit* entry=textWindow.inlineTextEntry_;
 check(entry&&entry->isVisible(),"Texto nao apareceu dentro da mesa");
 check(entry->parentWidget()==tv.viewport(),"Texto nao pertence a viewport");
 check(entry->styleSheet().contains("background:transparent"),"Texto nao e transparente");
 QTest::keyClicks(entry,"Teste");QApplication::processEvents();
 check(tv.editor.state().doc.contours.empty(),"Texto em digitacao alterou o documento");
 QTest::keyClick(entry,Qt::Key_Return);QApplication::processEvents();
 check(textWindow.inlineTextEntry_==nullptr,"Campo de texto ficou ativo apos concluir");
 check(!tv.editor.state().doc.contours.empty(),"Texto nao criou contornos vetoriais");
 check(tv.editor.undoCount()==1,"Texto nao criou transacao unica");
 pressKey(textWindow,Qt::Key_Z,Qt::ControlModifier);
 check(tv.editor.state().doc.contours.empty(),"Undo do texto falhou");
 LabWindow transformWindow(Document{});transformWindow.show();QApplication::processEvents();
 auto& v=transformWindow.view;v.setTransform(QTransform::fromScale(5.,5.));
 v.centerOn(QPointF(60,50));QApplication::processEvents();
 const int id=v.editor.rectangle({40,40},20,10);v.rebuild();
 clickButton(transformWindow,"select");clickScene(transformWindow,{50,40});
 check(v.selected.count(id)>0,"Selecao do retangulo nao funcionou");
 const auto before=v.editor.undoCount();
 auto drag=[&](QPoint press,QPoint target,Qt::KeyboardModifiers mods=Qt::NoModifier){
  QTest::mousePress(v.viewport(),Qt::LeftButton,mods,press);
  const QPointF local(target),global(v.viewport()->mapToGlobal(target));
  QMouseEvent moved(QEvent::MouseMove,local,global,Qt::NoButton,Qt::LeftButton,mods);
  QApplication::sendEvent(v.viewport(),&moved);QApplication::processEvents();
  QTest::mouseRelease(v.viewport(),Qt::LeftButton,mods,target);
  QApplication::processEvents();
 };
 drag(v.mapFromScene({60,40}),v.mapFromScene({80,30}));
 check(v.editor.undoCount()==before+1,"Escala nao criou um passo de undo");
 const Contour* scaled=get(v.editor.state().doc,{id,0});
 check(scaled!=nullptr,"Escala perdeu objeto");
 double minX=1e20,minY=1e20,maxX=-1e20,maxY=-1e20;
 for(const auto& n:scaled->nodes){
  minX=std::min(minX,n.p.x);maxX=std::max(maxX,n.p.x);
  minY=std::min(minY,n.p.y);maxY=std::max(maxY,n.p.y);
 }
 check(std::abs((maxX-minX)-40.)<1.,"Escala proporcional largura incorreta");
 check(std::abs((maxY-minY)-20.)<1.,"Escala proporcional altura incorreta");
 const auto stateScaled=stateText(v.editor.state());
 const QPointF center=v.mapFromScene({60,40});
 const QPointF rotateStart=v.mapFromScene({80,30})+QPointF(17,-17);
 const QPointF d=rotateStart-center;
 const QPointF rotateEnd=center+QPointF(-d.y(),d.x());
 drag(rotateStart.toPoint(),rotateEnd.toPoint());
 check(v.editor.undoCount()==before+2,"Rotacao nao criou passo de undo");
 check(stateText(v.editor.state())!=stateScaled,"Rotacao nao alterou vertices");
 pressKey(transformWindow,Qt::Key_Z,Qt::ControlModifier);
 check(stateText(v.editor.state())==stateScaled,"Undo da rotacao nao restaurou desenho");
 // Shift permite escala nao proporcional; Ctrl fixa o centro.
 const int freeId=v.editor.rectangle({110,45},20,10);
 v.clearSelection();v.rebuild();clickScene(transformWindow,{115,45});
 check(v.selected.count(freeId)>0,"Retangulo de escala livre nao selecionado");
 drag(v.mapFromScene({130,45}),v.mapFromScene({150,43}),Qt::ShiftModifier);
 auto boundsFree=[&]()->QRectF{
  const auto* c=get(v.editor.state().doc,{freeId,0});
  check(c!=nullptr,"Retangulo transformado nao encontrado");
  double x0=1e20,y0=1e20,x1=-1e20,y1=-1e20;
  for(const auto& n:c->nodes){x0=std::min(x0,n.p.x);x1=std::max(x1,n.p.x);
   y0=std::min(y0,n.p.y);y1=std::max(y1,n.p.y);}
  return {QPointF(x0,y0),QPointF(x1,y1)};
 };
 const QRectF freeBounds=boundsFree();
 check(freeBounds.width()>35.&&freeBounds.height()<16.,
       "Shift nao liberou proporcao entre largura e altura");
 // Teste independente de escala com ancora central.
 v.clearSelection();v.activate({freeId,0});
 const QRectF centerBefore=boundsFree();
 const QPointF corner=centerBefore.topRight();
 const QPointF target=corner+QPointF(10.,-6.);
 drag(v.mapFromScene(corner),v.mapFromScene(target),Qt::ControlModifier|Qt::ShiftModifier);
 const QRectF centerAfter=boundsFree();
 check(QLineF(centerBefore.center(),centerAfter.center()).length()<0.8,
       "Ctrl nao preservou o centro ao redimensionar");
 transformWindow.grab().save(directory+"/TRANSFORMACOES_EDITOR_19_8.png");
 textWindow.grab().save(directory+"/TEXTO_DIRETO_19_8.png");
 return {{"status","PASS"},{"inline_text",true},{"transparent_overlay",true},
  {"text_atomic_undo",true},{"scale_proportional",true},
  {"rotation_handle",true},{"undo_rotate",true},{"shift_free_scale",true},
  {"ctrl_center_scale",true},{"hardware_commands_sent",false}};
}
static QJsonObject editorMultitool198Test(const QString& directory){
 // Gesto multi-nos, fechamento e linha por arraste legada.
 LabWindow w(Document{});w.show();QApplication::processEvents();
 auto& v=w.view;
 v.setTransform(QTransform::fromScale(4.,4.));v.centerOn(QPointF(80,55));
 QApplication::processEvents();
 clickButton(w,"line");
 clickScene(w,{20,20});clickScene(w,{40,20});clickScene(w,{40,40});
 check(v.isPolylineActive()&&v.polylineVertexCount()==3,
       "Linha continua nao manteve os tres nos no mesmo gesto");
 clickScene(w,{20,20});
 check(!v.isPolylineActive(),"Fechamento na origem nao finalizou a linha");
 const auto* closed=get(v.editor.state().doc,{v.lastCreated(),0});
 check(closed&&closed->closed&&closed->nodes.size()==3,
       "Fechamento da linha nao gerou um contorno unico");
 check(v.editor.undoCount()==1,"Linha fechada criou multiplas acoes de undo");
 pressKey(w,Qt::Key_Z,Qt::ControlModifier);
 check(v.editor.state().doc.contours.empty(),"Undo da linha fechada nao funcionou");
 pressKey(w,Qt::Key_Y,Qt::ControlModifier);
 check(v.editor.state().doc.contours.size()==1,"Redo da linha fechada nao funcionou");
 clickScene(w,{48,20});clickScene(w,{60,23});clickScene(w,{67,31});
 QTest::mouseClick(v.viewport(),Qt::RightButton,Qt::NoModifier,v.mapFromScene(QPointF(67,31)));
 QApplication::processEvents();
 check(!v.isPolylineActive(),"Botao direito nao encerrou a polilinha");
 const auto* opened=get(v.editor.state().doc,{v.lastCreated(),0});
 check(opened&&!opened->closed&&opened->nodes.size()==3,
       "Botao direito deveria salvar um caminho aberto com tres nos");
 const QPoint a=v.mapFromScene(QPointF(75,67)),b=v.mapFromScene(QPointF(87,79));
 QTest::mousePress(v.viewport(),Qt::LeftButton,Qt::NoModifier,a);
 QTest::mouseMove(v.viewport(),b);
 QTest::mouseRelease(v.viewport(),Qt::LeftButton,Qt::NoModifier,b);
 QApplication::processEvents();
 const auto* legacy=get(v.editor.state().doc,{v.lastCreated(),0});
 check(legacy&&!legacy->closed&&legacy->nodes.size()==2,
       "Gesto antigo de arrastar uma linha deixou de funcionar");
 clickScene(w,{80,25});check(v.isPolylineActive(),"Linha nao iniciou para cancelar");
 const auto beforeEsc=stateText(v.editor.state());
 const auto undoBeforeEsc=v.editor.undoCount();
 pressKey(w,Qt::Key_Escape);
 check(!v.isPolylineActive(),"Esc nao cancelou a linha");
 check(stateText(v.editor.state())==beforeEsc,
       "Esc salvou ou alterou contorno ainda incompleto");
 check(v.editor.undoCount()==undoBeforeEsc,
       "Esc criou passo indevido no historico");
 check(v.tool==QtEditorLab::Tool::Select,
       "Esc nao restaurou ferramenta Selecao");
 // Caixa: direita contem; esquerda cruza, em cima de geometria real.
 LabWindow selection(Document{});selection.show();QApplication::processEvents();
 auto& sv=selection.view;
 sv.setTransform(QTransform::fromScale(4.,4.));sv.centerOn(QPointF(100,65));
 const int first=sv.editor.rectangle({50,50},20,20);
 const int second=sv.editor.rectangle({100,50},20,20);
 const int third=sv.editor.rectangle({150,50},20,20);
 sv.rebuild();QApplication::processEvents();
 auto dragBox=[&](QPointF from,QPointF to,Qt::KeyboardModifiers mod=Qt::NoModifier){
  const QPoint x=sv.mapFromScene(from),y=sv.mapFromScene(to);
  QTest::mousePress(sv.viewport(),Qt::LeftButton,mod,x);
  QApplication::processEvents();
  check(sv.isMarqueeActive(),"A caixa de selecao nao iniciou no espaco vazio");
  QTest::mouseMove(sv.viewport(),y);QApplication::processEvents();
  QTest::mouseRelease(sv.viewport(),Qt::LeftButton,mod,y);QApplication::processEvents();
  check(!sv.isMarqueeActive(),"A caixa de selecao ficou ativa apos soltar");
 };
 dragBox({45,45},{75,75});
 check(sv.selected.size()==1&&sv.selected.count(first),
       "Caixa esquerda-direita nao selecionou somente forma inteiramente contida");
 dragBox({124,54},{116,66});
 check(sv.selected.size()==1&&sv.selected.count(second),
       "Caixa direita-esquerda nao selecionou contorno cruzado");
 dragBox({45,45},{75,75},Qt::ShiftModifier);
 check(sv.selected.size()==2&&sv.selected.count(first)&&sv.selected.count(second),
       "Shift nao acrescentou selecao por caixa");
 pressKey(selection,Qt::Key_A,Qt::ControlModifier);
 check(sv.selected.size()==3&&sv.selected.count(third),"Ctrl+A nao selecionou as formas");
 pressKey(selection,Qt::Key_I,Qt::ControlModifier|Qt::ShiftModifier);
 check(sv.selected.empty(),"Ctrl+Shift+I nao inverteu a selecao");
 selection.grab().save(directory+"/EDITOR_MULTIFERRAMENTAS_19_8.png");
 return {{"status","PASS"},{"line_continuous",true},
         {"line_closed_and_open",true},{"legacy_drag",true},
         {"marquee_enclosing_and_crossing",true},
         {"multi_selection_modifiers",true},{"hardware_commands_sent",false}};
}
static QJsonObject lightburnImport198Test(const QString& directory){
 QTemporaryDir temp(QDir(directory).filePath("lbrn2_XXXXXX"));
 check(temp.isValid(),"nao criou pasta temporaria de importacao");
 const QString filename=temp.filePath("formas.lbrn2");
 const QByteArray xml=R"XML(<?xml version="1.0" encoding="UTF-8"?>
<LightBurnProject AppVersion="1.7" FormatVersion="1" MirrorX="False" MirrorY="False">
 <CutSetting type="Cut"><index Value="1"/><speed Value="1000"/><maxPower Value="100"/></CutSetting>
 <Shape Type="Rect" CutIndex="1" W="20" H="10" Cr="0">
  <XForm>1 0 0 1 50 70</XForm>
 </Shape>
 <Shape Type="Group" CutIndex="0">
  <XForm>1 0 0 1 100 200</XForm>
  <Children>
   <Shape Type="Rect" CutIndex="2" W="10" H="10" Cr="0"><XForm>1 0 0 1 5 7</XForm></Shape>
  </Children>
 </Shape>
 <Shape Type="Ellipse" CutIndex="3" W="30" H="20"><XForm>1 0 0 1 0 0</XForm></Shape>
 <Shape Type="Path" CutIndex="4">
  <VertList>V0 0c0x1c1x1V10 0c0x1c1x1V10 10c0x1c1x1</VertList>
  <PrimList>L0 1L1 2</PrimList><XForm>1 0 0 1 8 9</XForm>
 </Shape>
 <Shape Type="Path" CutIndex="5">
  <VertList>V0 0c0x5c0y5c1x1V10 0c0x1c1x5c1y5</VertList>
  <PrimList>B0 1</PrimList><XForm>1 0 0 1 0 0</XForm>
 </Shape>
</LightBurnProject>)XML";
 QFile f(filename);check(f.open(QIODevice::WriteOnly),"falhou fixture LightBurn");
 check(f.write(xml)==xml.size(),"fixture LightBurn truncada");f.close();
 auto imported=lightburn::readGeometry(filename);
 check(imported.shapeCount==5&&imported.state.doc.contours.size()==5,"5 formas nao foram reconhecidas");
 check(imported.state.layers.size()==6,"camadas do XML nao foram mapeadas");
 check(imported.state.objects.at(1).layer==1&&imported.state.objects.at(2).layer==2,
       "indices de camada foram perdidos");
 const auto& first=imported.state.doc.contours.at(0);
 check(first.nodes.size()==4&&std::abs(first.nodes[0].p.x-40.)<1e-8&&
       std::abs(first.nodes[0].p.y-65.)<1e-8,"retangulo e XForm incorretos");
 const auto& nested=imported.state.doc.contours.at(1);
 check(std::abs(nested.nodes[0].p.x-100.)<1e-8&&
       std::abs(nested.nodes[0].p.y-202.)<1e-8,"transformacao de grupo incorreta");
 const auto& ellipse=imported.state.doc.contours.at(2);
 check(ellipse.nodes.size()==384,"elipse nao foi convertida para 384 pontos");
 const auto& path=imported.state.doc.contours.at(3);
 check(!path.closed&&path.nodes.size()==3&&std::abs(path.nodes[0].p.x-8.)<1e-8,
       "Path com segmentos retos ou XForm incorreto");
 const auto& cubic=imported.state.doc.contours.at(4);
 check(!cubic.closed&&cubic.nodes.size()==25&&
       std::abs(cubic.nodes[12].p.y-3.75)<1e-8,
       "Cubic Bezier importada com interpolacao errada");
 validate(imported.state);

 // Projetos mistos sao RECUSADOS, nunca importados parcialmente.
 const QString invalid=temp.filePath("incompativel.lbrn2");
 QFile bad(invalid);check(bad.open(QIODevice::WriteOnly),"fixture incompleta nao criada");
 bad.write(R"XML(<?xml version="1.0"?><LightBurnProject>
 <Shape Type="Rect" W="10" H="10"/>
 <Shape Type="Path" CutIndex="0"><VertList>V0 0</VertList></Shape>
 </LightBurnProject>)XML");bad.close();
 bool rejected=false;
 try{(void)lightburn::readGeometry(invalid);}catch(const std::runtime_error&){rejected=true;}
 check(rejected,"importador aceitou perda silenciosa de Path");


 // Casos reais LightBurn 2.1: LineClosed, LineOpen e reuso de PrimID/VertID.
 const QString sharedFile=temp.filePath("compartilhadas.lbrn2");
 QFile shared(sharedFile);
 check(shared.open(QIODevice::WriteOnly),"fixture compartilhada nao criada");
 const QByteArray sharedXml=R"XML(<?xml version="1.0"?>
 <LightBurnProject MirrorX="False" MirrorY="False">
  <Shape Type="Path" CutIndex="0" PrimID="5" VertID="21">
   <VertList>V0 0c0x1c1x1V10 0c0x1c1x1V10 10c0x1c1x1V0 10c0x1c1x1</VertList>
   <PrimList>LineClosed</PrimList>
  </Shape>
  <Shape Type="Group" CutIndex="0"><Children>
   <Shape Type="Path" CutIndex="1" PrimID="5" VertID="22">
    <VertList>V20 0c0x1c1x1V30 0c0x1c1x1V30 10c0x1c1x1V20 10c0x1c1x1</VertList>
   </Shape>
   <Shape Type="Path" CutIndex="2" PrimID="9" VertID="23">
    <VertList>V40 0c0x1c1x1V50 0c0x1c1x1</VertList>
    <PrimList>LineOpen</PrimList>
   </Shape>
   <Shape Type="Path" CutIndex="3" PrimID="9" VertID="23"/>
  </Children></Shape>
 </LightBurnProject>)XML";
 shared.write(sharedXml);shared.close();
 const auto sharedResult=lightburn::readGeometry(sharedFile);
 check(sharedResult.shapeCount==4&&sharedResult.state.doc.contours.size()==4,
   "LineClosed/LineOpen ou PrimID compartilhado falhou");
 check(sharedResult.state.doc.contours[0].closed&&
       sharedResult.state.doc.contours[1].closed&&
       !sharedResult.state.doc.contours[2].closed,
   "Listas compactas alteraram topologia");
 check(sharedResult.state.doc.contours[3].nodes.size()==2,
   "VertID e PrimID compartilhados nao foram resolvidos");
 check(sharedResult.state.objects.at(2).layer==1&&
       sharedResult.state.objects.at(3).layer==2,
   "Grupo aninhado perdeu camadas");
 check(sharedResult.state.objects.at(2).group>0&&
       sharedResult.state.objects.at(2).group==sharedResult.state.objects.at(4).group,
   "Group nao sobreviveu como metadado editavel");
 // Referencia sem definicao nunca e importada parcialmente.
 const QString invalidShared=temp.filePath("prim_desconhecido.lbrn2");
 QFile missing(invalidShared);check(missing.open(QIODevice::WriteOnly),
   "fixture PrimID ausente nao criada");
 missing.write(R"XML(<?xml version="1.0"?>
 <LightBurnProject><Shape Type="Path" PrimID="77">
  <VertList>V0 0V10 0V10 10</VertList>
 </Shape></LightBurnProject>)XML");
 missing.close();
 bool noDefinitionRejected=false;
 try{(void)lightburn::readGeometry(invalidShared);}
 catch(const std::exception&){noDefinitionRejected=true;}
 check(noDefinitionRejected,"PrimID desconhecido foi aceito sem geometria");
 LabWindow w(Document{});w.show();QApplication::processEvents();
 QMimeData mime;mime.setUrls({QUrl::fromLocalFile(filename)});
 QDragEnterEvent drag(QPoint(20,20),Qt::CopyAction,&mime,Qt::LeftButton,Qt::NoModifier);
 QApplication::sendEvent(&w,&drag);
 check(drag.isAccepted(),"janela nao aceita arrastar .lbrn2");
 return {{"status","PASS"},{"rect_transform",true},{"group_transform",true},
         {"ellipse_384_segments",true},{"path_lines",true},{"path_bezier",true},{"layers",true},{"unsupported_path_rejected",true},
         {"drag_enter_lbrn2",true},{"lineclosed_shared_primid",true},{"shared_verts",true},{"unknown_primid_rejected",true},{"hardware_commands_sent",false}};
}


static QJsonObject actualLightBurnFile198Test(const QString& input){
 QFile source(input);
 check(source.open(QIODevice::ReadOnly),"Arquivo LBRN2 real nao acessivel");
 const QByteArray bytes=source.readAll();source.close();
 const QString sha=QString::fromLatin1(QCryptographicHash::hash(
  bytes,QCryptographicHash::Sha256).toHex());
 const auto imported=lightburn::readGeometry(input);
 validate(imported.state);
 const auto rerun=lightburn::readGeometry(input);
 check(serialize(encode(imported.state.doc))==serialize(encode(rerun.state.doc)),
  "Importacao real nao deterministica");
 int closed=0;
 for(const auto& c:imported.state.doc.contours)if(c.closed)closed++;
 const int nodes=int(countNodes(imported.state.doc));
 std::set<int> groups;int groupedObjects=0;
 for(const auto& [id,meta]:imported.state.objects)if(meta.group>0){
  groups.insert(meta.group);groupedObjects++;
 }
 const bool known=sha=="3336269e9bae2aa5cb2778123ff7ffea2ea1a8a515d2aca648fb44e22ebbad06";
 if(known){
  check(imported.shapeCount==17&&nodes==1331&&closed==13,
   "SUPORTE PRA ALEXA: importacao incompleta ou geometria diferente do arquivo real");
  check(imported.state.doc.contours.size()==17,
   "SUPORTE PRA ALEXA: 17 formas editaveis nao preservadas");
  check(groups.size()==3&&groupedObjects==6,
   "SUPORTE PRA ALEXA: grupos LightBurn nao foram preservados");
 }
 // Nao grava no arquivo origem; nenhum envio ao laser.
 return {{"status","PASS"},{"source_sha256",sha},
  {"identified_uploaded_fixture",known},{"shape_count",imported.shapeCount},
  {"node_count",nodes},{"closed_paths",closed},
  {"layers",int(imported.state.layers.size())},
  {"groups",int(groups.size())},{"grouped_objects",groupedObjects},
  {"deterministic",true},{"hardware_commands_sent",false}};
}

int main(int argc,char** argv){
 QApplication app(argc,argv);QString mode=argc>1?argv[1]:"--interactive",csv=argc>2?argv[2]:"fixtures/web_nodes_normalized.csv",out=argc>3?argv[3]:"results/TESTE_6";if(mode=="--recovery-19-8-test"){
  QDir().mkpath(out);QJsonObject report;bool pass=true;
  try{report=recovery198Test(out);}catch(const std::exception& e){pass=false;report={{"status","FAIL"},{"error",e.what()}};}
  QFile file(out+"/RESULTADO_RECUPERACAO_19_8.json");if(!file.open(QIODevice::WriteOnly))return 2;
  file.write(QJsonDocument(report).toJson());return pass?0:1;
 }if(mode=="--edit-tools-19-7-test"){
  QDir().mkpath(out);QJsonObject report;bool pass=true;
  try{report=editTools197Test(out);}catch(const std::exception& e){pass=false;report={{"status","FAIL"},{"error",e.what()}};}
  QFile file(out+"/RESULTADO_TESTE_19_7_GUI.json");if(!file.open(QIODevice::WriteOnly))return 2;
  file.write(QJsonDocument(report).toJson());return pass?0:1;
 }if(mode=="--lbrn2-file-19-8-test"){
  QDir().mkpath(out);QJsonObject report;bool pass=true;
  try{report=actualLightBurnFile198Test(csv);}
  catch(const std::exception& e){pass=false;report={{"status","FAIL"},{"error",e.what()}};}
  QFile f(out+"/RESULTADO_LBRN2_REAL_19_8.json");
  if(!f.open(QIODevice::WriteOnly))return 2;
  f.write(QJsonDocument(report).toJson());return pass?0:1;
 }if(mode=="--scissor-closures-19-8-test"){
  QDir().mkpath(out);QJsonObject report;bool pass=true;
  try{report=scissorClosures198Test(out);}
  catch(const std::exception& e){pass=false;report={{"status","FAIL"},{"error",e.what()}};}
  QFile f(out+"/RESULTADO_CASO_REAL_19_8.json");if(!f.open(QIODevice::WriteOnly))return 2;
  f.write(QJsonDocument(report).toJson());return pass?0:1;
 }if(mode=="--layers-ungroup-19-8-test"){
  QDir().mkpath(out);QJsonObject report;bool pass=true;
  try{report=layersUngroup198Test(out);}
  catch(const std::exception& e){pass=false;report={{"status","FAIL"},{"error",e.what()}};}
  QFile f(out+"/RESULTADO_CAMADAS_19_8.json");if(!f.open(QIODevice::WriteOnly))return 2;
  f.write(QJsonDocument(report).toJson());return pass?0:1;
 }if(mode=="--inline-transforms-19-8-test"){
  QDir().mkpath(out);QJsonObject report;bool pass=true;
  try{report=inlineTransforms198Test(out);}
  catch(const std::exception& e){pass=false;report={{"status","FAIL"},{"error",e.what()}};}
  QFile f(out+"/RESULTADO_TEXTO_TRANSFORMACOES_19_8.json");
  if(!f.open(QIODevice::WriteOnly))return 2;
  f.write(QJsonDocument(report).toJson());return pass?0:1;
 }if(mode=="--editor-multitool-19-8-test"){
  QDir().mkpath(out);QJsonObject report;bool pass=true;
  try{report=editorMultitool198Test(out);}
  catch(const std::exception& e){pass=false;report={{"status","FAIL"},{"error",e.what()}};}
  QFile f(out+"/RESULTADO_EDITOR_MULTIFERRAMENTAS_19_8.json");
  if(!f.open(QIODevice::WriteOnly))return 2;
  f.write(QJsonDocument(report).toJson());return pass?0:1;
 }if(mode=="--drawing-tools-19-8-test"){
  QDir().mkpath(out);QJsonObject report;bool pass=true;
  try{report=newDrawingTools198Test(out);}
  catch(const std::exception& e){pass=false;report={{"status","FAIL"},{"error",e.what()}};}
  QFile f(out+"/RESULTADO_TEXTO_LAPIS_19_8.json");if(!f.open(QIODevice::WriteOnly))return 2;
  f.write(QJsonDocument(report).toJson());return pass?0:1;
 }if(mode=="--lbrn2-import-test"){
  QDir().mkpath(out);QJsonObject report;bool pass=true;
  try{report=lightburnImport198Test(out);}
  catch(const std::exception& e){pass=false;report={{"status","FAIL"},{"error",e.what()}};}
  QFile f(out+"/RESULTADO_IMPORTACAO_LBRN2.json");if(!f.open(QIODevice::WriteOnly))return 2;
  f.write(QJsonDocument(report).toJson());return pass?0:1;
 }if(mode=="--shape-operations-test"){
  QDir().mkpath(out);QJsonObject report;bool pass=true;
  try{report=shapeOperationsTest(out);}catch(const std::exception& e){pass=false;report={{"status","FAIL"},{"error",e.what()}};}
  QFile file(out+"/RESULTADO_TESTE_19_4_GUI.json");if(!file.open(QIODevice::WriteOnly))return 2;
  file.write(QJsonDocument(report).toJson());return pass?0:1;
 }if(mode=="--reference-test"){
  const QString results=argc>4?argv[4]:"results/TESTE_19_3";
  QDir().mkpath(results);QJsonObject report;bool pass=true;
  try{report=referenceTest(csv,out);}catch(const std::exception& e){pass=false;report={{"status","FAIL"},{"error",e.what()}};}
  QFile file(results+"/RESULTADO_TESTE_19_3.json");if(!file.open(QIODevice::WriteOnly))return 2;
  file.write(QJsonDocument(report).toJson());return pass?0:1;
 }
 QDir().mkpath(out);
 if(mode=="--interactive"){
  LabWindow w(readCsv(csv.toStdString()));w.show();w.view.setTransform(QTransform::fromScale(6.,6.));w.view.reportZoom();w.view.centerOn(toQt(w.view.editor.state().doc.contours[0].nodes[15].p));
  QObject::connect(&app,&QCoreApplication::aboutToQuit,[&]{QFile f(out+"/SESSAO_INTERATIVA_TESTE_6.json");if(f.open(QIODevice::WriteOnly))f.write(QJsonDocument(telemetry(w)).toJson());});return app.exec();
 }
 QJsonObject report;bool success=true;
 try{if(mode=="--write")report=integratedTest(csv,out);else if(mode=="--reopen")report=reopenTest(out);else if(mode=="--navigation-test")report=navigationTest(csv,out);else if(mode=="--workspace-test")report=workspaceLayoutTest(csv,out);else throw std::runtime_error("unknown mode");}
 catch(const std::exception& e){success=false;report={{"status","FAIL"},{"error",e.what()}};}
 report["test"]=mode=="--workspace-test"?"TESTE_19_2":mode=="--navigation-test"?"TESTE_19_1":"TESTE_6";report["mode"]=mode;report["qt"]=qVersion();report["pid"]=QCoreApplication::applicationPid();
 QFile f(out+(mode=="--workspace-test"?"/RESULTADO_TESTE_19_2.json":mode=="--navigation-test"?"/RESULTADO_TESTE_19_1.json":mode=="--reopen"?"/REABERTURA_TESTE_6.json":"/RESULTADO_TESTE_6.json"));if(!f.open(QIODevice::WriteOnly))return 2;f.write(QJsonDocument(report).toJson());
 qInfo().noquote()<<mode<<report["status"].toString()<<report["error"].toString();return success?0:1;
}
