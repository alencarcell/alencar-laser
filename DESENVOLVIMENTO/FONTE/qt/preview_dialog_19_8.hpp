#pragma once
// Qt-only, read-only job-preview prototype. No serial, TCP, G-code or machine transport.
#include "preview_sim_19_8.hpp"
#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QColor>
#include <algorithm>
namespace alaser::sim198 {
inline void showPreviewDialog(QWidget* parent,const EditorState& source,
                               double drawingWidthMm,double drawingHeightMm){
 const Plan plan=build(source);
 QDialog dialog(parent);
 dialog.setWindowTitle("Alencar Laser 19.8 - Previa virtual, SEM EMISSAO");
 dialog.resize(780,650);
 auto* column=new QVBoxLayout(&dialog);
 auto* note=new QLabel("SIMULACAO LOCAL: sem G-code, rede ou comandos fisicos. "
    "Apenas contornos Line, sem Fill/Imagem, aceleracao ou otimizacao. "
    "Velocidades abaixo sao hipoteticas; nao representam sua maquina.");
 note->setWordWrap(true);column->addWidget(note);
 auto* form=new QFormLayout;
 auto* cut=new QDoubleSpinBox;cut->setRange(10,100000);cut->setValue(3000);
 cut->setDecimals(0);cut->setSuffix(" mm/min");
 auto* rapid=new QDoubleSpinBox;rapid->setRange(10,100000);rapid->setValue(7000);
 rapid->setDecimals(0);rapid->setSuffix(" mm/min");
 form->addRow("Linha (hipotetica)",cut);form->addRow("Deslocamento (hipotetico)",rapid);
 column->addLayout(form);
 auto* stats=new QLabel;stats->setWordWrap(true);column->addWidget(stats);
 auto* view=new QLabel;view->setMinimumHeight(350);view->setAlignment(Qt::AlignCenter);
 column->addWidget(view,1);
 auto* progress=new QSlider(Qt::Horizontal);
 progress->setRange(0,1000);progress->setValue(1000);
 progress->setToolTip("Percorrer a ordem de movimentos virtuais");
 column->addWidget(progress);
 auto* showTravel=new QCheckBox("Mostrar viagens sem laser (vermelho)");
 showTravel->setChecked(true);column->addWidget(showTravel);
 auto* close=new QPushButton("Fechar");column->addWidget(close);
 QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::accept);
 auto render=[&](){
  const double total=minutes(plan,cut->value(),rapid->value());
  stats->setText(QString("Linha %1 mm | viagens %2 mm | tempo nominal %3 min | "
    "movimentos %4 | excluidos: Fill %5, Saida off %6, T1/T2 %7")
    .arg(plan.cutMm,0,'f',1).arg(plan.travelMm,0,'f',1).arg(total,0,'f',2)
    .arg(qulonglong(plan.moves.size())).arg(qulonglong(plan.ignoredFill))
    .arg(qulonglong(plan.ignoredDisabled)).arg(qulonglong(plan.ignoredTool)));
  QImage img(700,450,QImage::Format_ARGB32);img.fill(QColor("#14222C"));
  QPainter p(&img);p.setRenderHint(QPainter::Antialiasing);
  const double w=std::max(1.,drawingWidthMm),h=std::max(1.,drawingHeightMm);
  const double factor=std::min(630./w,385./h);
  auto map=[&](Pt pt){return QPointF(32+pt.x*factor,32+pt.y*factor);};
  p.setPen(QPen(QColor("#568875"),1.5));
  p.drawRect(QRectF(32,32,w*factor,h*factor));
  const size_t n=std::min(plan.moves.size(),size_t(double(plan.moves.size())*progress->value()/1000.));
  for(size_t i=0;i<n;++i){
   const auto& segment=plan.moves[i];
   if(!segment.laser&&!showTravel->isChecked())continue;
   p.setPen(QPen(segment.laser?QColor("#E4E9E9"):QColor("#F26767"),segment.laser?1.4:.9));
   p.drawLine(map(segment.from),map(segment.to));
  }
  p.end();view->setPixmap(QPixmap::fromImage(img));
 };
 QObject::connect(progress,&QSlider::valueChanged,&dialog,[&](int){render();});
 QObject::connect(cut,QOverload<double>::of(&QDoubleSpinBox::valueChanged),&dialog,[&](double){render();});
 QObject::connect(rapid,QOverload<double>::of(&QDoubleSpinBox::valueChanged),&dialog,[&](double){render();});
 QObject::connect(showTravel,&QCheckBox::toggled,&dialog,[&](bool){render();});
 render();dialog.exec();
}
} // namespace alaser::sim198
