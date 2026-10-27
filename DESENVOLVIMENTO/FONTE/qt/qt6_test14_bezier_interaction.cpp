// TESTE_14: teste REAL de mouse / viewport Qt 6.5+ preparado para execução.
// Abre curvas LightBurn, seleciona pelo traco, arrasta uma ancora, confere
// pixels contra cena reconstruida e desfaz. Não conecta a maquina laser.
#include "bezier_interaction.hpp"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QPainter>
#include <QPen>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QImage>
#include <QTest>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QElapsedTimer>
#include <QPointer>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <algorithm>
using namespace alaser;
static void expect(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
static QPainterPath toQt(const CurveContour& c){
 QPainterPath p;
 p.moveTo(c.nodes.front().anchor.x,c.nodes.front().anchor.y);
 for(size_t e=0;e<edgeCount(c);e++){
  const auto v=edgeAt(c,e);
  if(c.edgeKind[e])p.cubicTo(v.p1.x,v.p1.y,v.p2.x,v.p2.y,v.p3.x,v.p3.y);
  else p.lineTo(v.p3.x,v.p3.y);
 }
 if(c.closed)p.closeSubpath();
 return p;
}
static const CurveContour& requireContour(const CurveDocument& d,Key key){
 for(const auto& c:d.contours)if(c.key==key)return c;
 throw std::runtime_error("Qt selected curve not found");
}
class CurveEditor14 : public QGraphicsView {
 QGraphicsScene ownedScene;
 CurveDragSession session_;
 std::map<Key,QGraphicsPathItem*> shapes;
 std::vector<QGraphicsEllipseItem*> marks;
 double scale_;
 QPointF center_;
 bool pressed_ = false;
 void refresh(){
  for(const auto& c:session_.visible().contours){
   auto* item=shapes.at(c.key);
   item->setPath(toQt(c));
   QPen pen(session_.selected()&&*session_.selected()==c.key?QColor(255,144,30):QColor(75,82,92));
   pen.setCosmetic(true);pen.setWidthF(session_.selected()&&*session_.selected()==c.key?2:1.2);
   pen.setCapStyle(Qt::RoundCap);pen.setJoinStyle(Qt::RoundJoin);
   item->setPen(pen);item->setZValue(session_.selected()&&*session_.selected()==c.key?10:0);
  }
  for(auto* m:marks)delete m;
  marks.clear();
  if(session_.selected()){
   const auto& c=requireContour(session_.visible(),*session_.selected());
   for(const auto& n:c.nodes){
    QPen markerPen(QColor(20,20,20));
    markerPen.setCosmetic(true);markerPen.setWidthF(1/devicePixelRatioF());
    double radius=3/(scale_*devicePixelRatioF());
    auto* mark=ownedScene.addEllipse(n.anchor.x-radius,n.anchor.y-radius,2*radius,2*radius,
      markerPen,QBrush(Qt::white));
    mark->setZValue(50);marks.push_back(mark);
   }
  }
  viewport()->update();QApplication::processEvents();
 }
protected:
 void mousePressEvent(QMouseEvent* ev) override {
  if(ev->button()!=Qt::LeftButton){QGraphicsView::mousePressEvent(ev);return;}
  Pt scenePt{mapToScene(ev->pos()).x(),mapToScene(ev->pos()).y()};
  if(session_.selected()&&!(ev->modifiers()&Qt::AltModifier)){
   auto hit=pickCurveNode(session_.committed(),*session_.selected(),scenePt,scale_,devicePixelRatioF(),7);
   if(hit&&session_.start(*hit)){pressed_=true;refresh();ev->accept();return;}
  }
  auto hits=session_.pickEdges(scenePt,scale_,devicePixelRatioF(),6);
  if(!hits.empty()){
   // Alternancia deterministica Alt+clique, sem edicao no clique de troca.
   size_t i=0;
   if(ev->modifiers()&Qt::AltModifier){
    if(session_.selected())for(size_t j=0;j<hits.size();j++)
     if(hits[j].key==*session_.selected()){i=(j+1)%hits.size();break;}
   }
   session_.select(hits[i].key);
  }
  refresh();ev->accept();
 }
 void mouseMoveEvent(QMouseEvent* ev) override {
  if(pressed_&&session_.dragging()){
   QPointF p=mapToScene(ev->pos());session_.update({p.x(),p.y()});refresh();ev->accept();return;
  }
  QGraphicsView::mouseMoveEvent(ev);
 }
 void mouseReleaseEvent(QMouseEvent* ev) override {
  if(pressed_&&ev->button()==Qt::LeftButton){
   QPointF p=mapToScene(ev->pos());
   session_.update({p.x(),p.y()});session_.commit();pressed_=false;refresh();ev->accept();return;
  }
  QGraphicsView::mouseReleaseEvent(ev);
 }
 void keyPressEvent(QKeyEvent* ev) override {
  if(ev->key()==Qt::Key_Escape&&session_.dragging()){
   session_.cancel();pressed_=false;refresh();ev->accept();return;
  }
  if((ev->modifiers()&Qt::ControlModifier)&&(ev->key()==Qt::Key_Z||ev->key()==Qt::Key_Y)){
   // A history command must not leave a mouse gesture half-active.
   if(session_.dragging()){ev->accept();return;}
   if(ev->key()==Qt::Key_Y||(ev->modifiers()&Qt::ShiftModifier))session_.redo();
   else session_.undo();
   refresh();ev->accept();return;
  }
  QGraphicsView::keyPressEvent(ev);
 }
public:
 CurveEditor14(CurveDocument d,double zoom,QPointF center)
 : QGraphicsView(), session_(std::move(d)), scale_(zoom),center_(center) {
  setScene(&ownedScene);setFixedSize(760,520);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setRenderHint(QPainter::Antialiasing);
  setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
  ownedScene.setSceneRect(center.x()-10000,center.y()-10000,20000,20000);
  setBackgroundBrush(Qt::white);
  for(const auto& c:session_.committed().contours){
   auto* item=ownedScene.addPath(toQt(c));shapes[c.key]=item;
   item->setAcceptedMouseButtons(Qt::NoButton);
  }
  setTransform(QTransform::fromScale(scale_,scale_));centerOn(center_);show();QApplication::processEvents();refresh();
 }
 const CurveDocument& committed()const{return session_.committed();}
 const CurveDocument& visible()const{return session_.visible();}
 std::optional<Key> selectedKey()const{return session_.selected();}
 bool dragging()const{return session_.dragging();}
 size_t undoCount()const{return session_.undoCount();}
 QImage shot(){QApplication::processEvents();return viewport()->grab().toImage().convertToFormat(QImage::Format_ARGB32);}
 QPoint screen(Pt point){return mapFromScene(QPointF(point.x,point.y));}
 void chooseForReference(Key key){session_.select(key);refresh();}
};
static size_t differingPixels(const QImage& a,const QImage& b){
 expect(a.size()==b.size(),"viewport image dimensions mismatch");
 size_t count=0;for(int y=0;y<a.height();y++)for(int x=0;x<a.width();x++)count+=a.pixel(x,y)!=b.pixel(x,y);
 return count;
}
static bool bitsEq(Pt a,Pt b){return std::bit_cast<uint64_t>(a.x)==std::bit_cast<uint64_t>(b.x)&&std::bit_cast<uint64_t>(a.y)==std::bit_cast<uint64_t>(b.y);}
static bool identical(const CurveDocument& a,const CurveDocument& b){
 if(a.contours.size()!=b.contours.size()||a.nextId!=b.nextId)return false;
 for(size_t i=0;i<a.contours.size();i++){
  const auto&x=a.contours[i];const auto&y=b.contours[i];
  if(x.key!=y.key||x.closed!=y.closed||x.edgeKind!=y.edgeKind||x.nodes.size()!=y.nodes.size())return false;
  for(size_t j=0;j<x.nodes.size();j++){
   auto&m=x.nodes[j];auto&n=y.nodes[j];
   if(m.id!=n.id||!bitsEq(m.anchor,n.anchor)||!bitsEq(m.handleIn,n.handleIn)||!bitsEq(m.handleOut,n.handleOut))return false;
  }
 }return true;
}

int main(int argc,char**argv){
 QApplication app(argc,argv);
 try{
  expect(argc==3,"usage: alencar_test14_qt_interactive lightburn_cubics.csv results_directory");
  QString resultDir=QString::fromLocal8Bit(argv[2]);expect(QDir().mkpath(resultDir),"can't create output");
  CurveDocument all=loadLightBurnCurves(argv[1]);
  // Cenário isolado com UMA curva real, selecionada através de clique no traçado.
  const auto c=all.contours[0];expect(c.nodes.size()>=3,"fixture lacks editable nodes");
  CurveDocument single;single.contours.push_back(c);single.nextId=all.nextId;
  const size_t node=1;
  Pt anchor=c.nodes[node].anchor;
  int cases=0;size_t totalDiffering=0;
  for(double zoom:{6.,80.,1200.}){
   CurveEditor14 editor(single,zoom,QPointF(anchor.x,anchor.y));
   // Clicar na extremidade do contorno garante ponto visivel em qualquer zoom.
   // Ainda nao ha selecao, portanto o clique percorre o hit-test do traçado.
   QTest::mouseClick(editor.viewport(),Qt::LeftButton,Qt::NoModifier,editor.screen(anchor));
   expect(editor.selectedKey()&&*editor.selectedKey()==c.key,"real mouse click failed to select curve");
   QImage initial=editor.shot();
   QTest::mousePress(editor.viewport(),Qt::LeftButton,Qt::NoModifier,editor.screen(anchor));
   expect(editor.dragging(),"real press did not grab selected anchor");
   QPoint destination=editor.screen(anchor)+QPoint(18,-13);
   QTest::mouseMove(editor.viewport(),destination,20);
   expect(editor.dragging(),"drag ended before mouse released");
   expect(identical(editor.committed(),single),"preview mutated committed geometry");
   QImage preview=editor.shot();
   expect(differingPixels(initial,preview)>0,"native Bezier preview did not change pixels");
   CurveEditor14 reference(editor.visible(),zoom,QPointF(anchor.x,anchor.y));
   reference.chooseForReference(c.key);
   QImage expected=reference.shot();
   auto difference=differingPixels(preview,expected);
   totalDiffering+=difference;
   // Strict raster comparison can be platform-specific. Treat any mismatch as FAILURE;
   // save evidence for diagnosis, not a false green status.
   if(difference!=0){preview.save(resultDir+QString("/DIVERGENCIA_ZOOM_%1.png").arg(zoom));}
   expect(difference==0,"Qt incremental curve preview differs from freshly built scene");
   if(zoom==80.){
    initial.save(resultDir+"/QT14_ANTES.png");preview.save(resultDir+"/QT14_DURANTE.png");
   }
   QTest::mouseRelease(editor.viewport(),Qt::LeftButton,Qt::NoModifier,destination);
   expect(!editor.dragging()&&editor.undoCount()==1,"release failed to commit exactly one history action");
   QImage after=editor.shot();
   const auto committedAfter=editor.committed();
   expect(!identical(committedAfter,single),"release did not change geometry");
   if(zoom==80.)after.save(resultDir+"/QT14_DEPOIS.png");
   QTest::keyClick(&editor,Qt::Key_Z,Qt::ControlModifier);
   expect(editor.undoCount()==0,"Ctrl+Z did not undo Bezier drag");
   expect(differingPixels(initial,editor.shot())==0,"ghost pixels remain after undo");
   expect(identical(editor.committed(),single),"undo changed geometric identities");
   QTest::keyClick(&editor,Qt::Key_Y,Qt::ControlModifier);
   expect(identical(editor.committed(),committedAfter),"Ctrl+Y failed exact redo");
   QTest::keyClick(&editor,Qt::Key_Z,Qt::ControlModifier);
   QTest::keyClick(&editor,Qt::Key_Z,Qt::ControlModifier|Qt::ShiftModifier);
   expect(identical(editor.committed(),committedAfter),"Ctrl+Shift+Z failed exact redo");
   QTest::keyClick(&editor,Qt::Key_Z,Qt::ControlModifier);
   QTest::mousePress(editor.viewport(),Qt::LeftButton,Qt::NoModifier,editor.screen(anchor));
   QTest::mouseMove(editor.viewport(),destination,1);
   QTest::keyClick(&editor,Qt::Key_Z,Qt::ControlModifier);
   expect(editor.dragging()&&identical(editor.committed(),single),"history interrupted live gesture");
   QTest::keyClick(&editor,Qt::Key_Escape);
   QTest::mouseRelease(editor.viewport(),Qt::LeftButton,Qt::NoModifier,destination);
   expect(!editor.dragging()&&identical(editor.committed(),single)&&editor.undoCount()==0,"Escape failed exact cancel");
   expect(differingPixels(initial,editor.shot())==0,"cancel left ghost pixels");
   cases++;
  }
  // Coincident contours: Alt at an anchor must cycle, never start a drag.
  CurveDocument overlap=single;
  auto duplicate=c;duplicate.key={900001,2};
  for(auto& n:duplicate.nodes)n.id=overlap.nextId++;
  overlap.contours.push_back(duplicate);validateCurve(overlap);
  CurveEditor14 stacked(overlap,80.,QPointF(anchor.x,anchor.y));
  QTest::mouseClick(stacked.viewport(),Qt::LeftButton,Qt::NoModifier,stacked.screen(anchor));
  expect(stacked.selectedKey()==c.key,"first overlapping identity wrong");
  for(int cycle=0;cycle<10;cycle++){
   QTest::mouseClick(stacked.viewport(),Qt::LeftButton,Qt::AltModifier,stacked.screen(anchor));
   expect(stacked.selectedKey()==(cycle%2==0?duplicate.key:c.key),"Alt anchor click failed contour cycling");
   expect(!stacked.dragging()&&identical(stacked.committed(),overlap),"cycling mutated geometry");
  }
  QTest::mouseClick(stacked.viewport(),Qt::LeftButton,Qt::AltModifier,stacked.screen(anchor));
  QTest::mousePress(stacked.viewport(),Qt::LeftButton,Qt::NoModifier,stacked.screen(anchor));
  // Release coordinates are authoritative even without a preceding mouseMove.
  QPoint release=stacked.screen(anchor)+QPoint(20,-10);
  QTest::mouseRelease(stacked.viewport(),Qt::LeftButton,Qt::NoModifier,release);
  auto expectedOverlap=overlap;
  auto pos=stacked.mapToScene(release);
  CurveHit target;target.key=duplicate.key;target.nodeId=duplicate.nodes[node].id;target.part=CurvePart::Anchor;
  setCurvePart(expectedOverlap,target,{pos.x(),pos.y()});
  expect(identical(stacked.committed(),expectedOverlap),"drag edited wrong coincident contour or ignored release");
  QTest::keyClick(&stacked,Qt::Key_Z,Qt::ControlModifier);
  expect(identical(stacked.committed(),overlap),"overlap undo failed");
  QJsonObject result{{"test",14},{"qt_version",qVersion()},{"interactive_zoom_cases",cases},
    {"pixel_mismatches",int(totalDiffering)},{"overlap_cycles",10},{"exact_identity_checks",true},{"redo_shortcuts",true},{"escape_cancel",true},{"scale_factor",qEnvironmentVariable("QT_SCALE_FACTOR")},{"device_pixel_ratio",stacked.devicePixelRatioF()},{"machine_connected",false},{"windows_homologated",false}};
  QFile file(resultDir+"/RESULTADOS_QT14.json");expect(file.open(QIODevice::WriteOnly),"couldn't save Qt results");
  file.write(QJsonDocument(result).toJson());file.close();
  std::cout<<"PASS Qt Test14 mouse-selected Bezier; drag, undo and pixels validated\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL Qt Test14 "<<e.what()<<"\n";return 1;}
}
