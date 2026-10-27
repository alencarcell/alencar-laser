// TESTE_12: QGraphicsScene/QPainter real e comparacao contra cena reconstruida.
// Necessita SDK Qt6. Nao executa maquina. Qt offscreen nao substitui mouse/Windows.
#include "geometry.hpp"
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QPainter>
#include <QPainterPath>
#include <QImage>
#include <QPen>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QColor>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <algorithm>
using namespace alaser;
static void ensure(bool b,const char* reason){if(!b)throw std::runtime_error(reason);}
static QPainterPath pathFor(const Contour& c,int64_t moving=-1,Pt substitute={}){
 QPainterPath p;
 for(size_t i=0;i<c.nodes.size();++i){
  Pt pt=(c.nodes[i].id==moving?substitute:c.nodes[i].p);
  if(i==0)p.moveTo(pt.x,pt.y);else p.lineTo(pt.x,pt.y);
 }
 if(c.closed && c.nodes.size()>2)p.closeSubpath();
 return p;
}
static QPen stroke(const QColor& color){
 QPen pen(color);pen.setWidth(0);pen.setCosmetic(true);
 pen.setCapStyle(Qt::RoundCap);pen.setJoinStyle(Qt::RoundJoin);return pen;
}
static QImage capture(QGraphicsScene& scene,const QRectF& source){
 QImage image(600,400,QImage::Format_ARGB32_Premultiplied);image.fill(Qt::black);
 QPainter painter(&image);painter.setRenderHint(QPainter::Antialiasing,true);
 scene.render(&painter,QRectF(0,0,600,400),source,Qt::IgnoreAspectRatio);
 painter.end();return image;
}
static void addOthers(QGraphicsScene& scene,const Contour& c){
 auto* background=scene.addPath(pathFor(c),stroke(QColor(95,102,109)));
 background->setZValue(1);
 // Segundo caminho parcialmente coincidente: mede o destaque acima de outro objeto.
 QPainterPath overlap;
 for(size_t i=0;i<std::min<size_t>(10,c.nodes.size());++i){
  Pt pt=c.nodes[i].p;
  if(i==0)overlap.moveTo(pt.x,pt.y);else overlap.lineTo(pt.x,pt.y);
 }
 auto* foreground=scene.addPath(overlap,stroke(QColor(80,102,120)));
 foreground->setZValue(1);
}
static int differences(const QImage&a,const QImage&b){
 ensure(a.size()==b.size(),"image size mismatch");
 int pixels=0;
 for(int y=0;y<a.height();y++)for(int x=0;x<a.width();x++)if(a.pixel(x,y)!=b.pixel(x,y))pixels++;
 return pixels;
}
int main(int argc,char**argv){
 QApplication app(argc,argv);
 try{
  ensure(argc==3,"usage: qt12 fixture.csv outdir");
  QDir().mkpath(argv[2]);Document doc=readCsv(argv[1]);
  ensure(doc.contours.size()==575,"fixture contours mismatch");
  const Contour* largest=nullptr;
  for(const auto& c:doc.contours)if(!largest||c.nodes.size()>largest->nodes.size())largest=&c;
  ensure(largest && largest->nodes.size()>=16,"no dense contour found");
  const auto&c=*largest;int64_t nodeId=c.nodes[c.nodes.size()/2].id;
  Pt old=c.nodes[c.nodes.size()/2].p;
  QGraphicsScene incremental;addOthers(incremental,c);
  auto* active=incremental.addPath(pathFor(c),stroke(QColor(255,141,31)));
  active->setZValue(9);
  int total=0, pixelFailures=0, changes=0,maxPixelDifference=0;
  QElapsedTimer timer;double updateMs=0,renderMs=0;
  QImage last,original;
  for(double zoom:{.75,6.,80.,1200.})for(double dpr:{1.,2.}){
   // Viewport physico em pixels, com zoom e DPR aplicados ao retangulo fonte.
   double scale=zoom*dpr;
   QRectF source(old.x-300./scale,old.y-200./scale,600./scale,400./scale);
   active->setPath(pathFor(c));auto initial=capture(incremental,source);
   for(int frame=0;frame<12;frame++){
    Pt moved{old.x+double(frame+1)*25./(12*scale),old.y-double(frame+1)*13./(12*scale)};
    timer.start();active->setPath(pathFor(c,nodeId,moved));updateMs+=timer.nsecsElapsed()/1e6;
    timer.start();auto actual=capture(incremental,source);renderMs+=timer.nsecsElapsed()/1e6;
    QGraphicsScene reference;addOthers(reference,c);
    reference.addPath(pathFor(c,nodeId,moved),stroke(QColor(255,141,31)))->setZValue(9);
    auto expected=capture(reference,source);
    int mismatch=differences(actual,expected);
    pixelFailures+=(mismatch>0);maxPixelDifference=std::max(maxPixelDifference,mismatch);
    changes+=(differences(actual,initial)>0);
    total++;
    if(frame==11 && zoom==80. && dpr==2.){
     actual.save(QString::fromLocal8Bit(argv[2])+"/QT6_TESTE12_DURANTE.png");
     initial.save(QString::fromLocal8Bit(argv[2])+"/QT6_TESTE12_ANTES.png");
    }
   }
   active->setPath(pathFor(c));auto restored=capture(incremental,source);
   ensure(differences(restored,initial)==0,"cancel/restore leaves ghost pixels");
  }
  ensure(total==96,"unexpected case count");
  ensure(pixelFailures==0,"incremental Qt scene does not match full rebuilt scene");
  ensure(changes>0,"test did not exercise visible geometry updates");
  QJsonObject summary{
    {"test",12},{"qt",qVersion()},{"render_backend","QGraphicsScene::render QPainter offscreen"},
    {"frames",total},{"scene_mismatch_frames",pixelFailures},{"max_pixel_mismatches",maxPixelDifference},
    {"changed_frames",changes},{"update_path_ms_total",updateMs},{"render_ms_total",renderMs},
    {"cancel_ghosts",0},{"physical_mouse_tested",false},{"windows_homologated",false},
    {"machine_connected",false}};
  QFile result(QString::fromLocal8Bit(argv[2])+"/QT6_TESTE12_RESULTADO.json");
  ensure(result.open(QIODevice::WriteOnly),"can't write qt result");
  result.write(QJsonDocument(summary).toJson());result.close();
  std::cout<<"PASS Qt6 Teste12: "<<total<<" frames, 0 image mismatches, "<<changes<<" visible changes\n";
  return 0;
 }catch(const std::exception&e){std::cerr<<"FAIL Qt6 Teste12: "<<e.what()<<"\n";return 1;}
}
