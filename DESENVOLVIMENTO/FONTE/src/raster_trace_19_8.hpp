#pragma once
// Rastreio local experimental: imagem -> preto/branco -> fronteiras vetoriais.
// NUNCA envia G-code, controla laser, consulta rede ou altera arquivo original.
#include "editor_tools.hpp"
#include <QImage>
#include <QColor>
#include <QPointF>
#include <QtGlobal>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <map>
#include <queue>
#include <stdexcept>
#include <vector>

namespace alaser::trace198 {
struct Options{
 int threshold=128;      // pixels com cinza <= threshold sao pretos
 int minPixels=5;        // remove ilhas isoladas, nao pequenos furos de letras
 double optimizePx=.45;  // distancia max. da simplificacao, em pixels
 bool smooth=false;      // arredondamento opcional (pode deformar cantos)
 bool antiStaircase=false; // elimina microdegraus de pixels sem fechar buracos
 double widthMm=100.;
};
struct Result{
 QImage binary;
 std::vector<std::vector<Pt>> contours;
 size_t blackPixels=0;
 int discardedComponents=0;
 int sourceWidth=0,sourceHeight=0;
};
inline QImage grayscale(const QImage& image){
 if(image.isNull())throw std::runtime_error("Imagem ausente");
 if(image.width()>10000||image.height()>10000)
  throw std::runtime_error("Imagem acima do limite de 10000 px");
 QImage source=image.convertToFormat(QImage::Format_ARGB32);
 int w=source.width(),h=source.height();
 QImage output(w,h,QImage::Format_Grayscale8);
 for(int y=0;y<h;++y){
  const auto* p=reinterpret_cast<const QRgb*>(source.constScanLine(y));
  auto* dest=output.scanLine(y);
  for(int x=0;x<w;++x){
   const int a=qAlpha(p[x]),r=qRed(p[x]),g=qGreen(p[x]),b=qBlue(p[x]);
   const int luminance=(54*r+183*g+19*b)/256;
   dest[x]=uchar((luminance*a+255*(255-a))/255);
  }
 }
 return output;
}
inline int otsuThreshold(const QImage& gray){
 std::array<int,256> hist{};
 for(int y=0;y<gray.height();++y){
  const auto* row=gray.constScanLine(y);
  for(int x=0;x<gray.width();++x)hist[row[x]]++;
 }
 const double count=double(gray.width())*gray.height();
 double sum=0;for(int i=0;i<256;++i)sum+=double(i)*hist[size_t(i)];
 double leftSum=0,leftCount=0,best=-1;int threshold=128;
 for(int i=0;i<255;++i){
  leftCount+=hist[size_t(i)];leftSum+=double(i)*hist[size_t(i)];
  const double rightCount=count-leftCount;
  if(leftCount<1||rightCount<1)continue;
  const double score=(leftSum/leftCount-(sum-leftSum)/rightCount);
  const double variance=leftCount*rightCount*score*score;
  if(variance>best){best=variance;threshold=i;}
 }
 return threshold;
}
inline double pointLineDistance(Pt p,Pt a,Pt b){
 const double dx=b.x-a.x,dy=b.y-a.y,len=std::hypot(dx,dy);
 if(len<1e-10)return dist(p,a);
 return std::abs((p.x-a.x)*dy-(p.y-a.y)*dx)/len;
}
inline std::vector<Pt> simplifyClosed(std::vector<Pt> pts,double eps){
 if(pts.size()<5||eps<=0)return pts;
 // Desbastar degraus sem perder vertices que formam cantos agudos reais.
 for(int pass=0;pass<5;++pass){
  bool modified=false;std::vector<Pt> next;next.reserve(pts.size());
  for(size_t i=0;i<pts.size();++i){
   const auto& prev=pts[(i+pts.size()-1)%pts.size()];
   const auto& cur=pts[i];
   const auto& after=pts[(i+1)%pts.size()];
   // Explicit namespace avoids ADL collision with a same-named geometry helper.
   const double d=::alaser::trace198::pointLineDistance(cur,prev,after);
   if(pts.size()-next.size()>3&&d<eps){
    const double vx=prev.x-cur.x,vy=prev.y-cur.y;
    const double wx=after.x-cur.x,wy=after.y-cur.y;
    const double lv=std::hypot(vx,vy),lw=std::hypot(wx,wy);
    const double cosAngle=(lv*lw>1e-8)?(vx*wx+vy*wy)/(lv*lw):-1.;
    // Angulos <= 105 graus: manter, mesmo quando uma linha e curtissima.
    if(cosAngle<-.26){modified=true;continue;}
   }
   next.push_back(cur);
  }
  if(next.size()<3||!modified)break;
  pts=std::move(next);
 }
 return pts;
}
// Optional, conservative anti-staircase pass in PIXEL coordinates.
// A real 90-degree corner has one turn and remains untouched. A diagonal
// represented by a sawtooth of short orthogonal pixel edges has multiple
// nearby turns and is nudged inward by at most 0.25 pixel. No nodes created
// and original source image remains unchanged.
inline std::vector<Pt> reducePixelStaircases(std::vector<Pt> ring){
 if(ring.size()<9)return ring;
 const std::size_t n=ring.size();
 for(int pass=0;pass<2;++pass){
  const auto before=ring;
  for(std::size_t i=0;i<n;++i){
   const Pt pm2=before[(i+n-2)%n],pm1=before[(i+n-1)%n];
   const Pt at=before[i],pp1=before[(i+1)%n],pp2=before[(i+2)%n];
   const Pt trend=pp2-pm2;
   if(std::abs(trend.x)<1.0||std::abs(trend.y)<1.0)continue;
   const auto edgeA=at-pm1,edgeB=pp1-at;
   if(std::hypot(edgeA.x,edgeA.y)>1.5||
      std::hypot(edgeB.x,edgeB.y)>1.5)continue;
   int turns=0;
   for(int j=-2;j<=1;++j){
    const Pt u=before[(i+n+j+1)%n]-before[(i+n+j)%n];
    const Pt v=before[(i+n+j+2)%n]-before[(i+n+j+1)%n];
    if(std::abs(u.x*v.y-u.y*v.x)>0.3)++turns;
   }
   if(turns<2)continue;
   const Pt mid=(pm1+pp1)*0.5;
   const Pt displacement=mid-at;
   if(std::hypot(displacement.x,displacement.y)>0.75)continue;
   ring[i]=at+displacement*0.50;
  }
 }
 return ring;
}
inline Result trace(const QImage& input,Options opt){
 if(input.isNull())throw std::runtime_error("Imagem nao carregada");
 if(opt.threshold<0||opt.threshold>255||opt.minPixels<1||opt.minPixels>10000||
    !std::isfinite(opt.widthMm)||opt.widthMm<1||opt.widthMm>400||
    !std::isfinite(opt.optimizePx)||opt.optimizePx<0||opt.optimizePx>2)
  throw std::runtime_error("Parametros de rastreio fora dos limites");
 QImage small=input;
 if(std::max(small.width(),small.height())>960)
  small=small.scaled(960,960,Qt::KeepAspectRatio,Qt::SmoothTransformation);
 const QImage gray=grayscale(small);
 const int w=gray.width(),h=gray.height();
 const size_t area=size_t(w)*h;
 std::vector<uint8_t> mask(area,0);
 for(int y=0;y<h;++y){
  const auto* row=gray.constScanLine(y);
  for(int x=0;x<w;++x)mask[size_t(y)*w+x]=row[x]<=opt.threshold;
 }
 // Componentes 4-conectados: remove poeira mas preserva pontos de letras
 // e buracos (fronteiras internas sao extraidas posteriormente).
 int discarded=0;
 std::vector<size_t> queue;queue.reserve(area);
 for(size_t start=0;start<area;++start){
  if(mask[start]!=1)continue;
  queue.clear();queue.push_back(start);mask[start]=2;
  for(size_t at=0;at<queue.size();++at){
   const size_t p=queue[at];const int x=int(p%w),y=int(p/w);
   const auto push=[&](size_t n){if(mask[n]==1){mask[n]=2;queue.push_back(n);}};
   if(x>0)push(p-1);if(x+1<w)push(p+1);
   if(y>0)push(p-w);if(y+1<h)push(p+w);
  }
  if(int(queue.size())<opt.minPixels){
   for(auto p:queue)mask[p]=0;discarded++;
  }else for(auto p:queue)mask[p]=3;
 }
 Result out;
 out.sourceWidth=w;out.sourceHeight=h;out.discardedComponents=discarded;
 out.binary=QImage(w,h,QImage::Format_Grayscale8);
 for(int y=0;y<h;++y){
  auto* dst=out.binary.scanLine(y);
  for(int x=0;x<w;++x){
   bool on=mask[size_t(y)*w+x]==3;
   dst[x]=on?0:255;if(on)out.blackPixels++;
  }
 }
 // Fronteiras externas e furos, com orientacao consistente por pixel.
 struct Edge{int x,y,X,Y;bool used=false;};
 std::vector<Edge> edges;edges.reserve(std::min<size_t>(area*2,500000));
 std::map<uint64_t,std::vector<int>> outgoing;
 const auto key=[](int x,int y)->uint64_t{
  return (uint64_t(uint32_t(y))<<32)|uint32_t(x);
 };
 auto black=[&](int x,int y){
  return x>=0&&x<w&&y>=0&&y<h&&mask[size_t(y)*w+x]==3;
 };
 auto add=[&](int x,int y,int X,int Y){
  if(edges.size()>=500000)throw std::runtime_error("Imagem complexa: limite de 500 mil arestas");
  const int id=int(edges.size());edges.push_back({x,y,X,Y,false});
  outgoing[key(x,y)].push_back(id);
 };
 for(int y=0;y<h;++y)for(int x=0;x<w;++x)if(black(x,y)){
  if(!black(x,y-1))add(x,y,x+1,y);
  if(!black(x+1,y))add(x+1,y,x+1,y+1);
  if(!black(x,y+1))add(x+1,y+1,x,y+1);
  if(!black(x-1,y))add(x,y+1,x,y);
 }
 double mm=opt.widthMm/w;
 size_t total=0;
 for(size_t i=0;i<edges.size();++i){
  if(edges[i].used)continue;
  std::vector<Pt> ring;
  int edge=int(i),firstX=edges[i].x,firstY=edges[i].y;
  bool closed=false;
  for(size_t safety=0;safety<=edges.size();++safety){
   Edge& current=edges[size_t(edge)];
   if(current.used)break;
   current.used=true;
   ring.push_back({double(current.x),double(current.y)});
   if(current.X==firstX&&current.Y==firstY){closed=true;break;}
   const auto it=outgoing.find(key(current.X,current.Y));
   if(it==outgoing.end())break;
   int next=-1,best=100;
   const int oldDx=current.X-current.x,oldDy=current.Y-current.y;
   const int oldDir=(oldDx==1)?0:(oldDy==1)?1:(oldDx==-1)?2:3;
   for(int candidate:it->second){
    const Edge& e=edges[size_t(candidate)];if(e.used)continue;
    const int dx=e.X-e.x,dy=e.Y-e.y;
    const int dir=(dx==1)?0:(dy==1)?1:(dx==-1)?2:3;
    const int turn=(dir-oldDir+4)%4;
    const int priority=turn==1?0:turn==0?1:turn==3?2:3;
    if(priority<best){best=priority;next=candidate;}
   }
   if(next<0)break;
   edge=next;
  }
  if(!closed||ring.size()<4)continue;
  if(opt.antiStaircase)ring=reducePixelStaircases(std::move(ring));
  ring=simplifyClosed(std::move(ring),opt.optimizePx);
  if(ring.size()<3)continue;
  if(opt.smooth&&ring.size()>=6){
   std::vector<Pt> smoothed;smoothed.reserve(ring.size()*2);
   for(size_t j=0;j<ring.size();++j){
    const Pt a=ring[j],b=ring[(j+1)%ring.size()];
    smoothed.push_back(a*.75+b*.25);
    smoothed.push_back(a*.25+b*.75);
   }
   ring=std::move(smoothed);
  }
  for(auto& p:ring){p.x*=mm;p.y*=mm;}
  total+=ring.size();
  if(total>50000||out.contours.size()>=1000)
   throw std::runtime_error("Rastreio acima de 50 mil nos ou 1000 contornos: aumente otimizacao, corte a imagem ou reduza ruido");
  out.contours.push_back(std::move(ring));
 }
 if(out.contours.empty())throw std::runtime_error("Nenhum contorno reconhecido; ajuste limiar/contraste");
 return out;
}
} // namespace alaser::trace198
