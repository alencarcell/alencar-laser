#pragma once
// Bitmap embutido no metadado do projeto v5 (sem formato externo novo).
// Salva PNG e coordenadas reais de mesa em JSON/ZIP, com SHA256 e limites.
#include "bezier_package.hpp"
#include <QBuffer>
#include <QByteArray>
#include <QCryptographicHash>
#include <QImage>
#include <QRectF>
#include <QString>
#include <cmath>
#include <optional>
#include <stdexcept>

namespace alaser::rasterproject198 {
namespace json=boost::json;
struct Raster {QImage image;QRectF bounds;};
inline constexpr std::size_t maxPngBytes=16u*1024u*1024u;
inline json::value encode(const QImage& image,const QRectF& bounds){
 if(image.isNull()||image.width()>10000||image.height()>10000||
    image.width()<1||image.height()<1||!bounds.isValid()||
    !(bounds.width()>0&&bounds.height()>0))
  throw std::runtime_error("Imagem ou dimensoes invalidas ao salvar");
 const double nums[]={bounds.x(),bounds.y(),bounds.width(),bounds.height()};
 for(double n:nums)if(!std::isfinite(n)||std::abs(n)>1e7)
  throw std::runtime_error("Imagem fora do limite de coordenadas");
 QByteArray png;QBuffer buffer(&png);
 if(!buffer.open(QIODevice::WriteOnly)||!image.save(&buffer,"PNG"))
  throw std::runtime_error("Nao foi possivel preparar PNG interno");
 if(std::size_t(png.size())>maxPngBytes)
  throw std::runtime_error("PNG excede 16 MiB: reduza resolucao antes de salvar");
 const QByteArray base64=png.toBase64();
 const QByteArray hash=QCryptographicHash::hash(png,QCryptographicHash::Sha256).toHex();
 return json::object{
  {"schema","alaser-raster-1"},{"encoding","PNG/base64"},
  {"png_base64",std::string(base64.constData(),std::size_t(base64.size()))},
  {"sha256",std::string(hash.constData(),std::size_t(hash.size()))},
  {"x",bounds.x()},{"y",bounds.y()},
  {"w",bounds.width()},{"h",bounds.height()},
  {"pixelWidth",image.width()},{"pixelHeight",image.height()}
 };
}
inline std::optional<Raster> decode(const IntegratedProject& project){
 if(!project.legacy.layers.is_object())return {};
 const auto& root=project.legacy.layers.as_object();
 auto it=root.find("raster_object_19_8");
 if(it==root.end())return {};
 if(!it->value().is_object())throw std::runtime_error("Metadados bitmap invalidos");
 const auto& v=it->value().as_object();
 if(v.at("schema").as_string()!="alaser-raster-1"||
    v.at("encoding").as_string()!="PNG/base64")
  throw std::runtime_error("Versao de imagem nao suportada; arquivo preservado");
 const auto& raw=v.at("png_base64").as_string();
 if(raw.size()>(maxPngBytes*4/3+8))
  throw std::runtime_error("Imagem PNG codificada excede limite");
 const QByteArray encoded(raw.data(),qsizetype(raw.size()));
 auto result=QByteArray::fromBase64Encoding(encoded,QByteArray::AbortOnBase64DecodingErrors);
 if(!result)throw std::runtime_error("PNG base64 inconsistente");
 const QByteArray png=result.decoded;
 if(std::size_t(png.size())>maxPngBytes)throw std::runtime_error("PNG fora do limite");
 const QByteArray actual=QCryptographicHash::hash(png,QCryptographicHash::Sha256).toHex();
 const auto& expected=v.at("sha256").as_string();
 if(actual!=QByteArray(expected.data(),qsizetype(expected.size())))
  throw std::runtime_error("Imagem corrompida (SHA256); carga cancelada");
 QImage image=QImage::fromData(png,"PNG");
 if(image.isNull()||image.width()>10000||image.height()>10000||
    image.width()!=v.at("pixelWidth").to_number<int>()||
    image.height()!=v.at("pixelHeight").to_number<int>())
  throw std::runtime_error("Imagem salva incompatível com metadados");
 QRectF bounds(v.at("x").to_number<double>(),v.at("y").to_number<double>(),
               v.at("w").to_number<double>(),v.at("h").to_number<double>());
 if(!bounds.isValid()||bounds.width()<=0||bounds.height()<=0)
  throw std::runtime_error("Area do bitmap invalida");
 const double nums[]={bounds.x(),bounds.y(),bounds.width(),bounds.height()};
 for(double n:nums)if(!std::isfinite(n)||std::abs(n)>1e7)
  throw std::runtime_error("Coordenada de imagem fora do limite");
 return Raster{std::move(image),bounds};
}
inline void apply(IntegratedProject& project,const QImage* image,const QRectF& bounds){
 auto& root=project.legacy.layers.as_object();
 root.erase("raster_object_19_8");
 if(image&&!image->isNull())root["raster_object_19_8"]=encode(*image,bounds);
}
} // namespace alaser::rasterproject198
