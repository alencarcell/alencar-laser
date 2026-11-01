#pragma once
// Alencar Laser 19.9: gerador de engrenagem somente editorial/offline.
#include "gear_generator_19_9.hpp"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPainter>
#include <QWidget>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainterPath>
#include <QPolygonF>
#include <QPushButton>
#include <QPen>
#include <cmath>

namespace alaser::gear {
class GearPreview final:public QWidget{
 Parameters current_{};
 Result generated_{};
 bool valid_=false;
public:
 explicit GearPreview(QWidget* p=nullptr):QWidget(p){
  setMinimumHeight(220);setObjectName("gear_preview_19_9");
  setToolTip("Prévia de engrenagem evolvente aproximada, não é simulação de corte.");
 }
 void setGear(const Parameters& params){
  current_=params;valid_=false;
  try{generated_=generate(params);valid_=true;}catch(const std::exception&){}
  update();
 }
protected:
 void paintEvent(QPaintEvent*)override{
  QPainter p(this);p.setRenderHint(QPainter::Antialiasing,true);
  p.fillRect(rect(),QColor("#1B252E"));
  p.setPen(QPen(QColor("#44596A"),1));
  p.drawRect(rect().adjusted(1,1,-2,-2));
  if(!valid_)return;
  const QPointF center=rect().center();
  const double scale=.45*std::min(width(),height())/generated_.outerRadius;
  auto shape=[&](const std::vector<Point>& poly)->QPolygonF{
   QPolygonF q;q.reserve(int(poly.size()));
   for(const auto& v:poly)q<<QPointF(center.x()+scale*v.x,center.y()-scale*v.y);
   return q;
  };
  QPainterPath path;
  path.setFillRule(Qt::OddEvenFill);
  path.addPolygon(shape(generated_.outer));path.closeSubpath();
  if(!generated_.bore.empty()){
   path.addPolygon(shape(generated_.bore));path.closeSubpath();
  }
  p.setBrush(QColor("#344756"));p.setPen(QPen(QColor("#F6A247"),1.8));
  p.drawPath(path);
  p.setBrush(Qt::NoBrush);
  p.setPen(QPen(QColor("#75A2C0"),1,Qt::DashLine));
  p.drawEllipse(center,generated_.pitchRadius*scale,generated_.pitchRadius*scale);
 }
};

class GearDialog final:public QDialog{
 QSpinBox* teeth_=nullptr;
 QDoubleSpinBox *module_=nullptr,*pressure_=nullptr,*bore_=nullptr,*centerX_=nullptr,*centerY_=nullptr;
 QLabel* info_=nullptr;
 GearPreview* preview_=nullptr;
 Parameters current()const{
  Parameters p;
  p.teeth=teeth_->value();p.moduleMm=module_->value();
  p.pressureDeg=pressure_->value();p.boreMm=bore_->value();return p;
 }
 void refresh(){
  const auto p=current();
  preview_->setGear(p);
  try{
   const auto shape=generate(p);
   QString message=QString("Diâmetro externo %1 mm  ·  diâmetro primitivo %2 mm  ·  raiz %3 mm\n"
     "Contorno: %4 nós  ·  Furo: %5 nós")
     .arg(2*shape.outerRadius,0,'f',2).arg(2*shape.pitchRadius,0,'f',2)
     .arg(2*shape.rootRadius,0,'f',2).arg(shape.outer.size()).arg(shape.bore.size());
   if(shape.undercutWarning)message+="\nAviso: poucas dentes podem gerar interferência/undercut.";
   if(centerX_->value()-shape.outerRadius<0||centerY_->value()-shape.outerRadius<0||
      centerX_->value()+shape.outerRadius>bedW_||
      centerY_->value()+shape.outerRadius>bedH_)message+="\nAtenção: desenho ultrapassa a área configurada.";
   message+="\nCorte mecânico requer medição de folga e compensação de kerf.";
   info_->setText(message);
  }catch(const std::exception& e){info_->setText(QString("Parâmetros inválidos: ")+e.what());}
 }
 double bedW_=400.,bedH_=400.;
public:
 explicit GearDialog(double bedW,double bedH,QWidget* parent=nullptr):QDialog(parent),
  bedW_(bedW),bedH_(bedH){
  setObjectName("gear_generator_dialog_19_9");
  setWindowTitle("Gerador de Engrenagens — Alencar Laser");
  setMinimumSize(520,585);resize(580,645);
  auto* column=new QVBoxLayout(this);
  auto* hint=new QLabel("GERAÇÃO VETORIAL OFFLINE • sem motor, movimento ou emissão de laser.",this);
  hint->setWordWrap(true);column->addWidget(hint);
  auto* form=new QFormLayout;
  teeth_=new QSpinBox(this);teeth_->setObjectName("gear_teeth_19_9");
  teeth_->setRange(12,120);teeth_->setValue(24);
  form->addRow("Quantidade de dentes:",teeth_);
  auto metric=[&](const char* key,double min,double max,double value,int decimals,const QString& suffix){
   auto* f=new QDoubleSpinBox(this);f->setObjectName(key);f->setRange(min,max);
   f->setDecimals(decimals);f->setSingleStep(.5);f->setValue(value);f->setSuffix(suffix);return f;
  };
  module_=metric("gear_module_19_9",.5,20.,3.,2," mm");
  module_->setToolTip("Módulo métrico: diâmetro primitivo = módulo × número de dentes.");
  form->addRow("Módulo:",module_);
  pressure_=metric("gear_pressure_19_9",14.,30.,20.,1,"°");
  pressure_->setToolTip("Ângulo de pressão do perfil evolvente aproximado.");
  form->addRow("Ângulo de pressão:",pressure_);
  bore_=metric("gear_bore_19_9",0.,2000.,6.,2," mm");
  form->addRow("Furo central (0 = sem furo):",bore_);
  centerX_=metric("gear_center_x_19_9",0.,10000.,bedW/2.,1," mm");
  centerY_=metric("gear_center_y_19_9",0.,10000.,bedH/2.,1," mm");
  form->addRow("Centro X:",centerX_);form->addRow("Centro Y:",centerY_);
  column->addLayout(form);
  preview_=new GearPreview(this);column->addWidget(preview_);
  info_=new QLabel(this);info_->setObjectName("gear_summary_19_9");
  info_->setWordWrap(true);info_->setMinimumHeight(80);column->addWidget(info_);
  auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);
  buttons->button(QDialogButtonBox::Ok)->setText("Criar contornos no desenho");
  buttons->button(QDialogButtonBox::Cancel)->setText("Cancelar");
  column->addWidget(buttons);
  connect(buttons,&QDialogButtonBox::accepted,this,&GearDialog::accept);
  connect(buttons,&QDialogButtonBox::rejected,this,&GearDialog::reject);
  connect(teeth_,QOverload<int>::of(&QSpinBox::valueChanged),this,[this](int){refresh();});
  for(auto* f:{module_,pressure_,bore_,centerX_,centerY_})
   connect(f,QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,
           [this](double){refresh();});
  refresh();
 }
 Parameters parameters()const{return current();}
 Point centerMm()const{return {centerX_->value(),centerY_->value()};}
 void accept()override{
  try{(void)generate(current());}
  catch(const std::exception& e){
   QMessageBox::warning(this,"Engrenagem inválida",e.what());return;
  }
  QDialog::accept();
 }
};
} // namespace alaser::gear
