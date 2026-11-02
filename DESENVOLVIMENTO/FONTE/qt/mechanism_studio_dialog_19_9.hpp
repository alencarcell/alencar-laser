#pragma once
// Oficina didatica: somente Qt/QPainter + matematica; SEM CAM/GRBL/hardware.
#include "mechanism_studio_19_9.hpp"
#include "tool_glyphs.hpp"
#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QTimer>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QScrollArea>
#include <QFrame>
#include <algorithm>
#include <numbers>
#include <cmath>
namespace alaser::mechanisms {
class MechanismCanvas final :public QWidget {
 Evaluation geometry_{};
 double angle_=0.;
public:
 explicit MechanismCanvas(QWidget* parent=nullptr):QWidget(parent){
  setObjectName("mechanism_animation_canvas_19_9");
  setMinimumSize(350,210);setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  setToolTip("Prévia animada: rotação e translação ideais. Sem validação de colisão ou contato real.");
 }
 void setMechanism(const Evaluation& g){geometry_=g;update();}
 void setAngle(double rad){angle_=rad;update();}
 double angle()const{return angle_;}
protected:
 void paintEvent(QPaintEvent*)override{
  QPainter painter(this);painter.setRenderHint(QPainter::Antialiasing,true);
  painter.fillRect(rect(),QColor("#18232D"));
  painter.setPen(QPen(QColor("#405264"),1));
  painter.drawRect(rect().adjusted(1,1,-2,-2));
  if(geometry_.parts.empty())return;
  // Enquadramento constante em toda a volta: a animação não deve "respirar".
  // Para pinhões, utiliza circunferência envolvente; para cremalheira, curso
  // máximo ±pi * (mm por radiano) considerando o ângulo normalizado.
  double minX=1e20,minY=1e20,maxX=-1e20,maxY=-1e20;
  for(const auto& part:geometry_.parts){
   if(part.rack){
    const double slide=std::numbers::pi*std::abs(part.slideRatio);
    for(const auto& p:part.outer){
     minX=std::min(minX,part.origin.x+p.x-slide);
     maxX=std::max(maxX,part.origin.x+p.x+slide);
     minY=std::min(minY,part.origin.y+p.y);
     maxY=std::max(maxY,part.origin.y+p.y);
    }
   }else{
    double radius=0.;
    for(const auto& p:part.outer)radius=std::max(radius,std::hypot(p.x,p.y));
    minX=std::min(minX,part.origin.x-radius);
    maxX=std::max(maxX,part.origin.x+radius);
    minY=std::min(minY,part.origin.y-radius);
    maxY=std::max(maxY,part.origin.y+radius);
   }
  }
  if(!(maxX>minX&&maxY>minY))return;
  const double sx=double(std::max(10,width()-44))/(maxX-minX);
  const double sy=double(std::max(10,height()-44))/(maxY-minY);
  const double scale=std::min(sx,sy);
  const QPointF mid((minX+maxX)/2.,(minY+maxY)/2.);
  auto map=[&](Point v)->QPointF{
   return {width()/2.+(v.x-mid.x())*scale,
           height()/2.-(v.y-mid.y())*scale};
  };
  const QColor strokes[3]={QColor("#FFAB57"),QColor("#81C4E2"),QColor("#D3B4FF")};
  const QColor fills[3]={QColor("#354759"),QColor("#2B4653"),QColor("#453A52")};
  for(size_t i=0;i<geometry_.parts.size();++i){
   const auto& part=geometry_.parts[i];
   QPainterPath profile;profile.setFillRule(Qt::OddEvenFill);
   auto add=[&](const std::vector<Point>& poly){
    if(poly.size()<3)return;
    QPainterPath path;
    const auto first=map(transform(part,poly.front(),angle_));
    path.moveTo(first);
    for(size_t j=1;j<poly.size();++j)path.lineTo(map(transform(part,poly[j],angle_)));
    path.closeSubpath();profile.addPath(path);
   };
   add(part.outer);add(part.bore);
   painter.setPen(QPen(strokes[i%3],1.4));
   painter.setBrush(fills[i%3]);painter.drawPath(profile);
   if(!part.rack){
    const auto c=map(part.origin);
    painter.setPen(QPen(strokes[i%3],1,Qt::DashLine));
    painter.setBrush(Qt::NoBrush);painter.drawEllipse(c,2.8,2.8);
   }
  }
 }
};
class MechanismStudioDialog final :public QDialog {
 QComboBox* preset_=nullptr;
 QComboBox* material_=nullptr;
 QDoubleSpinBox *thickness_=nullptr,*module_=nullptr,*pressure_=nullptr,
                *bore_=nullptr,*kerf_=nullptr,*backlash_=nullptr,*speed_=nullptr;
 QSpinBox *driver_=nullptr,*driven_=nullptr;
 QLabel* specs_=nullptr,*warnings_=nullptr;
 QPushButton* play_=nullptr,*pause_=nullptr;
 MechanismCanvas* scene_=nullptr;
 QTimer timer_;
 double radians_=0.;
 Evaluation result_{};
 bool valid_=false;
 static QDoubleSpinBox* metric(QWidget* parent,const QString& key,double low,double high,
                               double val,double step=0.1){
  auto* s=new QDoubleSpinBox(parent);s->setObjectName(key);s->setRange(low,high);
  s->setDecimals(3);s->setSingleStep(step);s->setSuffix(" mm");s->setValue(val);
  return s;
 }
 void pause(){
  timer_.stop();
  play_->setEnabled(valid_);pause_->setEnabled(false);
 }
 void applyPreset(int index){
  if(index<0||index>2)return;
  const Settings d=defaults(Template(index));
  const QSignalBlocker m(module_),p(pressure_),b(bore_),t(driver_),f(driven_);
  module_->setValue(d.moduleMm);pressure_->setValue(d.pressureDeg);
  bore_->setValue(d.boreMm);driver_->setValue(d.teeth1);driven_->setValue(d.teeth2);
  driven_->setPrefix(index==2?"Dentes da barra: ":"");
  refresh();
 }
 void refresh(){
  try{
   result_=generate(settings());valid_=true;scene_->setMechanism(result_);
   specs_->setText(QString("Mecanismo: %1  |  %2 mm  |  %3\n"
     "Distância nominal: %4 mm  |  Relação de velocidades: %5\n"
     "Kerf informado: %6 mm  |  Folga pretendida: %7 mm")
     .arg(QString::fromUtf8(presetName(settings().preset)))
     .arg(settings().thicknessMm,0,'f',2)
     .arg(QString::fromUtf8(materialName(settings().material)))
     .arg(result_.centerDistanceMm,0,'f',2)
     .arg(settings().preset==Template::SchoolRack?QString("rack linear"):
           QString("%1:1").arg(result_.speedRatio,0,'f',3))
     .arg(settings().kerfMm,0,'f',2)
     .arg(settings().backlashMm,0,'f',2));
   QStringList notes;for(const auto& w:result_.warnings)notes<<QString::fromUtf8(w);
   warnings_->setText(notes.join("\n"));
  }catch(const std::exception& ex){
   valid_=false;timer_.stop();scene_->setMechanism({});
   specs_->setText("Parâmetros inválidos: "+QString::fromUtf8(ex.what()));
   warnings_->setText("Nenhuma peça foi alterada.");
  }
  if(play_)play_->setEnabled(valid_&&!timer_.isActive());
  if(pause_)pause_->setEnabled(valid_&&timer_.isActive());
  if(auto* ok=findChild<QPushButton*>("mechanism_insert_19_9"))ok->setEnabled(valid_);
 }
public:
 explicit MechanismStudioDialog(QWidget* parent=nullptr):QDialog(parent){
  setObjectName("mechanism_studio_dialog_19_9");
  setWindowTitle("Alencar Laser — Oficina de Mecanismos (OFFLINE)");
  setMinimumSize(680,650);resize(820,740);
  auto* root=new QVBoxLayout(this);root->setContentsMargins(12,10,12,10);
  root->setSpacing(7);
  auto* banner=new QLabel("SIMULAÇÃO DIDÁTICA • SEM EMISSÃO, MOVIMENTO FÍSICO OU COMANDOS À MÁQUINA",this);
  banner->setWordWrap(true);banner->setStyleSheet("QLabel{color:#FFB86C;font-weight:600}");
  root->addWidget(banner);
  auto* scroll=new QScrollArea(this);scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  auto* inputs=new QWidget(scroll);
  auto* form=new QFormLayout(inputs);form->setLabelAlignment(Qt::AlignLeft);
  form->setHorizontalSpacing(14);form->setVerticalSpacing(6);
  preset_=new QComboBox(inputs);preset_->setObjectName("mechanism_preset_19_9");
  for(int i=0;i<3;++i)preset_->addItem(QString::fromUtf8(presetName(Template(i))));
  form->addRow("Biblioteca de mecanismos:",preset_);
  material_=new QComboBox(inputs);material_->setObjectName("mechanism_material_19_9");
  for(int i=0;i<3;++i)material_->addItem(QString::fromUtf8(materialName(Material(i))));
  form->addRow("Material:",material_);
  thickness_=metric(inputs,"mechanism_thickness_19_9",1.,30.,3.);
  form->addRow("Espessura do material:",thickness_);
  driver_=new QSpinBox(inputs);driver_->setObjectName("mechanism_teeth_driver_19_9");
  driver_->setRange(12,120);form->addRow("Dentes do motor:",driver_);
  driven_=new QSpinBox(inputs);driven_->setObjectName("mechanism_teeth_driven_19_9");
  driven_->setRange(1,120);form->addRow("Dentes movidos / barra:",driven_);
  module_=metric(inputs,"mechanism_module_19_9",.5,20.,3.);
  form->addRow("Módulo comum:",module_);
  pressure_=metric(inputs,"mechanism_pressure_19_9",14.,30.,20.,1.);
  pressure_->setSuffix("°");form->addRow("Ângulo de pressão:",pressure_);
  bore_=metric(inputs,"mechanism_bore_19_9",0.,500.,6.);
  form->addRow("Diâmetro do eixo/furo:",bore_);
  kerf_=metric(inputs,"mechanism_kerf_19_9",0.,1.,.15,.01);
  kerf_->setToolTip("Estimativa informativa. NÃO é aplicada nos vetores.");
  form->addRow("Kerf medido (referência):",kerf_);
  backlash_=metric(inputs,"mechanism_backlash_19_9",0.,3.,.12,.01);
  backlash_->setToolTip("Folga informativa; NÃO altera perfil dos dentes.");
  form->addRow("Folga desejada (referência):",backlash_);
  speed_=metric(inputs,"mechanism_speed_19_9",.1,30.,3.);
  speed_->setSuffix(" rpm");form->addRow("Velocidade de prévia:",speed_);
  scroll->setWidget(inputs);
  scroll->setMaximumHeight(315);root->addWidget(scroll);
  scene_=new MechanismCanvas(this);root->addWidget(scene_,1);
  auto* playback=new QHBoxLayout;
  play_=new QPushButton("Iniciar simulação",this);
  play_->setIcon(alaser::toolGlyph("play"));play_->setIconSize(QSize(20,20));
  play_->setObjectName("mechanism_play_19_9");
  pause_=new QPushButton("Pausar",this);
  pause_->setIcon(alaser::toolGlyph("pause"));pause_->setIconSize(QSize(20,20));pause_->setObjectName("mechanism_pause_19_9");
  auto* reset=new QPushButton("Reiniciar posição",this);
  reset->setIcon(alaser::toolGlyph("undo"));reset->setIconSize(QSize(20,20));
  reset->setObjectName("mechanism_reset_19_9");
  playback->addWidget(play_);playback->addWidget(pause_);playback->addWidget(reset);
  playback->addStretch();
  root->addLayout(playback);
  specs_=new QLabel(this);specs_->setObjectName("mechanism_info_19_9");specs_->setWordWrap(true);
  root->addWidget(specs_);
  warnings_=new QLabel(this);warnings_->setObjectName("mechanism_warnings_19_9");
  warnings_->setWordWrap(true);warnings_->setStyleSheet("QLabel{color:#E6BE85;}");
  root->addWidget(warnings_);
  auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);
  auto* insert=buttons->button(QDialogButtonBox::Ok);
  insert->setText("Inserir conjunto no desenho");
  insert->setObjectName("mechanism_insert_19_9");
  buttons->button(QDialogButtonBox::Cancel)->setText("Fechar sem inserir");
  root->addWidget(buttons);
  connect(buttons,&QDialogButtonBox::accepted,this,&MechanismStudioDialog::accept);
  connect(buttons,&QDialogButtonBox::rejected,this,&MechanismStudioDialog::reject);
  connect(preset_,QOverload<int>::of(&QComboBox::currentIndexChanged),this,
          [this](int i){pause();radians_=0.;scene_->setAngle(0.);applyPreset(i);});
  connect(material_,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this](int){refresh();});
  connect(driver_,QOverload<int>::of(&QSpinBox::valueChanged),this,[this](int){refresh();});
  connect(driven_,QOverload<int>::of(&QSpinBox::valueChanged),this,[this](int){refresh();});
  for(auto* field:{thickness_,module_,pressure_,bore_,kerf_,backlash_})
   connect(field,QOverload<double>::of(&QDoubleSpinBox::valueChanged),
           this,[this](double){refresh();});
  timer_.setInterval(40);
  connect(&timer_,&QTimer::timeout,this,[this]{
   radians_=std::remainder(radians_+speed_->value()*2.*std::numbers::pi*0.04/60.,
                          2.*std::numbers::pi);
   scene_->setAngle(radians_);
  });
  connect(play_,&QPushButton::clicked,this,[this]{if(valid_){timer_.start();refresh();}});
  connect(pause_,&QPushButton::clicked,this,[this]{pause();});
  connect(reset,&QPushButton::clicked,this,[this]{pause();radians_=0.;scene_->setAngle(0.);});
  preset_->setCurrentIndex(1);
  applyPreset(1);pause();
 }
 Settings settings()const{
  Settings s;s.preset=Template(preset_->currentIndex());
  s.material=Material(material_->currentIndex());s.thicknessMm=thickness_->value();
  s.moduleMm=module_->value();s.pressureDeg=pressure_->value();
  s.boreMm=bore_->value();s.kerfMm=kerf_->value();
  s.backlashMm=backlash_->value();
  s.teeth1=driver_->value();s.teeth2=driven_->value();
  return s;
 }
 void accept()override{
  try{result_=generate(settings());}
  catch(const std::exception& e){QMessageBox::warning(this,"Projeto inválido",e.what());return;}
  pause();QDialog::accept();
 }
 void reject()override{pause();QDialog::reject();}
};
} // namespace alaser::mechanisms
