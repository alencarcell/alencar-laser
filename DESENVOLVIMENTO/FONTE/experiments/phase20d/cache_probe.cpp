// ALENCAR LASER Phase20d: ISOLATED Qt raster-cache experiment only.
// No project files, laser transport, editing or production GUI.
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPainter>
#include <QImage>
#include <QPen>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

struct Result {
 double medianMs=0.;
 quint64 changedPixels=0;
 quint64 maxChannelDiff=0;
};

static QPainterPath makePath(int index, int nodes) {
 QPainterPath p;
 const double cx=12.+(index%25)*16.0, cy=12.+(index/25)*16.0;
 const double r=2.0+double(index%7)*0.15;
 for (int j=0;j<nodes;++j) {
  const double a=2.0*3.14159265358979323846*double(j)/double(nodes);
  const QPointF pt(cx+r*std::cos(a),cy+r*std::sin(a));
  if(j==0)p.moveTo(pt);else p.lineTo(pt);
 }
 p.closeSubpath();
 return p;
}

static void makeScene(QGraphicsScene& scene, int count, bool cached) {
 scene.setSceneRect(0,0,420,420);
 for(int i=0;i<count;++i) {
  // Fixed complexity distribution: 282/188/91/14 in the full fixture.
  const int nodes=(i<282)?6:(i<470)?20:(i<561)?64:160;
  auto* item=scene.addPath(makePath(i,nodes),QPen(QColor(20,20,20),0.35));
  item->setBrush(Qt::NoBrush);
  // Cache only the most complex 129+ node paths; no geometry rewrite.
  if(cached && nodes>=129) item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
 }
}
static QImage frame(QGraphicsScene& scene,double zoom) {
 constexpr int W=900,H=700;
 QImage img(W,H,QImage::Format_ARGB32_Premultiplied);
 img.fill(Qt::white);
 QPainter painter(&img);
 painter.setRenderHint(QPainter::Antialiasing,true);
 painter.setRenderHint(QPainter::TextAntialiasing,true);
 painter.scale(zoom,zoom);
 scene.render(&painter,QRectF(0,0,W/zoom,H/zoom),QRectF(0,0,420,420),
              Qt::IgnoreAspectRatio);
 painter.end();
 return img;
}
static Result run(QGraphicsScene& scene,double zoom,int repeats) {
 QElapsedTimer timer;
 std::vector<double> elapsed;
 elapsed.reserve(repeats);
 (void)frame(scene,zoom); // warm up both policies; excluded from timing
 for(int k=0;k<repeats;++k) {
  timer.start();
  auto img=frame(scene,zoom);
  elapsed.push_back(double(timer.nsecsElapsed())/1e6);
  if(img.isNull())qFatal("Image creation failed");
 }
 std::sort(elapsed.begin(),elapsed.end());
 Result out;out.medianMs=elapsed[elapsed.size()/2];
 return out;
}
static void compareImages(const QImage& a,const QImage& b,Result& out) {
 if(a.size()!=b.size() || a.format()!=b.format())qFatal("Image formats differ");
 for(int y=0;y<a.height();++y) {
  const auto* pa=reinterpret_cast<const QRgb*>(a.constScanLine(y));
  const auto* pb=reinterpret_cast<const QRgb*>(b.constScanLine(y));
  for(int x=0;x<a.width();++x) {
   const QRgb va=pa[x],vb=pb[x];
   const int delta=std::max({std::abs(qRed(va)-qRed(vb)),
                 std::abs(qGreen(va)-qGreen(vb)),
                 std::abs(qBlue(va)-qBlue(vb)),std::abs(qAlpha(va)-qAlpha(vb))});
   if(delta)++out.changedPixels;
   out.maxChannelDiff=std::max(out.maxChannelDiff,quint64(delta));
  }
 }
}
int main(int argc,char**argv) {
 QApplication app(argc,argv);
 QString outFile;
 for(int i=1;i+1<argc;++i)if(QString::fromLocal8Bit(argv[i])=="--out")
  outFile=QString::fromLocal8Bit(argv[++i]);
 if(outFile.isEmpty()) {
  std::fprintf(stderr,"Usage: alencar_phase20d_cache --out REPORT.json\n");
  return 2;
 }
 QJsonArray cases;
 bool integrity=true;
 for(int count:{40,128,575}) {
  for(double zoom:{1.0,1.2,2.0}) {
   QGraphicsScene baseline,experimental;
   makeScene(baseline,count,false);makeScene(experimental,count,true);
   const QImage baseImage=frame(baseline,zoom);
   const QImage cacheImage=frame(experimental,zoom);
   Result raw=run(baseline,zoom,12),cached=run(experimental,zoom,12);
   compareImages(baseImage,cacheImage,cached);
   // No subjective equivalence threshold: report EVERY changed pixel.
   if(baseImage.isNull()||cacheImage.isNull()||raw.medianMs<=0||
      cached.medianMs<=0)integrity=false;
   cases.append(QJsonObject{
    {"contours",count},{"zoom",zoom},
    {"cache_policy","DeviceCoordinateCache_only_129_plus"},
    {"baseline_median_ms",raw.medianMs},
    {"cached_median_ms",cached.medianMs},
    {"changed_pixels",double(cached.changedPixels)},
    {"max_channel_difference",double(cached.maxChannelDiff)},
    {"pixels_compared",double(baseImage.width())*baseImage.height()}
   });
   std::printf("%d items / zoom %.1f: OFF %.3f ms / CACHE %.3f ms / pixel changes %llu / max diff %llu\n",
    count,zoom,raw.medianMs,cached.medianMs,
    static_cast<unsigned long long>(cached.changedPixels),
    static_cast<unsigned long long>(cached.maxChannelDiff));
  }
 }
 const QJsonObject report{
  {"schema","alencar-laser-phase20d-isolated-cache-1"},
  {"status",integrity?"MEASURED_VISUAL_REVIEW_REQUIRED":"FAIL_INTEGRITY"},
  {"cache_in_production",false},{"hardware_commands_sent",false},
  {"automatic_promotion_allowed",false},{"geometry_changed",false},
  {"test_kind","offscreen_scene_raster_benchmark_not_real_mouse"},
  {"cases",cases}
 };
 QFile out(outFile);
 if(!out.open(QIODevice::WriteOnly|QIODevice::Truncate))return 3;
 const QByteArray data=QJsonDocument(report).toJson(QJsonDocument::Indented);
 if(out.write(data)!=data.size())return 4;
 out.close();
 return integrity?0:5;
}
