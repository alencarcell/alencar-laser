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
#include <numbers>
#include <set>
#include <map>
#include <stdexcept>
#include <vector>

namespace alaser::lightburn {
struct GeometryShape {
 std::vector<Pt> points;
 int layer=0;
 bool closed=true;
 int group=0;
};
struct ParseNode{
 QString type;
 QTransform xform;
 double w=0,h=0,radius=0;
 int cutIndex=0;
 int vertId=-1,primId=-1;
 QString vertList,primList;
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
 auto sharedId=[&](const char* key)->int{
  const auto raw=attrs.value(QLatin1String(key));
  if(raw.isEmpty())return -1;
  bool ok=false;const int id=raw.toString().toInt(&ok);
  if(!ok||id<0)throw std::runtime_error("Identificador compartilhado LBRN2 invalido");
  return id;
 };
 node.vertId=sharedId("VertID");node.primId=sharedId("PrimID");
 node.w=attrDouble(attrs,"W");
 node.h=attrDouble(attrs,"H");
 node.radius=attrDouble(attrs,"Cr");
 bool validLayer=false;
 node.cutIndex=attrs.value("CutIndex").toString().toInt(&validLayer);
 if(!validLayer)node.cutIndex=0;
 if(node.cutIndex<0||node.cutIndex>31)throw std::runtime_error("CutIndex fora das camadas 0-31");
 if(node.type!="Group"&&node.type!="Rect"&&node.type!="Ellipse"&&node.type!="Path"){
  throw std::runtime_error(("Tipo de forma LightBurn ainda nao suportado: "+node.type).toStdString());
 }
 // Raio valido de cantos arredondados; sem retangularizar silenciosamente.
 if(node.type=="Rect"&&(node.radius<0||node.radius>std::min(node.w,node.h)/2.+1e-9))
  throw std::runtime_error("Raio Cr fora dos limites de Rect");
 if(node.type!="Group"&&node.type!="Path"&&(!(node.w>0)||!(node.h>0)))
  throw std::runtime_error("Dimensoes ausentes para Rect/Ellipse");
 while(xml.readNextStartElement()){
  const QString tag=xml.name().toString();
  if(tag=="XForm")node.xform=parseXForm(xml.readElementText());
  else if(tag=="VertList")node.vertList=xml.readElementText();
  else if(tag=="PrimList")node.primList=xml.readElementText();
  else if(tag=="Children"){
   while(xml.readNextStartElement()){
    if(xml.name()==QLatin1String("Shape"))node.children.push_back(readShape(xml,depth+1,seen));
    else xml.skipCurrentElement();
   }
  }else xml.skipCurrentElement();
 }
 return node;
}

inline void gatherSharedGeometry(const ParseNode& n,
      std::map<int,QString>& verts,std::map<int,QString>& prims){
 if(n.type=="Path"){
  auto add=[&](int id,const QString& value,std::map<int,QString>& definitions){
   if(id<0||value.isEmpty())return;
   const auto [it,inserted]=definitions.emplace(id,value);
   if(!inserted&&it->second!=value)
    throw std::runtime_error("LBRN2 referencia compartilhada tem duas definicoes distintas");
  };
  add(n.vertId,n.vertList,verts);
  add(n.primId,n.primList,prims);
 }
 for(const auto& child:n.children)gatherSharedGeometry(child,verts,prims);
}
inline void resolveSharedGeometry(ParseNode& n,
      const std::map<int,QString>& verts,const std::map<int,QString>& prims){
 if(n.type=="Path"){
  if(n.vertList.isEmpty()){
   auto it=verts.find(n.vertId);
   if(it==verts.end())throw std::runtime_error("LBRN2 VertID referenciado sem VertList");
   n.vertList=it->second;
  }
  if(n.primList.isEmpty()){
   auto it=prims.find(n.primId);
   if(it==prims.end())throw std::runtime_error("LBRN2 PrimID referenciado sem PrimList");
   n.primList=it->second;
  }
 }
 for(auto& child:n.children)resolveSharedGeometry(child,verts,prims);
}
struct PackedVertex {
 Pt p{};
 Pt out{},in{};
 bool hasOutX=false,hasOutY=false,hasInX=false,hasInY=false;
};
inline std::vector<PackedVertex> decodeVertices(const QString& encoded){
 static const QRegularExpression vexpr(
  R"(V([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)\s+([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)([^V]*))");
 static const QRegularExpression hExpr(
  R"(c([01])([xy])([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?))");
 std::vector<PackedVertex> result;
 auto it=vexpr.globalMatch(encoded);
 qsizetype offset=0;
 while(it.hasNext()){
  auto m=it.next();
  if(!encoded.mid(offset,m.capturedStart()-offset).trimmed().isEmpty())
   throw std::runtime_error("VertList contem dados nao reconhecidos");
  offset=m.capturedEnd();
  if(result.size()>=50000)throw std::runtime_error("Excesso de vertices LightBurn");
  PackedVertex v;
  bool okX=false,okY=false;
  v.p.x=m.captured(1).toDouble(&okX);v.p.y=m.captured(2).toDouble(&okY);
  if(!okX||!okY||!std::isfinite(v.p.x)||!std::isfinite(v.p.y))
   throw std::runtime_error("Coordenada LightBurn invalida");
  const QString tail=m.captured(3);
  auto h=hExpr.globalMatch(tail);qsizetype k=0;
  while(h.hasNext()){
   const auto a=h.next();
   QString between=tail.mid(k,a.capturedStart()-k).trimmed();
   if(between!="S"&&!between.isEmpty())throw std::runtime_error("Controle de Bezier desconhecido");
   k=a.capturedEnd();
   bool ok=false;const double val=a.captured(3).toDouble(&ok);
   if(!ok||!std::isfinite(val))throw std::runtime_error("Controle de Bezier invalido");
   const bool isOut=a.captured(1)=="0",x=a.captured(2)=="x";
   if(isOut){if(x){v.out.x=val;v.hasOutX=true;}else{v.out.y=val;v.hasOutY=true;}}
   else{if(x){v.in.x=val;v.hasInX=true;}else{v.in.y=val;v.hasInY=true;}}
  }
  QString trailing=tail.mid(k).trimmed();
  if(trailing!="S"&&!trailing.isEmpty())throw std::runtime_error("Dados extras VertList desconhecidos");
  result.push_back(v);
 }
 if(!encoded.mid(offset).trimmed().isEmpty()||result.empty())
  throw std::runtime_error("VertList nao decodificavel");
 return result;
}
inline std::pair<std::vector<Pt>,bool> decodePath(const ParseNode& n){
 const auto vertices=decodeVertices(n.vertList);
 // Compactacao real LightBurn 2.x: vertices em ordem, ligados por linha.
 // LineClosed fecha o ultimo no ao primeiro; LineOpen mantem aberto.
 // Os controles de Bezier sem PrimList nao sao interpretados como cubicas.
 const QString type=n.primList.trimmed();
 if(type=="LineClosed"||type=="LineOpen"){
  const bool closed=type=="LineClosed";
  std::vector<Pt> points;
  points.reserve(vertices.size());
  for(const auto& vertex:vertices)points.push_back(vertex.p);
  if(closed&&points.size()>=2&&dist(points.front(),points.back())<1e-7)
   points.pop_back();
  if(points.size()<(closed?3U:2U))
   throw std::runtime_error("Path LineOpen/LineClosed com poucos vertices");
  return {std::move(points),closed};
 }
 static const QRegularExpression primitive(R"(([LB])(\d+)\s+(\d+))");
 auto it=primitive.globalMatch(n.primList);
 std::vector<Pt> points;
 int first=-1,previous=-1,primitives=0;
 qsizetype offset=0;
 while(it.hasNext()){
  auto m=it.next();
  if(!n.primList.mid(offset,m.capturedStart()-offset).trimmed().isEmpty())
   throw std::runtime_error("PrimList contem trecho desconhecido");
  offset=m.capturedEnd();
  bool o1=false,o2=false;
  int a=m.captured(2).toInt(&o1),b=m.captured(3).toInt(&o2);
  if(!o1||!o2||a<0||b<0||a>=int(vertices.size())||b>=int(vertices.size()))
   throw std::runtime_error("Indice PrimList fora de VertList");
  if(first<0){first=a;points.push_back(vertices[size_t(a)].p);}
  else if(previous!=a)throw std::runtime_error("Path contem multiplas cadeias; importacao parcial bloqueada");
  if(++primitives>50000)throw std::runtime_error("Excesso de segmentos LightBurn");
  if(m.captured(1)=="B"){
   const auto& va=vertices[size_t(a)],& vb=vertices[size_t(b)];
   if(!(va.hasOutX&&va.hasOutY&&vb.hasInX&&vb.hasInY))
    throw std::runtime_error("Bezier LightBurn sem duas alcas completas");
   const Pt p0=va.p,p1=va.out,p2=vb.in,p3=vb.p;
   // Aproximacao segura em 24 segmentos por cubica; curvas nativas continuam
   // pendentes de importador proprio (sem emitir comandos para laser).
   for(int step=1;step<=24;step++){
    const double t=double(step)/24.,u=1.-t;
    points.push_back(p0*(u*u*u)+p1*(3*u*u*t)+p2*(3*u*t*t)+p3*(t*t*t));
   }
  }else points.push_back(vertices[size_t(b)].p);
  previous=b;
 }
 if(!n.primList.mid(offset).trimmed().isEmpty()||primitives==0)
  throw std::runtime_error("PrimList desconhecida ou vazia");
 const bool closed=first==previous;
 if(closed&&!points.empty()&&dist(points.front(),points.back())<1e-7)points.pop_back();
 if(points.size()<(closed?3U:2U))
  throw std::runtime_error("Path sem nos suficientes");
 return {std::move(points),closed};
}


inline std::vector<std::pair<std::vector<Pt>,bool>> decodePathPieces(const ParseNode& n){
 // Splits disconnected primitive chains WITHOUT inserting a connecting edge.
 // Delegates every contiguous run to the existing strict Bezier/Line decoder.
 const QString encoded=n.primList.trimmed();
 if(encoded=="LineClosed"||encoded=="LineOpen")return {decodePath(n)};
 static const QRegularExpression primitive(R"(([LB])(\d+)\s+(\d+))");
 auto it=primitive.globalMatch(encoded);
 std::vector<QString> runs;
 QString run;
 int last=-1,count=0;
 qsizetype offset=0;
 while(it.hasNext()){
  const auto m=it.next();
  if(!encoded.mid(offset,m.capturedStart()-offset).trimmed().isEmpty())
   throw std::runtime_error("PrimList contem trecho desconhecido");
  offset=m.capturedEnd();
  bool okA=false,okB=false;
  const int a=m.captured(2).toInt(&okA),b=m.captured(3).toInt(&okB);
  if(!okA||!okB||a<0||b<0)
   throw std::runtime_error("Indice PrimList invalido");
  if(last>=0&&last!=a){
   runs.push_back(run);run.clear();
  }
  if(++count>50000)throw std::runtime_error("Excesso de segmentos LightBurn");
  if(!run.isEmpty())run+=" ";
  run+=m.captured(0);
  last=b;
 }
 if(!encoded.mid(offset).trimmed().isEmpty()||count==0)
  throw std::runtime_error("PrimList desconhecida ou vazia");
 if(!run.isEmpty())runs.push_back(run);
 if(runs.size()>30000)throw std::runtime_error("Excesso de subcaminhos LightBurn");
 std::vector<std::pair<std::vector<Pt>,bool>> result;
 result.reserve(runs.size());
 for(const auto& part:runs){
  ParseNode piece=n;
  piece.primList=part;
  result.push_back(decodePath(piece));
 }
 return result;
}

inline void flatten(const ParseNode& n,const QTransform& parent,
                    std::vector<GeometryShape>& result,
                    int& nextGroup,int inheritedGroup=0){
 // A composicao Qt opera com coordenadas como vetores linha:
 // transformacao local PRIMEIRO, grupo pai DEPOIS.
 const QTransform total=n.xform*parent;
 if(n.type=="Group"){
  // EditorState suporta um nivel de agrupamento. Preservar grupo externo.
  const int group=inheritedGroup>0?inheritedGroup:nextGroup++;
  for(const auto& c:n.children)flatten(c,total,result,nextGroup,group);
  return;
 }
 if(!n.children.empty())throw std::runtime_error("Forma com filhos inesperados");
 std::vector<Pt> pts;
 if(n.type=="Rect"){
  const double x=n.w/2.,y=n.h/2.;
  const double radius=std::min(n.radius,std::min(x,y));
  if(radius<=1e-9)pts={{-x,-y},{x,-y},{x,y},{-x,y}};
  else{
   constexpr int arcSteps=24;
   const std::array<Pt,4> centers{{{x-radius,-y+radius},
    {x-radius,y-radius},{-x+radius,y-radius},{-x+radius,-y+radius}}};
   for(int corner=0;corner<4;++corner)
    for(int j=0;j<=arcSteps;++j){
     const double angle=(-90.+90.*corner+90.*j/arcSteps)*std::numbers::pi/180.;
     pts.push_back({centers[size_t(corner)].x+radius*std::cos(angle),
      centers[size_t(corner)].y+radius*std::sin(angle)});
    }
  }
 }else if(n.type=="Path"){
  const auto pieces=decodePathPieces(n);
  if(n.children.size())throw std::runtime_error("Path com filhos nao suportada");
  for(auto piece:pieces){
   for(auto& p:piece.first){
    const QPointF q=total.map(QPointF(p.x,p.y));
    if(!std::isfinite(q.x())||!std::isfinite(q.y())||std::abs(q.x())>1e7||std::abs(q.y())>1e7)
     throw std::runtime_error("Path fora dos limites seguros");
    p={q.x(),q.y()};
   }
   result.push_back({std::move(piece.first),n.cutIndex,piece.second,inheritedGroup});
  }
  return;
 }else if(n.type=="Ellipse"){
  constexpr int steps=384;
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
 result.push_back({std::move(pts),n.cutIndex,true,inheritedGroup});
}
inline ImportResult readGeometry(const QString& path){
 QFile file(path);
 if(!file.open(QIODevice::ReadOnly))throw std::runtime_error("Arquivo LightBurn nao acessivel");
 if(file.size()>32LL*1024*1024)throw std::runtime_error("Arquivo LightBurn excede 32 MB");
 QXmlStreamReader xml(&file);
 if(!xml.readNextStartElement()||xml.name()!=QLatin1String("LightBurnProject"))
  throw std::runtime_error("Formato nao e LightBurnProject XML");
 // A convencao de espelhamento/origem depende do perfil LightBurn. Evitar
 // converter silenciosamente coordenadas para outro quadrante da mesa.
 const auto rootAttrs=xml.attributes();
 for(const char* key:{"MirrorX","MirrorY"}){
  const QString value=rootAttrs.value(QLatin1String(key)).toString();
  if(value.compare("True",Qt::CaseInsensitive)==0||value=="1")
   throw std::runtime_error("Projeto LightBurn com MirrorX/MirrorY ativo: orientacao pendente de homologacao");
 }
 std::vector<ParseNode> roots;
 int seen=0;
 while(xml.readNextStartElement()){
  if(xml.name()==QLatin1String("Shape"))roots.push_back(readShape(xml,0,seen));
  else xml.skipCurrentElement(); // ignorar metadados de corte; NUNCA carregar configuracao da maquina
 }
 if(xml.hasError())throw std::runtime_error(("XML LightBurn invalido: "+xml.errorString()).toStdString());
 // Algumas formas reutilizam o PrimID/VertID de outra Shape e nao repetem
 // PrimList/VertList. Resolver DEPOIS do XML completo para aceitar referencias
 // anteriores ou posteriores, inclusive em Group/Children aninhados.
 std::map<int,QString> sharedVertices,sharedPrimitives;
 for(const auto& root:roots)gatherSharedGeometry(root,sharedVertices,sharedPrimitives);
 for(auto& root:roots)resolveSharedGeometry(root,sharedVertices,sharedPrimitives);
 std::vector<GeometryShape> shapes;
 int nextGroup=1;
 for(const auto& root:roots)flatten(root,QTransform(),shapes,nextGroup);
 if(shapes.empty())throw std::runtime_error("Projeto sem formas vetoriais compativeis");
 if(shapes.size()>30000)throw std::runtime_error("Limite de objetos excedido");
 // Construir documento isolado diretamente, sem registrar uma copia integral
 // por objeto no historico. Evita custo quadratico em projetos grandes.
 EditorState state=fromDocument(Document{});
 int object=0;
 size_t totalNodes=0;
 for(auto& shape:shapes){
  totalNodes+=shape.points.size();
  if(totalNodes>250000)throw std::runtime_error("Limite de 250 mil nos na importacao LBRN2");
  if(!state.layers.count(shape.layer)){
   state.layers[shape.layer]={shape.layer,"LightBurn C"+std::to_string(shape.layer),true,false};
  }
  Contour contour;contour.key={++object,0};contour.closed=shape.closed;
  for(Pt p:shape.points)contour.nodes.push_back({state.doc.nextId++,p});
  state.doc.contours.push_back(std::move(contour));
  state.objects[object]={shape.layer,shape.group};
 }
 state.nextGroup=nextGroup;
 validate(state);
 return ImportResult(std::move(state),int(shapes.size()));
}
} // namespace alaser::lightburn
