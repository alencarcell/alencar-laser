#pragma once
// Qt painter glyphs: consistent, high-contrast icons with no external font dependency.
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <cmath>
#include <QString>
namespace alaser {
inline QIcon toolGlyph(const QString& id){
 QPixmap pix(32,32);pix.fill(Qt::transparent);
 QPainter p(&pix);p.setRenderHint(QPainter::Antialiasing);
 QPen pen(QColor("#DE914C"),2.3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
 p.setPen(pen);p.setBrush(Qt::NoBrush);
 if(id=="select"){
  QPolygonF arrow;arrow<<QPointF(6,4)<<QPointF(9,26)<<QPointF(14,19)<<QPointF(20,27)<<QPointF(24,24)<<QPointF(18,17)<<QPointF(27,15);
  p.drawPolygon(arrow);
 }else if(id=="nodes"){
  p.drawLine(6,23,26,8);p.setBrush(QColor("#DE914C"));
  p.drawRect(4,21,5,5);p.drawRect(23,5,5,5);
 }else if(id=="bezier"){
  QPainterPath path;path.moveTo(5,24);path.cubicTo(5,4,27,28,27,7);p.drawPath(path);
  p.setBrush(QColor("#DE914C"));p.drawEllipse(QPointF(5,24),2.5,2.5);p.drawEllipse(QPointF(27,7),2.5,2.5);
 }else if(id=="new_bezier"){
  QPainterPath path;path.moveTo(5,24);path.cubicTo(11,3,24,27,27,7);
  p.drawPath(path);p.drawLine(5,24,11,5);p.drawLine(27,7,24,27);
  p.setBrush(QColor("#DE914C"));p.drawEllipse(QPointF(5,24),2.2,2.2);
  p.drawEllipse(QPointF(27,7),2.2,2.2);
 }else if(id=="image"){
  p.drawRect(QRectF(4,6,24,21));
  p.drawEllipse(QPointF(11,12),2.2,2.2);
  QPolygonF hills;hills<<QPointF(6,24)<<QPointF(14,16)<<QPointF(18,20)
                        <<QPointF(22,14)<<QPointF(27,24);
  p.drawPolyline(hills);
 }else if(id=="align_left"||id=="align_right"||id=="align_center_x"||
          id=="align_top"||id=="align_bottom"||id=="align_center_y"){
  const bool vertical=id=="align_left"||id=="align_right"||id=="align_center_x";
  if(vertical){
   const int x=id=="align_left"?5:(id=="align_right"?27:16);
   p.setPen(QPen(QColor("#F1B575"),1.5,Qt::DashLine));p.drawLine(x,3,x,29);p.setPen(pen);
   if(id=="align_left"){p.drawLine(6,10,18,10);p.drawLine(6,20,26,20);}
   else if(id=="align_right"){p.drawLine(14,10,26,10);p.drawLine(6,20,26,20);}
   else {p.drawLine(10,10,22,10);p.drawLine(6,20,26,20);}
  }else{
   const int y=id=="align_top"?5:(id=="align_bottom"?27:16);
   p.setPen(QPen(QColor("#F1B575"),1.5,Qt::DashLine));p.drawLine(3,y,29,y);p.setPen(pen);
   if(id=="align_top"){p.drawRect(7,6,8,10);p.drawRect(19,6,8,18);}
   else if(id=="align_bottom"){p.drawRect(7,16,8,10);p.drawRect(19,8,8,18);}
   else {p.drawRect(7,11,8,10);p.drawRect(19,7,8,18);}
  }
 }else if(id=="flip_h"||id=="flip_v"){
  if(id=="flip_h"){
   p.setPen(QPen(QColor("#F1B575"),1.3,Qt::DashLine));p.drawLine(16,3,16,29);p.setPen(pen);
   p.drawPolyline(QPolygonF()<<QPointF(5,9)<<QPointF(13,5)<<QPointF(13,24)<<QPointF(5,9));
   p.drawPolyline(QPolygonF()<<QPointF(27,9)<<QPointF(19,5)<<QPointF(19,24)<<QPointF(27,9));
  }else{
   p.setPen(QPen(QColor("#F1B575"),1.3,Qt::DashLine));p.drawLine(3,16,29,16);p.setPen(pen);
   p.drawPolyline(QPolygonF()<<QPointF(9,5)<<QPointF(5,13)<<QPointF(24,13)<<QPointF(9,5));
   p.drawPolyline(QPolygonF()<<QPointF(9,27)<<QPointF(5,19)<<QPointF(24,19)<<QPointF(9,27));
  }
 }else if(id=="rotate_90"){
  p.drawArc(QRectF(6,5,21,21),30*16,275*16);
  p.drawLine(25,5,28,14);p.drawLine(28,14,20,11);
 }else if(id=="preview"){
  p.drawRect(4,7,24,17);p.drawLine(10,28,22,28);
  p.setBrush(QColor("#DE914C"));p.drawEllipse(QPointF(16,15),4,4);
 }else if(id=="save"){
  p.drawRect(6,5,20,22);p.drawRect(10,6,12,7);p.drawRect(11,20,10,7);
 }else if(id=="open"){
  p.drawPolyline(QPolygonF()<<QPointF(5,12)<<QPointF(5,7)<<QPointF(15,7)<<QPointF(19,11));
  p.drawPolyline(QPolygonF()<<QPointF(5,27)<<QPointF(10,14)<<QPointF(28,14)<<QPointF(23,27)<<QPointF(5,27));
 }else if(id=="undo"||id=="redo"){
  const bool redo=id=="redo";
  p.drawArc(QRectF(7,8,19,17),redo?(-10*16):(10*16),redo?(225*16):(-225*16));
  if(redo){p.drawLine(25,7,28,15);p.drawLine(28,15,19,12);}
  else{p.drawLine(7,7,4,15);p.drawLine(4,15,13,12);}
 }else if(id=="pause"){
  p.drawRect(8,6,5,20);p.drawRect(19,6,5,20);
 }else if(id=="play"){
  p.drawPolygon(QPolygonF()<<QPointF(8,6)<<QPointF(25,16)<<QPointF(8,26));
 }else if(id=="stop"){
  p.drawRect(8,8,16,16);
 }else if(id=="fit"){
  p.drawLine(5,12,5,5);p.drawLine(5,5,12,5);
  p.drawLine(20,5,27,5);p.drawLine(27,5,27,12);
  p.drawLine(5,20,5,27);p.drawLine(5,27,12,27);
  p.drawLine(20,27,27,27);p.drawLine(27,27,27,20);
 }else if(id=="settings"){
  p.drawEllipse(QPointF(16,16),10,10);p.drawEllipse(QPointF(16,16),4,4);
  for(int k=0;k<8;++k){double a=k*3.14159265358979323846/4.;
    p.drawLine(QPointF(16+10*std::cos(a),16+10*std::sin(a)),
               QPointF(16+13*std::cos(a),16+13*std::sin(a)));}
 }else if(id=="scissors"){
  p.drawEllipse(QPointF(9,23),4,4);p.drawEllipse(QPointF(23,23),4,4);
  p.drawLine(11,20,25,5);p.drawLine(21,20,7,5);
 }else if(id=="pencil"){
  p.drawLine(7,24,24,7);p.drawLine(5,27,12,25);
  QPolygonF tip;tip<<QPointF(24,7)<<QPointF(28,11)<<QPointF(12,27)<<QPointF(7,23);
  p.drawPolyline(tip);
 }else if(id=="text"){
  QFont f=p.font();f.setBold(true);f.setPixelSize(25);p.setFont(f);
  p.drawText(QRectF(4,2,24,28),Qt::AlignCenter,"T");
 }else if(id=="line")p.drawLine(5,26,27,5);
 else if(id=="rectangle")p.drawRect(QRectF(6,7,20,18));
 else if(id=="ellipse")p.drawEllipse(QRectF(5,7,22,18));
 else if(id=="polygon"){
  QPolygonF poly;poly<<QPointF(16,4)<<QPointF(28,13)<<QPointF(23,27)<<QPointF(9,27)<<QPointF(4,13);p.drawPolygon(poly);
 }
 p.end();return QIcon(pix);
}
}
