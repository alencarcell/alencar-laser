#pragma once
// TESTE_19.3 — Leitura de arquivos locais SOMENTE LEITURA.
// Nao importa para o documento nativo, nao converte velocidades e NAO comunica com laser.
#include <QByteArray>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QPixmap>
#include <QRegularExpression>
#include <QScrollArea>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QXmlStreamReader>
#include <stdexcept>

namespace alaser::reference {
struct LightBurnSummary {
 QString appVersion,deviceName,formatVersion,error,sourceName;
 QMap<QString,int> types,layers;
 struct Setting {QString name,type,speed,power,index;};
 QList<Setting> settings;
 int count=0,anchors=0,withPrimitives=0,withoutPrimitives=0,bezierSegments=0,lineSegments=0,transforms=0;
 QByteArray embeddedPreview;
 bool valid=false;
 QString report() const {
  QString r="ALENCAR LASER  |  ANÁLISE DE REFERÊNCIA LIGHTBURN\n";
  r+="Arquivo: "+sourceName+"\n";
  r+="Formato: "+formatVersion+"  |  LightBurn: "+appVersion+"  |  Dispositivo declarado: "+deviceName+"\n\n";
  r+="GEOMETRIA (inventário, NÃO importada para edição)\n";
  r+=QString("Figuras: %1 | âncoras Path: %2 | XForm: %3\n").arg(count).arg(anchors).arg(transforms);
  for(auto it=types.cbegin();it!=types.cend();++it)r+=QString("  %1: %2\n").arg(it.key()).arg(it.value());
  r+=QString("Path com PrimList: %1 | Path sem PrimList: %2\n").arg(withPrimitives).arg(withoutPrimitives);
  r+=QString("Primitivas explícitas B: %1 | L: %2\n\n").arg(bezierSegments).arg(lineSegments);
  r+="CAMADAS (CutIndex associado aos objetos)\n";
  for(auto it=layers.cbegin();it!=layers.cend();++it)r+=QString("  Índice %1: %2 objetos\n").arg(it.key()).arg(it.value());
  r+="\nAJUSTES GRAVADOS NO XML — VALORES BRUTOS, NÃO PRESCRIÇÕES DE LASER\n";
  for(const auto& s:settings)r+=QString("  %1 [índice %2]: %3 | velocidade bruta %4 | potência máxima bruta %5\n")
    .arg(s.name,s.index,s.type,s.speed,s.power);
  r+="\nATENÇÃO\n"
    "O arquivo original NÃO foi alterado. A miniatura é extraída do próprio LightBurn, não é uma renderização vetorial. "
    "XForm, curvas, bitmap e coordenadas fora da mesa ainda não são importados. "
    "Nenhuma unidade de velocidade ou compatibilidade de máquina é inferida. "
    "Emissão, movimentos e G-code permanecem desativados.\n";
  return r;
 }
};
inline int countPattern(const QString& value,const QRegularExpression& expression) {
 int n=0;auto it=expression.globalMatch(value);while(it.hasNext()){it.next();++n;}return n;
}
inline LightBurnSummary readLightBurn(const QString& path){
 LightBurnSummary s;s.sourceName=QFileInfo(path).fileName();
 QFile file(path);
 if(!file.open(QIODevice::ReadOnly)){s.error="Não foi possível abrir o XML em modo leitura.";return s;}
 if(file.size()>64LL*1024*1024){s.error="Arquivo maior que o limite de 64 MiB para inspeção.";return s;}
 QXmlStreamReader xml(&file);
 bool inProject=false,inSetting=false,inShape=false,pathWithPrims=false;
 QString shapeType;
 LightBurnSummary::Setting current;
 static const QRegularExpression anchorPattern(R"(V(?=\s*[-+]?\d))");
 static const QRegularExpression bezierPattern(R"(B\s*\d+\s+\d+)");
 static const QRegularExpression linePattern(R"(L\s*\d+\s+\d+)");
 while(!xml.atEnd()){
  const auto token=xml.readNext();
  if(token==QXmlStreamReader::StartElement){
   const auto tag=xml.name().toString();const auto attributes=xml.attributes();
   if(!inProject){
    if(tag!="LightBurnProject"){s.error="Raiz não é LightBurnProject.";return s;}
    inProject=true;s.appVersion=attributes.value("AppVersion").toString();
    s.deviceName=attributes.value("DeviceName").toString();
    s.formatVersion=attributes.value("FormatVersion").toString();continue;
   }
   if(tag=="Thumbnail"){
    const QString v=attributes.value("Source").toString();
    if(v.size()<4*1024*1024)s.embeddedPreview=QByteArray::fromBase64(v.toLatin1());
   }else if(tag=="Shape"){
    // Esta fase suporta inventário de Shape; não aceita grupos aninhados como geometria nativa.
    if(inShape){s.error="Shape aninhada: estrutura requer importador específico.";return s;}
    inShape=true;pathWithPrims=false;shapeType=attributes.value("Type").toString();
    if(shapeType.isEmpty())shapeType="Tipo não declarado";
    s.types[shapeType]++;s.layers[attributes.value("CutIndex").toString()]++;s.count++;
   }else if(tag=="CutSetting"||tag=="CutSetting_Img"){
    inSetting=true;current={};current.type=attributes.value("type").toString();
   }else if(tag=="XForm"&&inShape){
    ++s.transforms;
   }else if(tag=="VertList"&&inShape){
    const auto v=xml.readElementText(QXmlStreamReader::SkipChildElements);
    s.anchors+=countPattern(v,anchorPattern);
   }else if(tag=="PrimList"&&inShape){
    const auto v=xml.readElementText(QXmlStreamReader::SkipChildElements);
    pathWithPrims=true;
    s.bezierSegments+=countPattern(v,bezierPattern);
    s.lineSegments+=countPattern(v,linePattern);
   }else if(inSetting&&(tag=="name"||tag=="index"||tag=="speed"||tag=="maxPower")){
    auto value=attributes.value("Value").toString();
    if(tag=="name")current.name=value;
    else if(tag=="index")current.index=value;
    else if(tag=="speed")current.speed=value;
    else if(tag=="maxPower")current.power=value;
   }
  }else if(token==QXmlStreamReader::EndElement){
   const auto tag=xml.name().toString();
   if(tag=="Shape"&&inShape){
    if(shapeType=="Path"){
     if(pathWithPrims)++s.withPrimitives;else ++s.withoutPrimitives;
    }
    inShape=false;
   }else if(tag=="CutSetting"||tag=="CutSetting_Img"){
    s.settings.push_back(current);inSetting=false;
   }
  }
 }
 if(xml.hasError())s.error="Falha de leitura XML: "+xml.errorString();
 else if(!inProject)s.error="Cabeçalho LightBurn ausente.";
 else s.valid=true;
 return s;
}
struct GrblLogSummary {
 QString sourceName,error,firstTime,lastTime;
 qint64 lines=0,inputs=0,outputs=0,idleFrames=0,runFrames=0,statusQueries=0,
  settingsQueries=0,gcodePackets=0,timeoutMessages=0,cutBounds=0;
 QMap<QString,QString> settings;
 bool valid=false;
 QString report() const{
  QString r="ALENCAR LASER  |  AUDITORIA DE LOG LIGHTBURN (SEM CONEXÃO)\n";
  r+="Arquivo: "+sourceName+"\n";
  r+=QString("Primeiro/último horário registrado: %1 / %2\n").arg(firstTime,lastTime);
  r+=QString("Linhas: %1 | I: %2 | O: %3\n").arg(lines).arg(inputs).arg(outputs);
  r+=QString("Consultas ? (pacotes): %1 | $$: %2 | pacotes de G-code: %3\n")
       .arg(statusQueries).arg(settingsQueries).arg(gcodePackets);
  r+=QString("Ocorrências textuais de Idle: %1 | Run: %2\n").arg(idleFrames).arg(runFrames);
  r+=QString("Timeout textual: %1 | Registros Cut Bounds: %2\n\n").arg(timeoutMessages).arg(cutBounds);
  r+="PARÂMETROS GRBL OBSERVADOS (somente informações capturadas)\n";
  for(const auto& k: {"$30","$31","$32","$100","$101","$110","$111","$130","$131","$132"})
   if(settings.contains(k))r+=QString("  %1 = %2\n").arg(QString(k),settings.value(k));
  r+="\nOBSERVAÇÃO: pacote de saída não implica execução física concluída. "
    "Este log foi gerado pelo LightBurn, não pelo Alencar Laser. "
    "Nenhum byte foi enviado à controladora nesta inspeção.\n";
  return r;
 }
};
inline GrblLogSummary readGrblLog(const QString& path){
 GrblLogSummary s;s.sourceName=QFileInfo(path).fileName();QFile f(path);
 if(!f.open(QIODevice::ReadOnly)){s.error="Não foi possível abrir log para leitura.";return s;}
 if(f.size()>128LL*1024*1024){s.error="Log maior que 128 MiB.";return s;}
 static const QRegularExpression paramPattern(R"(\$(\d{1,3})=([^\s\r\n]+))");
 while(!f.atEnd()){
  const auto line=f.readLine();++s.lines;
  if(line.size()>=12&&line.at(2)==':'&&line.at(5)==':'&&line.at(8)=='.'){
   const auto time=QString::fromLatin1(line.left(12));
   if(s.firstTime.isEmpty())s.firstTime=time;s.lastTime=time;
  }
  if(line.contains("Command sending with timeout"))++s.timeoutMessages;
  if(line.contains("Cut Bounds :"))++s.cutBounds;
  const int in=line.indexOf("I: \"");const int out=line.indexOf("O: \"");
  if(in<0&&out<0)continue;
  const bool input=in>=0;const int idx=input?in:out;
  if(input)++s.inputs;else ++s.outputs;
  const int start=idx+4;const int end=line.indexOf('"',start);
  if(end<start)continue;
  const QByteArray encoded=line.mid(start,end-start);
  // Log LightBurn registra bytes em hexadecimal: recusar linhas não-hex.
  if(encoded.isEmpty()||encoded.size()%2!=0||encoded.size()>1024*1024)continue;
  bool hex=true;for(char ch:encoded)if(!((ch>='0'&&ch<='9')||(ch>='a'&&ch<='f')||(ch>='A'&&ch<='F'))){hex=false;break;}
  if(!hex)continue;
  const QByteArray packet=QByteArray::fromHex(encoded);
  if(!input){
   if(packet=="?")++s.statusQueries;
   if(packet.startsWith("$$"))++s.settingsQueries;
   if(packet.startsWith('G')||packet.startsWith('M'))++s.gcodePackets;
  }else{
   s.idleFrames+=packet.count("<Idle");s.runFrames+=packet.count("<Run");
   const QString text=QString::fromLatin1(packet);
   auto it=paramPattern.globalMatch(text);
   while(it.hasNext()){
    const auto match=it.next();s.settings["$"+match.captured(1)]=match.captured(2);
   }
  }
 }
 s.valid=true;return s;
}
inline void showReport(QWidget* parent,const QString& title,const QString& report,const QByteArray& thumbnail={}){
 QDialog dlg(parent);dlg.setWindowTitle(title+"  •  somente leitura");dlg.resize(740,620);
 auto* body=new QVBoxLayout(&dlg);
 auto* intro=new QLabel("Análise local de referência • Não altera o desenho aberto, as camadas nem o histórico de edição.");
 intro->setWordWrap(true);body->addWidget(intro);
 if(!thumbnail.isEmpty()){
  QPixmap image; if(image.loadFromData(thumbnail)){
   auto* preview=new QLabel;preview->setAlignment(Qt::AlignCenter);
   preview->setPixmap(image.scaled(235,235,Qt::KeepAspectRatio,Qt::SmoothTransformation));
   preview->setToolTip("Miniatura incorporada ao próprio projeto LightBurn");
   body->addWidget(preview);
  }
 }
 auto* display=new QTextEdit;display->setReadOnly(true);display->setPlainText(report);
 display->setLineWrapMode(QTextEdit::NoWrap);
 display->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));body->addWidget(display,1);
 auto* btns=new QDialogButtonBox(QDialogButtonBox::Close);body->addWidget(btns);
 QObject::connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
 dlg.exec();
}
inline void inspectLightBurn(QWidget* parent,const QString& path){
 const auto s=readLightBurn(path);
 if(!s.valid)throw std::runtime_error(s.error.toStdString());
 showReport(parent,"Projeto LightBurn",s.report(),s.embeddedPreview);
}
inline void inspectGrblLog(QWidget* parent,const QString& path){
 const auto s=readGrblLog(path);
 if(!s.valid)throw std::runtime_error(s.error.toStdString());
 showReport(parent,"Log GRBL",s.report());
}
} // namespace alaser::reference
