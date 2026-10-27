#pragma once
// TESTE_19.8: importacao CONSERVADORA de geometria LightBurn.
// Nunca interpreta potencia, velocidade, G-code, firmware ou configuracao fisica.
// Se houver alguma Shape nao suportada, FALHA INTEIRA sem alterar documento.
#include "editor_tools.hpp"
#include <QFile>
#include <QFileInfo>
#include <QPointF>
#include <QRegularExpression>
#include <QTransform>
#include <QXmlStreamReader>
#include <cmath>
#include <set>
#include <stdexcept>
#include <vector>

namespace alaser::lightburn {
struct GeometryShape {
 std::vector<Pt> points;
 int layer=0;
};
struct ParseNode{
 QString type;
 QTransform xform;
 double w=0,h=0,radius=0;
 int cutIndex=0;
 std::vector<ParseNode> children;
};
struct ImportResult {
 EditorState state;
 int shapeCount=0;
 explicit ImportResult(EditorState s,int n):state(std::move(s)),shapeCount(n){}
};
inline double attrDouble(const QXmlStreamAttributes& a,const char* key,double fallback=0){
 auto v=a.value(QLatin1String(key));
 if(v.isEmpty())return fallback;
 bool ok=false;double n=v.toString().toDouble(&ok);
 if(!ok||!std::isfinite(n))throw std::runtime_error("atributo numerico invalido no arquivo LightBurn");
 return n;
}
inline QTransform parseXForm(const QString& value){
 static const QRegularExpression sep(R"(\s+)");
 const auto parts=value.trimmed().split(sep,Qt::SkipEmptyParts);
 if(parts.size()!=6)throw std::runtime_error("XForm LightBurn deve conter seis valores");
 double nums[6];for(int i=0;i<6;i++){
  bool ok=false;nums[i]=parts[i].toDouble(&ok);
  if(!ok||!std::isfinite(nums[i]))throw std::runtime_error("XForm invalido");
 }
 return QTransform(nums[0],nums[1],nums[2],nums[3],nums[4],nums[5]);
}
inline ParseNode readShape(QXmlStreamReader& xml,int depth,int& seen){
 if(depth>16||++seen>30000)throw std::runtime_error("Limite de formas/grupos excedido");
 const auto attrs=xml.attributes();
 ParseNode node;
 node.type=attrs.value("Type").toString();
 node.w=attrDouble(attrs,"W");
 node.h=attrDouble(attrs,"H");
 node.radius=attrDouble(attrs,"Cr");
 bool validLayer=false;
 node.cutIndex=attrs.value("CutIndex").toString().toInt(&validLayer);
 if(!validLayer)node.cutIndex=0;
 if(node.cutIndex<0||node.cutIndex>31)throw std::runtime_error("CutIndex fora das camadas 0-31");
 if(node.type!="Group"&&node.type!="Rect"&&node.type!="Ellipse"){
  throw std::runtime_error(("Tipo de forma LightBurn ainda nao suportado: "+node.type).toStdString());
 }
 // Arcos arredondados ainda precisam de conversao de bezier; nao fingir que sao retos.
 if(node.type=="Rect"&&std::abs(node.radius)>1e-9)
  throw std::runtime_error("Retangulo com cantos arredondados ainda nao suportado");
 if(node.type!="Group"&&(!(node.w>0)||!(node.h>0)))
  throw std::runtime_error("Dimensoes ausentes para Rect/Ellipse");
 while(xml.readNextStartElement()){
  const QString tag=xml.name().toString();
  if(tag=="XForm")node.xform=parseXForm(xml.readElementText());
  else if(tag=="Children"){
   while(xml.readNextStartElement()){
    if(xml.name()==QLatin1String("Shape"))node.children.push_back(readShape(xml,depth+1,seen));
    else xml.skipCurrentElement();
   }
  }else xml.skipCurrentElement();
 }
 return node;
}
inline void flatten(const ParseNode& n,const QTransform& parent,
                    std::vector<GeometryShape>& result){
 // A composicao Qt opera com coordenadas como vetores linha:
 // transformacao local PRIMEIRO, grupo pai DEPOIS.
 const QTransform total=n.xform*parent;
 if(n.type=="Group"){
  for(const auto& c:n.children)flatten(c,total,result);
  return;
 }
 if(!n.children.empty())throw std::runtime_error("Forma com filhos inesperados");
 std::vector<Pt> pts;
 if(n.type=="Rect"){
  const double x=n.w/2.,y=n.h/2.;
  pts={{-x,-y},{x,-y},{x,y},{-x,y}};
 }else if(n.type=="Ellipse"){
  constexpr int steps=96;
  for(int i=0;i<steps;i++){
   const double angle=2*std::numbers::pi*i/steps;
   pts.push_back({(n.w/2.)*std::cos(angle),(n.h/2.)*std::sin(angle)});
  }
 }
 for(auto& p:pts){
  QPointF q=total.map(QPointF(p.x,p.y));
  if(!std::isfinite(q.x())||!std::isfinite(q.y())||std::abs(q.x())>1e7||std::abs(q.y())>1e7)
   throw std::runtime_error("Posicao transformada fora dos limites seguros");
  p={q.x(),q.y()};
 }
 result.push_back({std::move(pts),n.cutIndex});
}
inline ImportResult readGeometry(const QString& path){
 QFile file(path);
 if(!file.open(QIODevice::ReadOnly))throw std::runtime_error("Arquivo LightBurn nao acessivel");
 if(file.size()>32LL*1024*1024)throw std::runtime_error("Arquivo LightBurn excede 32 MB");
 QXmlStreamReader xml(&file);
 if(!xml.readNextStartElement()||xml.name()!=QLatin1String("LightBurnProject"))
  throw std::runtime_error("Formato nao e LightBurnProject XML");
 std::vector<ParseNode> roots;
 int seen=0;
 while(xml.readNextStartElement()){
  if(xml.name()==QLatin1String("Shape"))roots.push_back(readShape(xml,0,seen));
  else xml.skipCurrentElement(); // ignorar metadados de corte; NUNCA carregar configuracao da maquina
 }
 if(xml.hasError())throw std::runtime_error(("XML LightBurn invalido: "+xml.errorString()).toStdString());
 std::vector<GeometryShape> shapes;
 for(const auto& root:roots)flatten(root,QTransform(),shapes);
 if(shapes.empty())throw std::runtime_error("Projeto sem formas vetoriais compativeis");
 if(shapes.size()>30000)throw std::runtime_error("Limite de objetos excedido");
 Editor editor(Document{});
 std::set<int> requiredLayers;
 for(const auto& shape:shapes)requiredLayers.insert(shape.layer);
 for(int id:requiredLayers)if(id>0)editor.addLayer(id,"LightBurn C"+std::to_string(id));
 for(auto& shape:shapes)editor.addContour(std::move(shape.points),true,shape.layer);
 return ImportResult(editor.state(),int(shapes.size()));
}
} // namespace alaser::lightburn
