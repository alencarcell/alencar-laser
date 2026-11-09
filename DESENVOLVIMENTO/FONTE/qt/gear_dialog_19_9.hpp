#pragma once
// Alencar Laser 19.9: gerador de engrenagem somente editorial/offline.
#include "gear_generator_19_9.hpp"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSignalBlocker>
#include <QCheckBox>
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
#include <QScrollArea>
#include <QFrame>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPointF>
#include <functional>
#include <algorithm>
#include <cmath>

namespace alaser::gear {
class GearPreview final:public QWidget{
 Parameters current_{};
 Result generated_{};
 bool valid_=false;
 double zoom_=1.;
 QPointF pan_{0.,0.},lastDrag_{0.,0.};
 bool dragging_=false;
 std::function<void(double)> zoomChanged_{};
 double baseScale()const{
  if(!valid_||generated_.outerRadius<=0.)return 1.;
  return .45*double(std::min(width(),height()))/generated_.outerRadius;
 }
 void notifyZoom(){
  if(zoomChanged_)zoomChanged_(zoom_);
 }
public:
 explicit GearPreview(QWidget* p=nullptr):QWidget(p){
  setMinimumHeight(175);setObjectName("gear_preview_19_9");
  setCursor(Qt::OpenHandCursor);
  setToolTip("Roda do mouse: zoom na posição do cursor. Arrastar com botão esquerdo ou central: deslocar. Ajustar: restaurar.");
 }
 void onZoomChanged(std::function<void(double)> callback){
  zoomChanged_=std::move(callback);notifyZoom();
 }
 double zoomFactor()const noexcept{return zoom_;}
 QPointF panOffset()const noexcept{return pan_;}
 void zoomBy(double factor,QPointF anchor){
  if(!valid_||!std::isfinite(factor)||factor<=0.)return;
  const double next=std::clamp(zoom_*factor,0.5,20.);
  if(std::abs(next-zoom_)<1e-10)return;
  const QPointF center=QPointF(width()/2.,height()/2.);
  // Preserve the point underneath the cursor for stable, predictable zoom.
  pan_=anchor-center-(anchor-center-pan_)*(next/zoom_);
  zoom_=next;
  notifyZoom();update();
 }
 void fitView(){
  zoom_=1.;pan_={0.,0.};notifyZoom();update();
 }
 void setGear(const Parameters& params){
  current_=params;valid_=false;
  try{generated_=generate(params);valid_=true;}catch(const std::exception&){}
  update();
 }
protected:
 void wheelEvent(QWheelEvent* e)override{
  const double steps=double(e->angleDelta().y())/120.;
  if(std::abs(steps)>1e-8){
   zoomBy(std::pow(1.2,steps),e->position());
  }
  e->accept();
 }
 void mousePressEvent(QMouseEvent* e)override{
  if(valid_&&(e->button()==Qt::LeftButton||e->button()==Qt::MiddleButton)){
   dragging_=true;lastDrag_=e->position();setCursor(Qt::ClosedHandCursor);
   e->accept();return;
  }
  QWidget::mousePressEvent(e);
 }
 void mouseMoveEvent(QMouseEvent* e)override{
  if(dragging_){
   pan_+=e->position()-lastDrag_;lastDrag_=e->position();
   update();e->accept();return;
  }
  QWidget::mouseMoveEvent(e);
 }
 void mouseReleaseEvent(QMouseEvent* e)override{
  if(dragging_&&(e->button()==Qt::LeftButton||e->button()==Qt::MiddleButton)){
   dragging_=false;setCursor(Qt::OpenHandCursor);e->accept();return;
  }
  QWidget::mouseReleaseEvent(e);
 }
 void paintEvent(QPaintEvent*)override{
  QPainter p(this);p.setRenderHint(QPainter::Antialiasing,true);
  p.fillRect(rect(),QColor("#1B252E"));
  p.setPen(QPen(QColor("#44596A"),1));
  p.drawRect(rect().adjusted(1,1,-2,-2));
  if(!valid_)return;
  const QPointF center=QPointF(width()/2.,height()/2.)+pan_;
  const double scale=baseScale()*zoom_;
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
 QComboBox* profile_=nullptr;
 QComboBox* definition_=nullptr;
 QLabel* zoomLabel_=nullptr;
 QDoubleSpinBox* includedAngle_=nullptr;
 QDoubleSpinBox *module_=nullptr,*circularPitch_=nullptr,*pressure_=nullptr,*bore_=nullptr,*centerX_=nullptr,*centerY_=nullptr;
 QDoubleSpinBox *baseWidth_=nullptr,*tipPosition_=nullptr;
 QCheckBox* previewEnabled_=nullptr;
 QLabel* info_=nullptr;
 GearPreview* preview_=nullptr;
 Parameters current()const{
  Parameters p;
  p.teeth=teeth_->value();p.moduleMm=module_->value();
  p.pressureDeg=pressure_->value();p.boreMm=bore_->value();
  p.profile=static_cast<ToothProfile>(profile_->currentIndex());
  p.angularBasePercent=baseWidth_->value();
  p.tipPositionPercent=tipPosition_->value();
  p.angularDefinition=static_cast<AngularDefinition>(definition_->currentIndex());
  p.includedTipAngleDeg=includedAngle_->value();
  return p;
 }
 void refresh(){
  const auto p=current();
  const bool angular=p.profile!=ToothProfile::Involute;
  pressure_->setEnabled(!angular);
  baseWidth_->setEnabled(angular);
  tipPosition_->setEnabled(p.profile==ToothProfile::Sawtooth);
  definition_->setEnabled(angular);
  includedAngle_->setEnabled(angular&&p.angularDefinition==AngularDefinition::ByIncludedTipAngle);
  preview_->setGear(p);
  try{
   const auto shape=generate(p);
   QString message=QString("Módulo %1 mm  ·  Passo circular %2 mm\n"
     "Diâmetro externo %3 mm  ·  diâmetro primitivo %4 mm  ·  raiz %5 mm\n"
     "Contorno: %6 nós  ·  Furo: %7 nós")
     .arg(p.moduleMm,0,'f',4).arg(circularPitchFromModule(p.moduleMm),0,'f',4)
     .arg(2*shape.outerRadius,0,'f',2).arg(2*shape.pitchRadius,0,'f',2)
     .arg(2*shape.rootRadius,0,'f',2).arg(shape.outer.size()).arg(shape.bore.size());
   if(shape.undercutWarning)message+="\nAviso: poucos dentes podem gerar interferência/undercut.";
   if(angular){
    const double actual=includedAngleAtTipDeg(shape.outer.at(0),shape.outer.at(1),shape.outer.at(2));
    message+=QString("\nÂngulo INTERNO medido na ponta: %1°  ·  Altura radial: %2 mm")
       .arg(actual,0,'f',2).arg(shape.outerRadius-shape.rootRadius,0,'f',2);
    message+="\nNão confundir com ângulo da face na raiz nem com bloqueio/impulso da âncora.";
    message+="\nPerfil visual: encaixe, kerf e resistência NÃO validados.";
   }
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
  setMinimumSize(540,610);resize(625,720);
  auto* column=new QVBoxLayout(this);column->setSpacing(5);
  auto* hint=new QLabel("GERAÇÃO VETORIAL OFFLINE • sem motor, movimento ou emissão de laser.",this);
  hint->setWordWrap(true);column->addWidget(hint);
  auto* formHost=new QWidget(this);
  auto* form=new QFormLayout(formHost);form->setVerticalSpacing(4);
  form->setContentsMargins(4,2,4,2);
  teeth_=new QSpinBox(this);teeth_->setObjectName("gear_teeth_19_9");
  teeth_->setRange(12,120);teeth_->setValue(24);
  form->addRow("Quantidade de dentes:",teeth_);
  profile_=new QComboBox(this);
  profile_->setObjectName("gear_profile_20q");
  profile_->addItem("Mecânico padrão (involuta)");
  profile_->addItem("Ponta angular (simétrica)");
  profile_->addItem("Serrilhado (ponta inclinada)");
  profile_->setToolTip("Ponta angular e serrilhado são perfis vetoriais ilustrativos. Não são perfis de engrenamento certificados.");
  form->addRow("Perfil do dente:",profile_);
  definition_=new QComboBox(this);
  definition_->setObjectName("gear_definition_20r");
  definition_->addItem("Por largura da base (%)");
  definition_->addItem("Por ângulo INTERNO na ponta (°)");
  definition_->setToolTip("O ângulo interno é medido entre as duas faces retas na ponta. Não é o ângulo de pressão nem o de contato com a âncora.");
  form->addRow("Definição angular:",definition_);
  auto metric=[&](const char* key,double min,double max,double value,int decimals,const QString& suffix){
   auto* f=new QDoubleSpinBox(this);f->setObjectName(key);f->setDecimals(decimals);
   f->setRange(min,max);f->setSingleStep(.5);f->setValue(value);f->setSuffix(suffix);return f;
  };
  module_=metric("gear_module_19_9",.5,20.,3.,6," mm");
  module_->setToolTip("Módulo métrico: diâmetro primitivo = módulo × número de dentes.");
  form->addRow("Módulo:",module_);
  circularPitch_=metric("gear_circular_pitch_19_9",
     std::numbers::pi*0.5,std::numbers::pi*20.,3.*std::numbers::pi,6," mm");
  circularPitch_->setToolTip("Passo circular: distância no círculo primitivo entre dentes, p = pi × módulo. Alterar um campo atualiza o outro.");
  form->addRow("Passo circular:",circularPitch_);
  pressure_=metric("gear_pressure_19_9",14.,30.,20.,1,"°");
  pressure_->setToolTip("Ângulo de pressão do perfil evolvente aproximado.");
  form->addRow("Ângulo de pressão:",pressure_);
  baseWidth_=metric("gear_angular_base_20q",45.,90.,75.,1," %");
  baseWidth_->setSingleStep(5.);
  baseWidth_->setToolTip("Largura da base do dente como porcentagem do passo angular. Maior = ponta com base mais larga.");
  form->addRow("Base do dente angular:",baseWidth_);
  includedAngle_=metric("gear_included_angle_20r",30.,160.,90.,1,"°");
  includedAngle_->setSingleStep(1.);
  includedAngle_->setToolTip("Ângulo interno real na PONTA entre as duas faces. O programa ajusta a altura da ponta, sem ultrapassar o raio máximo nominal.");
  form->addRow("Ângulo interno na ponta:",includedAngle_);
  tipPosition_=metric("gear_angular_tip_20q",15.,85.,50.,1," %");
  tipPosition_->setSingleStep(5.);
  tipPosition_->setToolTip("Somente serrilhado: 50% = ponta central; 80% = ponta inclinada para a direita.");
  form->addRow("Posição da ponta:",tipPosition_);
  bore_=metric("gear_bore_19_9",0.,2000.,6.,2," mm");
  form->addRow("Furo central (0 = sem furo):",bore_);
  centerX_=metric("gear_center_x_19_9",0.,10000.,bedW/2.,1," mm");
  centerY_=metric("gear_center_y_19_9",0.,10000.,bedH/2.,1," mm");
  form->addRow("Centro X:",centerX_);form->addRow("Centro Y:",centerY_);
  auto* formScroll=new QScrollArea(this);
  formScroll->setObjectName("gear_form_scroll_20r");
  formScroll->setFrameShape(QFrame::NoFrame);
  formScroll->setWidgetResizable(true);
  formScroll->setWidget(formHost);
  formScroll->setMinimumHeight(185);
  formScroll->setMaximumHeight(345);
  column->addWidget(formScroll,1);
  previewEnabled_=new QCheckBox("Pré-visualizar",this);
  previewEnabled_->setObjectName("gear_preview_enabled_19_9");
  previewEnabled_->setChecked(true);
  column->addWidget(previewEnabled_);
  auto* tools=new QHBoxLayout;
  auto* zoomOut=new QPushButton("−",this);zoomOut->setObjectName("gear_zoom_out_20r");
  auto* zoomIn=new QPushButton("+",this);zoomIn->setObjectName("gear_zoom_in_20r");
  auto* fit=new QPushButton("Ajustar",this);fit->setObjectName("gear_zoom_fit_20r");
  zoomLabel_=new QLabel("100%",this);zoomLabel_->setObjectName("gear_zoom_label_20r");
  for(auto* btn:{zoomOut,zoomIn})btn->setFixedWidth(33);
  tools->addStretch();tools->addWidget(new QLabel("Zoom:",this));
  tools->addWidget(zoomOut);tools->addWidget(zoomLabel_);
  tools->addWidget(zoomIn);tools->addWidget(fit);
  column->addLayout(tools);
  preview_=new GearPreview(this);column->addWidget(preview_,2);
  preview_->onZoomChanged([this](double factor){
   zoomLabel_->setText(QString::number(int(std::round(factor*100.)))+"%");
  });
  connect(zoomOut,&QPushButton::clicked,this,[this]{
   preview_->zoomBy(1./1.25,QPointF(preview_->width()/2.,preview_->height()/2.));
  });
  connect(zoomIn,&QPushButton::clicked,this,[this]{
   preview_->zoomBy(1.25,QPointF(preview_->width()/2.,preview_->height()/2.));
  });
  connect(fit,&QPushButton::clicked,preview_,&GearPreview::fitView);
  connect(previewEnabled_,&QCheckBox::toggled,preview_,&QWidget::setVisible);
  info_=new QLabel(this);info_->setObjectName("gear_summary_19_9");
  info_->setWordWrap(true);info_->setMinimumHeight(95);column->addWidget(info_);
  auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);
  buttons->button(QDialogButtonBox::Ok)->setText("Criar contornos no desenho");
  buttons->button(QDialogButtonBox::Cancel)->setText("Cancelar");
  column->addWidget(buttons);
  connect(buttons,&QDialogButtonBox::accepted,this,&GearDialog::accept);
  connect(buttons,&QDialogButtonBox::rejected,this,&GearDialog::reject);
  connect(teeth_,QOverload<int>::of(&QSpinBox::valueChanged),this,[this](int){refresh();});
  connect(profile_,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this](int idx){
   const QSignalBlocker guard(tipPosition_);
   tipPosition_->setValue(idx==2?80.:50.);
   refresh();
  });
  connect(definition_,QOverload<int>::of(&QComboBox::currentIndexChanged),this,
          [this](int){refresh();});
  connect(module_,QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,
          [this](double m){
            const QSignalBlocker guard(circularPitch_);
            circularPitch_->setValue(circularPitchFromModule(m));
            refresh();
          });
  connect(circularPitch_,QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,
          [this](double pitch){
            const QSignalBlocker guard(module_);
            module_->setValue(moduleFromCircularPitch(pitch));
            refresh();
          });
  for(auto* f:{pressure_,bore_,centerX_,centerY_,baseWidth_,tipPosition_,includedAngle_})
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
