#pragma once
// Alencar Laser 20zc — deterministic SVG 1.1 vector-only export (millimetres).
// Offline. Never imports/exports CAM, G-code, raster, speed or power settings.
#include "editor_tools.hpp"
#include "bezier_native.hpp"
#include <QByteArray>
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QXmlStreamWriter>
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

namespace alaser::svg20zc {
inline constexpr const char* kSaveDirectoryKey="files/lastSaveDirectory";

// The save-folder preference has its own INI-backed UserScope store.
// This avoids depending on the editor's recovery/UI-persistence switch and
// makes the file location explicit and independently verifiable on Windows.
inline std::unique_ptr<QSettings> saveLocationSettings(){
 const QString overrideFile=qEnvironmentVariable("ALENCAR_SAVE_LOCATIONS_TEST_FILE_20ZD");
 if(!overrideFile.isEmpty())
  return std::make_unique<QSettings>(overrideFile,QSettings::IniFormat);
 return std::make_unique<QSettings>(QSettings::IniFormat,QSettings::UserScope,
                                    "AlencarLaser","DesktopBeta");
}
inline QString normalizedExistingDirectory(QString directory){
 if(directory.isEmpty())return {};
 QDir candidate(directory);
 if(!candidate.exists())return {};
 QString canonical=candidate.canonicalPath();
 if(canonical.isEmpty())canonical=candidate.absolutePath();
 return QDir::cleanPath(canonical);
}
inline QString initialSavePath(QSettings& settings,const QString& name,
                               const QString& fallbackFile={},bool migrateLegacy=true){
 settings.setFallbacksEnabled(false);
 settings.sync();
 QString directory=normalizedExistingDirectory(settings.value(kSaveDirectoryKey).toString());
 // Migrate the 20zc native QSettings key to the dedicated UserScope INI store.
 if(directory.isEmpty()&&migrateLegacy){
  QSettings legacy("AlencarLaser","DesktopBeta");
  legacy.sync();
  const QString oldDirectory=normalizedExistingDirectory(
      legacy.value(kSaveDirectoryKey).toString());
  if(!oldDirectory.isEmpty()){
   settings.setValue(kSaveDirectoryKey,oldDirectory);
   settings.sync();
   if(settings.status()==QSettings::NoError)
    directory=normalizedExistingDirectory(settings.value(kSaveDirectoryKey).toString());
  }
 }
 if(directory.isEmpty()){
  const QFileInfo prior(fallbackFile);
  if(!fallbackFile.isEmpty()&&prior.absoluteDir().exists())
   directory=normalizedExistingDirectory(prior.absolutePath());
  else directory=normalizedExistingDirectory(
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
 }
 if(directory.isEmpty())directory=QDir::homePath();
 const QString basename=QFileInfo(name).fileName();
 return QDir(directory).filePath(basename.isEmpty()?"Alencar_Laser.svg":basename);
}
inline bool rememberSaveDirectory(QSettings& settings,const QString& savedPath,
                                  QString* error=nullptr){
 const auto fail=[&](const QString& why){if(error)*error=why;return false;};
 if(savedPath.isEmpty())return fail("caminho vazio");
 const QFileInfo chosen(savedPath);
 if(!chosen.isAbsolute()||!chosen.absoluteDir().exists())
  return fail("pasta escolhida nao existe ou caminho nao e absoluto");
 QString directory=normalizedExistingDirectory(chosen.absolutePath());
 if(directory.isEmpty())return fail("nao foi possivel resolver a pasta escolhida");
 settings.setFallbacksEnabled(false);
 settings.setValue(kSaveDirectoryKey,directory);
 settings.sync();
 if(settings.status()!=QSettings::NoError)
  return fail("QSettings nao conseguiu gravar a preferencia");
 // Verify through a NEW settings object, simulating application restart.
 QSettings verify(settings.fileName(),settings.format());
 verify.setFallbacksEnabled(false);
 verify.sync();
 const QString readBack=normalizedExistingDirectory(verify.value(kSaveDirectoryKey).toString());
 if(verify.status()!=QSettings::NoError||readBack!=directory)
  return fail("a preferencia nao passou na verificacao de leitura");
 // Keep the old native key synchronized for compatibility with 20zc builds.
 QSettings legacy("AlencarLaser","DesktopBeta");
 legacy.setValue(kSaveDirectoryKey,directory);
 legacy.sync();
 return true;
}

struct Summary {
 int polylinePaths=0,bezierPaths=0,segments=0;
 double widthMm=0,heightMm=0;
};
inline QString number(double value){
 if(!std::isfinite(value)||std::abs(value)>1e8)
  throw std::invalid_argument("SVG: coordenada nao finita ou fora do limite");
 return QString::number(value,'f',9);
}
inline QString coord(Pt p){return number(p.x)+" "+number(p.y);}
struct Bounds {
 double left=std::numeric_limits<double>::infinity();
 double top=std::numeric_limits<double>::infinity();
 double right=-std::numeric_limits<double>::infinity();
 double bottom=-std::numeric_limits<double>::infinity();
 void include(Pt p){
  (void)number(p.x);(void)number(p.y);
  left=std::min(left,p.x);top=std::min(top,p.y);
  right=std::max(right,p.x);bottom=std::max(bottom,p.y);
 }
 bool valid()const{return std::isfinite(left)&&std::isfinite(top)&&
    std::isfinite(right)&&std::isfinite(bottom);}
};

inline QString serializeSvg(const EditorState& state,const CurveDocument& curves,
                            Summary* summaryOut=nullptr){
 // No modification to EditorState, curves or history; use the committed vectors.
 validate(state);
 validateCurve(curves);
 Bounds bounds;
 struct VectorPath {QString d;};
 std::vector<VectorPath> paths;
 Summary stats;
 std::size_t vertices=0;
 const auto append=[&](QString&& d){
  if(paths.size()>=50000)throw std::runtime_error("SVG: limite de contornos excedido");
  paths.push_back({std::move(d)});
 };
 for(const Contour& c:state.doc.contours){
  const auto meta=state.objects.find(c.key.object);
  if(meta==state.objects.end())throw std::runtime_error("SVG: objeto sem metadata");
  const auto layer=state.layers.find(meta->second.layer);
  if(layer==state.layers.end())throw std::runtime_error("SVG: camada ausente");
  if(!layer->second.visible||c.nodes.size()<2)continue;
  vertices+=c.nodes.size();
  if(vertices>1000000)throw std::runtime_error("SVG: limite de nos excedido");
  QString d="M "+coord(c.nodes.front().p);
  bounds.include(c.nodes.front().p);
  for(std::size_t i=1;i<c.nodes.size();++i){
   d+=" L "+coord(c.nodes[i].p);
   bounds.include(c.nodes[i].p);
   ++stats.segments;
  }
  if(c.closed){
   d+=" Z";
   ++stats.segments;
  }
  append(std::move(d));
  ++stats.polylinePaths;
 }
 for(const CurveContour& c:curves.contours){
  auto layer=state.layers.find(c.layer);
  // Legacy imported native curves can use a layer not represented by the
  // polygon metadata; treat missing layers as visible, as Qt does for native.
  if(layer!=state.layers.end()&&!layer->second.visible)continue;
  if(c.nodes.size()<2)continue;
  vertices+=c.nodes.size();
  if(vertices>1000000)throw std::runtime_error("SVG: limite de nos excedido");
  QString d="M "+coord(c.nodes.front().anchor);
  bounds.include(c.nodes.front().anchor);
  for(std::size_t i=0;i<edgeCount(c);++i){
   const Cubic edge=edgeAt(c,i);
   if(c.edgeKind.at(i)==1){
    d+=" C "+coord(edge.p1)+" "+coord(edge.p2)+" "+coord(edge.p3);
    bounds.include(edge.p1);bounds.include(edge.p2);
   }else d+=" L "+coord(edge.p3);
   bounds.include(edge.p3);
   ++stats.segments;
  }
  if(c.closed)d+=" Z";
  append(std::move(d));
  ++stats.bezierPaths;
 }
 if(paths.empty()||!bounds.valid())
  throw std::runtime_error("SVG: nenhum contorno vetorial visivel para exportar");
 // The viewBox is in mm; physical width/height are also in mm, no px scaling.
 const double width=std::max(0.01,bounds.right-bounds.left);
 const double height=std::max(0.01,bounds.bottom-bounds.top);
 if(width>1e8||height>1e8)
  throw std::runtime_error("SVG: dimensoes invalidas");
 const double x=bounds.left-std::max(0.,width-(bounds.right-bounds.left))/2.;
 const double y=bounds.top-std::max(0.,height-(bounds.bottom-bounds.top))/2.;
 QString xml;
 QXmlStreamWriter w(&xml);
 w.setAutoFormatting(true);
 w.writeStartDocument("1.0");
 w.writeStartElement("svg");
 w.writeDefaultNamespace("http://www.w3.org/2000/svg");
 w.writeAttribute("version","1.1");
 w.writeAttribute("width",number(width)+"mm");
 w.writeAttribute("height",number(height)+"mm");
 w.writeAttribute("viewBox",number(x)+" "+number(y)+" "+number(width)+" "+number(height));
 w.writeAttribute("fill","none");
 w.writeAttribute("stroke","#000000");
 w.writeAttribute("stroke-width","0.1");
 w.writeAttribute("stroke-linecap","round");
 w.writeAttribute("stroke-linejoin","round");
 for(const auto& p:paths){
  w.writeEmptyElement("path");
  w.writeAttribute("d",p.d);
 }
 w.writeEndElement();
 w.writeEndDocument();
 if(w.hasError())throw std::runtime_error("SVG: falha na serializacao XML");
 stats.widthMm=width;stats.heightMm=height;
 if(summaryOut)*summaryOut=stats;
 return xml;
}
inline Summary saveSvg(const QString& destination,const EditorState& state,
                       const CurveDocument& curves){
 if(QFileInfo(destination).suffix().compare("svg",Qt::CaseInsensitive)!=0)
  throw std::runtime_error("SVG: escolha arquivo com extensao .svg");
 Summary stats;
 const QByteArray bytes=serializeSvg(state,curves,&stats).toUtf8();
 QSaveFile file(destination);
 file.setDirectWriteFallback(false);
 if(!file.open(QIODevice::WriteOnly))throw std::runtime_error("SVG: destino indisponivel");
 if(file.write(bytes)!=bytes.size()||!file.commit())
  throw std::runtime_error("SVG: erro ao gravar com seguranca");
 return stats;
}
} // namespace alaser::svg20zc
