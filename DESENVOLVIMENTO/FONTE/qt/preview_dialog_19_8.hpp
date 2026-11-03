#pragma once
// Alencar Laser 19.8 - read-only preview. No serial/TCP/G-code/machine calls.
// Motion estimate is approximate: simplified axes + stop/start at angular runs.
#include "preview_sim_19_8.hpp"
#include "preview_native_19_8.hpp"
#include "preview_motion_19_8.hpp"
#include "preview_raster_19_8.hpp"
#include "preview_route_19_8.hpp"
#include "preview_workarea_19_8.hpp"
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
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QPainterPath>
#include <QPen>
#include <QScrollArea>
#include <QScrollBar>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScreen>
#include <QGuiApplication>
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
// Preview camera only. Wheel zoom never changes plan, speed, time or geometry.
class PreviewCanvas198 final:public QGraphicsView {
 QGraphicsScene scene_;
 QGraphicsPixmapItem* picture_=nullptr;
 QGraphicsRectItem* bed_=nullptr;
 QGraphicsPathItem* linePaths_=nullptr;
 QGraphicsPathItem* travelPaths_=nullptr;
 QGraphicsPathItem* activePath_=nullptr;
 double zoom_=1.0;
 bool moving_=false;
 QPoint last_{};
public:
 explicit PreviewCanvas198(QWidget* parent=nullptr):QGraphicsView(parent){
  setObjectName("preview_canvas_zoom_19_8");
  setScene(&scene_);
  picture_=scene_.addPixmap(QPixmap());
  picture_->setTransformationMode(Qt::SmoothTransformation);
  bed_=scene_.addRect(QRectF(35,35,660,365),QPen(QColor("#598777"),1.0));
  linePaths_=scene_.addPath(QPainterPath());
  travelPaths_=scene_.addPath(QPainterPath());
  activePath_=scene_.addPath(QPainterPath());
  QPen cutting(QColor("#E8ECEC"),1.3);
  QPen moving(QColor("#F26969"),1.0);
  QPen active(QColor("#F6C068"),1.6);
  for(QPen* pen:{&cutting,&moving,&active})pen->setCosmetic(true);
  linePaths_->setPen(cutting);travelPaths_->setPen(moving);activePath_->setPen(active);
  for(QGraphicsPathItem* item:{linePaths_,travelPaths_,activePath_}){
   item->setBrush(Qt::NoBrush);item->setAcceptedMouseButtons(Qt::NoButton);
   item->setVisible(false);
  }
  bed_->setVisible(false);bed_->setAcceptedMouseButtons(Qt::NoButton);
  setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);
  setBackgroundBrush(QColor("#16252F"));
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  setResizeAnchor(QGraphicsView::AnchorViewCenter);
  setFrameShape(QFrame::NoFrame);
 }
 ~PreviewCanvas198()override{setScene(nullptr);}
 void showImage(const QImage& image){
  picture_->setPixmap(QPixmap::fromImage(image));
  picture_->setVisible(true);bed_->setVisible(false);
  for(QGraphicsPathItem* item:{linePaths_,travelPaths_,activePath_})item->setVisible(false);
  scene_.setSceneRect(picture_->boundingRect());
 }
 // Mantem a geometria vetorial na cena, sem QImage 740x440 esticada no zoom.
 // Cores, larguras e trajetos sao apenas visuais: nenhuma emissao ou G-code.
 void showPaths(const QPainterPath& lines,const QPainterPath& travels,
                const QPainterPath& current,bool showTravel,
                double drawingWidth,double drawingHeight){
  picture_->setVisible(false);
  const double w=std::max(1.,drawingWidth),h=std::max(1.,drawingHeight);
  const double factor=std::min(660./w,365./h);
  bed_->setRect(QRectF(35,35,w*factor,h*factor));bed_->setVisible(true);
  linePaths_->setPath(lines);linePaths_->setVisible(true);
  travelPaths_->setPath(travels);travelPaths_->setVisible(showTravel);
  activePath_->setPath(current);activePath_->setVisible(!current.isEmpty());
  scene_.setSceneRect(QRectF(0,0,740,440));
 }
 bool vectorMode()const{return !picture_->isVisible();}
 void resetView(){
  if(scene_.sceneRect().isEmpty())return;
  resetTransform();
  fitInView(scene_.sceneRect(),Qt::KeepAspectRatio);
  zoom_=1.0;
 }
 double zoomLevel()const{return zoom_;}
protected:
 void wheelEvent(QWheelEvent* event)override{
  if(event->angleDelta().y()==0){event->ignore();return;}
  const double factor=event->angleDelta().y()>0?1.22:1./1.22;
  if(zoom_*factor>=.1&&zoom_*factor<=20.){
   scale(factor,factor);zoom_*=factor;
  }
  event->accept();
 }
 void mousePressEvent(QMouseEvent* event)override{
  if(event->button()==Qt::MiddleButton){
   moving_=true;last_=event->position().toPoint();
   viewport()->setCursor(Qt::ClosedHandCursor);event->accept();return;
  }
  QGraphicsView::mousePressEvent(event);
 }
 void mouseMoveEvent(QMouseEvent* event)override{
  if(moving_){
   QPoint delta=event->position().toPoint()-last_;last_=event->position().toPoint();
   horizontalScrollBar()->setValue(horizontalScrollBar()->value()-delta.x());
   verticalScrollBar()->setValue(verticalScrollBar()->value()-delta.y());
   event->accept();return;
  }
  QGraphicsView::mouseMoveEvent(event);
 }
 void mouseReleaseEvent(QMouseEvent* event)override{
  if(moving_&&event->button()==Qt::MiddleButton){
   moving_=false;viewport()->setCursor(Qt::ArrowCursor);
   event->accept();return;
  }
  QGraphicsView::mouseReleaseEvent(event);
 }
};
inline void showPreviewDialog(QWidget* parent,const EditorState& source,
                               double drawingWidthMm,double drawingHeightMm,
                               const QImage& sourceImage=QImage(),
                               const QRectF& imageBounds=QRectF(),
                               const CurveDocument* nativeCurves=nullptr){
 const auto nativePreview=withNativeCurves(source,nativeCurves);
 const EditorState& planningSource=nativePreview.state;
 const Plan sourcePlan=build(planningSource);
 Plan shown=sourcePlan;
 motion::Settings settings;
 motion::Timeline timeline;
 QDialog dialog(parent);
 dialog.setWindowTitle("Alencar Laser 19.8 - Simulacao OFFLINE (sem disparos)");
 const QScreen* screen=parent?parent->screen():QGuiApplication::primaryScreen();
 const QRect area=screen?screen->availableGeometry():QRect(0,0,1000,700);
 dialog.resize(std::min(980,std::max(520,area.width()-38)),
               std::min(740,std::max(410,area.height()-50)));
 auto* column=new QVBoxLayout(&dialog);
 column->setContentsMargins(9,7,9,7);
 auto* settingsBody=new QWidget;
 auto* settingsColumn=new QVBoxLayout(settingsBody);
 settingsColumn->setContentsMargins(5,4,5,4);
 settingsColumn->setSpacing(4);
 auto* note=new QLabel(
   "PREVIA EXPERIMENTAL OFFLINE: Line, curvas Bézier v5 (abertas/fechadas), "
   "Fill vetorial e bitmap P&B. Curvas nativas fechadas sao uma unica forma "
   "editavel no projeto; apenas a previa cria segmentos temporarios. " 
   "Varredura e overscan simplificados, somente visuais. "
   "Nao replica o planejador GRBL/LightBurn, nem calibra potencia ou movimento fisico. "
   "Sem envio de G-code, rede ou comandos ao laser.");
 note->setWordWrap(true);settingsColumn->addWidget(note);
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
 settingsColumn->addLayout(settingsGrid);
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
 interval->setObjectName("preview_global_line_interval_19_8");
 interval->setDecimals(3);interval->setSingleStep(.05);
 interval->setValue(.20);interval->setSuffix(" mm");
 interval->setToolTip("Intervalo global: aplica-se às imagens; "
    "ao Fill aplica-se apenas quando desmarcado o uso dos intervalos por camada.");
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
 rasterControls->addWidget(new QLabel("Intervalo global:"),1,0);
 rasterControls->addWidget(interval,1,1);
 rasterControls->addWidget(new QLabel("Overscan virtual:"),1,2);
 rasterControls->addWidget(overscan,1,3);
 rasterControls->addWidget(bidirectional,2,0,1,2);
 rasterControls->addWidget(bitmapMode,2,2);
 rasterControls->addWidget(bitmapThreshold,2,3);
 settingsColumn->addLayout(rasterControls);
 auto* useLayerIntervals=new QCheckBox("Fill: usar intervalo entre linhas salvo por camada");
 useLayerIntervals->setObjectName("preview_use_layer_intervals_19_8");
 useLayerIntervals->setToolTip("Ativado: cada camada Fill usa seu intervalo em mm. "
    "Desativado: Fill usa o global. Imagens sempre usam o intervalo global. "
    "Somente simulação OFFLINE; independente de M3/M4.");
 // Preserve the legacy global interval on projects without custom Fill
 // settings. Once the operator changes a Fill layer pitch, select per-layer
 // scanning automatically; users can still override with the checkbox.
 bool customFillPitch=false;
 for(const auto& [id,layer]:source.layers){
  (void)id;
  if(layer.mode=="Fill"&&std::abs(layer.lineIntervalMm-0.20)>1e-8){
   customFillPitch=true;break;
  }
 }
 useLayerIntervals->setChecked(customFillPitch);
 settingsColumn->addWidget(useLayerIntervals);
 auto* optimizeTravel=new QCheckBox("Reduzir deslocamentos (heuristica local por camada)");
 optimizeTravel->setToolTip("Vizinho mais proximo; reordena apenas Line, nao garante a rota global mais curta. Sem modificar o projeto.");
 auto* innerFirst=new QCheckBox("Contornos internos primeiro (corte virtual)");
 innerFirst->setObjectName("preview_inner_first");
 innerFirst->setChecked(true);
 innerFirst->setEnabled(false);
 innerFirst->setToolTip("Somente formas estritamente contidas por contornos fechados da mesma camada; pode aumentar deslocamentos.");
 QObject::connect(optimizeTravel,&QCheckBox::toggled,innerFirst,&QWidget::setEnabled);
 auto* restrictBed=new QCheckBox("Simular somente dentro da mesa (recomendado)");
 restrictBed->setObjectName("preview_restrict_bed_19_8");
 restrictBed->setChecked(true);
 restrictBed->setToolTip("Nao altera desenhos; corta trajetos somente na simulacao e mostra quantos ficaram fora da mesa.");
 auto* boundsWarning=new QLabel;
 boundsWarning->setObjectName("preview_bed_warning");
 boundsWarning->setWordWrap(true);
 // Warning remains visible above the scrolling settings.
 boundsWarning->setStyleSheet("color:#FFA64B; background:#36261B; padding:7px; border:1px solid #B96922;");
 boundsWarning->setAccessibleName("Aviso de geometria fora da mesa");
 boundsWarning->setMinimumHeight(36);
 settingsColumn->addWidget(restrictBed);
 column->addWidget(boundsWarning);
 // Layer registration and unverified LightBurn machining parameters are
 // separate conditions; neither is a physical bed-limit issue.
 auto* nativeLayerWarning=new QLabel;
 nativeLayerWarning->setObjectName("preview_native_layer_warning_19_8");
 nativeLayerWarning->setWordWrap(true);
 nativeLayerWarning->setStyleSheet("color:#FFD48A; background:#3A2919; padding:7px; border:1px solid #CE8235;");
 nativeLayerWarning->setText(QString(
   "ATENÇÃO — %1 curva(s) Bézier com camada ausente; %2 curva(s) com parâmetros "
   "de camada NÃO verificados após importação. Os identificadores da origem "
   "foram preservados. Velocidade, Fill, potência e intervalo desta prévia são "
   "apenas hipóteses virtuais, não configurações para produção. "
   "Não envie ao laser até revisão específica da fabricação.")
   .arg(qulonglong(nativePreview.missingLayers))
   .arg(qulonglong(nativePreview.unverifiedLayerParameters)));
 nativeLayerWarning->setVisible(nativePreview.missingLayers>0||
                                nativePreview.unverifiedLayerParameters>0);
 column->addWidget(nativeLayerWarning);
 auto* useLayerFeeds=new QCheckBox("Utilizar velocidades virtuais salvas por camada");
 useLayerFeeds->setChecked(true);
 useLayerFeeds->setToolTip("Velocidade e passes de cada camada influenciam a simulacao. Potencia/M3/M4 continuam apenas registrados.");
 settingsColumn->addWidget(optimizeTravel);settingsColumn->addWidget(innerFirst);
 settingsColumn->addWidget(useLayerFeeds);
 auto* stats=new QLabel;stats->setWordWrap(true);settingsColumn->addWidget(stats);
 auto* optionsScroll=new QScrollArea;
 optionsScroll->setObjectName("preview_settings_scroll_19_8");
 optionsScroll->setWidgetResizable(true);
 optionsScroll->setWidget(settingsBody);
 optionsScroll->setMinimumHeight(95);
 optionsScroll->setMaximumHeight(265);
 column->addWidget(optionsScroll,0);
 auto* canvas=new PreviewCanvas198(&dialog);
 canvas->setMinimumHeight(110);
 column->addWidget(canvas,1);
 auto* progress=new QSlider(Qt::Horizontal);progress->setRange(0,1000);
 progress->setValue(1000);progress->setToolTip("Percorrer o TEMPO virtual, nao apenas o numero de linhas");
 column->addWidget(progress);
 auto* controls=new QHBoxLayout;
 auto* play=new QPushButton("▶ Reproduzir");play->setObjectName("preview_play");
 auto* resetZoom=new QPushButton("Ajustar imagem");
 resetZoom->setObjectName("preview_fit_19_8");
 resetZoom->setToolTip("Roda do mouse: zoom no ponteiro. Botao do meio: mover.");
 auto* speed=new QDoubleSpinBox;speed->setRange(0.1,40);speed->setValue(10);
 speed->setSingleStep(1);speed->setSuffix("×");speed->setToolTip("Velocidade apenas da animacao");
 auto* timeLabel=new QLabel("0 / 0 s");timeLabel->setObjectName("preview_time");
 auto* showTravel=new QCheckBox("Mostrar viagens em vermelho");showTravel->setChecked(false);
 showTravel->setToolTip("Exibir deslocamentos sem gravação; desligado para destacar os contornos de gravação.");
 controls->addWidget(play);controls->addWidget(resetZoom);
 controls->addWidget(speed);controls->addWidget(timeLabel);
 controls->addStretch();controls->addWidget(showTravel);column->addLayout(controls);
 auto* footer=new QHBoxLayout;
 auto* exportReport=new QPushButton("Exportar relatorio JSON");
 exportReport->setObjectName("preview_export_report");
 exportReport->setToolTip("Apenas metricas e hipoteses, sem coordenadas do desenho");
 auto* close=new QPushButton("Fechar previsualizacao");
 footer->addWidget(exportReport);footer->addStretch();footer->addWidget(close);
 column->addLayout(footer);
 QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::accept);
 QObject::connect(resetZoom,&QPushButton::clicked,canvas,&PreviewCanvas198::resetView);
 QTimer clock(&dialog);clock.setInterval(60);
 QElapsedTimer elapsedClock;
 double playbackTime=0;
 bool playbackPaused=false;
 std::size_t nestedPairs=0;
 double optimizedLineTravelMm=sourcePlan.travelMm;
 double nominalSeconds=0.0;
 bed198::Report bedReport;
 auto stop=[&](bool paused=false){
  clock.stop();playbackPaused=paused;
  play->setText(paused?"▶ Continuar":"▶ Reproduzir");
  play->setToolTip(paused?"Retomar simulação offline do ponto atual":
                             "Reproduzir a simulação offline, sem laser físico");
 };
 // Cache incremental: durante reproducao avanca so o trecho novo;
 // ao retroceder pelo slider, reconstrói a partir do inicio.
 std::size_t renderedMoves=0;
 QPainterPath renderedLines,renderedTravels;
 auto render=[&](){
  const double now=timeline.totalSeconds*double(progress->value())/1000.0;
  timeLabel->setText(QString("%1 / %2 s").arg(now,0,'f',1).arg(timeline.totalSeconds,0,'f',1));
  const double w=std::max(1.0,drawingWidthMm),h=std::max(1.0,drawingHeightMm);
  const double factor=std::min(660.0/w,365.0/h);
  const auto map=[&](Pt p){return QPointF(35+p.x*factor,35+p.y*factor);};
  const std::size_t count=shown.moves.size();
  std::size_t completed=0;
  if(progress->value()==1000)completed=count;
  else if(progress->value()>0&&count>0){
   completed=std::size_t(std::lower_bound(timeline.endSeconds.begin(),
                      timeline.endSeconds.end(),now)-timeline.endSeconds.begin());
  }
  if(completed<renderedMoves){
   renderedMoves=0;renderedLines=QPainterPath();renderedTravels=QPainterPath();
  }
  for(std::size_t i=renderedMoves;i<completed;++i){
   const auto& move=shown.moves[i];
   QPainterPath& target=move.laser?renderedLines:renderedTravels;
   target.moveTo(map(move.from));target.lineTo(map(move.to));
  }
  renderedMoves=completed;
  QPainterPath partial;
  if(completed<count&&progress->value()>0&&completed<timeline.endSeconds.size()){
   const double start=completed?timeline.endSeconds[completed-1]:0.0;
   const double finish=timeline.endSeconds[completed];
   const double fraction=finish>start?std::clamp((now-start)/(finish-start),0.0,1.0):0.0;
   const auto& move=shown.moves[completed];
   if(move.laser||showTravel->isChecked()){
    const Pt tip{move.from.x+(move.to.x-move.from.x)*fraction,
                 move.from.y+(move.to.y-move.from.y)*fraction};
    partial.moveTo(map(move.from));partial.lineTo(map(tip));
   }
  }
  canvas->showPaths(renderedLines,renderedTravels,partial,
                    showTravel->isChecked(),drawingWidthMm,drawingHeightMm);
 };
 auto recalculate=[&](){
  stop();
  renderedMoves=0;renderedLines=QPainterPath();renderedTravels=QPainterPath();
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
      route::nearestNeighborLines(planningSource,300000,1500,innerFirst->isChecked(),&nestedPairs):
      sourcePlan;
   optimizedLineTravelMm=combined.travelMm;
   if(enableFill->isChecked()){
    raster::appendFill(combined,planningSource,scan,250000,useLayerIntervals->isChecked());
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
   const Plan repeated=repeatedByLayer(combined,planningSource,passes->value());
   // Diagnose ALWAYS, including when limiting the preview is unchecked.
   // Otherwise the operator loses the warning when inspecting full geometry.
   const Plan bounded=bed198::restrictToBed(repeated,drawingWidthMm,drawingHeightMm,&bedReport);
   shown=restrictBed->isChecked()?bounded:repeated;
   // Completely external paths can remain in the editor as spare art.
   // Highlight ONLY active laser lines that cross the bed boundary.
   const bool crossesBed=bed198::partiallyOutsideActiveGeometry(bedReport);
   boundsWarning->setVisible(crossesBed);
   if(crossesBed){
    boundsWarning->setText(QString(
      "ATENÇÃO — %1 trecho(s) de gravação cruzam o limite da mesa. "
      "%2 mm de percurso foram cortados nesses cruzamentos. "
      "%3. Objetos totalmente externos não precisam ser executados.")
      .arg(qulonglong(bedReport.clippedLaserSegments))
      .arg(bedReport.clippedLaserMm,0,'f',2)
      .arg(restrictBed->isChecked()?
        "Prévia parcial: ajuste a arte que cruza a borda":
        "Recorte desligado apenas para inspeção; laser físico bloqueado"));
   }
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
   shown={};timeline={};nominalSeconds=0.0;bedReport={};
   boundsWarning->setVisible(true);
   boundsWarning->setText("Prévia indisponível até corrigir dados.");
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
   {"native_bezier_contours",int(nativePreview.paths)},
   {"native_bezier_closed",int(nativePreview.closed)},
   {"native_bezier_open",int(nativePreview.open)},
   {"native_bezier_virtual_segments",int(nativePreview.segments)},
   {"native_bezier_missing_layer_fallbacks",int(nativePreview.missingLayers)},
   {"native_bezier_unverified_layer_parameters",int(nativePreview.unverifiedLayerParameters)},
   {"native_bezier_layer_mapping_requires_review",nativePreview.missingLayers>0||
      nativePreview.unverifiedLayerParameters>0},
   {"native_bezier_source_unchanged",true},
   {"experimental",true},{"machine_commands_sent",false},
   {"model","kinematic-start-stop-angular-runs-12deg"},
   {"limited_to_bed",restrictBed->isChecked()},
   {"outside_segments_omitted",int(bedReport.outsideSegments)},
   {"outside_segments_clipped",int(bedReport.clippedSegments)},
   {"outside_laser_segments_omitted",int(bedReport.outsideLaserSegments)},
   {"outside_laser_segments_clipped",int(bedReport.clippedLaserSegments)},
   {"outside_laser_mm_omitted",bedReport.omittedLaserMm},
   {"outside_laser_mm_from_clipped",bedReport.clippedLaserMm},
   {"partial_workarea_warning",bed198::partiallyOutsideActiveGeometry(bedReport)},
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
   {"fill_layer_intervals_enabled",useLayerIntervals->isChecked()},
   {"fill_layer_intervals_are_machine_commands",false},
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
 QObject::connect(restrictBed,&QCheckBox::toggled,&dialog,[&](bool){recalculate();});
 QObject::connect(useLayerIntervals,&QCheckBox::toggled,&dialog,[&](bool){recalculate();});
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
  if(clock.isActive()){stop(true);return;}
  if(timeline.totalSeconds<=0)return;
  if(progress->value()>=1000){progress->setValue(0);playbackPaused=false;}
  playbackTime=timeline.totalSeconds*double(progress->value())/1000.0;
  elapsedClock.start();clock.start();playbackPaused=false;
  play->setText("Ⅱ Pausar");
  play->setToolTip("Pausar a simulação; o mesmo botão permite continuar");
 });
 QObject::connect(&clock,&QTimer::timeout,&dialog,[&](){
  playbackTime+=elapsedClock.restart()*0.001*speed->value();
  if(playbackTime>=timeline.totalSeconds){progress->setValue(1000);stop();return;}
  progress->setValue(int(std::clamp(playbackTime/timeline.totalSeconds*1000.0,0.0,1000.0)));
 });
 recalculate();
 QTimer::singleShot(0,canvas,&PreviewCanvas198::resetView);
 dialog.exec();
}
} // namespace alaser::sim198
