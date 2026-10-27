// TESTE_15: gate de GUI real, na MESMA janela do TESTE_6/14.
// Um processo grava; um segundo processo reabre e verifica identidade v5.
#define main alencar_previous_main
#include "qt6_test6.cpp"
#undef main
#include <iostream>
static void expect15(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
static CurveDocument fixture15(){
 CurveDocument d;d.nextId=10;CurveContour c;c.key={450,1};c.nodes={
  {1,{15,15},{15,15},{25,25}},
  {2,{35,15},{30,25},{40,5}},
  {3,{55,15},{50,5},{60,25}},
  {4,{75,15},{65,25},{75,15}}};
 c.edgeKind={1,1,1};d.contours.push_back(c);validateCurve(d);return d;
}
static QPoint pixel15(LabWindow& w,Pt p){return w.view.mapFromScene(toQt(p));}
static int run15(const QString& mode,const QString& destination){
 if(mode=="--write"){
  LabWindow window(Document{},fixture15());window.projectPath=destination;
  window.show();window.activateWindow();QApplication::processEvents();
  window.view.setTransform(QTransform::fromScale(6,6));window.view.centerOn(35,15);
  QApplication::processEvents();clickButton(window,"bezier");
  auto& v=window.view;auto& session=*window.curves;
  expect15(v.nativeMode()&&v.nativeItemCount()==1,"Bézier missing from integrated Qt scene");
  auto pathBefore=v.nativeItemPath({450,1});
  QString evidence=QFileInfo(destination).absolutePath();QDir().mkpath(evidence);
  v.viewport()->grab().save(evidence+"/TESTE15_ANTES.png");
  QTest::mouseClick(v.viewport(),Qt::LeftButton,Qt::NoModifier,pixel15(window,{15,15}));
  expect15(v.nativeSelected()&&*v.nativeSelected()==Key{450,1},"mouse cannot select Bezier curve");
  // Shift+clique: inserir nó no contorno selecionado.
  const auto midpoint=cubicPoint(edgeAt(session.committed().contours.at(0),0),.5);
  QTest::mouseClick(v.viewport(),Qt::LeftButton,Qt::ShiftModifier,pixel15(window,midpoint));
  expect15(session.committed().contours[0].nodes.size()==5,"Shift+click did not insert node");
  // Ctrl+Z remove a inserção; Ctrl+Y refaz.
  QTest::keyClick(&v,Qt::Key_Z,Qt::ControlModifier);QApplication::processEvents();
  expect15(session.committed().contours[0].nodes.size()==4,"Ctrl+Z did not undo insert");
  QTest::keyClick(&v,Qt::Key_Y,Qt::ControlModifier);QApplication::processEvents();
  expect15(session.committed().contours[0].nodes.size()==5,"Ctrl+Y did not redo insert");
  // Seleciona a ancora original e move a alca sem alterar o documento durante preview.
  QTest::mouseClick(v.viewport(),Qt::LeftButton,Qt::NoModifier,pixel15(window,{35,15}));
  auto old=serialize(encodeCurves(session.committed()));
  QPoint start=pixel15(window,{40,5}),end=start+QPoint(14,-9);
  QTest::mousePress(v.viewport(),Qt::LeftButton,Qt::NoModifier,start);
  QTest::mouseMove(v.viewport(),end,5);
  QApplication::processEvents();
  expect15(v.nativeItemPath({450,1})==nativeCurvePath(session.visible().contours[0]),
       "painted Bezier path differs from live preview geometry");
  v.viewport()->grab().save(evidence+"/TESTE15_DURANTE.png");
  expect15(serialize(encodeCurves(session.committed()))==old,"preview mutated committed curve");
  expect15(v.nativeItemPath({450,1})!=pathBefore,"curve preview not rendered");
  QTest::mouseRelease(v.viewport(),Qt::LeftButton,Qt::NoModifier,end);
  expect15(!session.dragging()&&serialize(encodeCurves(session.committed()))!=old,"handle drag not committed");
  expect15(v.nativeItemPath({450,1})==nativeCurvePath(session.committed().contours[0]),
       "post-drag rendered path not canonical");
  v.viewport()->grab().save(evidence+"/TESTE15_DEPOIS.png");
  // Cancelamento Esc no gesto: nao altera o ultimo commit.
  auto snapshot=serialize(encodeCurves(session.committed()));
  QTest::mousePress(v.viewport(),Qt::LeftButton,Qt::NoModifier,pixel15(window,{15,15}));
  QTest::mouseMove(v.viewport(),pixel15(window,{15,15})+QPoint(17,-8),5);
  QTest::keyClick(&v,Qt::Key_Escape);QTest::mouseRelease(v.viewport(),Qt::LeftButton,Qt::NoModifier,pixel15(window,{15,15})+QPoint(17,-8));
  expect15(serialize(encodeCurves(session.committed()))==snapshot,"Esc did not cancel drag");
  expect15(v.nativeItemPath({450,1})==nativeCurvePath(session.committed().contours[0]),
       "Escape left ghost geometry");
  v.viewport()->grab().save(evidence+"/TESTE15_CANCELAMENTO.png");
  // O Del apaga EXATAMENTE a ancora selecionada, nunca outro contorno.
  QTest::mouseClick(v.viewport(),Qt::LeftButton,Qt::NoModifier,pixel15(window,{35,15}));
  expect15(v.nativeSelectedNode().has_value(),"selected node missing before Delete");
  QTest::keyClick(&v,Qt::Key_Delete);QApplication::processEvents();
  expect15(session.committed().contours[0].nodes.size()==4,"Delete failed in Qt integrated window");
  window.save();expect15(QFile::exists(destination),"Qt saved no .alaser v5");
  const auto actual=readIntegratedProject(destination.toStdString());
  expect15(actual.version==5&&actual.curves.contours[0].nodes.size()==4,"Qt save v5 lost changes");
  std::cout<<"TESTE15 QT WRITE PASS nodes="<<actual.curves.contours[0].nodes.size()<<"\n";
  return 0;
 }
 if(mode=="--verify"){
  LabWindow window(Document{});window.projectPath=destination;
  window.show();QApplication::processEvents();window.open();
  expect15(window.curves&&window.view.nativeItemCount()==1,"native curves not recreated on Qt reopen");
  clickButton(window,"bezier");
  expect15(window.view.nativeMode(),"Qt cannot re-enable Bezier after reopening");
  auto project=readIntegratedProject(destination.toStdString());
  expect15(project.version==5,"Qt reload version invalid");
  expect15(serialize(encodeCurves(window.curves->committed()))==serialize(encodeCurves(project.curves)),
     "Qt reload changed Bezier IDs or controls");
  expect15(window.view.nativeItemPath({450,1})==nativeCurvePath(project.curves.contours[0]),
     "Qt render differs from restored native geometry");
  std::cout<<"TESTE15 QT VERIFY PASS reloaded_contours="<<project.curves.contours.size()<<"\n";
  return 0;
 }
 throw std::runtime_error("use --write or --verify");
}
int main(int argc,char**argv){
 QApplication app(argc,argv);
 try{expect15(argc==3,"usage: test15 --write|--verify project.alaser");return run15(argv[1],argv[2]);}
 catch(const std::exception& e){std::cerr<<"TESTE15 QT FAIL: "<<e.what()<<"\n";return 1;}
}
