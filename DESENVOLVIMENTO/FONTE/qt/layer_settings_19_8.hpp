#pragma once
// Layer editor: local simulation metadata ONLY. M3/M4 here never becomes a
// controller command. No G-code, serial, power, or air-control calls.
#include "editor_tools.hpp"
#include <QDialog>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QString>
#include <optional>

namespace alaser::layerui198 {
inline std::optional<Layer> edit(QWidget* parent,const Layer& original){
 if(original.id==1001||original.id==1002)return std::nullopt;
 QDialog window(parent);window.setObjectName("layer_settings_19_8");
 window.setWindowTitle(QString("Configuracoes da camada %1 (simulacao)").arg(original.id));
 window.setMinimumWidth(460);
 auto* column=new QVBoxLayout(&window);
 auto* warning=new QLabel(
   "Parametros de edicao e estimativa OFFLINE. "
   "M3/M4 sao nomes de comportamento GRBL, mas nenhum comando M3/M4, "
   "potencia, movimento ou ar sera enviado nesta candidata.");
 warning->setWordWrap(true);column->addWidget(warning);
 auto* form=new QFormLayout;
 auto* name=new QLineEdit(QString::fromStdString(original.name));
 name->setObjectName("layer_settings_name");
 name->setMaxLength(90);
 auto* mode=new QComboBox;mode->setObjectName("layer_settings_mode");
 mode->addItem("Linha / contorno",QString("Line"));
 mode->addItem("Preenchimento",QString("Fill"));
 mode->setCurrentIndex(original.mode=="Fill"?1:0);
 auto* speed=new QDoubleSpinBox;speed->setObjectName("layer_settings_speed");
 speed->setRange(10.,100000.);speed->setDecimals(0);speed->setValue(original.speedMmMin);
 speed->setSuffix(" mm/min");
 auto* power=new QDoubleSpinBox;power->setObjectName("layer_settings_power");
 power->setRange(0.,100.);power->setDecimals(1);power->setSingleStep(1.);
 power->setValue(original.powerPercent);power->setSuffix(" %");
 power->setToolTip("Por enquanto e um valor salvo para referencia; NAO calcula emissao real.");
 auto* passes=new QSpinBox;passes->setObjectName("layer_settings_passes");
 passes->setRange(1,20);passes->setValue(original.passes);
 auto* powerMode=new QComboBox;powerMode->setObjectName("layer_settings_m3_m4");
 powerMode->addItem("M4 — potencia dinamica (recomendada)",QString("M4"));
 powerMode->addItem("M3 — potencia constante",QString("M3"));
 powerMode->setCurrentIndex(original.powerMode=="M3"?1:0);
 powerMode->setToolTip("M4 acompanha a velocidade (GRBL compativel); M3 mantém potencia constante. Somente metadados.");
 form->addRow("Nome",name);form->addRow("Tipo",mode);
 form->addRow("Velocidade virtual",speed);
 form->addRow("Potencia nominal",power);
 form->addRow("Passagens previstas",passes);
 form->addRow("Modo GRBL (referencia)",powerMode);
 column->addLayout(form);
 auto* optimize=new QCheckBox("Permitir otimizar rota desta camada na previa");
 optimize->setObjectName("layer_settings_optimize");
 optimize->setChecked(original.optimizeTravel);
 optimize->setToolTip("Heuristica de vizinho proximo, sem alterar o desenho; exige habilitar otimizacao global na previa.");
 column->addWidget(optimize);
 auto* note=new QLabel(
  "M3 nao e 'modo de perfuracao': e o modo de potencia constante. "
  "M4 e o modo de potencia dinamica. Os dois precisam de GRBL compatível "
  "e parametros reais antes de qualquer operacao fisica.");
 note->setWordWrap(true);column->addWidget(note);
 auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
 column->addWidget(buttons);
 QObject::connect(buttons,&QDialogButtonBox::accepted,&window,&QDialog::accept);
 QObject::connect(buttons,&QDialogButtonBox::rejected,&window,&QDialog::reject);
 if(window.exec()!=QDialog::Accepted)return std::nullopt;
 if(name->text().trimmed().isEmpty())return std::nullopt;
 Layer result=original;
 result.name=name->text().trimmed().toStdString();
 result.mode=mode->currentData().toString().toStdString();
 result.speedMmMin=speed->value();
 result.powerPercent=power->value();
 result.passes=passes->value();
 result.powerMode=powerMode->currentData().toString().toStdString();
 result.optimizeTravel=optimize->isChecked();
 return result;
}
} // namespace alaser::layerui198
