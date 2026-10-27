// TESTE_11: ensaio visual REAL com Qt 6, quando Qt SDK estiver disponivel.
// Mede emendas no QPainter para o mesmo contorno desenhado integralmente
// ou em 2 QPainterPaths, comparando SquareCap e FlatCap.
// Qt6 OFFSCREEN; NAO controla maquina e NAO homologa Windows sozinho.
#include "geometry.hpp"
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QImage>
#include <QPen>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QElapsedTimer>
#include <QString>
#include <cmath>
#include <algorithm>
#include <vector>
#include <stdexcept>
using namespace alaser;
struct Sample {QString label;std::vector<Pt> points;int split=8;};
struct Compare{int difference=0,maxDifference=0;double meanAbsolute=0;};
static constexpr int W=560,H=380;
static QImage imageOf(const Sample& s,double zoom,double dpr,double penWidth,double subpixel,Qt::PenCapStyle cap,bool split){
 QImage img(W,H,QImage::Format_ARGB32_Premultiplied);img.fill(Qt::black);
 QPainter painter(&img);painter.setRenderHint(QPainter::Antialiasing,true);
 QPen pen(Qt::white);pen.setWidthF(penWidth);pen.setCapStyle(cap);pen.setJoinStyle(Qt::RoundJoin);painter.setPen(pen);painter.setBrush(Qt::NoBrush);
 auto mid=s.points.at(s.split);
 auto pathOf=[&](int first,int last){QPainterPath path;
  for(int i=first;i<=last;i++){
   const auto& n=s.points[i];QPointF q(280+subpixel+(n.x-mid.x)*zoom*dpr,190+subpixel+(n.y-mid.y)*zoom*dpr);
   if(i==first)path.moveTo(q);else path.lineTo(q);
  }return path;};
 if(split){painter.drawPath(pathOf(0,s.split));painter.drawPath(pathOf(s.split,int(s.points.size())-1));}
 else painter.drawPath(pathOf(0,int(s.points.size())-1));
 painter.end();return img;
}
static Compare compare(const QImage&full,const QImage&part){
 Compare r;long long sum=0;
 for(int y=173;y<208;y++)for(int x=263;x<298;x++){
  int a=qRed(full.pixel(x,y)),b=qRed(part.pixel(x,y));int d=std::abs(a-b);
  r.difference+=(d>0);r.maxDifference=std::max(r.maxDifference,d);sum+=d;
 }r.meanAbsolute=double(sum)/(35*35);return r;
}
static std::vector<Sample> samples(const Document&d){
 std::vector<const Contour*> candidates;
 for(const auto&c:d.contours)if(c.nodes.size()>138)candidates.push_back(&c);
 std::sort(candidates.begin(),candidates.end(),[](auto a,auto b){return a->nodes.size()>b->nodes.size();});
 std::vector<Sample> r;
 for(auto*c:candidates){
  for(int start:{32,64,128,256}){
   if(start<8||size_t(start+8)>=c->nodes.size())continue;
   Sample s;s.label=QString("real_%1_%2").arg(c->key.object).arg(start);
   for(int i=start-8;i<=start+8;i++)s.points.push_back(c->nodes[i].p);
   r.push_back(std::move(s));if(r.size()>=12)break;
  }if(r.size()>=12)break;
 }
 for(int k=0;k<6;k++){
  Sample s;s.label=QString("synthetic_%1").arg(k);
  for(int i=0;i<17;i++){
   double x=i*(k==3?.2:.125),y=0;
   switch(k){case 0:y=0;break;case 1:y=std::sin(i*.45);break;case 2:y=.03*(i-8)*(i-8);break;
    case 3:y=i>8?(i-8)*.4:0;break;case 4:y=((i%2)?-.01:.01);break;case 5:y=(i-8)*(i>8?.3:-.3);break;}
   s.points.push_back({x,y});
  }r.push_back(std::move(s));
 }return r;
}
int main(int argc,char**argv){
 QGuiApplication app(argc,argv);try{
  if(argc<3)throw std::runtime_error("usage: qt11 fixture.csv output_folder");
  QDir().mkpath(argv[2]);Document doc=readCsv(argv[1]);auto testCases=samples(doc);
  if(testCases.size()!=18)throw std::runtime_error("sample count != 18");
  QFile csv(QString::fromLocal8Bit(argv[2])+"/QT6_EMENDAS_TESTE_11.csv");
  if(!csv.open(QIODevice::WriteOnly|QIODevice::Text))throw std::runtime_error("cannot create CSV");
  QTextStream output(&csv);output<<"sample,zoom,dpr,width,subpixel,square_changed,square_max,flat_changed,flat_max,full_ms,square_ms,flat_ms\n";
  int total=0,squareNonzero=0,flatNonzero=0,improved=0,worsened=0;
  int maxSquare=0,maxFlat=0;double maxDiff=-1;QImage worstFull,worstSquare,worstFlat;
  QElapsedTimer timer;
  for(auto&s:testCases)for(double zoom:{.75,6.,80.,1200.})for(double dpr:{1.,2.})for(double width:{1.,3.})for(double offset:{0.,.37}){
   timer.start();auto full=imageOf(s,zoom,dpr,width,offset,Qt::SquareCap,false);double tf=timer.nsecsElapsed()/1e6;
   timer.start();auto square=imageOf(s,zoom,dpr,width,offset,Qt::SquareCap,true);double ts=timer.nsecsElapsed()/1e6;
   timer.start();auto flat=imageOf(s,zoom,dpr,width,offset,Qt::FlatCap,true);double tl=timer.nsecsElapsed()/1e6;
   auto a=compare(full,square),b=compare(full,flat);
   total++;squareNonzero+=(a.difference>0);flatNonzero+=(b.difference>0);
   improved+=(b.difference<a.difference);worsened+=(b.difference>a.difference);
   maxSquare=std::max(maxSquare,a.maxDifference);maxFlat=std::max(maxFlat,b.maxDifference);
   if(a.meanAbsolute+b.meanAbsolute>maxDiff){maxDiff=a.meanAbsolute+b.meanAbsolute;worstFull=full;worstSquare=square;worstFlat=flat;}
   output<<s.label<<','<<zoom<<','<<dpr<<','<<width<<','<<offset<<','<<a.difference<<','<<a.maxDifference<<','<<b.difference<<','<<b.maxDifference<<','<<tf<<','<<ts<<','<<tl<<'\n';
  }csv.close();
  QImage contact(W*3,H,QImage::Format_ARGB32_Premultiplied);contact.fill(Qt::black);
  {QPainter p(&contact);p.drawImage(0,0,worstFull);p.drawImage(W,0,worstSquare);p.drawImage(W*2,0,worstFlat);}
  if(!contact.save(QString::fromLocal8Bit(argv[2])+"/QT6_EMENDAS_PIOR_CASO.png"))throw std::runtime_error("cannot save PNG");
  // MEASURED: nao inventa aprovacao para nenhum metodo; evidencia para decisao.
  QJsonObject result{{"status","MEASURED"},{"qt_version",qVersion()},{"render_backend","QPainter QImage offscreen"},
   {"cases",total},{"square_changed_cases",squareNonzero},{"flat_changed_cases",flatNonzero},
   {"flat_improved_cases",improved},{"flat_worsened_cases",worsened},{"max_difference_square",maxSquare},
   {"max_difference_flat",maxFlat},{"perfect_visual_equivalence",squareNonzero==0||flatNonzero==0},
   {"production_approved",false},{"laser_connected",false}};
  QFile json(QString::fromLocal8Bit(argv[2])+"/QT6_EMENDAS_RESUMO_TESTE_11.json");
  if(!json.open(QIODevice::WriteOnly))throw std::runtime_error("cannot create JSON");
  json.write(QJsonDocument(result).toJson());json.close();
  qInfo("QT6 TESTE_11: cases=%d square changes=%d flat changes=%d",total,squareNonzero,flatNonzero);
  return 0;
 }catch(const std::exception&e){qCritical("QT6 TESTE_11 FAIL: %s",e.what());return 1;}
}
