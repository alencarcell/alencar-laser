#pragma once
// Iconografia proprietaria ALENCAR: glifos 32x32, 2x DPI, completos e contrastantes.
// Sem fontes externas, emojis, capturas ou desenhos copiados do LightBurn.
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QFont>
#include <QColor>
#include <cmath>
#include <array>
namespace alaser {
inline QPixmap renderAlencarGlyph(const QString& id,bool disabled=false){
 QPixmap pix(64,64);pix.setDevicePixelRatio(2.);pix.fill(Qt::transparent);
 QPainter p(&pix);p.setRenderHint(QPainter::Antialiasing,true);p.scale(2.,2.);
 const QColor fg=disabled?QColor("#A6AFB8"):QColor("#F3F4F6");
 const QColor accent=disabled?QColor("#969DA5"):QColor("#FFAB55");
 const QPen normal(fg,2.1,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
 p.setPen(normal);p.setBrush(Qt::NoBrush);
 auto line=[&](double x,double y,double xx,double yy){p.drawLine(QPointF(x,y),QPointF(xx,yy));};
 auto rect=[&](double x,double y,double w,double h){p.drawRoundedRect(QRectF(x,y,w,h),1.,1.);};
 auto square=[&](double x,double y){
  p.setBrush(accent);p.drawRect(QRectF(x-2.2,y-2.2,4.4,4.4));p.setBrush(Qt::NoBrush);
 };
 if(id=="select"){
  QPolygonF arrow;arrow<<QPointF(7,4)<<QPointF(8,27)<<QPointF(14,21)<<QPointF(19,29)
   <<QPointF(22,27)<<QPointF(17,19)<<QPointF(26,17);
  p.setBrush(accent);p.drawPolygon(arrow);
 }else if(id=="nodes"){
  p.drawLine(QPointF(6,25),QPointF(25,7));square(6,25);square(25,7);square(15,16);
 }else if(id=="bezier"){
  QPainterPath c;c.moveTo(5,23);c.cubicTo(5,5,26,27,27,8);p.drawPath(c);
  p.setPen(QPen(accent,1.3,Qt::DashLine));line(5,23,7,8);line(27,8,24,24);
  p.setPen(normal);square(5,23);square(27,8);
 }else if(id=="new_bezier"){
  QPainterPath c;c.moveTo(4,25);c.lineTo(13,15);c.cubicTo(20,2,23,25,28,7);p.drawPath(c);
  square(4,25);square(13,15);square(28,7);
 }else if(id=="scissors"){
  p.setPen(QPen(accent,2.2,Qt::SolidLine,Qt::RoundCap));
  p.drawEllipse(QPointF(9,25),4,4);p.drawEllipse(QPointF(23,25),4,4);
  p.setPen(normal);line(12,21,26,5);line(20,21,6,5);
 }else if(id=="line"){
  line(6,25,26,7);square(6,25);square(26,7);
 }else if(id=="rectangle"){
  rect(5,7,22,18);p.setPen(QPen(accent,2.));line(5,7,27,7);
 }else if(id=="ellipse"){
  p.drawEllipse(QRectF(4,7,24,18));
  p.setPen(QPen(accent,2.));p.drawArc(QRectF(4,7,24,18),0,115*16);
 }else if(id=="polygon"){
  QPolygonF a;a<<QPointF(16,4)<<QPointF(28,13)<<QPointF(24,27)
   <<QPointF(8,27)<<QPointF(4,13);p.drawPolygon(a);square(16,4);
 }else if(id=="pencil"){
  p.save();p.translate(16,16);p.rotate(45);
  p.drawRoundedRect(QRectF(-3,-12,6,22),1,1);
  p.setBrush(accent);p.drawPolygon(QPolygonF()<<QPointF(-3,10)<<QPointF(3,10)<<QPointF(0,15));
  p.restore();
 }else if(id=="text"){
  QFont font=p.font();font.setBold(true);font.setPixelSize(26);p.setFont(font);
  p.setPen(fg);p.drawText(QRectF(2,1,28,30),Qt::AlignCenter,"T");
  p.setPen(QPen(accent,1.5));line(7,28,25,28);
 }else if(id=="image"){
  rect(4,5,24,23);p.setBrush(accent);p.drawEllipse(QPointF(11,12),2.4,2.4);
  p.setBrush(Qt::NoBrush);p.drawPolyline(QPolygonF()<<QPointF(5,26)<<QPointF(13,17)
    <<QPointF(18,22)<<QPointF(23,15)<<QPointF(27,22));
 }else if(id.startsWith("align_")){
  if(id=="align_both"){
   p.setPen(QPen(accent,1.3,Qt::DashLine));line(16,3,16,29);line(3,16,29,16);
   p.setPen(normal);rect(6,10,11,12);rect(14,7,11,17);
  }else{
   const bool vertical=id=="align_left"||id=="align_right"||id=="align_center_x";
   const double axis=vertical?(id=="align_left"?6:id=="align_right"?26:16):
     (id=="align_top"?6:id=="align_bottom"?26:16);
   p.setPen(QPen(accent,1.2,Qt::DashLine));
   if(vertical)line(axis,3,axis,29);else line(3,axis,29,axis);
   p.setPen(normal);
   if(id=="align_left"){rect(7,8,11,5);rect(7,19,19,5);}
   else if(id=="align_right"){rect(15,8,11,5);rect(7,19,19,5);}
   else if(id=="align_center_x"){rect(11,8,10,5);rect(6,19,20,5);}
   else if(id=="align_top"){rect(8,7,5,11);rect(19,7,5,19);}
   else if(id=="align_bottom"){rect(8,15,5,11);rect(19,7,5,19);}
   else {rect(8,11,5,10);rect(19,6,5,20);}
  }
 }else if(id=="center_bed"){
  rect(3,3,26,26);p.setPen(QPen(accent,1.3,Qt::DashLine));
  line(16,4,16,28);line(4,16,28,16);p.setPen(normal);rect(12,12,8,8);
 }else if(id=="flip_h"||id=="flip_v"){
  if(id=="flip_h"){
   p.setPen(QPen(accent,1.3,Qt::DashLine));line(16,3,16,29);p.setPen(normal);
   p.drawPolygon(QPolygonF()<<QPointF(5,16)<<QPointF(13,7)<<QPointF(13,25));
   p.drawPolygon(QPolygonF()<<QPointF(27,16)<<QPointF(19,7)<<QPointF(19,25));
  }else{
   p.setPen(QPen(accent,1.3,Qt::DashLine));line(3,16,29,16);p.setPen(normal);
   p.drawPolygon(QPolygonF()<<QPointF(16,5)<<QPointF(7,13)<<QPointF(25,13));
   p.drawPolygon(QPolygonF()<<QPointF(16,27)<<QPointF(7,19)<<QPointF(25,19));
  }
 }else if(id=="rotate_90"){
  p.drawArc(QRectF(5,5,22,22),55*16,265*16);
  p.setBrush(accent);p.drawPolygon(QPolygonF()<<QPointF(28,15)<<QPointF(20,13)<<QPointF(24,21));
 }else if(id=="preview"){
  p.drawEllipse(QRectF(3,9,26,14));
  p.setBrush(accent);p.drawEllipse(QPointF(16,16),4.2,4.2);
 }else if(id=="open"||id=="open_other"){
  p.drawPolyline(QPolygonF()<<QPointF(4,13)<<QPointF(4,7)<<QPointF(14,7)<<QPointF(19,12));
  p.drawPolygon(QPolygonF()<<QPointF(4,14)<<QPointF(28,14)<<QPointF(24,27)<<QPointF(5,27));
  if(id=="open_other"){p.setPen(QPen(accent,2.));line(22,4,22,12);line(18,8,26,8);}
 }else if(id=="save"||id=="save_as"){
  rect(6,4,20,24);rect(10,5,12,9);rect(11,20,10,8);
  p.setPen(QPen(accent,2.));line(11,20,21,20);
  if(id=="save_as"){line(26,20,26,28);line(22,24,30,24);}
 }else if(id=="undo"||id=="redo"){
  const bool redo=id=="redo";
  p.drawArc(QRectF(6,7,20,20),redo?(-30*16):(30*16),
            redo?(245*16):(-245*16));
  p.setBrush(accent);
  if(redo)p.drawPolygon(QPolygonF()<<QPointF(27,8)<<QPointF(18,8)<<QPointF(25,16));
  else p.drawPolygon(QPolygonF()<<QPointF(5,8)<<QPointF(14,8)<<QPointF(7,16));
 }else if(id=="play"){
  p.setPen(QPen(accent,2.));p.setBrush(accent);
  p.drawPolygon(QPolygonF()<<QPointF(8,5)<<QPointF(27,16)<<QPointF(8,27));
 }else if(id=="pause"){
  p.setPen(Qt::NoPen);p.setBrush(accent);
  p.drawRoundedRect(QRectF(7,5,7,22),1,1);
  p.drawRoundedRect(QRectF(19,5,7,22),1,1);
 }else if(id=="stop"){
  p.setPen(Qt::NoPen);p.setBrush(accent);p.drawRoundedRect(QRectF(7,7,18,18),2,2);
 }else if(id=="fit"){
  for(const auto& lineSeg:std::array<std::array<double,4>,8>{
   {{4,11,4,4}},{{4,4,11,4}},{{21,4,28,4}},{{28,4,28,11}},
   {{4,21,4,28}},{{4,28,11,28}},{{21,28,28,28}},{{28,21,28,28}}})
   line(lineSeg[0],lineSeg[1],lineSeg[2],lineSeg[3]);
 }else if(id=="settings"||id=="mechanism"){
  const double pi=3.14159265358979323846;
  QPolygonF gear;
  for(int i=0;i<24;++i){
   const double theta=i*pi/12.;
   const double radius=(i%3==0||i%3==1)?13.:10.;
   gear<<QPointF(16+radius*std::cos(theta),16+radius*std::sin(theta));
  }
  p.drawPolygon(gear);p.setBrush(accent);p.drawEllipse(QPointF(16,16),4.5,4.5);
 }else{
  // Icone nao implementado NUNCA vira caixa vazia/fragmento incompreensivel.
  p.drawRoundedRect(QRectF(4,4,24,24),4,4);
  p.setPen(QPen(accent,2.));line(11,11,21,21);line(21,11,11,21);
 }
 p.end();return pix;
}
inline QIcon toolGlyph(const QString& id){
 QIcon icon;icon.addPixmap(renderAlencarGlyph(id,false),QIcon::Normal,QIcon::Off);
 icon.addPixmap(renderAlencarGlyph(id,true),QIcon::Disabled,QIcon::Off);
 return icon;
}
} // namespace alaser
