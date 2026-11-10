#pragma once
// Alencar Laser phase20s — experimental escape-wheel vector study.
// Anchor pivot, pallets, lock/impulse guides are NEVER exported as contours.
#include "escapement_study_20s.hpp"
#include <QDialog>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QPolygonF>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPointF>
#include <QPen>
#include <QFrame>
#include <QScrollArea>
#include <algorithm>
#include <cmath>
namespace alaser::escape20s {
class Preview final:public QWidget{
 Study study_{};
 bool valid_=false,dragging_=false;
 double zoom_=1.;
 QPointF pan_{},last_{};
 double baseScale()const{
  if(!valid_||study_.outerRadius<=0)return 1.;
  const double r=study_.outerRadius,d=study_.guidePivotDistanceMm;
  return std::min(double(width())/(2.6*r),double(height())/(d+2.5*r));
 }
public:
 explicit Preview(QWidget* p=nullptr):QWidget(p){
  setObjectName("escape_preview_20s");setMinimumHeight(250);
  setMouseTracking(true);setCursor(Qt::OpenHandCursor);
  setToolTip("Zoom pela roda do mouse; arraste para deslocar; Ajustar para retornar.");
 }
 double zoomFactor()const{return zoom_;}
 QPointF panOffset()const{return pan_;}
 void setStudy(const Settings& settings){
  valid_=false;
  try{study_=generate(settings);valid_=true;}catch(const std::exception&){}
  update();
 }
 void fit(){zoom_=1.;pan_={};update();}
 void zoom(double factor,QPointF anchor){
  if(!valid_||!std::isfinite(factor)||factor<=0)return;
  const double next=std::clamp(zoom_*factor,.5,20.);
  const QPointF center(width()/2.,height()/2.);
  pan_=anchor-center-(anchor-center-pan_)*(next/zoom_);
  zoom_=next;update();
 }
protected:
 void wheelEvent(QWheelEvent* e) override{
  const auto steps=double(e->angleDelta().y())/120.;
  if(std::abs(steps)>1e-7)zoom(std::pow(1.2,steps),e->position());
  e->accept();
 }
 void mousePressEvent(QMouseEvent* e) override{
  if(e->button()==Qt::LeftButton||e->button()==Qt::MiddleButton){
   dragging_=true;last_=e->position();setCursor(Qt::ClosedHandCursor);
   e->accept();return;
  }
  QWidget::mousePressEvent(e);
 }
 void mouseMoveEvent(QMouseEvent* e) override{
  if(dragging_){
   pan_+=e->position()-last_;last_=e->position();
   update();e->accept();return;
  }
  QWidget::mouseMoveEvent(e);
 }
 void mouseReleaseEvent(QMouseEvent* e) override{
  if(dragging_){
   dragging_=false;setCursor(Qt::OpenHandCursor);e->accept();return;
  }
  QWidget::mouseReleaseEvent(e);
 }
 void paintEvent(QPaintEvent*) override{
  QPainter p(this);p.setRenderHint(QPainter::Antialiasing,true);
  p.fillRect(rect(),QColor("#1B252E"));
  if(!valid_)return;
  const double scale=baseScale()*zoom_;
  const QPointF origin=QPointF(width()/2.,height()/2.)+pan_;
  const double verticalCenter=study_.guidePivotDistanceMm*.35;
  auto point=[&](Pt v)->QPointF{
   return origin+QPointF(v.x*scale,-(v.y-verticalCenter)*scale);
  };
  auto polygon=[&](const std::vector<Pt>& poly){
   QPolygonF q;q.reserve(int(poly.size()));
   for(const auto& v:poly)q<<point(v);
   return q;
  };
  QPainterPath shape;shape.setFillRule(Qt::OddEvenFill);
  shape.addPolygon(polygon(study_.wheel));shape.closeSubpath();
  if(!study_.axleHole.empty()){
   shape.addPolygon(polygon(study_.axleHole));shape.closeSubpath();
  }
  p.setBrush(QColor("#344756"));
  p.setPen(QPen(QColor("#F5A145"),1.5));p.drawPath(shape);
  const auto pivot=point(study_.anchorPivot);
  const auto entry=point(study_.entryReference),exit=point(study_.exitReference);
  p.setBrush(Qt::NoBrush);p.setPen(QPen(QColor("#7A9BAE"),1,Qt::DashLine));
  p.drawLine(pivot,entry);p.drawLine(pivot,exit);
  p.drawEllipse(pivot,3.,3.);
  QPainterPath arc;
  if(!study_.anchorReferenceArc.empty()){
   arc.moveTo(point(study_.anchorReferenceArc.front()));
   for(size_t i=1;i<study_.anchorReferenceArc.size();++i)
    arc.lineTo(point(study_.anchorReferenceArc[i]));
   p.drawPath(arc);
  }
  p.setBrush(QColor("#7BD0E4"));p.setPen(Qt::NoPen);
  p.drawEllipse(entry,3.,3.);p.drawEllipse(exit,3.,3.);
  p.setPen(QColor("#D0E8F0"));
  p.drawText(pivot+QPointF(8,-7),"Pivô (guia)");
  p.drawText(origin+QPointF(9,verticalCenter*scale),"Roda");
 }
};
class Dialog final:public QDialog{
 QSpinBox* teeth_=nullptr;
 QDoubleSpinBox *diameter_=nullptr,*root_=nullptr,*bore_=nullptr,
  *tip_=nullptr,*trailing_=nullptr,*leading_=nullptr,*span_=nullptr,
  *x_=nullptr,*y_=nullptr;
 QComboBox* rotation_=nullptr;
 QLabel* summary_=nullptr;
 Preview* preview_=nullptr;
 QPushButton* accept_=nullptr;
 Settings settingsFromControls()const{
  Settings s;
  s.teeth=teeth_->value();
  s.outerDiameterMm=diameter_->value();
  s.rootFraction=root_->value()/100.;
  s.axleHoleMm=bore_->value();
  s.tipThicknessDeg=tip_->value();
  s.trailingRootOffsetDeg=trailing_->value();
  s.leadingRootOffsetDeg=leading_->value();
  s.palletSpanTeeth=span_->value();
  s.rotation=static_cast<Rotation>(rotation_->currentIndex());
  return s;
 }
 void refresh(){
  const auto config=settingsFromControls();
  preview_->setStudy(config);
  try{
   const auto s=generate(config);
   if(accept_)accept_->setEnabled(true);
   summary_->setText(QString(
    "Roda %1 dentes · Ø%2 mm · raiz Ø%3 mm · passo %4°\n"
    "Pivô de referência: %5 mm do eixo da roda · arco de palhetas: %6 passos\n"
    "Roda e furo: CONTORNOS EDITÁVEIS. Pivô, arco e palhetas: APENAS GUIAS.\n"
    "Sem validação de bloqueio, impulso, queda, folgas, kerf ou resistência do MDF.")
    .arg(config.teeth).arg(config.outerDiameterMm,0,'f',1)
    .arg(2*s.rootRadius,0,'f',1).arg(s.pitchDeg,0,'f',2)
    .arg(s.guidePivotDistanceMm,0,'f',2)
    .arg(config.palletSpanTeeth,0,'f',1));
  }catch(const std::exception& e){
   if(accept_)accept_->setEnabled(false);
   summary_->setText(QString("PARÂMETROS INVÁLIDOS: ")+e.what());
  }
 }
public:
 explicit Dialog(double bedW,double bedH,QWidget* parent=nullptr):QDialog(parent){
  setObjectName("escape_study_dialog_20s");
  setWindowTitle("Roda de escape — estudo geométrico (Alencar Laser)");
  setMinimumSize(610,700);resize(670,790);
  auto* layout=new QVBoxLayout(this);
  auto* warning=new QLabel("ESTUDO OFFLINE. A âncora é apenas uma guia visual: não exportar como peça mecânica.",this);
  warning->setWordWrap(true);layout->addWidget(warning);
  auto* formHost=new QWidget(this);
  auto* form=new QFormLayout(formHost);form->setVerticalSpacing(4);
  form->setContentsMargins(4,3,4,3);
  teeth_=new QSpinBox(this);teeth_->setObjectName("escape_teeth_20s");
  teeth_->setRange(15,90);teeth_->setValue(30);
  form->addRow("Dentes da roda:",teeth_);
  auto metric=[&](const char* key,double lo,double hi,double value,int precision,const QString& suffix){
   auto* v=new QDoubleSpinBox(this);v->setObjectName(key);
   v->setRange(lo,hi);v->setDecimals(precision);v->setSingleStep(.1);
   v->setValue(value);v->setSuffix(suffix);return v;
  };
  diameter_=metric("escape_diameter_20s",20.,400.,80.,2," mm");
  root_=metric("escape_root_20s",60.,90.,75.,1," %");
  bore_=metric("escape_bore_20s",0.,100.,6.,2," mm");
  tip_=metric("escape_tip_20s",.15,5.,.6,2,"°");
  trailing_=metric("escape_trailing_20s",.1,15.,4.2,2,"°");
  leading_=metric("escape_leading_20s",.1,15.,2.6,2,"°");
  span_=metric("escape_span_20s",2.5,15.,7.5,1," passos");
  span_->setSingleStep(.5);
  x_=metric("escape_center_x_20s",0.,10000.,bedW/2.,1," mm");
  y_=metric("escape_center_y_20s",0.,10000.,bedH/2.,1," mm");
  form->addRow("Diâmetro externo:",diameter_);
  form->addRow("Raiz (% do raio externo):",root_);
  form->addRow("Furo do eixo:",bore_);
  form->addRow("Espessura angular da ponta:",tip_);
  form->addRow("Afastamento da face posterior:",trailing_);
  form->addRow("Afastamento da face anterior:",leading_);
  form->addRow("Distância entre palhetas:",span_);
  rotation_=new QComboBox(this);rotation_->setObjectName("escape_rotation_20s");
  rotation_->addItems({"Horária","Anti-horária"});form->addRow("Rotação nominal:",rotation_);
  form->addRow("Centro X (roda):",x_);form->addRow("Centro Y (roda):",y_);
  auto* formScroll=new QScrollArea(this);
  formScroll->setObjectName("escape_form_scroll_20s");
  formScroll->setFrameShape(QFrame::NoFrame);
  formScroll->setWidgetResizable(true);formScroll->setWidget(formHost);
  formScroll->setMinimumHeight(165);formScroll->setMaximumHeight(345);
  layout->addWidget(formScroll,1);
  auto* tools=new QHBoxLayout;
  auto* less=new QPushButton("−",this);less->setObjectName("escape_zoom_out_20s");
  auto* more=new QPushButton("+",this);more->setObjectName("escape_zoom_in_20s");
  auto* fit=new QPushButton("Ajustar",this);fit->setObjectName("escape_zoom_fit_20s");
  tools->addStretch();tools->addWidget(new QLabel("Prévia:",this));
  tools->addWidget(less);tools->addWidget(more);tools->addWidget(fit);
  layout->addLayout(tools);
  preview_=new Preview(this);layout->addWidget(preview_,2);
  connect(less,&QPushButton::clicked,this,[this]{
   preview_->zoom(.8,QPointF(preview_->width()/2.,preview_->height()/2.));});
  connect(more,&QPushButton::clicked,this,[this]{
   preview_->zoom(1.25,QPointF(preview_->width()/2.,preview_->height()/2.));});
  connect(fit,&QPushButton::clicked,preview_,&Preview::fit);
  summary_=new QLabel(this);summary_->setObjectName("escape_summary_20s");
  summary_->setWordWrap(true);layout->addWidget(summary_);
  auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);
  accept_=buttons->button(QDialogButtonBox::Ok);
  accept_->setText("Criar APENAS roda e furo");
  connect(buttons,&QDialogButtonBox::accepted,this,&QDialog::accept);
  connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);
  layout->addWidget(buttons);
  connect(teeth_,QOverload<int>::of(&QSpinBox::valueChanged),this,[this](int){refresh();});
  connect(rotation_,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this](int){refresh();});
  for(auto* v:{diameter_,root_,bore_,tip_,trailing_,leading_,span_,x_,y_})
   connect(v,QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,[this](double){refresh();});
  refresh();
 }
 Settings parameters()const{return settingsFromControls();}
 Pt centerMm()const{return {x_->value(),y_->value()};}
};
} // namespace alaser::escape20s
