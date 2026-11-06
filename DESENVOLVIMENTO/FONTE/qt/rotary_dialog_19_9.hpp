#pragma once
// TESTE_19.9: configuraçao de rotativo apenas para PRÉVIA OFFLINE.
// Sem teste de motor, geração de G-code, escrita USB/TCP ou firmware.
#include "rotary_geometry_19_9.hpp"
#include <QDialog>
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QPainter>
#include <QSignalBlocker>
#include <QMessageBox>
#include <QPaintEvent>
#include <cmath>
#include <optional>

namespace alaser::rotary {
class RotaryDrawing : public QWidget {
 Type mode_=Type::Roller;
public:
 explicit RotaryDrawing(QWidget* parent=nullptr):QWidget(parent){
  setMinimumSize(270,120);setMaximumHeight(138);
  setAccessibleName("Diagrama do acessório rotativo");
 }
 void setMode(Type mode){mode_=mode;update();}
protected:
 void paintEvent(QPaintEvent*)override {
  QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
  const QRectF r=rect().adjusted(10,7,-10,-7);
  p.setPen(QPen(QColor("#889CAD"),2));
  p.setBrush(QColor("#26303A"));
  p.drawRoundedRect(QRectF(r.left()+14,r.bottom()-27,r.width()-28,22),6,6);
  if(mode_==Type::Roller){
   for(int i=0;i<2;++i){
    const qreal y=r.center().y()+(i==0?-20:24);
    p.setPen(QPen(QColor("#A6B2BE"),2));p.setBrush(QColor("#485766"));
    p.drawRoundedRect(QRectF(r.left()+30,y-10,r.width()-60,20),10,10);
   }
   p.setBrush(QColor("#566D7B"));p.setPen(QPen(QColor("#F2A14C"),2));
   p.drawEllipse(QRectF(r.center().x()-40,r.center().y()-31,80,62));
   p.setPen(QColor("#ECF0F4"));p.drawText(r.adjusted(10,0,-10,-6),Qt::AlignTop|Qt::AlignHCenter,"ROLO");
  }else{
   p.setPen(QPen(QColor("#B6C4D0"),2));p.setBrush(QColor("#536575"));
   p.drawRoundedRect(QRectF(r.center().x()-78,r.center().y()-25,120,50),18,18);
   p.setBrush(QColor("#F2A14C"));
   p.drawEllipse(QRectF(r.center().x()-108,r.center().y()-25,39,50));
   p.setPen(QColor("#ECF0F4"));p.drawText(r.adjusted(10,0,-10,-6),Qt::AlignTop|Qt::AlignHCenter,"MANDRIL");
  }
 }
};

class RotarySetupDialog final : public QDialog {
 QSettings& settings_;
 std::optional<double> artworkExtentMm_;
 QRadioButton* roller_=nullptr;
 QRadioButton* chuck_=nullptr;
 QCheckBox* enabled_=nullptr;
 QCheckBox* mirror_=nullptr;
 QComboBox* axis_=nullptr;
 QDoubleSpinBox* mmPerTurn_=nullptr;
 QDoubleSpinBox* rollerDiameter_=nullptr;
 QDoubleSpinBox* objectDiameter_=nullptr;
 QDoubleSpinBox* circumference_=nullptr;
 QLabel* preview_=nullptr;
 RotaryDrawing* diagram_=nullptr;
 bool syncMeasurements_=false;
 static QDoubleSpinBox* mmField(QWidget* parent,const QString& id,
                               const QString& tip){
  auto* b=new QDoubleSpinBox(parent);
  b->setObjectName(id);b->setDecimals(3);b->setRange(0.,100000.);
  b->setSingleStep(1.);b->setSpecialValueText("Não informado");
  b->setSuffix(" mm");b->setToolTip(tip);
  b->setMinimumWidth(135);
  return b;
 }
 void refresh(){
  const bool isRoller=roller_->isChecked();
  rollerDiameter_->setVisible(isRoller);
  auto* caption=findChild<QLabel*>("rotary_roller_label_19_9");
  if(caption)caption->setVisible(isRoller);
  diagram_->setMode(isRoller?Type::Roller:Type::Chuck);
  const double object=objectDiameter_->value();
  QString line=object>0?
    QString("Circunferência do objeto: %1 mm").arg(circumferenceMm(object),0,'f',2):
    QString("Informe o diâmetro do objeto para calcular a circunferência.");
  if(artworkExtentMm_&&object>0){
   const auto c=circumferenceMm(object);
   line+=QString("\nAltura da seleção no eixo Y visual: %1 mm (%2% do perímetro).")
      .arg(*artworkExtentMm_,0,'f',2).arg(100*(*artworkExtentMm_)/c,0,'f',1);
   if(*artworkExtentMm_>c+1e-7)
    line+="\nATENÇÃO: a arte ultrapassa a circunferência calculada.";
  }
  line+="\nEixo e calibração são apenas dados salvos; nenhum motor será movimentado.";
  preview_->setText(line);
 }
 Profile current()const{
  Profile p;
  p.type=roller_->isChecked()?Type::Roller:Type::Chuck;
  p.requestedAxis=static_cast<Axis>(axis_->currentIndex());
  p.previewEnabled=enabled_->isChecked();
  p.mirrorPreview=mirror_->isChecked();
  p.mmPerRotation=mmPerTurn_->value();
  p.rollerDiameterMm=rollerDiameter_->value();
  p.objectDiameterMm=objectDiameter_->value();
  return p;
 }
 void load(){
  settings_.beginGroup("rotary_offline_v1");
  int type=settings_.value("type",0).toInt();
  if(type==1)chuck_->setChecked(true);else roller_->setChecked(true);
  const int axis=settings_.value("axis",0).toInt();
  axis_->setCurrentIndex(axis>=0&&axis<=2?axis:0);
  enabled_->setChecked(settings_.value("preview_enabled",false).toBool());
  mirror_->setChecked(settings_.value("mirror_preview",false).toBool());
  mmPerTurn_->setValue(settings_.value("mm_per_turn",0.).toDouble());
  rollerDiameter_->setValue(settings_.value("roller_diameter",0.).toDouble());
  objectDiameter_->setValue(settings_.value("object_diameter",0.).toDouble());
  settings_.endGroup();
  refresh();
 }
public:
 explicit RotarySetupDialog(QSettings& settings,
                  std::optional<double> extentMm=std::nullopt,
                  QWidget* parent=nullptr):QDialog(parent),settings_(settings),
                    artworkExtentMm_(extentMm){
  setObjectName("rotary_setup_offline_19_9");
  setWindowTitle("Configuração do rotativo — Alencar Laser (OFFLINE)");
  setMinimumSize(560,585);resize(640,650);
  auto* root=new QVBoxLayout(this);root->setSpacing(8);
  auto* warning=new QLabel("PRÉVIA OFFLINE • MÁQUINA DESCONECTADA • EMISSÃO BLOQUEADA",this);
  warning->setObjectName("rotary_warning_19_9");
  warning->setStyleSheet("color:#F2A14C;font-weight:bold;padding:5px;");
  warning->setWordWrap(true);root->addWidget(warning);
  auto* typeBox=new QGroupBox("Tipo de rotativo",this);
  auto* typeCol=new QVBoxLayout(typeBox);
  auto* choice=new QHBoxLayout;
  roller_=new QRadioButton("Rolo (Ortur — modelo a confirmar)",typeBox);
  roller_->setObjectName("rotary_roller_19_9");roller_->setChecked(true);
  chuck_=new QRadioButton("Mandril",typeBox);
  chuck_->setObjectName("rotary_chuck_19_9");
  choice->addWidget(roller_);choice->addWidget(chuck_);typeCol->addLayout(choice);
  diagram_=new RotaryDrawing(typeBox);diagram_->setObjectName("rotary_diagram_19_9");
  typeCol->addWidget(diagram_);root->addWidget(typeBox);
  enabled_=new QCheckBox("Habilitar modo rotativo na simulação (sem ligação física)",this);
  enabled_->setObjectName("rotary_enabled_offline_19_9");root->addWidget(enabled_);
  mirror_=new QCheckBox("Espelhar orientação na prévia rotativa",this);
  mirror_->setObjectName("rotary_mirror_preview_19_9");root->addWidget(mirror_);
  auto* form=new QFormLayout;form->setHorizontalSpacing(10);form->setVerticalSpacing(7);
  axis_=new QComboBox(this);axis_->setObjectName("rotary_axis_19_9");
  axis_->addItems({"Eixo Y (não confirmado)","Eixo Z (não confirmado)",
                    "Eixo A (não confirmado)"});
  axis_->setToolTip("Referência no planejamento. A conexão real dependerá do manual do controlador.");
  form->addRow("Eixo pretendido:",axis_);
  mmPerTurn_=mmField(this,"rotary_mm_per_turn_19_9",
    "Movimento linear comandado para uma volta do rolo; obter do manual e confirmar.");
  form->addRow("mm por rotação:",mmPerTurn_);
  rollerDiameter_=mmField(this,"rotary_roller_diameter_19_9",
    "Meça o rolete MOTORIZADO, não o copo.");
  auto* rollerLabel=new QLabel("Diâmetro do rolete:",this);
  rollerLabel->setObjectName("rotary_roller_label_19_9");
  form->addRow(rollerLabel,rollerDiameter_);
  objectDiameter_=mmField(this,"rotary_object_diameter_19_9",
    "Diâmetro medido do objeto. Em rolo, serve à circunferência da prévia.");
  form->addRow("Diâmetro do objeto:",objectDiameter_);
  circumference_=mmField(this,"rotary_circumference_19_9",
    "Ajustar a circunferência atualiza o diâmetro do objeto.");
  form->addRow("Circunferência:",circumference_);
  root->addLayout(form);
  preview_=new QLabel(this);preview_->setObjectName("rotary_preview_19_9");
  preview_->setWordWrap(true);preview_->setMinimumHeight(65);
  root->addWidget(preview_);
  auto* footer=new QHBoxLayout;
  auto* test=new QPushButton("Testar rotação — BLOQUEADO",this);
  test->setObjectName("rotary_test_motor_disabled_19_9");test->setEnabled(false);
  test->setToolTip("A integração física ainda não foi homologada.");
  footer->addWidget(test);
  auto* actions=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,this);
  actions->button(QDialogButtonBox::Save)->setText("Salvar perfil offline");
  actions->button(QDialogButtonBox::Cancel)->setText("Cancelar");
  footer->addWidget(actions);root->addLayout(footer);
  connect(actions,&QDialogButtonBox::accepted,this,&RotarySetupDialog::accept);
  connect(actions,&QDialogButtonBox::rejected,this,&RotarySetupDialog::reject);
  connect(roller_,&QRadioButton::toggled,this,[this](bool){refresh();});
  connect(objectDiameter_,QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this,[this](double diameter){
   if(syncMeasurements_)return;
   syncMeasurements_=true;
   {const QSignalBlocker b(circumference_);
    circumference_->setValue(diameter>0?circumferenceMm(diameter):0.);}
   syncMeasurements_=false;refresh();
  });
  connect(circumference_,QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this,[this](double circumference){
   if(syncMeasurements_)return;
   syncMeasurements_=true;
   {const QSignalBlocker b(objectDiameter_);
    objectDiameter_->setValue(circumference>0?
      circumference/std::numbers::pi:0.);}
   syncMeasurements_=false;refresh();
  });
  load();
 }
 void accept()override{
  const Profile p=current();
  try{validate(p);}catch(const std::exception& e){
   QMessageBox::warning(this,"Perfil incompleto",
     QString::fromUtf8(e.what())+
     "\nDesative a prévia ou informe medidas reais. Não invente calibração.");
   return;
  }
  settings_.beginGroup("rotary_offline_v1");
  settings_.setValue("type",int(p.type));
  settings_.setValue("axis",int(p.requestedAxis));
  settings_.setValue("preview_enabled",p.previewEnabled);
  settings_.setValue("mirror_preview",p.mirrorPreview);
  settings_.setValue("mm_per_turn",p.mmPerRotation);
  settings_.setValue("roller_diameter",p.rollerDiameterMm);
  settings_.setValue("object_diameter",p.objectDiameterMm);
  settings_.endGroup();settings_.sync();
  QDialog::accept();
 }
};
} // namespace alaser::rotary
