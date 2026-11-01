#pragma once
// Alencar Laser 19.9 — pares e cremalheiras NOMINAIS para desenho offline.
// Nao habilita CAM, G-code, transporte, emissao nem movimento fisico.
#include "gear_generator_19_9.hpp"
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPolygonF>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QPaintEvent>
#include <utility>
#include <QString>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace alaser::gear {
enum class CompoundKind { Pair, Rack };

class CompoundGearPreview final:public QWidget{
 std::vector<std::vector<std::vector<Point>>> components_;
public:
 explicit CompoundGearPreview(QWidget* parent=nullptr):QWidget(parent){
  setMinimumHeight(240);
  setObjectName("gear_compound_preview_19_9");
  setToolTip("Geometria nominal. Posicao dos dentes, folgas e kerf nao foram homologados.");
 }
 void setContours(std::vector<std::vector<std::vector<Point>>> components){
  components_=std::move(components);update();
 }
protected:
 void paintEvent(QPaintEvent*)override{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing,true);
  painter.fillRect(rect(),QColor("#19242E"));
  painter.setPen(QPen(QColor("#4B6170"),1));
  painter.drawRect(rect().adjusted(1,1,-2,-2));
  if(components_.empty())return;
  double minX=std::numeric_limits<double>::infinity(),maxX=-minX;
  double minY=minX,maxY=-minX;
  for(const auto& group:components_)
   for(const auto& polygon:group)
    for(const auto& v:polygon){
     minX=std::min(minX,v.x);maxX=std::max(maxX,v.x);
     minY=std::min(minY,v.y);maxY=std::max(maxY,v.y);
    }
  if(!std::isfinite(minX)||maxX<=minX||!std::isfinite(minY)||maxY<=minY)return;
  const double scale=std::min(double(width()-32)/(maxX-minX),
                              double(height()-32)/(maxY-minY));
  const double midX=(minX+maxX)/2.,midY=(minY+maxY)/2.;
  auto map=[&](Point v)->QPointF{
   return {width()/2.+(v.x-midX)*scale,height()/2.-(v.y-midY)*scale};
  };
  const QColor pens[2]={QColor("#F5A145"),QColor("#78B2D4")};
  const QColor fills[2]={QColor("#384858"),QColor("#263B49")};
  for(size_t i=0;i<components_.size();++i){
   QPainterPath path;path.setFillRule(Qt::OddEvenFill);
   for(const auto& poly:components_[i]){
    if(poly.size()<3)continue;
    QPolygonF polygon;
    polygon.reserve(int(poly.size()));
    for(const auto& v:poly)polygon<<map(v);
    path.addPolygon(polygon);path.closeSubpath();
   }
   painter.setPen(QPen(pens[i%2],1.6));
   painter.setBrush(fills[i%2]);
   painter.drawPath(path);
  }
 }
};

class CompoundGearDialog final:public QDialog{
 CompoundKind kind_;
 QSpinBox* teeth_=nullptr;
 QSpinBox* otherTeeth_=nullptr;
 QDoubleSpinBox *module_=nullptr,*pressure_=nullptr,*bore_=nullptr;
 QDoubleSpinBox *otherBore_=nullptr,*rackBase_=nullptr,*x_=nullptr,*y_=nullptr;
 CompoundGearPreview* preview_=nullptr;
 QLabel* summary_=nullptr;
 double bedW_=400.,bedH_=400.;
 static void shift(std::vector<Point>& poly,double dx,double dy){
  for(auto& p:poly){p.x+=dx;p.y+=dy;}
 }
 static QDoubleSpinBox* number(QWidget* parent,const char* id,
                 double low,double high,double value,int decimals=3){
  auto* box=new QDoubleSpinBox(parent);
  box->setObjectName(id);box->setDecimals(decimals);
  box->setRange(low,high);box->setValue(value);
  box->setSuffix(" mm");return box;
 }
 void refresh(){
  try{
   if(kind_==CompoundKind::Pair){
    const auto result=generatePairNominal(pairParameters());
    std::vector<Point> first=result.driver.outer,firstHole=result.driver.bore;
    std::vector<Point> second=result.driven.outer,secondHole=result.driven.bore;
    shift(second,result.centerDistanceMm,0.);
    shift(secondHole,result.centerDistanceMm,0.);
    preview_->setContours({{std::move(first),std::move(firstHole)},
                           {std::move(second),std::move(secondHole)}});
    summary_->setText(QString("Centros: %1 mm   |   Razão nominal: %2 : 1\n"
       "Diâmetros primitivos: %3 / %4 mm\n"
       "Sem fase angular, folga ou validação de engrenamento.")
      .arg(result.centerDistanceMm,0,'f',3)
      .arg(result.ratio,0,'f',3)
      .arg(2*result.driver.pitchRadius,0,'f',2)
      .arg(2*result.driven.pitchRadius,0,'f',2));
   }else{
    const auto result=generateRackNominal(rackParameters());
    preview_->setContours({{result.outline}});
    summary_->setText(QString("Comprimento nominal: %1 mm   |   Altura: %2 mm\n"
       "Passo circular: %3 mm\n"
       "Dentes teóricos; não certifica engrenamento.")
      .arg(result.widthMm,0,'f',2)
      .arg(result.heightMm,0,'f',2)
      .arg(result.circularPitchMm,0,'f',3));
   }
  }catch(const std::exception& e){
   preview_->setContours({});
   summary_->setText(QString("Parâmetros inválidos: ")+e.what());
  }
 }
public:
 explicit CompoundGearDialog(CompoundKind kind,double bedW,double bedH,
                              QWidget* parent=nullptr)
   :QDialog(parent),kind_(kind),bedW_(bedW),bedH_(bedH){
  const bool pair=kind_==CompoundKind::Pair;
  setObjectName(pair?"gear_pair_dialog_19_9":"gear_rack_dialog_19_9");
  setWindowTitle(pair?"Par de engrenagens — geometria nominal":
                      "Cremalheira — geometria nominal");
  setMinimumSize(530,530);resize(610,620);
  auto* layout=new QVBoxLayout(this);
  auto* warning=new QLabel(
   "EDITORIAL / OFFLINE. Sem G-code ou envio ao laser. "
   "Geometria aproximada: faça testes dimensionais antes de uso mecânico.",this);
  warning->setWordWrap(true);layout->addWidget(warning);
  auto* form=new QFormLayout;
  teeth_=new QSpinBox(this);
  teeth_->setObjectName("gear_compound_teeth_19_9");
  teeth_->setRange(pair?12:1,120);teeth_->setValue(pair?19:10);
  form->addRow(pair?"Dentes do pinhão motor:":"Dentes da cremalheira:",teeth_);
  if(pair){
   otherTeeth_=new QSpinBox(this);
   otherTeeth_->setObjectName("gear_pair_driven_teeth_19_9");
   otherTeeth_->setRange(12,120);otherTeeth_->setValue(38);
   form->addRow("Dentes do pinhão movido:",otherTeeth_);
  }
  module_=number(this,"gear_compound_module_19_9",0.5,20.,3.,5);
  form->addRow("Módulo comum:",module_);
  pressure_=number(this,"gear_compound_pressure_19_9",14.,30.,20.,1);
  pressure_->setSuffix("°");
  form->addRow("Ângulo de pressão:",pressure_);
  if(pair){
   bore_=number(this,"gear_pair_bore1_19_9",0.,2000.,6.);
   otherBore_=number(this,"gear_pair_bore2_19_9",0.,2000.,6.);
   form->addRow("Furo do motor:",bore_);
   form->addRow("Furo do movido:",otherBore_);
  }else{
   rackBase_=number(this,"gear_rack_base_19_9",0.1,600.,9.);
   form->addRow("Espessura da base:",rackBase_);
  }
  x_=number(this,"gear_compound_x_19_9",0.,10000.,bedW/2.);
  y_=number(this,"gear_compound_y_19_9",0.,10000.,bedH/2.);
  form->addRow(pair?"Centro X do pinhão motor:":"X do primeiro dente:",x_);
  form->addRow(pair?"Centro Y do pinhão motor:":"Y da linha primitiva:",y_);
  layout->addLayout(form);
  preview_=new CompoundGearPreview(this);layout->addWidget(preview_);
  summary_=new QLabel(this);
  summary_->setObjectName("gear_compound_summary_19_9");
  summary_->setWordWrap(true);summary_->setMinimumHeight(74);
  layout->addWidget(summary_);
  auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);
  buttons->button(QDialogButtonBox::Ok)->setText("Criar contornos nominais");
  layout->addWidget(buttons);
  connect(buttons,&QDialogButtonBox::accepted,this,&CompoundGearDialog::accept);
  connect(buttons,&QDialogButtonBox::rejected,this,&CompoundGearDialog::reject);
  connect(teeth_,QOverload<int>::of(&QSpinBox::valueChanged),this,[this](int){refresh();});
  if(otherTeeth_)connect(otherTeeth_,QOverload<int>::of(&QSpinBox::valueChanged),
                         this,[this](int){refresh();});
  for(auto* field:{module_,pressure_,bore_,otherBore_,rackBase_})
   if(field)connect(field,QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this,[this](double){refresh();});
  refresh();
 }
 CompoundKind kind()const{return kind_;}
 Point origin()const{return {x_->value(),y_->value()};}
 PairParameters pairParameters()const{
  if(kind_!=CompoundKind::Pair)throw std::logic_error("not a pair");
  PairParameters p;
  p.driver.teeth=teeth_->value();p.drivenTeeth=otherTeeth_->value();
  p.driver.moduleMm=module_->value();p.driver.pressureDeg=pressure_->value();
  p.driver.boreMm=bore_->value();p.drivenBoreMm=otherBore_->value();
  return p;
 }
 RackParameters rackParameters()const{
  if(kind_!=CompoundKind::Rack)throw std::logic_error("not a rack");
  RackParameters p;
  p.teeth=teeth_->value();p.moduleMm=module_->value();
  p.pressureDeg=pressure_->value();p.baseThicknessMm=rackBase_->value();
  return p;
 }
 void accept()override{
  try{
   if(kind_==CompoundKind::Pair)(void)generatePairNominal(pairParameters());
   else (void)generateRackNominal(rackParameters());
  }catch(const std::exception& e){
   QMessageBox::warning(this,"Geometria inválida",e.what());return;
  }
  QDialog::accept();
 }
};
} // namespace alaser::gear
