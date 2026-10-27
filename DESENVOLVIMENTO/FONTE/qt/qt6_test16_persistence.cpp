// TESTE_16 Qt gate: mesma LabWindow TESTE_6/15 com import v5 experimental
// sem metadados completos; prova abertura, salvamento e outro processo Qt.
#define main alencar_earlier_main
#include "qt6_test6.cpp"
#undef main
#include <iostream>
int main(int argc,char** argv){
 QApplication app(argc,argv);
 try{
  check(argc==4,"usage: --write|--verify input.alaser output.alaser");
  const QString source=QString::fromLocal8Bit(argv[2]);
  const QString target=QString::fromLocal8Bit(argv[3]);
  const auto origin=readIntegratedProject(source.toStdString());
  LabWindow window(Document{});
  window.show();QApplication::processEvents();
  if(QString::fromLocal8Bit(argv[1])=="--write"){
   window.projectPath=source;
   window.open();
   check(window.view.editor.state().doc.contours.size()==origin.legacy.doc.contours.size(),"GUI v5 imported wrong polyline count");
   check(window.committedCurves()&&window.committedCurves()->contours.size()==origin.curves.contours.size(),"GUI import lost curves");
   clickButton(window,"bezier");
   check(window.view.nativeMode(),"GUI cannot select imported native mode");
   check(window.view.nativeItemPath(origin.curves.contours.front().key)==nativeCurvePath(origin.curves.contours.front()),"GUI renders noncanonical path");
   window.projectPath=target;
   window.save();
   check(QFile::exists(target),"GUI failed to save");
   const auto saved=readIntegratedProject(target.toStdString());
   check(saved.legacy.curveSource==origin.legacy.curveSource,"GUI silently removed source samples");
   check(serialize(saved.legacy.operations)==serialize(safeOperations(origin.legacy.operations)),"GUI silently rewrote operation settings");
   check(saved.legacy.layers.as_object().at("schema").as_string()=="editor-meta-1","GUI failed to upgrade experimental layer metadata");
   auto f=origin.legacy.layers.as_object().find("source");
   if(f!=origin.legacy.layers.as_object().end())check(saved.legacy.layers.as_object().at("source")==f->value(),"GUI removed metadata provenance");
   std::cout<<"TESTE16 QT WRITE PASS polygons="<<saved.legacy.doc.contours.size()<<" native="<<saved.curves.contours.size()<<"\n";
  }else if(QString::fromLocal8Bit(argv[1])=="--verify"){
   window.projectPath=target;window.open();
   const auto saved=readIntegratedProject(target.toStdString());
   check(saved.version==5,"schema version lost after Qt re-open");
   check(window.committedCurves()!=nullptr,"Qt reader lost curves");
   check(serialize(encodeCurves(*window.committedCurves()))==serialize(encodeCurves(saved.curves)),"Qt native ids differ after restart");
   check(serialize(metadata(window.view.editor.state()))==serialize(metadata(fromMetadata(saved.legacy.doc,saved.legacy.layers))),"Qt editor metadata differs after restart");
   check(saved.legacy.curveSource==origin.legacy.curveSource,"source lost in independent Qt process");
   check(serialize(saved.legacy.operations)==serialize(safeOperations(origin.legacy.operations)),"Qt reader lost settings");
   clickButton(window,"bezier");check(window.view.nativeMode(),"reopened Qt lacks native edit mode");
   std::cout<<"TESTE16 QT VERIFY PASS separate_process=true\n";
  }else throw std::runtime_error("unknown mode");
  return 0;
 }catch(const std::exception& e){std::cerr<<"TESTE16 QT FAIL: "<<e.what()<<"\n";return 1;}
}
