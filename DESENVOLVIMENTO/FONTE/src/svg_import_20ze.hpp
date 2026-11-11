#pragma once
// Alencar Laser 20ze — importacao vetorial SVG offline.
// Importa geometria, nunca estilos de corte, bitmap, texto ou comandos de maquina.
#include "bezier_native.hpp"
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <vector>

namespace alaser::svg20ze {

struct Matrix {
 double a=1,b=0,c=0,d=1,e=0,f=0;
 Pt map(Pt p)const{return {a*p.x+c*p.y+e,b*p.x+d*p.y+f};}
};
inline Matrix compose(const Matrix& x,const Matrix& y){
 // x(y(point)): SVG transform lists are post-multiplied.
 return {
  x.a*y.a+x.c*y.b,
  x.b*y.a+x.d*y.b,
  x.a*y.c+x.c*y.d,
  x.b*y.c+x.d*y.d,
  x.a*y.e+x.c*y.f+x.e,
  x.b*y.e+x.d*y.f+x.f
 };
}
inline std::vector<double> numbers(const QString& value){
 static const QRegularExpression re(
  QStringLiteral("[+-]?(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)(?:[eE][+-]?[0-9]+)?"));
 std::vector<double> out;
 auto it=re.globalMatch(value);
 while(it.hasNext()){
  const auto m=it.next();bool ok=false;
  const double n=m.captured().toDouble(&ok);
  if(!ok||!std::isfinite(n))throw std::runtime_error("SVG: numero invalido");
  out.push_back(n);
 }
 return out;
}
inline double lengthMm(const QString& value){
 static const QRegularExpression re(
  QStringLiteral("^\\s*([+-]?(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)(?:[eE][+-]?[0-9]+)?)\\s*(mm|cm|in|pt|pc|px|q)?\\s*$"),
  QRegularExpression::CaseInsensitiveOption);
 const auto m=re.match(value);
 if(!m.hasMatch())throw std::runtime_error("SVG: dimensao fisica ausente ou unidade percentual nao suportada");
 bool ok=false;double n=m.captured(1).toDouble(&ok);
 if(!ok||!std::isfinite(n)||n<=0)throw std::runtime_error("SVG: dimensao fisica invalida");
 const QString unit=m.captured(2).toLower();
 if(unit=="mm")return n;
 if(unit=="cm")return n*10.;
 if(unit=="in")return n*25.4;
 if(unit=="pt")return n*25.4/72.;
 if(unit=="pc")return n*25.4/6.;
 if(unit=="q")return n*.25;
 return n*25.4/96.; // CSS px e unidade sem sufixo.
}
inline Matrix parseTransform(const QString& source){
 if(source.trimmed().isEmpty())return {};
 static const QRegularExpression re(QStringLiteral("([A-Za-z]+)\\s*\\(([^)]*)\\)"));
 Matrix result;
 qsizetype consumed=0;
 auto it=re.globalMatch(source);
 while(it.hasNext()){
  const auto m=it.next();
  if(!source.mid(consumed,m.capturedStart()-consumed).trimmed().isEmpty())
   throw std::runtime_error("SVG: lista transform invalida");
  consumed=m.capturedEnd();
  const QString op=m.captured(1);
  const auto v=numbers(m.captured(2));
  Matrix local;
  if(op=="matrix"&&v.size()==6)
   local={v[0],v[1],v[2],v[3],v[4],v[5]};
  else if(op=="translate"&&(v.size()==1||v.size()==2)){
   local.e=v[0];local.f=v.size()==2?v[1]:0.;
  }else if(op=="scale"&&(v.size()==1||v.size()==2)){
   local.a=v[0];local.d=v.size()==2?v[1]:v[0];
  }else if(op=="rotate"&&(v.size()==1||v.size()==3)){
   const double r=v[0]*std::numbers::pi/180.,cs=std::cos(r),sn=std::sin(r);
   local={cs,sn,-sn,cs,0,0};
   if(v.size()==3){
    Matrix toCenter;toCenter.e=v[1];toCenter.f=v[2];
    Matrix fromCenter;fromCenter.e=-v[1];fromCenter.f=-v[2];
    local=compose(compose(toCenter,local),fromCenter);
   }
  }else if(op=="skewX"&&v.size()==1)
   local.c=std::tan(v[0]*std::numbers::pi/180.);
  else if(op=="skewY"&&v.size()==1)
   local.b=std::tan(v[0]*std::numbers::pi/180.);
  else throw std::runtime_error("SVG: transform nao suportado ou malformado");
  result=compose(result,local);
 }
 if(!source.mid(consumed).trimmed().isEmpty())
  throw std::runtime_error("SVG: transform incompleto");
 return result;
}

struct ImportResult {
 CurveDocument curves;
 int pathElements=0;
 int sourceShapes=0;
 std::size_t anchors=0;
 std::size_t cubicSegments=0;
 double widthMm=0,heightMm=0;
};

inline void appendContour(ImportResult& out,CurveContour contour,const Matrix& transform){
 if(contour.nodes.size()<2)return;
 if(contour.closed&&contour.nodes.size()<3)
  throw std::runtime_error("SVG: contorno fechado degenerado");
 if(out.curves.contours.size()>=50000)
  throw std::runtime_error("SVG: limite de 50 mil contornos excedido");
 if(contour.nodes.size()>1000000-out.anchors)
  throw std::runtime_error("SVG: limite de um milhao de ancoras excedido");
 for(auto& node:contour.nodes){
  for(Pt* p:{&node.anchor,&node.handleIn,&node.handleOut}){
   *p=transform.map(*p);
   if(!std::isfinite(p->x)||!std::isfinite(p->y)||
      std::abs(p->x)>1e7||std::abs(p->y)>1e7)
    throw std::runtime_error("SVG: coordenada fora do limite seguro em milimetros");
  }
  node.id=out.curves.nextId++;
 }
 contour.key={int(out.curves.contours.size()+1),0};
 contour.layer=0;
 out.anchors+=contour.nodes.size();
 for(uint8_t edge:contour.edgeKind)if(edge==1)++out.cubicSegments;
 out.curves.contours.push_back(std::move(contour));
}

class PathScanner {
 const QString& value_;
 qsizetype pos_=0;
 void separators(){
  while(pos_<value_.size()&&(value_[pos_].isSpace()||value_[pos_]==QLatin1Char(',')))++pos_;
 }
public:
 explicit PathScanner(const QString& value):value_(value){}
 bool atEnd(){separators();return pos_>=value_.size();}
 bool takeCommand(QChar& command){
  separators();
  if(pos_<value_.size()&&value_[pos_].isLetter()){
   command=value_[pos_++];return true;
  }
  return false;
 }
 bool hasNumber(){
  separators();
  if(pos_>=value_.size())return false;
  const QChar c=value_[pos_];
  return c.isDigit()||c==QLatin1Char('+')||c==QLatin1Char('-')||c==QLatin1Char('.');
 }
 double number(){
  separators();
  const qsizetype begin=pos_;
  if(pos_<value_.size()&&(value_[pos_]==QLatin1Char('+')||value_[pos_]==QLatin1Char('-')))++pos_;
  bool digits=false;
  while(pos_<value_.size()&&value_[pos_].isDigit()){digits=true;++pos_;}
  if(pos_<value_.size()&&value_[pos_]==QLatin1Char('.')){
   ++pos_;
   while(pos_<value_.size()&&value_[pos_].isDigit()){digits=true;++pos_;}
  }
  if(!digits)throw std::runtime_error("SVG path: numero esperado");
  if(pos_<value_.size()&&(value_[pos_]==QLatin1Char('e')||value_[pos_]==QLatin1Char('E'))){
   ++pos_;
   if(pos_<value_.size()&&(value_[pos_]==QLatin1Char('+')||value_[pos_]==QLatin1Char('-')))++pos_;
   const qsizetype exponent=pos_;
   while(pos_<value_.size()&&value_[pos_].isDigit())++pos_;
   if(exponent==pos_)throw std::runtime_error("SVG path: expoente invalido");
  }
  bool ok=false;const double n=value_.mid(begin,pos_-begin).toDouble(&ok);
  if(!ok||!std::isfinite(n))throw std::runtime_error("SVG path: numero nao finito");
  return n;
 }
};

class PathBuilder {
 ImportResult& output_;
 Matrix transform_;
 CurveContour current_;
 Pt point_{},start_{};
 bool active_=false;
 CurveNode node(Pt p)const{return {0,p,p,p};}
 void flush(){
  if(active_&&current_.nodes.size()>=2)
   appendContour(output_,std::move(current_),transform_);
  current_=CurveContour{};active_=false;
 }
 void begin(Pt p){
  flush();current_=CurveContour{};current_.nodes.push_back(node(p));
  point_=start_=p;active_=true;
 }
 void ensure(){
  if(!active_)begin(point_);
 }
public:
 PathBuilder(ImportResult& out,Matrix transform):output_(out),transform_(transform){}
 Pt point()const{return point_;}
 void moveTo(Pt p){begin(p);}
 void lineTo(Pt p){
  ensure();
  if(dist(point_,p)<1e-10)return;
  if(current_.nodes.size()>=20000)
   throw std::runtime_error("SVG: contorno excede 20 mil ancoras");
  current_.edgeKind.push_back(0);current_.nodes.push_back(node(p));point_=p;
 }
 void cubicTo(Pt c1,Pt c2,Pt p){
  ensure();
  if(current_.nodes.size()>=20000)
   throw std::runtime_error("SVG: contorno excede 20 mil ancoras");
  current_.nodes.back().handleOut=c1;
  CurveNode next=node(p);next.handleIn=c2;
  current_.edgeKind.push_back(1);current_.nodes.push_back(next);point_=p;
 }
 void close(){
  if(!active_)return;
  if(current_.nodes.size()<2){flush();point_=start_;return;}
  if(dist(current_.nodes.back().anchor,start_)<1e-8){
   current_.nodes.front().handleIn=current_.nodes.back().handleIn;
   current_.nodes.pop_back();
  }else current_.edgeKind.push_back(0);
  current_.closed=true;
  if(current_.nodes.size()<3)
   throw std::runtime_error("SVG: caminho fechado com menos de tres ancoras");
  flush();point_=start_;
 }
 void finish(){flush();}
};

inline Pt relativePoint(double x,double y,bool relative,Pt current){
 return relative?Pt{current.x+x,current.y+y}:Pt{x,y};
}
inline Pt readPoint(PathScanner& scanner,bool relative,Pt current){
 const double x=scanner.number();
 const double y=scanner.number();
 return relativePoint(x,y,relative,current);
}
inline void arcTo(PathBuilder& builder,double rx,double ry,double rotation,
                  bool largeArc,bool sweep,Pt end){
 Pt start=builder.point();
 rx=std::abs(rx);ry=std::abs(ry);
 if(rx<1e-12||ry<1e-12||dist(start,end)<1e-12){builder.lineTo(end);return;}
 const double phi=std::remainder(rotation,360.)*std::numbers::pi/180.;
 const double cs=std::cos(phi),sn=std::sin(phi);
 const double dx=(start.x-end.x)*.5,dy=(start.y-end.y)*.5;
 const double xp=cs*dx+sn*dy,yp=-sn*dx+cs*dy;
 double rx2=rx*rx,ry2=ry*ry,xp2=xp*xp,yp2=yp*yp;
 const double lambda=xp2/rx2+yp2/ry2;
 if(lambda>1.){const double scale=std::sqrt(lambda);rx*=scale;ry*=scale;rx2=rx*rx;ry2=ry*ry;}
 const double denominator=rx2*yp2+ry2*xp2;
 const double numerator=std::max(0.,rx2*ry2-rx2*yp2-ry2*xp2);
 const double sign=(largeArc==sweep)?-1.:1.;
 const double factor=denominator>1e-24?sign*std::sqrt(numerator/denominator):0.;
 const double cxp=factor*(rx*yp/ry),cyp=factor*(-ry*xp/rx);
 const double cx=cs*cxp-sn*cyp+(start.x+end.x)*.5;
 const double cy=sn*cxp+cs*cyp+(start.y+end.y)*.5;
 const auto angle=[](double ux,double uy,double vx,double vy){
  return std::atan2(ux*vy-uy*vx,ux*vx+uy*vy);
 };
 const double ux=(xp-cxp)/rx,uy=(yp-cyp)/ry;
 const double vx=(-xp-cxp)/rx,vy=(-yp-cyp)/ry;
 double theta=std::atan2(uy,ux);
 double delta=angle(ux,uy,vx,vy);
 if(!sweep&&delta>0)delta-=2.*std::numbers::pi;
 if(sweep&&delta<0)delta+=2.*std::numbers::pi;
 const int segments=std::max(1,int(std::ceil(std::abs(delta)/(std::numbers::pi*.5))));
 const double step=delta/segments;
 const auto position=[&](double t){
  return Pt{cx+rx*cs*std::cos(t)-ry*sn*std::sin(t),
            cy+rx*sn*std::cos(t)+ry*cs*std::sin(t)};
 };
 const auto derivative=[&](double t){
  return Pt{-rx*cs*std::sin(t)-ry*sn*std::cos(t),
            -rx*sn*std::sin(t)+ry*cs*std::cos(t)};
 };
 for(int i=0;i<segments;++i){
  const double next=theta+step;
  const double alpha=4./3.*std::tan(step/4.);
  const Pt p0=position(theta),p3=(i+1==segments)?end:position(next);
  const Pt p1=p0+derivative(theta)*alpha;
  const Pt p2=p3-derivative(next)*alpha;
  builder.cubicTo(p1,p2,p3);
  theta=next;
 }
}

inline void parsePath(const QString& data,const Matrix& transform,ImportResult& output){
 PathScanner scanner(data);PathBuilder builder(output,transform);
 QChar command;char previous=0;Pt cubicControl{},quadControl{};
 while(!scanner.atEnd()){
  if(scanner.takeCommand(command)){}
  else if(command.isNull())throw std::runtime_error("SVG path: comando ausente");
  const char raw=command.toLatin1();
  const bool relative=std::islower(static_cast<unsigned char>(raw))!=0;
  const char op=char(std::toupper(static_cast<unsigned char>(raw)));
  if(op=='Z'){
   builder.close();builder.finish();previous='Z';command=QChar();continue;
  }
  if(op=='M'){
   if(!scanner.hasNumber())throw std::runtime_error("SVG path: M sem coordenadas");
   const Pt p=readPoint(scanner,relative,builder.point());
   builder.moveTo(p);
   while(scanner.hasNumber()){
    const Pt q=readPoint(scanner,relative,builder.point());
    builder.lineTo(q);
   }
   command=relative?QLatin1Char('l'):QLatin1Char('L');
   previous='M';continue;
  }
  int groups=0;
  while(scanner.hasNumber()){
   const Pt current=builder.point();
   if(op=='L'){
    builder.lineTo(readPoint(scanner,relative,current));
   }else if(op=='H'){
    const double x=scanner.number();builder.lineTo({relative?current.x+x:x,current.y});
   }else if(op=='V'){
    const double y=scanner.number();builder.lineTo({current.x,relative?current.y+y:y});
   }else if(op=='C'){
    const Pt c1=readPoint(scanner,relative,current);
    const Pt c2=readPoint(scanner,relative,current);
    const Pt end=readPoint(scanner,relative,current);
    builder.cubicTo(c1,c2,end);cubicControl=c2;
   }else if(op=='S'){
    const Pt c1=(previous=='C'||previous=='S')?current*2.-cubicControl:current;
    const Pt c2=readPoint(scanner,relative,current);
    const Pt end=readPoint(scanner,relative,current);
    builder.cubicTo(c1,c2,end);cubicControl=c2;
   }else if(op=='Q'){
    const Pt q=readPoint(scanner,relative,current);
    const Pt end=readPoint(scanner,relative,current);
    const Pt c1=current+(q-current)*(2./3.),c2=end+(q-end)*(2./3.);
    builder.cubicTo(c1,c2,end);quadControl=q;
   }else if(op=='T'){
    const Pt q=(previous=='Q'||previous=='T')?current*2.-quadControl:current;
    const Pt end=readPoint(scanner,relative,current);
    const Pt c1=current+(q-current)*(2./3.),c2=end+(q-end)*(2./3.);
    builder.cubicTo(c1,c2,end);quadControl=q;
   }else if(op=='A'){
    const double rx=scanner.number();
    const double ry=scanner.number();
    const double rotation=scanner.number();
    const double large=scanner.number();
    const double sweep=scanner.number();
    if((large!=0.&&large!=1.)||(sweep!=0.&&sweep!=1.))
     throw std::runtime_error("SVG path: flags de arco invalidas");
    const Pt end=readPoint(scanner,relative,current);
    arcTo(builder,rx,ry,rotation,large==1.,sweep==1.,end);
   }else throw std::runtime_error("SVG path: comando nao suportado");
   ++groups;
   if(op!='C'&&op!='S')cubicControl=builder.point();
   if(op!='Q'&&op!='T')quadControl=builder.point();
   previous=op;
  }
  if(groups==0)throw std::runtime_error("SVG path: comando sem parametros");
 }
 builder.finish();
}

inline double attrNumber(const QXmlStreamAttributes& a,const char* key,double fallback=0.){
 const auto raw=a.value(QLatin1String(key));
 if(raw.isEmpty())return fallback;
 bool ok=false;const double value=raw.toString().toDouble(&ok);
 if(!ok||!std::isfinite(value))throw std::runtime_error("SVG: atributo numerico invalido");
 return value;
}
inline bool hiddenElement(const QXmlStreamAttributes& a){
 const QString display=a.value(QLatin1String("display")).toString().trimmed().toLower();
 const QString visibility=a.value(QLatin1String("visibility")).toString().trimmed().toLower();
 const QString style=a.value(QLatin1String("style")).toString().toLower().remove(QLatin1Char(' '));
 return display=="none"||visibility=="hidden"||
        style.contains("display:none")||style.contains("visibility:hidden");
}
inline void checkNoClip(const QXmlStreamAttributes& a){
 for(const char* key:{"clip-path","mask","filter"}){
  const QString value=a.value(QLatin1String(key)).toString().trimmed();
  if(!value.isEmpty()&&value!="none")
   throw std::runtime_error("SVG: clip-path, mask e filter precisam ser convertidos em caminhos antes da importacao");
 }
}
inline void appendPoints(const std::vector<double>& values,bool closed,
                         const Matrix& transform,ImportResult& output){
 if(values.size()%2!=0||values.size()<(closed?6u:4u)||values.size()/2>20000)
  throw std::runtime_error("SVG: lista de pontos incompleta ou acima de 20 mil ancoras");
 CurveContour contour;contour.closed=closed;
 for(std::size_t i=0;i<values.size();i+=2)
  contour.nodes.push_back({0,{values[i],values[i+1]},
                            {values[i],values[i+1]},
                            {values[i],values[i+1]}});
 const std::size_t edges=contour.nodes.size()-1+(closed?1u:0u);
 contour.edgeKind.assign(edges,0);
 appendContour(output,std::move(contour),transform);
}
inline void appendEllipse(double cx,double cy,double rx,double ry,
                          const Matrix& transform,ImportResult& output){
 if(!(rx>0&&ry>0))throw std::runtime_error("SVG: elipse com dimensao invalida");
 constexpr double k=.5522847498307936;
 CurveContour c;c.closed=true;
 const std::array<Pt,4> p{{{cx+rx,cy},{cx,cy+ry},{cx-rx,cy},{cx,cy-ry}}};
 const std::array<Pt,4> out{{
  {cx+rx,cy+k*ry},{cx-k*rx,cy+ry},
  {cx-rx,cy-k*ry},{cx+k*rx,cy-ry}}};
 const std::array<Pt,4> in{{
  {cx+rx,cy-k*ry},{cx+k*rx,cy+ry},
  {cx-rx,cy+k*ry},{cx-k*rx,cy-ry}}};
 for(std::size_t i=0;i<4;++i)c.nodes.push_back({0,p[i],in[i],out[i]});
 c.edgeKind.assign(4,1);
 appendContour(output,std::move(c),transform);
}
inline void appendRect(double x,double y,double w,double h,double rx,double ry,
                       const Matrix& transform,ImportResult& output){
 if(!(w>0&&h>0))throw std::runtime_error("SVG: retangulo com dimensao invalida");
 rx=std::clamp(std::abs(rx),0.,w*.5);ry=std::clamp(std::abs(ry),0.,h*.5);
 if(rx==0||ry==0){
  appendPoints({x,y,x+w,y,x+w,y+h,x,y+h},true,transform,output);return;
 }
 constexpr double k=.5522847498307936;
 CurveContour c;c.closed=true;
 const auto add=[&](Pt p,Pt in,Pt out){c.nodes.push_back({0,p,in,out});};
 add({x+rx,y},{x+rx-k*rx,y},{x+rx,y});
 add({x+w-rx,y},{x+w-rx,y},{x+w-rx+k*rx,y});
 add({x+w,y+ry},{x+w,y+ry-k*ry},{x+w,y+ry});
 add({x+w,y+h-ry},{x+w,y+h-ry},{x+w,y+h-ry+k*ry});
 add({x+w-rx,y+h},{x+w-rx+k*rx,y+h},{x+w-rx,y+h});
 add({x+rx,y+h},{x+rx,y+h},{x+rx-k*rx,y+h});
 add({x,y+h-ry},{x,y+h-ry+k*ry},{x,y+h-ry});
 add({x,y+ry},{x,y+ry},{x,y+ry-k*ry});
 c.edgeKind={0,1,0,1,0,1,0,1};
 appendContour(output,std::move(c),transform);
}
inline Matrix viewportTransform(const QXmlStreamAttributes& a,ImportResult& out){
 const QString viewBoxText=a.value(QLatin1String("viewBox")).toString();
 const auto vb=numbers(viewBoxText);
 const bool hasViewBox=!viewBoxText.trimmed().isEmpty();
 if(hasViewBox&&(vb.size()!=4||vb[2]<=0||vb[3]<=0))
  throw std::runtime_error("SVG: viewBox invalido");
 const bool hasWidth=!a.value(QLatin1String("width")).isEmpty();
 const bool hasHeight=!a.value(QLatin1String("height")).isEmpty();
 if(!hasViewBox&&!hasWidth&&!hasHeight)
  throw std::runtime_error("SVG: informe width/height ou viewBox");
 double vbX=0,vbY=0,vbW=0,vbH=0;
 if(hasViewBox){vbX=vb[0];vbY=vb[1];vbW=vb[2];vbH=vb[3];}
 const double pxMm=25.4/96.;
 if(hasWidth)out.widthMm=lengthMm(a.value(QLatin1String("width")).toString());
 if(hasHeight)out.heightMm=lengthMm(a.value(QLatin1String("height")).toString());
 if(hasViewBox){
  if(!hasWidth)out.widthMm=vbW*pxMm;
  if(!hasHeight)out.heightMm=vbH*pxMm;
  if(!hasWidth&&hasHeight)out.widthMm=out.heightMm*vbW/vbH;
  if(!hasHeight&&hasWidth)out.heightMm=out.widthMm*vbH/vbW;
 }else{
  if(!hasWidth||!hasHeight)
   throw std::runtime_error("SVG: width e height sao obrigatorios sem viewBox");
  vbW=out.widthMm/pxMm;vbH=out.heightMm/pxMm;
 }
 if(!(out.widthMm>0&&out.heightMm>0&&std::isfinite(out.widthMm)&&std::isfinite(out.heightMm)))
  throw std::runtime_error("SVG: dimensoes fisicas invalidas");
 const QString preserve=a.value(QLatin1String("preserveAspectRatio")).toString().trimmed();
 if(!hasViewBox)return {pxMm,0,0,pxMm,0,0};
 double sx=out.widthMm/vbW,sy=out.heightMm/vbH;
 if(preserve.startsWith("none"))return {sx,0,0,sy,-vbX*sx,-vbY*sy};
 if(preserve.contains("slice"))
  throw std::runtime_error("SVG: preserveAspectRatio=slice nao e suportado; converta para meet ou none");
 const double scale=std::min(sx,sy);
 const double extraX=out.widthMm-vbW*scale,extraY=out.heightMm-vbH*scale;
 double alignX=.5,alignY=.5;
 if(preserve.contains("xMin"))alignX=0.;else if(preserve.contains("xMax"))alignX=1.;
 if(preserve.contains("YMin"))alignY=0.;else if(preserve.contains("YMax"))alignY=1.;
 return {scale,0,0,scale,extraX*alignX-vbX*scale,extraY*alignY-vbY*scale};
}
inline void readChildren(QXmlStreamReader& xml,const Matrix& parent,ImportResult& out){
 while(xml.readNextStartElement()){
  const QString name=xml.name().toString();
  const auto a=xml.attributes();
  if(hiddenElement(a)){xml.skipCurrentElement();continue;}
  checkNoClip(a);
  const Matrix transform=compose(parent,parseTransform(a.value(QLatin1String("transform")).toString()));
  if(name=="g"){
   readChildren(xml,transform,out);continue;
  }
  if(name=="path"){
   const QString data=a.value(QLatin1String("d")).toString();
   if(!data.trimmed().isEmpty()){
    ++out.pathElements;parsePath(data,transform,out);
   }
   xml.skipCurrentElement();continue;
  }
  if(name=="line"){
   appendPoints({attrNumber(a,"x1"),attrNumber(a,"y1"),
                 attrNumber(a,"x2"),attrNumber(a,"y2")},false,transform,out);
   ++out.sourceShapes;xml.skipCurrentElement();continue;
  }
  if(name=="polyline"||name=="polygon"){
   appendPoints(numbers(a.value(QLatin1String("points")).toString()),
                name=="polygon",transform,out);
   ++out.sourceShapes;xml.skipCurrentElement();continue;
  }
  if(name=="rect"){
   const double x=attrNumber(a,"x"),y=attrNumber(a,"y");
   const double w=attrNumber(a,"width"),h=attrNumber(a,"height");
   double rx=attrNumber(a,"rx"),ry=attrNumber(a,"ry");
   if(rx==0&&ry!=0)rx=ry;if(ry==0&&rx!=0)ry=rx;
   appendRect(x,y,w,h,rx,ry,transform,out);
   ++out.sourceShapes;xml.skipCurrentElement();continue;
  }
  if(name=="circle"){
   const double r=attrNumber(a,"r");
   appendEllipse(attrNumber(a,"cx"),attrNumber(a,"cy"),r,r,transform,out);
   ++out.sourceShapes;xml.skipCurrentElement();continue;
  }
  if(name=="ellipse"){
   appendEllipse(attrNumber(a,"cx"),attrNumber(a,"cy"),
                 attrNumber(a,"rx"),attrNumber(a,"ry"),transform,out);
   ++out.sourceShapes;xml.skipCurrentElement();continue;
  }
  if(name=="defs"||name=="metadata"||name=="title"||name=="desc"||
     name=="style"||name=="namedview"||name=="clipPath"||name=="mask"||
     name=="linearGradient"||name=="radialGradient"||name=="pattern"||
     name=="marker"||name=="filter"){
   xml.skipCurrentElement();continue;
  }
  if(name=="text"||name=="image"||name=="use"||name=="foreignObject")
   throw std::runtime_error("SVG: texto, imagens e referencias use devem ser convertidos em caminhos vetoriais antes da importacao");
  throw std::runtime_error(QString("SVG: elemento vetorial nao suportado: %1").arg(name).toStdString());
 }
}
inline ImportResult readSvgGeometry(const QString& path){
 QFile file(path);
 if(!file.open(QIODevice::ReadOnly))
  throw std::runtime_error("SVG: arquivo nao acessivel");
 if(file.size()<=0||file.size()>32LL*1024*1024)
  throw std::runtime_error("SVG: arquivo vazio ou acima de 32 MB");
 QXmlStreamReader xml(&file);
 if(!xml.readNextStartElement()||xml.name()!=QLatin1String("svg"))
  throw std::runtime_error("SVG: documento raiz nao e <svg>");
 ImportResult out;
 const auto root=xml.attributes();
 const Matrix viewport=viewportTransform(root,out);
 const Matrix rootTransform=compose(viewport,parseTransform(root.value(QLatin1String("transform")).toString()));
 readChildren(xml,rootTransform,out);
 if(xml.hasError())
  throw std::runtime_error(QString("SVG: XML invalido: %1").arg(xml.errorString()).toStdString());
 if(out.curves.contours.empty())
  throw std::runtime_error("SVG: nenhum contorno vetorial compativel foi encontrado");
 validateCurve(out.curves);
 out.sourceShapes=std::max(out.sourceShapes,out.pathElements);
 return out;
}

} // namespace alaser::svg20ze
