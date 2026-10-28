#pragma once
// Alencar Laser 19.8: raster para contornos editoriais (NUNCA G-code/CAM).
// O bitmap permanece intacto; o rastreamento so e confirmado pelo usuario.
// Sem dependencia de OpenCV, para compilar no Qt6/UCRT64 instalado.
#include "editor_tools.hpp"
#include <QImage>
#include <QColor>
#include <QHash>
#include <QVector>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <queue>
#include <stdexcept>
#include <vector>
#include <limits>

namespace alaser::rastertrace {
struct Settings {
 int threshold=128;
 int minimumComponent=2;
 double simplifyPx=0.45;
 bool autoThreshold=true;
 bool protectSmallText=true;
};
struct Result {
 std::vector<std::vector<Pt>> loops;
 int nodes=0,ignored=0,threshold=128,width=0,height=0;
};
struct Edge{int x0,y0,x1,y1;};
inline quint64 coordKey(int x,int y) {
 return (quint64(uint32_t(x))<<32)|uint32_t(y);
}
inline double squaredPointDistance(Pt a,Pt b){
 return (a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y);
}
inline double distanceToSegment(Pt p,Pt a,Pt b){
 const double dx=b.x-a.x,dy=b.y-a.y;
 const double den=dx*dx+dy*dy;
 if(den<1e-16)return std::sqrt(squaredPointDistance(p,a));
 const double t=std::clamp(((p.x-a.x)*dx+(p.y-a.y)*dy)/den,0.,1.);
 return std::hypot(p.x-a.x-t*dx,p.y-a.y-t*dy);
}
inline void simplifyOpen(const std::vector<Pt>& pts,int first,int last,
                        double eps,std::vector<char>& keep){
 if(last<=first+1)return;
 double best=eps;int index=-1;
 for(int i=first+1;i<last;++i){
  const double d=distanceToSegment(pts[size_t(i)],pts[size_t(first)],pts[size_t(last)]);
  if(d>best){best=d;index=i;}
 }
 if(index>=0){keep[size_t(index)]=true;
  simplifyOpen(pts,first,index,eps,keep);
  simplifyOpen(pts,index,last,eps,keep);
 }
}
inline std::vector<Pt> simplifyClosed(const std::vector<Pt>& original,double eps){
 if(original.size()<6||eps<=0)return original;
 // Dividir o anel em dois arcos, evitando RDP degenerado com inicio=fim.
 size_t far=1;double best=0;
 for(size_t i=1;i<original.size();++i){
  double d=squaredPointDistance(original.front(),original[i]);
  if(d>best){best=d;far=i;}
 }
 if(far==0||far==original.size()-1&&original.size()<8)return original;
 std::vector<Pt> linear=original;linear.push_back(original.front());
 std::vector<char> keep(linear.size(),false);
 keep[0]=keep[far]=keep.back()=true;
 simplifyOpen(linear,0,int(far),eps,keep);
 simplifyOpen(linear,int(far),int(linear.size())-1,eps,keep);
 std::vector<Pt> result;
 for(size_t i=0;i<original.size();++i)if(keep[i])result.push_back(original[i]);
 return result.size()>=3?result:original;
}
inline int otsuThreshold(const std::vector<unsigned char>& gray){
 int bins[256]={};for(auto v:gray)bins[v]++;
 double total=gray.size(),sum=0;
 for(int i=0;i<256;++i)sum+=i*bins[i];
 double bgWeight=0,bgSum=0,maxVariance=-1;int result=128;
 for(int i=0;i<256;++i){
  bgWeight+=bins[i];if(bgWeight<1)continue;
  const double fgWeight=total-bgWeight;if(fgWeight<1)break;
  bgSum+=i*bins[i];
  const double diff=bgSum/bgWeight-(sum-bgSum)/fgWeight;
  const double variance=bgWeight*fgWeight*diff*diff;
  if(variance>maxVariance){maxVariance=variance;result=i;}
 }
 return result;
}
inline Result trace(const QImage& image,Pt origin,double mmPerPixel,Settings settings={}){
 if(image.isNull())throw std::runtime_error("Imagem vazia.");
 if(!(mmPerPixel>0)||!std::isfinite(mmPerPixel))
  throw std::runtime_error("Escala de imagem invalida.");
 const int w=image.width(),h=image.height();
 if(w<2||h<2||w>1600||h>1600||int64_t(w)*h>1500000)
  throw std::runtime_error("Imagem muito grande. Reduza para ate 1600 px, 1,5 milhao de pixels.");
 const QImage src=image.convertToFormat(QImage::Format_ARGB32);
 std::vector<unsigned char> gray(size_t(w)*h,255),mask(size_t(w)*h,0);
 for(int y=0;y<h;++y){
  const QRgb* line=reinterpret_cast<const QRgb*>(src.constScanLine(y));
  for(int x=0;x<w;++x){
   QRgb pixel=line[x];
   const int a=qAlpha(pixel);
   const int lum=(54*qRed(pixel)+183*qGreen(pixel)+19*qBlue(pixel)+128)/256;
   gray[size_t(y)*w+x]=static_cast<unsigned char>(255-((255-lum)*a+127)/255);
  }
 }
 const int threshold=settings.autoThreshold?otsuThreshold(gray):
    std::clamp(settings.threshold,0,255);
 // Inclusao inclusiva preserva pretos puros e traços pequenos.
 for(size_t i=0;i<gray.size();++i)mask[i]=gray[i]<=threshold;
 std::vector<char> marked(mask.size(),0);
 int ignored=0;
 const int minArea=std::clamp(settings.minimumComponent,1,500);
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){
  const int start=y*w+x;
  if(!mask[size_t(start)]||marked[size_t(start)])continue;
  std::queue<int> queue;queue.push(start);marked[size_t(start)]=1;
  std::vector<int> component;component.reserve(64);
  int minX=x,maxX=x,minY=y,maxY=y;
  while(!queue.empty()){
   const int id=queue.front();queue.pop();component.push_back(id);
   const int cx=id%w,cy=id/w;
   minX=std::min(minX,cx);maxX=std::max(maxX,cx);
   minY=std::min(minY,cy);maxY=std::max(maxY,cy);
   const int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};
   for(int k=0;k<4;++k){
    const int xx=cx+dx[k],yy=cy+dy[k];
    if(xx<0||yy<0||xx>=w||yy>=h)continue;
    const int next=yy*w+xx;
    if(mask[size_t(next)]&&!marked[size_t(next)]){
     marked[size_t(next)]=1;queue.push(next);
    }
   }
  }
  // Isolar sujeiras sem apagar automaticamente hastes e pontos de letras.
  const bool tinyText=(maxX-minX+1<=20&&maxY-minY+1<=20);
  const int limit=settings.protectSmallText&&tinyText?1:minArea;
  if(int(component.size())<limit){
   for(int id:component)mask[size_t(id)]=0;
   ++ignored;
  }
 }
 std::vector<Edge> edges;edges.reserve(size_t(w)*h/2);
 QHash<quint64,QVector<int>> starts;
 auto black=[&](int x,int y)->bool{
  return x>=0&&y>=0&&x<w&&y<h&&mask[size_t(y)*w+x];
 };
 auto add=[&](int x0,int y0,int x1,int y1){
  if(edges.size()>=250000)throw std::runtime_error("Rastreamento excedeu 250 mil bordas.");
  starts[coordKey(x0,y0)].push_back(int(edges.size()));
  edges.push_back({x0,y0,x1,y1});
 };
 for(int y=0;y<h;++y)for(int x=0;x<w;++x)if(black(x,y)){
  if(!black(x,y-1))add(x,y,x+1,y);
  if(!black(x+1,y))add(x+1,y,x+1,y+1);
  if(!black(x,y+1))add(x+1,y+1,x,y+1);
  if(!black(x-1,y))add(x,y+1,x,y);
 }
 std::vector<char> used(edges.size(),0);
 Result result;result.width=w;result.height=h;result.threshold=threshold;result.ignored=ignored;
 for(size_t seed=0;seed<edges.size();++seed){
  if(used[seed])continue;
  std::vector<Pt> ring;ring.reserve(32);
  int edgeId=int(seed);
  const quint64 start=coordKey(edges[seed].x0,edges[seed].y0);
  int guard=0;
  while(true){
   if(used[size_t(edgeId)])
    throw std::runtime_error("Rastreamento criou uma borda duplicada.");
   used[size_t(edgeId)]=1;const Edge e=edges[size_t(edgeId)];
   ring.push_back({double(e.x0),double(e.y0)});
   const quint64 end=coordKey(e.x1,e.y1);
   if(end==start)break;
   auto it=starts.constFind(end);
   if(it==starts.constEnd())throw std::runtime_error("Borda aberta nao suportada.");
   int next=-1,best=-999;
   const int vx=e.x1-e.x0,vy=e.y1-e.y0;
   for(int idx:it.value()){
    if(used[size_t(idx)])continue;
    const Edge n=edges[size_t(idx)];
    const int wx=n.x1-n.x0,wy=n.y1-n.y0;
    const int cross=vx*wy-vy*wx,dot=vx*wx+vy*wy;
    const int score=cross>0?3:dot>0?2:cross<0?1:0;
    if(score>best){best=score;next=idx;}
   }
   if(next<0)throw std::runtime_error("Contorno nao se fecha.");
   edgeId=next;
   if(++guard>int(edges.size()))throw std::runtime_error("Ciclo de bordas invalido.");
  }
  if(ring.size()<4)continue;
  double x0=std::numeric_limits<double>::infinity(),y0=x0,x1=-x0,y1=-x0;
  for(Pt p:ring){
   x0=std::min(x0,p.x);x1=std::max(x1,p.x);
   y0=std::min(y0,p.y);y1=std::max(y1,p.y);
  }
  // Letras pequenas: reduzir simplificacao para preservar vazados e serifas.
  const bool small=(x1-x0<32||y1-y0<32);
  const double tolerance=std::clamp(settings.simplifyPx,0.,2.)*
       (settings.protectSmallText&&small?0.3:1.);
  auto simplified=simplifyClosed(ring,tolerance);
  if(result.loops.size()>=4000)throw std::runtime_error("Mais de 4000 contornos: ajuste limpeza do ruido.");
  std::vector<Pt> physical;physical.reserve(simplified.size());
  for(Pt p:simplified)physical.push_back({origin.x+p.x*mmPerPixel,origin.y+p.y*mmPerPixel});
  if(physical.size()>=3){
   result.nodes+=int(physical.size());
   if(result.nodes>250000)throw std::runtime_error("Mais de 250 mil nos no rastreamento.");
   result.loops.push_back(std::move(physical));
  }
 }
 return result;
}
} // namespace alaser::rastertrace
