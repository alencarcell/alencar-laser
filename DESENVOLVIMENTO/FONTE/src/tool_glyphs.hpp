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
#include <QStringList>
#include <QImage>
#include <QSize>
#include <QtGlobal>
namespace alaser {
inline QPixmap renderAlencarGlyph(const QString& id,bool disabled=false,bool active=false){
 QPixmap pix(64,64);pix.setDevicePixelRatio(2.);pix.fill(Qt::transparent);
 // DPR=2 ja converte coordenadas logicas de 32x32 em 64x64 pixels.\n // Nao duplicar via p.scale(2,2): isso recorta os icones no quadrante.\n QPainter p(&pix);p.setRenderHint(QPainter::Antialiasing,true);
 const QColor fg=disabled?QColor("#B7C0C8"):(active?QColor("#FFF4E8"):QColor("#F3F4F6"));
 const QColor accent=disabled?QColor("#B7C0C8"):(active?QColor("#FF810E"):QColor("#FFAB55"));
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
   // Original do proprietário: alvo com centro comum, não dois retângulos sobrepostos.
   p.drawEllipse(QRectF(7,7,18,18));
   line(16,3,16,11);line(16,21,16,29);
   line(3,16,11,16);line(21,16,29,16);
   p.setBrush(accent);p.setPen(Qt::NoPen);p.drawEllipse(QPointF(16,16),3,3);
   p.setPen(normal);p.setBrush(Qt::NoBrush);
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
 }else if(id=="flip_h"||id=="flip_v"||id=="mirror_axis"){
  // Espelhar H: metades esquerda/direita | espelhar V: metades acima/abaixo.
  // NÃO inverter a orientação indicada nos desenhos manuscritos.
  if(id=="flip_h"){
   p.setPen(QPen(accent,1.4,Qt::DashLine));line(16,3,16,29);p.setPen(normal);
   p.drawPolygon(QPolygonF()<<QPointF(5,16)<<QPointF(13,7)<<QPointF(13,25));
   p.setBrush(accent);
   p.drawPolygon(QPolygonF()<<QPointF(27,16)<<QPointF(19,7)<<QPointF(19,25));
  }else if(id=="flip_v"){
   p.setPen(QPen(accent,1.4,Qt::DashLine));line(3,16,29,16);p.setPen(normal);
   p.drawPolygon(QPolygonF()<<QPointF(16,5)<<QPointF(7,13)<<QPointF(25,13));
   p.setBrush(accent);
   p.drawPolygon(QPolygonF()<<QPointF(16,27)<<QPointF(7,19)<<QPointF(25,19));
  }else{
   // Reflexão sobre linha inclinada: forma e sua cópia de lados opostos do eixo.
   p.setPen(QPen(accent,1.4,Qt::DashLine));line(4,28,28,4);p.setPen(normal);
   p.drawPolygon(QPolygonF()<<QPointF(5,8)<<QPointF(14,5)<<QPointF(11,16));
   p.setBrush(accent);
   p.drawPolygon(QPolygonF()<<QPointF(17,26)<<QPointF(27,21)<<QPointF(21,17));
  }
  p.setBrush(Qt::NoBrush);
 }else if(id=="rotate_90"){
  p.drawArc(QRectF(5,5,22,22),55*16,265*16);
  p.setBrush(accent);p.drawPolygon(QPolygonF()<<QPointF(28,15)<<QPointF(20,13)<<QPointF(24,21));
 }else if(id=="preview"){
  // Desenho manuscrito: monitor com olho central.
  rect(3.5,4,25,19);line(16,23,16,28);line(10,28,22,28);
  QPainterPath eye;eye.moveTo(7,14);eye.quadTo(16,6,25,14);eye.quadTo(16,22,7,14);
  p.drawPath(eye);p.setBrush(accent);p.drawEllipse(QPointF(16,14),2.9,2.9);
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
  line(4,11,4,4);line(4,4,11,4);
  line(21,4,28,4);line(28,4,28,11);
  line(4,21,4,28);line(4,28,11,28);
  line(21,28,28,28);line(28,21,28,28);
 }else if(id=="settings"||id=="mechanism"||id=="gear_simple"){
  const double pi=3.14159265358979323846;
  QPolygonF gear;
  for(int i=0;i<24;++i){
   const double theta=i*pi/12.;
   const double radius=(i%3==0||i%3==1)?13.:10.;
   gear<<QPointF(16+radius*std::cos(theta),16+radius*std::sin(theta));
  }
  p.drawPolygon(gear);p.setBrush(accent);p.drawEllipse(QPointF(16,16),4.5,4.5);
 
 }else if(id=="new"){
  // Folha com dobra e sinal de adição no canto inferior direito.
  p.drawPolygon(QPolygonF()<<QPointF(5,3)<<QPointF(20,3)<<QPointF(26,9)
                 <<QPointF(26,21)<<QPointF(5,21));
  line(20,3,20,9);line(20,9,26,9);
  p.setPen(QPen(accent,2.2,Qt::SolidLine,Qt::RoundCap));p.setBrush(QColor("#1A2731"));
  p.drawEllipse(QPointF(23.5,23.5),5.5,5.5);line(23.5,20.3,23.5,26.7);line(20.3,23.5,26.7,23.5);
 }else if(id=="import"){
  p.drawPolygon(QPolygonF()<<QPointF(5,3)<<QPointF(20,3)<<QPointF(25,8)
                  <<QPointF(25,18)<<QPointF(5,18));
  line(20,3,20,8);line(20,8,25,8);
  p.setPen(QPen(accent,2.3,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
  line(13,24,27,24);line(22,19,27,24);line(22,29,27,24);
 }else if(id=="device_settings"){
  // Chave e ferramenta cruzadas, visualmente diferente de configuração geral.
  p.setPen(QPen(accent,2.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
  line(7,25,24,8);line(8,8,24,24);
  p.setPen(normal);
  p.drawEllipse(QPointF(7,25),3,3);
  p.drawPolyline(QPolygonF()<<QPointF(20,5)<<QPointF(24,9)<<QPointF(28,5));
 }else if(id=="distribute_vertical"||id=="distribute_horizontal"){
  if(id=="distribute_vertical"){
   rect(5,4,22,5);rect(5,23,22,5);
   p.setPen(QPen(accent,2.1,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
   line(16,10,16,22);
   line(12,14,16,10);line(20,14,16,10);
   line(12,18,16,22);line(20,18,16,22);
  }else{
   rect(4,5,5,22);rect(23,5,5,22);
   p.setPen(QPen(accent,2.1,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
   line(10,16,22,16);
   line(14,12,10,16);line(14,20,10,16);
   line(18,12,22,16);line(18,20,22,16);
  }
 }else if(id=="same_width"||id=="same_height"){
  if(id=="same_width"){
   rect(4,5,24,5);rect(8,17,16,5);
   p.setPen(QPen(accent,2.1,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
   line(4,27,28,27);line(4,27,8,24);line(4,27,8,30);line(28,27,24,24);line(28,27,24,30);
  }else{
   rect(5,4,7,24);rect(20,8,7,16);
   p.setPen(QPen(accent,2.1,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
   line(16,4,16,28);line(16,4,13,8);line(16,4,19,8);line(16,28,13,24);line(16,28,19,24);
  }
 }else if(id=="rotary_config"){
  p.drawArc(QRectF(6,6,20,20),40*16,130*16);
  p.drawArc(QRectF(6,6,20,20),220*16,130*16);
  p.setBrush(accent);p.setPen(Qt::NoPen);
  p.drawPolygon(QPolygonF()<<QPointF(8,5)<<QPointF(16,7)<<QPointF(10,13));
  p.drawPolygon(QPolygonF()<<QPointF(24,27)<<QPointF(16,25)<<QPointF(22,19));
  p.drawEllipse(QPointF(16,16),3,3);
 }else if(id=="home"){
  p.drawPolyline(QPolygonF()<<QPointF(3,15)<<QPointF(16,4)<<QPointF(29,15));
  p.drawPolyline(QPolygonF()<<QPointF(7,13)<<QPointF(7,28)<<QPointF(25,28)<<QPointF(25,13));
  p.setBrush(accent);p.drawRect(QRectF(13,19,6,9));
 }else if(id=="triangle"||id=="pentagon"||id=="hexagon"||id=="octagon"||id=="star"){
  int n=id=="triangle"?3:id=="pentagon"?5:id=="hexagon"?6:id=="octagon"?8:5;
  QPolygonF poly;
  const double pi=3.14159265358979323846;
  for(int i=0;i<(id=="star"?10:n);++i){
   const double a=-pi/2.+i*(2*pi/(id=="star"?10:n));
   const double rad=id=="star"?(i%2==0?13.:6.):13.;
   poly<<QPointF(16+rad*std::cos(a),16+rad*std::sin(a));
  }
  if(id=="star")p.setBrush(accent);
  p.drawPolygon(poly);
 }else if(id=="edit_nodes"){
  QPolygonF shape;shape<<QPointF(16,4)<<QPointF(27,11)<<QPointF(24,27)
    <<QPointF(8,27)<<QPointF(5,11);p.drawPolygon(shape);
  for(const auto& q:shape)square(q.x(),q.y());
 }else if(id=="add_node"||id=="remove_node"||id=="delete_edge"||
          id=="smooth_node"||id=="corner_node"||
          id=="line_to_curve"||id=="curve_to_line"||id=="axis_align"){
  QPainterPath path;
  if(id=="smooth_node"||id=="line_to_curve"){
   path.moveTo(4,25);path.cubicTo(8,2,23,29,28,7);
  }else if(id=="curve_to_line"||id=="corner_node"){
   path.moveTo(4,25);path.lineTo(16,10);path.lineTo(28,25);
  }else{
   path.moveTo(4,23);path.lineTo(16,12);path.lineTo(28,23);
  }
  p.drawPath(path);
  square(4,23);square(28,23);
  if(id=="axis_align"){
   p.setPen(QPen(accent,1.5,Qt::DashLine));line(16,3,16,29);
   p.setPen(normal);square(16,12);
  }else if(id=="smooth_node"||id=="corner_node"){
   p.setPen(QPen(accent,1.6));square(16,12);
  }else if(id=="line_to_curve"||id=="curve_to_line"){
   p.setPen(QPen(accent,2.));line(21,25,28,25);line(25,22,28,25);line(25,28,28,25);
  }else{
   p.setPen(QPen(accent,2.4,Qt::SolidLine,Qt::RoundCap));square(16,12);
   if(id=="add_node"||id=="remove_node"){
    p.setBrush(QColor("#1C2835"));p.drawEllipse(QPointF(25,7),6.5,6.5);
    line(21,7,29,7);if(id=="add_node")line(25,3,25,11);
   }else{line(22,3,29,10);line(29,3,22,10);}
  }
 }else if(id.startsWith("move_")){
  const QStringList dirs={"nw","n","ne","w","e","sw","s","se"};
  const QString dir=id.mid(5);
  const int idx=dirs.indexOf(dir);
  if(idx>=0){
   const QPolygonF arrows={
    QPointF(16,3),QPointF(11,9),QPointF(14,9),
    QPointF(14,25),QPointF(18,25),QPointF(18,9),QPointF(21,9)
   };
   p.save();p.translate(16,16);
   double deg=0.;
   if(dir=="ne")deg=45.;else if(dir=="e")deg=90.;
   else if(dir=="se")deg=135.;else if(dir=="s")deg=180.;
   else if(dir=="sw")deg=225.;else if(dir=="w")deg=270.;
   else if(dir=="nw")deg=315.;
   p.rotate(deg);p.translate(-16,-16);
   p.setPen(QPen(accent,1.5));p.setBrush(accent);p.drawPolygon(arrows);p.restore();
  }
}else{
  // Icone nao implementado NUNCA vira caixa vazia/fragmento incompreensivel.
  p.drawRoundedRect(QRectF(4,4,24,24),4,4);
  p.setPen(QPen(accent,2.));line(11,11,21,21);line(21,11,11,21);
 }
 p.end();return pix;
}
inline QIcon toolGlyph(const QString& id){
 QIcon icon;
 icon.addPixmap(renderAlencarGlyph(id,false),QIcon::Normal,QIcon::Off);
 icon.addPixmap(renderAlencarGlyph(id,true),QIcon::Disabled,QIcon::Off);
 icon.addPixmap(renderAlencarGlyph(id,false,true),QIcon::Active,QIcon::Off);
 icon.addPixmap(renderAlencarGlyph(id,false,true),QIcon::Selected,QIcon::On);
 return icon;
}
inline const QStringList& alencarSketchIconIds(){
 static const QStringList ids={
  "new","open","save","import","undo","redo","preview","settings","device_settings",
  "flip_v","flip_h","mirror_axis","align_both","align_center_y","align_center_x",
  "distribute_vertical","distribute_horizontal","same_width","same_height",
  "rotary_config","home","select","line","rectangle","ellipse","triangle",
  "pentagon","hexagon","octagon","star","edit_nodes","add_node","remove_node",
  "delete_edge","smooth_node","corner_node","line_to_curve","curve_to_line",
  "axis_align","move_nw","move_n","move_ne","move_w","home","move_e",
  "move_sw","move_s","move_se","play","pause","stop","fit",
  "open_other","save_as","nodes","bezier","new_bezier","scissors","image",
  "align_left","align_right","align_top","align_bottom","center_bed",
  "flip_h","flip_v","rotate_90","mechanism","text","pencil"};
 return ids;
}
} // namespace alaser
