// TESTE_13: validar com Qt6 Widgets real a fidelidade de curvas nativas na viewport.
// Revalidado em Qt 6.8.3 Linux offscreen; ver results/work_qt14.
#include "bezier_native.hpp"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QPainterPath>
#include <QPen>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QElapsedTimer>
#include <QColor>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>
using namespace alaser;
static void check(bool b,const char* reason){if(!b)throw std::runtime_error(reason);}
static QPainterPath nativePath(const CurveContour& c){
 QPainterPath p;
 if(c.nodes.empty())return p;
 p.moveTo(c.nodes[0].anchor.x,c.nodes[0].anchor.y);
 for(size_t e=0;e<edgeCount(c);e++){
  Cubic v=edgeAt(c,e);
  if(c.edgeKind[e])p.cubicTo(v.p1.x,v.p1.y,v.p2.x,v.p2.y,v.p3.x,v.p3.y);
  else p.lineTo(v.p3.x,v.p3.y);
 }
 if(c.closed)p.closeSubpath();
 return p;
}
class CurveCanvas : public QGraphicsView {
 QGraphicsScene sceneOwned;
 QGraphicsPathItem *active=nullptr;
 QGraphicsEllipseItem *marker=nullptr;
public:
 CurveCanvas(const CurveContour& source,double zoom,QPointF center):QGraphicsView(){
  setScene(&sceneOwned);setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
  setRenderHints(QPainter::Antialiasing);setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setFixedSize(720,480);sceneOwned.setSceneRect(center.x()-500,center.y()-500,1000,1000);
  QPen pen(QColor(255,139,27));pen.setCosmetic(true);pen.setWidthF(1.4);
  pen.setCapStyle(Qt::RoundCap);pen.setJoinStyle(Qt::RoundJoin);
  active=sceneOwned.addPath(nativePath(source),pen);active->setZValue(10);
  QPen markerPen(Qt::white);markerPen.setCosmetic(true);markerPen.setWidthF(1/devicePixelRatioF());
  marker=sceneOwned.addEllipse(0,0,0,0,markerPen,QBrush(Qt::white));marker->setZValue(11);
  setTransform(QTransform::fromScale(zoom,zoom));centerOn(center);
  show();QApplication::processEvents();
 }
 void updateCurve(const CurveContour& c,size_t index){
  active->setPath(nativePath(c));auto p=c.nodes.at(index).anchor;
  // Marcador cosmético constante em pixels físicos independentemente de zoom.
  auto t=transform();double z=t.m11()*devicePixelRatioF();marker->setRect(p.x-3/z,p.y-3/z,6/z,6/z);
  viewport()->update();QApplication::processEvents();
 }
 QImage snap(){QApplication::processEvents();return viewport()->grab().toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);}
};
static size_t mismatches(const QImage&a,const QImage&b){
 check(a.size()==b.size(),"Qt viewport screenshot sizes differ");size_t m=0;
 for(int y=0;y<a.height();y++)for(int x=0;x<a.width();x++)m+=a.pixel(x,y)!=b.pixel(x,y);
 return m;
}
int main(int argc,char** argv){
 QApplication app(argc,argv);
 try{
  check(argc==3,"usage qt13 curves.csv output_dir");
  QDir().mkpath(argv[2]);CurveDocument d=loadLightBurnCurves(argv[1]);
  check(!d.contours.empty()&&d.contours[0].nodes.size()>3,"curve fixture insufficient");
  const auto& original=d.contours[0];
  const size_t node=2;const Pt old=original.nodes[node].anchor;
  size_t frames=0,failures=0,changes=0,maxDiff=0;
  double maxMs=0;
  for(double zoom:{.75,6.,80.,1200.}){
   QPointF center(old.x,old.y);
   CurveCanvas dynamic(original,zoom,center);dynamic.updateCurve(original,node);const QImage initial=dynamic.snap();
   for(int f=0;f<12;f++){
    auto moved=original;auto& n=moved.nodes[node];
    const Pt delta{(f+1)*9.0/(12*zoom),-(f+1)*6.0/(12*zoom)};
    n.anchor=n.anchor+delta;n.handleIn=n.handleIn+delta;n.handleOut=n.handleOut+delta;
    QElapsedTimer t;t.start();dynamic.updateCurve(moved,node);QImage actual=dynamic.snap();
    maxMs=std::max(maxMs,t.nsecsElapsed()/1e6);
    CurveCanvas reference(original,zoom,center);reference.updateCurve(moved,node);QImage expected=reference.snap();
    auto diff=mismatches(actual,expected);maxDiff=std::max(maxDiff,diff);failures+=(diff!=0);
    changes+=(mismatches(actual,initial)>0);frames++;
    if(f==11&&zoom==80.){
     initial.save(QString::fromLocal8Bit(argv[2])+"/BEZIER_QT_ANTES.png");
     actual.save(QString::fromLocal8Bit(argv[2])+"/BEZIER_QT_DURANTE.png");
    }
   }
   dynamic.updateCurve(original,node);
   check(mismatches(dynamic.snap(),initial)==0,"ghost pixels after cancel");
  }
  check(frames==48&&changes>0&&failures==0,"native cubic QGraphicsView mismatch");
  QJsonObject report{{"test",13},{"qt",qVersion()},{"frames",int(frames)},
   {"failed_frames",int(failures)},{"changed_frames",int(changes)},
   {"max_pixel_mismatches",int(maxDiff)},{"max_update_capture_ms",maxMs},
   {"machine_connected",false},{"windows_homologated",false}};
  QFile result(QString::fromLocal8Bit(argv[2])+"/QT6_TESTE13_RESULTADO.json");
  check(result.open(QIODevice::WriteOnly),"can't write result");
  result.write(QJsonDocument(report).toJson());result.close();
  std::cout<<"PASS Qt6 Bezier viewport frames="<<frames<<" mismatch=0\n";return 0;
 }catch(const std::exception&e){std::cerr<<"FAIL Qt6 Test13 "<<e.what()<<"\n";return 1;}
}
