#pragma once
// Alencar Laser 19.8 - read-only preview. No serial/TCP/G-code/machine calls.
// Motion estimate is approximate: simplified axes + stop/start at angular runs.
#include "preview_sim_19_8.hpp"
#include "preview_motion_19_8.hpp"
#include "preview_raster_19_8.hpp"
#include "preview_route_19_8.hpp"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QRectF>
#include <QCheckBox>
#include <QPushButton>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QColor>
#include <QTimer>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QSaveFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <algorithm>
#include <cmath>
#include <utility>

namespace alaser::sim198 {
inline void showPreviewDialog(QWidget* parent,const EditorState& source,
                               double drawingWidthMm,double drawingHeightMm,
                               const QImage& sourceImage=QImage(),
                               const QRectF& imageBounds=QRectF()){
 const Plan sourcePlan=build(source);
 Plan shown=sourcePlan;
 motion::Settings settings;
 motion::Timeline timeline;
 QDialog dialog(parent);
 dialog.setWindowTitle("Alencar Laser 19.8 - Simulacao OFFLINE (sem disparos)");
 dialog.resize(880,800);
 auto* column=new QVBoxLayout(&dialog);
 auto* note=new QLabel(
   "PREVIA EXPERIMENTAL OFFLINE: contornos Line, Fill vetorial e bitmap P&B "
   "com trajetorias de varredura simplificadas; overscan apenas visual. "
   "Nao replica o planejador GRBL/LightBurn, nem calibra potencia ou movimento fisico. "
   "Sem envio de G-code, rede ou comandos ao laser.");
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
 // Roteiros de Fill/Imagem sao virtuais; sourceImage nunca e modificada.
 auto* rasterControls=new QGridLayout;
 auto* enableFill=new QCheckBox("Simular Fill vetorial (contornos fechados)");
 enableFill->setChecked(true);
 auto* enableImage=new QCheckBox("Simular imagem importada P&B");
 const bool imagePresent=!sourceImage.isNull()&&imageBounds.isValid()&&
                         imageBounds.width()>0&&imageBounds.height()>0;
 enableImage->setChecked(imagePresent);enableImage->setEnabled(imagePresent);
 enableImage->setToolTip(imagePresent?
    "Imagem original intacta; simula limiar/dither sem aplicar vetorizacao":
    "Nenhuma imagem importada na mesa");
 auto* interval=new QDoubleSpinBox;interval->setRange(.05,5.0);
 interval->setDecimals(2);interval->setSingleStep(.05);
 interval->setValue(.20);interval->setSuffix(" mm");
 auto* overscan=new QDoubleSpinBox;overscan->setRange(0,20.0);
 overscan->setDecimals(1);overscan->setSingleStep(.5);
 overscan->setValue(1.0);overscan->setSuffix(" mm");
 auto* bidirectional=new QCheckBox("Varredura bidirecional");
 bidirectional->setChecked(true);
 auto* bitmapMode=new QComboBox;
 bitmapMode->addItem("Limiar P&B",0);bitmapMode->addItem("Pontilhamento ordenado (Bayer)",1);
 auto* bitmapThreshold=new QSpinBox;bitmapThreshold->setRange(1,254);
 bitmapThreshold->setValue(128);
 rasterControls->addWidget(enableFill,0,0,1,2);
 rasterControls->addWidget(enableImage,0,2,1,2);
 rasterControls->addWidget(new QLabel("Intervalo:"),1,0);
 rasterControls->addWidget(interval,1,1);
 rasterControls->addWidget(new QLabel("Overscan virtual:"),1,2);
 rasterControls->addWidget(overscan,1,3);
 rasterControls->addWidget(bidirectional,2,0,1,2);
 rasterControls->addWidget(bitmapMode,2,2);
 rasterControls->addWidget(bitmapThreshold,2,3);
 column->addLayout(rasterControls);
 auto* optimizeTravel=new QCheckBox("Reduzir deslocamentos (heuristica local por camada)");
 optimizeTravel->setToolTip("Vizinho mais proximo; reordena apenas Line, nao garante a rota global mais curta. Sem modificar o projeto.");
 auto* innerFirst=new QCheckBox("Contornos internos primeiro (corte virtual)");
 innerFirst->setObjectName("preview_inner_first");
 innerFirst->setChecked(true);
 innerFirst->setEnabled(false);
 innerFirst->setToolTip("Somente formas estritamente contidas por contornos fechados da mesma camada; pode aumentar deslocamentos.");
 QObject::connect(optimizeTravel,&QCheckBox::toggled,innerFirst,&QWidget::setEnabled);
 auto* useLayerFeeds=new QCheckBox("Utilizar velocidades virtuais salvas por camada");
 useLayerFeeds->setChecked(true);
 useLayerFeeds->setToolTip("Velocidade e passes de cada camada influenciam a simulacao. Potencia/M3/M4 continuam apenas registrados.");
 column->addWidget(optimizeTravel);column->addWidget(innerFirst);
 column->addWidget(useLayerFeeds);
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
 auto* footer=new QHBoxLayout;
 auto* exportReport=new QPushButton("Exportar relatorio JSON");
 exportReport->setObjectName("preview_export_report");
 exportReport->setToolTip("Apenas metricas e hipoteses, sem coordenadas do desenho");
 auto* close=new QPushButton("Fechar previsualizacao");
 footer->addWidget(exportReport);footer->addStretch();footer->addWidget(close);
 column->addLayout(footer);
 QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::accept);
 QTimer clock(&dialog);clock.setInterval(60);
 QElapsedTimer elapsedClock;
 double playbackTime=0;
 std::size_t nestedPairs=0;
 double optimizedLineTravelMm=sourcePlan.travelMm;
 double nominalSeconds=0.0;
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
  // Limite visual para manter o slider responsivo com imagens densas.
  const std::size_t stride=std::max<std::size_t>(1,shown.moves.size()/25000);
  for(std::size_t i=0;i<completed;i+=stride){
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
   raster::Settings scan;
   scan.intervalMm=interval->value();
   scan.overscanMm=overscan->value();
   scan.bidirectional=bidirectional->isChecked();
   nestedPairs=0;
   Plan combined=optimizeTravel->isChecked()?
      route::nearestNeighborLines(source,300000,1500,innerFirst->isChecked(),&nestedPairs):
      sourcePlan;
   optimizedLineTravelMm=combined.travelMm;
   if(enableFill->isChecked()){
    raster::appendFill(combined,source,scan);
    combined.ignoredFill=0; // Fill vetorial incluido; contornos abertos nao geram hatch.
   }
   if(enableImage->isChecked()&&imagePresent){
    // Amostragem de preview intencionalmente limitada. Sem guardar bitmap.
    const QImage image=sourceImage.scaled(320,320,Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);
    raster::Mask mask;
    mask.width=image.width();mask.height=image.height();
    mask.black.resize(std::size_t(mask.width)*std::size_t(mask.height));
    static constexpr int bayer[4][4]={
     {0,8,2,10},{12,4,14,6},{3,11,1,9},{15,7,13,5}};
    for(int y=0;y<mask.height;++y)for(int x=0;x<mask.width;++x){
     const QRgb px=image.pixel(x,y);
     const int alpha=qAlpha(px),luma=qGray(px);
     const int value=(alpha*luma+(255-alpha)*255)/255;
     const int bias=bitmapMode->currentIndex()==1?
       (bayer[y%4][x%4]*16-120):0;
     mask.black[std::size_t(y)*mask.width+x]=
       (value<std::clamp(bitmapThreshold->value()+bias,1,254))?1:0;
    }
    raster::appendMask(combined,mask,{imageBounds.x(),imageBounds.y(),
                       imageBounds.width(),imageBounds.height()},scan);
   }
   shown=repeatedByLayer(combined,source,passes->value());
   const auto layerSpeed=[&](int id){
    const auto it=source.layers.find(id);
    return useLayerFeeds->isChecked()&&it!=source.layers.end()?
      it->second.speedMmMin:settings.cutMmMin;
   };
   timeline=motion::estimateWithFeeds(shown,settings,layerSpeed);
   nominalSeconds=motion::nominalSecondsWithFeeds(shown,settings,layerSpeed);
   stats->setText(QString("Movimento virtual ativo %1 mm | viagem %2 mm | tempo nominal %3 s | "
     "tempo c/ aceleracao %4 s | grupos %5 | "
     "Fill omitido %6, Saida off %7, T1/T2 %8. "
     "Viagem original Line %9 mm; caminho escolhido %10 mm; "
     "restricoes interno-antes-externo %11. "
     "A precedencia pode aumentar a viagem. M3/M4 apenas referencia.")
     .arg(shown.cutMm,0,'f',1).arg(shown.travelMm,0,'f',1)
     .arg(nominalSeconds,0,'f',1).arg(timeline.totalSeconds,0,'f',1)
     .arg(qulonglong(timeline.runs)).arg(qulonglong(shown.ignoredFill))
     .arg(qulonglong(shown.ignoredDisabled)).arg(qulonglong(shown.ignoredTool))
     .arg(sourcePlan.travelMm,0,'f',1).arg(optimizedLineTravelMm,0,'f',1)
     .arg(qulonglong(nestedPairs)));
   play->setEnabled(!shown.moves.empty()&&timeline.totalSeconds>0);
   render();
  }catch(const std::exception& e){
   shown={};timeline={};nominalSeconds=0.0;
   stats->setText(QString("Previa limitada: %1").arg(QString::fromUtf8(e.what())));
   play->setEnabled(false);render();
  }
 };
 QObject::connect(exportReport,&QPushButton::clicked,&dialog,[&](){
  if(shown.moves.empty())return;
  const QString filename=QFileDialog::getSaveFileName(
     &dialog,"Relatorio local da simulacao",
     "Alencar_Laser_Previa_19_8.json","Relatorio JSON (*.json)");
  if(filename.isEmpty())return;
  QJsonObject data{
   {"schema","alencar-laser-simulation-report-19.8"},
   {"experimental",true},{"machine_commands_sent",false},
   {"model","kinematic-start-stop-angular-runs-12deg"},
   {"notes","Experimental Line, vector Fill, binary image. Simplified scan, no GRBL/LightBurn equivalence"},
   {"global_passes",passes->value()},
   {"layer_passes_applied",true},
   {"route_heuristic_enabled",optimizeTravel->isChecked()},
   {"inner_first_enabled",optimizeTravel->isChecked()&&innerFirst->isChecked()},
   {"strict_nesting_constraints",int(nestedPairs)},
   {"inner_first_only_same_layer",true},
   {"line_travel_original_mm",sourcePlan.travelMm},
   {"line_travel_optimized_mm",optimizedLineTravelMm},
   {"layer_feed_enabled",useLayerFeeds->isChecked()},
   {"layer_parameters_are_not_machine_commands",true},
   {"fill_simulation_enabled",enableFill->isChecked()},
   {"image_simulation_enabled",enableImage->isChecked()&&imagePresent},
   {"scan_line_interval_mm",interval->value()},
   {"virtual_overscan_mm",overscan->value()},
   {"bidirectional_scan",bidirectional->isChecked()},
   {"image_mode",bitmapMode->currentIndex()==1?"ordered_dither":"threshold"},
   {"image_threshold",bitmapThreshold->value()},
   {"cut_distance_mm",shown.cutMm},{"travel_distance_mm",shown.travelMm},
   {"nominal_seconds",nominalSeconds},
   {"nominal_speed_model","layer_feed_when_enabled_else_global"},
   {"accel_estimated_seconds",timeline.totalSeconds},
   {"line_seconds",timeline.lineSeconds},{"travel_seconds",timeline.travelSeconds},
   {"motion_runs",int(timeline.runs)},
   {"omitted_fill",int(shown.ignoredFill)},
   {"omitted_output_off",int(shown.ignoredDisabled)},
   {"omitted_tool_layers",int(shown.ignoredTool)},
   {"cut_feed_mm_min",settings.cutMmMin},{"travel_feed_mm_min",settings.rapidMmMin},
   {"x_max_mm_min",settings.maxXmmMin},{"y_max_mm_min",settings.maxYmmMin},
   {"line_accel_x_mm_s2",settings.cutAccelX},
   {"line_accel_y_mm_s2",settings.cutAccelY},
   {"travel_accel_x_mm_s2",settings.rapidAccelX},
   {"travel_accel_y_mm_s2",settings.rapidAccelY}
  };
  QSaveFile output(filename);
  const QByteArray bytes=QJsonDocument(data).toJson(QJsonDocument::Indented);
  if(!output.open(QIODevice::WriteOnly)||output.write(bytes)!=bytes.size()||!output.commit()){
   QMessageBox::warning(&dialog,"Exportacao falhou",
      "Nao foi possivel salvar o relatorio. Nenhum dado foi enviado.");
  }
 });
 QObject::connect(progress,&QSlider::valueChanged,&dialog,[&](int){render();});
 for(auto* box:{cut,rapid,maxX,maxY,accelX,accelY,rapidAx,rapidAy})
  QObject::connect(box,QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                   &dialog,[&](double){recalculate();});
 QObject::connect(passes,QOverload<int>::of(&QSpinBox::valueChanged),
                  &dialog,[&](int){recalculate();});
 for(auto* box:{interval,overscan})
  QObject::connect(box,QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                   &dialog,[&](double){recalculate();});
 for(auto* checkbox:{enableFill,enableImage,bidirectional,optimizeTravel,innerFirst,useLayerFeeds})
  QObject::connect(checkbox,&QCheckBox::toggled,&dialog,[&](bool){recalculate();});
 QObject::connect(bitmapMode,QOverload<int>::of(&QComboBox::currentIndexChanged),
                  &dialog,[&](int){recalculate();});
 QObject::connect(bitmapThreshold,QOverload<int>::of(&QSpinBox::valueChanged),
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
