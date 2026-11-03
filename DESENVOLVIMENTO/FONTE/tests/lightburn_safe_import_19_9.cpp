#include "../src/lightburn_geometry_import.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QTemporaryDir>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

static void require(bool ok,const char* reason){
 if(!ok)throw std::runtime_error(reason);
}
static QString makeXml(const QString& path,const QString& shape){
 QFile f(path);
 if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate))
  throw std::runtime_error("Could not create test fixture");
 const QString text=QStringLiteral("<?xml version=\"1.0\"?><LightBurnProject MirrorX=\"False\" MirrorY=\"False\">")
  +shape+QStringLiteral("</LightBurnProject>");
 const auto bytes=text.toUtf8();
 require(f.write(bytes)==bytes.size(),"Fixture was not written in full");
 f.close();
 return path;
}
int main(int argc,char** argv){
 QCoreApplication app(argc,argv);
 try{
  QTemporaryDir temp;
  require(temp.isValid(),"Temporary fixture folder unavailable");
  const QString ellipse=makeXml(temp.filePath("ellipse.lbrn2"),
   QStringLiteral("<Shape Type=\"Ellipse\" CutIndex=\"1\" Rx=\"2\" Ry=\"3\">"
                  "<XForm>1 0 0 1 10 20</XForm></Shape>"));
  const auto imported=alaser::lightburn::readGeometry(ellipse);
  require(imported.shapeCount==1,"Ellipse count changed");
  require(imported.state.doc.contours.size()==1,"Ellipse contour missing");
  const auto& contour=imported.state.doc.contours.front();
  require(contour.nodes.size()==384,"Ellipse resolution changed");
  require(imported.state.objects.at(contour.key.object).layer==1,"Ellipse layer lost");
  require(std::abs(contour.nodes.front().p.x-12.)<1e-6 &&
          std::abs(contour.nodes.front().p.y-20.)<1e-6,
          "Ellipse Rx/Ry or XForm mismatch");

  const QString conflict=makeXml(temp.filePath("conflict.lbrn2"),
   QStringLiteral("<Shape Type=\"Ellipse\" CutIndex=\"0\" Rx=\"2\" Ry=\"3\" W=\"100\" H=\"6\"/>"));
  bool rejectedConflict=false;
  try{(void)alaser::lightburn::readGeometry(conflict);}
  catch(const std::exception& e){rejectedConflict=std::string(e.what()).find("conflitantes")!=std::string::npos;}
  require(rejectedConflict,"Conflicting W and Rx must be rejected");

  // A synthetic 264k-output-node Path is rejected BEFORE decoding its
  // incomplete Bezier handles or constructing any editor geometry.
  QString primitives;
  primitives.reserve(11000*5);
  for(int i=0;i<11000;++i)primitives+=QStringLiteral("B0 1");
  const QString huge=makeXml(temp.filePath("too_many_nodes.lbrn2"),
   QStringLiteral("<Shape Type=\"Path\" CutIndex=\"1\" VertID=\"1\" PrimID=\"1\">"
                  "<VertList>V0 0V1 0</VertList><PrimList>")+
   primitives+QStringLiteral("</PrimList></Shape>"));
  bool rejectedBeforeDecode=false;
  try{(void)alaser::lightburn::readGeometry(huge);}
  catch(const std::exception& e){rejectedBeforeDecode=std::string(e.what()).find("250 mil nos")!=std::string::npos;}
  require(rejectedBeforeDecode,"Early node budget check failed");

  if(argc>1){
   // Read-only operator sample: large LightBurn projects MUST be rejected
   // safely instead of building all geometry or changing an open document.
   const auto original=QString::fromLocal8Bit(argv[1]);
   bool blocked=false;
   try{(void)alaser::lightburn::readGeometry(original);}
   catch(const std::exception& e){
    std::cout<<"REAL_FILE_RESULT: "<<e.what()<<"\n";
    blocked=std::string(e.what()).find("250 mil nos")!=std::string::npos;
   }
   require(blocked,"Expected the supplied >250k-node file to be refused safely");
  }
  std::cout<<"PASS LightBurn 19.9: Rx/Ry, strict dimensions, early node budget, no laser\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"FAIL LightBurn 19.9: "<<e.what()<<"\n";
  return 1;
 }
}
