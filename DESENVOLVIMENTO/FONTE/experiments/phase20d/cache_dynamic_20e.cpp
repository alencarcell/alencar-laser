// ALENCAR LASER 20e - independent synthetic cache experiment (NOT the editor).
// No laser commands, real .alaser files, or production configuration.
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
#include <QColor>
#include <QString>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <vector>
#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

static quint64 rssBytes() {
#ifdef Q_OS_WIN
 PROCESS_MEMORY_COUNTERS_EX m{};
 m.cb=sizeof(m);
 if(GetProcessMemoryInfo(GetCurrentProcess(),
     reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&m),sizeof(m)))
   return static_cast<quint64>(m.WorkingSetSize);
#endif
 return 0;
}
static QPainterPath contour(int id,int nodes){
 QPainterPath p;
 const double cx=12.0+(id%25)*16.0,cy=12.0+(id/25)*16.0;
 const double r=2.0+(id%7)*0.15;
 for(int j=0;j<nodes;++j){
  const double angle=6.283185307179586*double(j)/double(nodes);
  QPointF q(cx+r*std::cos(angle),cy+r*std::sin(angle));
  if(j==0)p.moveTo(q);else p.lineTo(q);
 }
 p.closeSubpath();return p;
}
static int populate(QGraphicsScene& scene,int count,bool useCache){
 scene.setSceneRect(0,0,420,420);
 int eligible=0;
 for(int i=0;i<count;++i){
  const int nodes=i<282?6:i<470?20:i<561?64:160;
  auto* item=scene.addPath(contour(i,nodes),QPen(QColor(20,20,20),0.35));
  item->setBrush(Qt::NoBrush);
  if(useCache&&nodes>=129){
   item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
   ++eligible;
  }
 }
 return eligible;
}
struct Step{double zoom;double panX;double panY;};
static QImage draw(QGraphicsScene& scene,const Step& step){
 constexpr int W=900,H=700;
 QImage out(W,H,QImage::Format_ARGB32_Premultiplied);
 out.fill(Qt::white);
 QPainter painter(&out);
 painter.setRenderHint(QPainter::Antialiasing,true);
 // Viewport stays at 900x700 physical pixels. Move source rectangle to pan.
 const QRectF source(step.panX,step.panY,420.0/step.zoom,420.0/step.zoom);
 scene.render(&painter,QRectF(0,0,W,H),source,Qt::IgnoreAspectRatio);
 painter.end();
 return out;
}
static quint64 diffPixels(const QImage& a,const QImage& b,int& maxDelta){
 if(a.size()!=b.size()||a.format()!=b.format())return UINT64_MAX;
 quint64 changed=0;
 for(int y=0;y<a.height();++y){
  const auto* left=reinterpret_cast<const QRgb*>(a.constScanLine(y));
  const auto* right=reinterpret_cast<const QRgb*>(b.constScanLine(y));
  for(int x=0;x<a.width();++x){
   const int delta=std::max({std::abs(qRed(left[x])-qRed(right[x])),
    std::abs(qGreen(left[x])-qGreen(right[x])),
    std::abs(qBlue(left[x])-qBlue(right[x])),
    std::abs(qAlpha(left[x])-qAlpha(right[x]))});
   if(delta){++changed;maxDelta=std::max(maxDelta,delta);}
  }
 }
 return changed;
}
static double median(std::vector<double> v){
 if(v.empty())return -1;
 std::sort(v.begin(),v.end());
 const auto m=v.size()/2;
 return v.size()%2?v[m]:(v[m-1]+v[m])/2.0;
}
struct Measurement{
 double baselineMs=0,cacheMs=0;
 quint64 changedPixels=0,maxDelta=0,peakRss=0;
 int eligible=0;
};
static Measurement round(int count,int n){
 // Balanced orders across rounds to reduce consistent first-run bias.
 QGraphicsScene first,second;
 const bool cacheFirst=(n%2)==1;
 const int eligibleA=populate(first,count,cacheFirst);
 const int eligibleB=populate(second,count,!cacheFirst);
 Measurement result;
 result.eligible=std::max(eligibleA,eligibleB);
 const std::array<Step,8> steps{{
  {1.0,0,0},{1.2,0,0},{2.0,0,0},{1.2,20,10},
  {1.0,40,20},{2.0,30,15},{1.2,5,25},{1.0,0,0}
 }};
 std::vector<double> offTimes,onTimes;
 // Warm-up deliberately excluded from timing.
 (void)draw(first,steps[0]);(void)draw(second,steps[0]);
 for(const Step& step:steps){
  QElapsedTimer timer;
  timer.start();
  const QImage a=draw(first,step);
  const double aMs=double(timer.nsecsElapsed())/1e6;
  timer.restart();
  const QImage b=draw(second,step);
  const double bMs=double(timer.nsecsElapsed())/1e6;
  if(cacheFirst){onTimes.push_back(aMs);offTimes.push_back(bMs);}
  else{offTimes.push_back(aMs);onTimes.push_back(bMs);}
  int maxChannel=0;
  const quint64 diff=diffPixels(a,b,maxChannel);
  result.changedPixels+=diff;
  result.maxDelta=std::max(result.maxDelta,quint64(maxChannel));
  result.peakRss=std::max(result.peakRss,rssBytes());
 }
 result.baselineMs=median(offTimes);
 result.cacheMs=median(onTimes);
 return result;
}
int main(int argc,char** argv){
 QApplication app(argc,argv);
 QString output;
 for(int i=1;i+1<argc;++i){
  if(QString::fromLocal8Bit(argv[i])=="--out"){
   output=QString::fromLocal8Bit(argv[++i]);
  }
 }
 if(output.isEmpty()){std::fprintf(stderr,"Usage: alencar_phase20e_dynamic --out report.json\n");return 2;}
 QJsonArray cases;
 bool valid=true;
 for(const int count:{40,128,575}){
  for(int i=0;i<6;++i){
   const Measurement result=round(count,i);
   if(result.baselineMs<=0||result.cacheMs<=0||
      result.eligible!=(count>561?count-561:0))valid=false;
   cases.append(QJsonObject{
    {"contours",count},{"round",i+1},{"cache_first",bool(i%2)},
    {"eligible_cache_items",result.eligible},
    {"baseline_median_frame_ms",result.baselineMs},
    {"cached_median_frame_ms",result.cacheMs},
    {"changed_pixels_all_steps",double(result.changedPixels)},
    {"max_channel_difference",double(result.maxDelta)},
    {"peak_process_working_set_bytes",double(result.peakRss)}
   });
   std::printf("%d objects / round %d: OFF %.3f ms CACHE %.3f ms / pixels changed %llu / peak RSS %llu\n",
    count,i+1,result.baselineMs,result.cacheMs,
    static_cast<unsigned long long>(result.changedPixels),
    static_cast<unsigned long long>(result.peakRss));
  }
 }
 const QJsonObject summary{
  {"schema","alencar-laser-phase20e-dynamic-cache-v1"},
  {"status",valid?"SYNTHETIC_MEASURED_MANUAL_PENDING":"FAIL_INTEGRITY"},
  {"no_production_change",true},{"hardware_commands_sent",false},
  {"speedup_claim_allowed",false},{"automatic_promotion_allowed",false},
  {"test_scope","offscreen QGraphicsScene synthetic frames, not interactive QGraphicsView"},
  {"protocol","6 balanced rounds per size; 8 changing zoom/pan steps; 40,128,575 items"},
  {"cases",cases}
 };
 QFile f(output);
 if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate))return 3;
 const QByteArray bytes=QJsonDocument(summary).toJson(QJsonDocument::Indented);
 if(f.write(bytes)!=bytes.size())return 4;
 f.close();
 return valid?0:5;
}
