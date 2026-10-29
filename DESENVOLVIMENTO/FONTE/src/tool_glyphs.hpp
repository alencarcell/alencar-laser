#pragma once
// Qt painter glyphs: consistent, high-contrast icons with no external font dependency.
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
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
