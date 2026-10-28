#pragma once
// Alencar Laser 19.8 - read-only preview. No serial/TCP/G-code/machine calls.
// Motion estimate is approximate: simplified axes + stop/start at angular runs.
#include "preview_sim_19_8.hpp"
#include "preview_motion_19_8.hpp"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QColor>
#include <QTimer>
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>
#include <utility>

namespace alaser::sim198 {
inline void showPreviewDialog(QWidget* parent,const EditorState& source,
                               double drawingWidthMm,double drawingHeightMm){
 const Plan sourcePlan=build(source);
 Plan shown=sourcePlan;
 motion::Settings settings;
 motion::Timeline timeline;
 QDialog dialog(parent);
 dialog.setWindowTitle("Alencar Laser 19.8 - Simulacao OFFLINE (sem disparos)");
 dialog.resize(880,800);
 auto* column=new QVBoxLayout(&dialog);
 auto* note=new QLabel(
   "PREVIA EXPERIMENTAL: apenas linhas vetoriais. Sem envio, G-code ou comandos fisicos. "
   "Perfil simplificado X/Y com parada em mudancas de direcao. Nao replica a "
   "cinematica GRBL, interpolacao, otimizacao ou preenchimento do LightBurn. "
   "Todos os valores abaixo sao HIPOTETICOS ate serem aferidos.");
 note->setWordWrap(true);column->addWidget(note);
 auto* settingsGrid=new QGridLayout;
 auto* cut=new QDoubleSpinBox;cut->setRange(10,100000);cut->setValue(3000);
 auto* rapid=new QDoubleSpinBox;rapid->setRange(10,100000);rapid->setValue(7000);
 auto* maxX=new QDoubleSpinBox;maxX->setRange(10,100000);maxX->setValue(12000);
 auto* maxY=new QDoubleSpinBox;maxY->setRange(10,100000);maxY->setValue(12000);
 auto* accelX=new QDoubleSpinBox;accelX->setRange(1,100000);accelX->setValue(400);
 auto* accelY=new QDoubleSpinBox;accelY->setRange(1,100000);accelY->setValue(300);
 auto* rapidAx=new QDoubleSpinBox;rapidAx->setRange(1,100000);rapidAx->setValue(500);
 auto* rapidAy=new QDoubleSpinBox;rapidAy->setRange(1,100000);rapidAy->setValue(400);
 auto* passes=new QSpinBox;passes->setRange(1,20);passes->setValue(1);
 for(auto* box:{cut,rapid,maxX,maxY,accelX,accelY,rapidAx,rapidAy}){
  box->setDecimals(0);box->setSingleStep(100);
 }
 for(auto* box:{cut,rapid,maxX,maxY})box->setSuffix(" mm/min");
 for(auto* box:{accelX,accelY,rapidAx,rapidAy})box->setSuffix(" mm/s²");
 auto place=[&](int row,int col,const QString& text,QWidget* widget){
  settingsGrid->addWidget(new QLabel(text),row,col*2);
  settingsGrid->addWidget(widget,row,col*2+1);
 };
 place(0,0,"Vel. linha",cut);place(0,1,"Vel. viagem",rapid);
 place(1,0,"Limite X",maxX);place(1,1,"Limite Y",maxY);
 place(2,0,"Aceleracao linha X",accelX);place(2,1,"Aceleracao linha Y",accelY);
 place(3,0,"Aceleracao viagem X",rapidAx);place(3,1,"Aceleracao viagem Y",rapidAy);
 place(4,0,"Passagens (plano inteiro)",passes);
 column->addLayout(settingsGrid);
 auto* stats=new QLabel;stats->setWordWrap(true);column->addWidget(stats);
 auto* canvas=new QLabel;canvas->setMinimumHeight(280);
 canvas->setAlignment(Qt::AlignCenter);column->addWidget(canvas,1);
 auto* progress=new QSlider(Qt::Horizontal);progress->setRange(0,1000);
 progress->setValue(1000);progress->setToolTip("Percorrer o TEMPO virtual, nao apenas o numero de linhas");
 column->addWidget(progress);
 auto* controls=new QHBoxLayout;
 auto* play=new QPushButton("▶ Reproduzir");play->setObjectName("preview_play");
 auto* speed=new QDoubleSpinBox;speed->setRange(0.1,40);speed->setValue(10);
 speed->setSingleStep(1);speed->setSuffix("×");speed->setToolTip("Velocidade apenas da animacao");
 auto* timeLabel=new QLabel("0 / 0 s");timeLabel->setObjectName("preview_time");
 auto* showTravel=new QCheckBox("Mostrar viagens em vermelho");showTravel->setChecked(true);
 controls->addWidget(play);controls->addWidget(speed);controls->addWidget(timeLabel);
 controls->addStretch();controls->addWidget(showTravel);column->addLayout(controls);
 auto* close=new QPushButton("Fechar previsualizacao");column->addWidget(close);
 QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::accept);
 QTimer clock(&dialog);clock.setInterval(60);
 QElapsedTimer elapsedClock;
 double playbackTime=0;
 auto stop=[&](){
  clock.stop();play->setText("▶ Reproduzir");
 };
 auto render=[&](){
  const double now=timeline.totalSeconds*double(progress->value())/1000.0;
  timeLabel->setText(QString("%1 / %2 s").arg(now,0,'f',1).arg(timeline.totalSeconds,0,'f',1));
  QImage image(740,440,QImage::Format_ARGB32);image.fill(QColor("#16252F"));
  QPainter painter(&image);painter.setRenderHint(QPainter::Antialiasing,true);
  const double w=std::max(1.0,drawingWidthMm),h=std::max(1.0,drawingHeightMm);
  const double factor=std::min(660.0/w,365.0/h);
  const auto map=[&](Pt p){return QPointF(35+p.x*factor,35+p.y*factor);};
  painter.setPen(QPen(QColor("#598777"),1.4));
  painter.drawRect(QRectF(35,35,w*factor,h*factor));
  const std::size_t count=shown.moves.size();
  std::size_t completed=0;
  if(progress->value()==1000)completed=count;
  else if(progress->value()>0&&count>0){
   completed=std::size_t(std::lower_bound(timeline.endSeconds.begin(),
                      timeline.endSeconds.end(),now)-timeline.endSeconds.begin());
  }
  for(std::size_t i=0;i<completed;++i){
   const auto& move=shown.moves[i];
   if(!move.laser&&!showTravel->isChecked())continue;
   painter.setPen(QPen(move.laser?QColor("#E8ECEC"):QColor("#F26969"),move.laser?1.3:1.0));
   painter.drawLine(map(move.from),map(move.to));
  }
  if(completed<count&&progress->value()>0){
   const double start=completed?timeline.endSeconds[completed-1]:0.0;
   const double finish=timeline.endSeconds[completed];
   const double fraction=finish>start?std::clamp((now-start)/(finish-start),0.0,1.0):0.0;
   const auto& move=shown.moves[completed];
   if(move.laser||showTravel->isChecked()){
    const Pt partial{move.from.x+(move.to.x-move.from.x)*fraction,
                     move.from.y+(move.to.y-move.from.y)*fraction};
    painter.setPen(QPen(move.laser?QColor("#E8ECEC"):QColor("#F26969"),1.3));
    painter.drawLine(map(move.from),map(partial));
   }
  }
  painter.end();canvas->setPixmap(QPixmap::fromImage(image));
 };
 auto recalculate=[&](){
  stop();
  try{
   settings.cutMmMin=cut->value();settings.rapidMmMin=rapid->value();
   settings.maxXmmMin=maxX->value();settings.maxYmmMin=maxY->value();
   settings.cutAccelX=accelX->value();settings.cutAccelY=accelY->value();
   settings.rapidAccelX=rapidAx->value();settings.rapidAccelY=rapidAy->value();
   shown=repeated(sourcePlan,passes->value());
   timeline=motion::estimate(shown,settings);
   const double nominal=minutes(shown,settings.cutMmMin,settings.rapidMmMin)*60.0;
   stats->setText(QString("Linha %1 mm | viagem %2 mm | tempo nominal %3 s | "
     "tempo c/ aceleracao %4 s | grupos de movimento %5 | "
     "ignorados: Fill %6, Saida off %7, guias T1/T2 %8. "
     "Passagens repetem o plano inteiro, sem ordenacao do LightBurn.")
     .arg(shown.cutMm,0,'f',1).arg(shown.travelMm,0,'f',1)
     .arg(nominal,0,'f',1).arg(timeline.totalSeconds,0,'f',1)
     .arg(qulonglong(timeline.runs)).arg(qulonglong(shown.ignoredFill))
     .arg(qulonglong(shown.ignoredDisabled)).arg(qulonglong(shown.ignoredTool)));
   play->setEnabled(!shown.moves.empty()&&timeline.totalSeconds>0);
   render();
  }catch(const std::exception& e){
   shown={};timeline={};
   stats->setText(QString("Previa limitada: %1").arg(QString::fromUtf8(e.what())));
   play->setEnabled(false);render();
  }
 };
 QObject::connect(progress,&QSlider::valueChanged,&dialog,[&](int){render();});
 for(auto* box:{cut,rapid,maxX,maxY,accelX,accelY,rapidAx,rapidAy})
  QObject::connect(box,QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                   &dialog,[&](double){recalculate();});
 QObject::connect(passes,QOverload<int>::of(&QSpinBox::valueChanged),
                  &dialog,[&](int){recalculate();});
 QObject::connect(showTravel,&QCheckBox::toggled,&dialog,[&](bool){render();});
 QObject::connect(play,&QPushButton::clicked,&dialog,[&](){
  if(clock.isActive()){stop();return;}
  if(timeline.totalSeconds<=0)return;
  if(progress->value()>=1000)progress->setValue(0);
  playbackTime=timeline.totalSeconds*double(progress->value())/1000.0;
  elapsedClock.start();clock.start();play->setText("Ⅱ Pausar");
 });
 QObject::connect(&clock,&QTimer::timeout,&dialog,[&](){
  playbackTime+=elapsedClock.restart()*0.001*speed->value();
  if(playbackTime>=timeline.totalSeconds){progress->setValue(1000);stop();return;}
  progress->setValue(int(std::clamp(playbackTime/timeline.totalSeconds*1000.0,0.0,1000.0)));
 });
 recalculate();
 dialog.exec();
}
} // namespace alaser::sim198
