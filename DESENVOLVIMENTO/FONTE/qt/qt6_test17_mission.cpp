// Executes the production laboratory window, with Qt mouse/key events.
#define main alencar_existing_main
#include "qt6_test6.cpp"
#undef main
#include <iostream>
static const CurveContour* curveContour(const CurveDocument& d,Key k){for(const auto& c:d.contours)if(c.key==k)return &c;throw std::runtime_error("missing curve key");}
static std::string exact17(const IntegratedProject& p){return serialize(encode(p.legacy.doc))+serialize(encodeCurves(p.curves))+serialize(p.legacy.layers)+serialize(p.legacy.operations)+p.legacy.curveSource;}
static std::string state17(const LabWindow& w){return stateText(w.view.editor.state())+(w.curves?serialize(encodeCurves(w.curves->committed())):"");}
static void drag17(LabWindow& w,Pt from,QPoint delta,bool cancel=false,CurvePart intended=CurvePart::Anchor){
 auto& v=w.view;auto p=v.mapFromScene(toQt(from));auto before=serialize(encodeCurves(w.curves->committed()));
 auto original=w.curves->committed();auto key=*v.nativeSelected();int64_t targetId=-1;
 for(const auto& c:original.contours)if(c.key==key)for(const auto& n:c.nodes){
  Pt at=intended==CurvePart::Anchor?n.anchor:intended==CurvePart::HandleIn?n.handleIn:n.handleOut;
  if(dist(at,from)<1e-8){targetId=n.id;break;}
 }
 check(targetId>=0,"oracle cannot identify requested control");
 QTest::mousePress(v.viewport(),Qt::LeftButton,Qt::NoModifier,p);
 check(w.curves->dragging(),"mouse did not start native drag");
 for(int i=1;i<=10;++i){QTest::mouseMove(v.viewport(),p+delta*i/10,1);QApplication::processEvents();
  check(serialize(encodeCurves(w.curves->committed()))==before,"preview mutated committed state");
  for(const auto& c:w.curves->visible().contours)check(v.nativeItemPath(c.key)==nativeCurvePath(c),"live path not canonical");
 }
 if(cancel)pressKey(w,Qt::Key_Escape);
 QTest::mouseRelease(v.viewport(),Qt::LeftButton,Qt::NoModifier,p+delta);QApplication::processEvents();
 auto expected=original;
 if(!cancel)for(auto& c:expected.contours)if(c.key==key)for(auto& n:c.nodes)if(n.id==targetId){
  Pt target=fromQt(v.mapToScene(p+delta));
  if(intended==CurvePart::Anchor){Pt shift=target-n.anchor;n.anchor=target;n.handleIn=n.handleIn+shift;n.handleOut=n.handleOut+shift;}
  else if(intended==CurvePart::HandleIn)n.handleIn=target;else n.handleOut=target;
 }
 check(serialize(encodeCurves(w.curves->committed()))==serialize(encodeCurves(expected)),"drag changed wrong control, id or contour");
 check(!w.curves->dragging(),"drag remains active");
 check((serialize(encodeCurves(w.curves->committed()))==before)==cancel,"wrong drag commit/cancel");
}
static void interaction17(const QString& folder){
 CurveDocument d;d.nextId=20;
 CurveContour c;c.key={900,1};c.nodes={{1,{15,15},{10,5},{25,25}},{2,{35,15},{30,25},{40,5}},{3,{55,15},{50,5},{65,25}}};c.edgeKind={1,1};
 d.contours.push_back(c);c.key={900,2};for(auto& n:c.nodes)n.id+=10;d.contours.push_back(c);
 LabWindow w(Document{},d);w.show();w.activateWindow();QApplication::processEvents();clickButton(w,"bezier");auto& v=w.view;
 for(double zoom:{1.,6.,24.,200.}){
  v.setTransform(QTransform::fromScale(zoom,zoom));Pt mid=cubicPoint(edgeAt(d.contours[0],0),.5);v.centerOn(toQt(mid));QApplication::processEvents();
  clickScene(w,toQt(mid));check(v.nativeSelected()==std::optional<Key>{{900,1}},"edge click picked wrong subpath");
  clickScene(w,toQt(mid),Qt::AltModifier);check(v.nativeSelected()==std::optional<Key>{{900,2}},"Alt did not cycle subpath");
  auto outside=v.mapFromScene(toQt(mid))+QPoint(0,int(std::ceil(10./v.viewport()->devicePixelRatioF())));
  QTest::mouseClick(v.viewport(),Qt::LeftButton,Qt::NoModifier,outside);QApplication::processEvents();
  check(v.nativeSelected()==std::optional<Key>{{900,2}},"pixel selection tolerance expanded with scene zoom");
  check(w.grab().save(folder+"/ZOOM_"+QString::number(zoom)+".png"),"zoom screenshot failed");
  // Insert must target the explicitly selected overlapping subpath.
  clickScene(w,toQt(mid),Qt::ShiftModifier);
  check(w.curves->committed().contours[0].nodes.size()==3&&w.curves->committed().contours[1].nodes.size()==4,"insert changed wrong overlapping contour");
  pressKey(w,Qt::Key_Z,Qt::ControlModifier);check(serialize(encodeCurves(w.curves->committed()))==serialize(encodeCurves(d)),"insert undo changed geometry/ids");
  pressKey(w,Qt::Key_Y,Qt::ControlModifier);check(w.curves->committed().contours[1].nodes.size()==4,"insert redo missing");pressKey(w,Qt::Key_Z,Qt::ControlModifier);
 }
 v.setTransform(QTransform::fromScale(6,6));v.centerOn(35,15);QApplication::processEvents();
 clickScene(w,{35,15},Qt::AltModifier); // select through real events, then assert before dragging
 auto selected=*v.nativeSelected();clickScene(w,{35,15});
 drag17(w,{40,5},{15,-12},false,CurvePart::HandleOut);drag17(w,{30,25},{-12,9},false,CurvePart::HandleIn);drag17(w,{35,15},{9,6});
 auto before=state17(w);auto key=*v.nativeSelected();auto* cc=curveContour(w.curves->committed(),key);
 auto node=cc->nodes[1];drag17(w,node.anchor,{15,-10},true);check(state17(w)==before,"Escape changed state");
 clickScene(w,toQt(node.anchor));pressKey(w,Qt::Key_Delete);check(curveContour(w.curves->committed(),key)->nodes.size()==2,"delete did not remove selected node");
 pressKey(w,Qt::Key_Z,Qt::ControlModifier);check(state17(w)==before,"delete undo lost identity");
 // An empty-space click must not change the document.
 clickScene(w,{95,85});check(state17(w)==before,"outside click changed geometry");
 for(const auto& item:w.curves->committed().contours)check(v.nativeItemPath(item.key)==nativeCurvePath(item),"ghost geometry");
 check(w.grab().save(folder+"/INTERACAO_TESTE_17.png"),"screenshot failed");
 w.close();QApplication::processEvents();
 std::cout<<"INTERACTION PASS zoom=1,6,24,200 overlap insert anchor in out delete undo redo escape outside\n";
}
static void persistence17(const QString& mode,const QString& input,const QString& output){
 const auto origin=readIntegratedProject(input.toStdString());
 LabWindow w(Document{});w.show();w.activateWindow();QApplication::processEvents();w.projectPath=mode=="--write"?input:output;clickButton(w,"open");
 if(mode=="--write"){
  check(!w.curves->committed().contours.empty(),"no native curves");
  auto c=w.curves->committed().contours.front();auto n=c.nodes.front();
  clickButton(w,"bezier");w.view.setTransform(QTransform::fromScale(8,8));w.view.centerOn(toQt(n.anchor));QApplication::processEvents();
  clickScene(w,toQt(n.anchor));check(w.view.nativeSelected()==std::optional<Key>{c.key},"imported curve selection mismatch");
  drag17(w,n.anchor,{13,-7});
  clickButton(w,"rectangle");w.view.centerOn(10,10);QApplication::processEvents();
  auto a=w.view.mapFromScene(QPointF(2,2)),b=w.view.mapFromScene(QPointF(8,9));
  QTest::mousePress(w.view.viewport(),Qt::LeftButton,Qt::NoModifier,a);QTest::mouseMove(w.view.viewport(),b,2);QTest::mouseRelease(w.view.viewport(),Qt::LeftButton,Qt::NoModifier,b);QApplication::processEvents();
  check(w.view.editor.state().doc.contours.size()==origin.legacy.doc.contours.size()+1,"legacy mouse edit missing");
  w.projectPath=output;pressKey(w,Qt::Key_S,Qt::ControlModifier);
  auto saved=readIntegratedProject(output.toStdString());
  check(serialize(encodeCurves(saved.curves))==serialize(encodeCurves(w.curves->committed())),"saved native geometry differs from GUI state");
  check(serialize(encode(saved.legacy.doc))==serialize(encode(w.view.editor.state().doc)),"saved legacy geometry differs from GUI state");
  check(serialize(encodeCurves(saved.curves))!=serialize(encodeCurves(origin.curves)),"native edit lost");
  check(saved.legacy.curveSource==origin.legacy.curveSource,"source lost");
  check(saved.legacy.operations==safeOperations(origin.legacy.operations),"operations lost");
  check(saved.legacy.layers==preserveEditorMetadata(origin.legacy.layers,w.view.editor.state()),"metadata lost");
  QFile expected(output+".expected");check(expected.open(QIODevice::WriteOnly),"cannot write oracle");expected.write(QByteArray::fromStdString(exact17(saved)));expected.close();
  w.grab().save(output+".png");
  // Invalid files must leave the live project untouched through the GUI handler.
  const auto baseline=state17(w);QFile corrupt(output+".corrupt");corrupt.open(QIODevice::WriteOnly);corrupt.write("not a ZIP");corrupt.close();
  w.projectPath=output+".corrupt";QTest::mouseClick(w.buttons.at("open"),Qt::LeftButton);QApplication::processEvents();
  check(!w.lastError.isEmpty()&&state17(w)==baseline,"corrupt file damaged live document");
  auto unknown=saved;unknown.legacy.layers.as_object()["schema"]="unsupported-future-17";writeIntegratedProject((output+".unknown").toStdString(),unknown);
  w.projectPath=output+".unknown";QTest::mouseClick(w.buttons.at("open"),Qt::LeftButton);QApplication::processEvents();
  check(!w.lastError.isEmpty()&&state17(w)==baseline,"unknown schema damaged live document");
 }else{
  auto saved=readIntegratedProject(output.toStdString());QFile expected(output+".expected");check(expected.open(QIODevice::ReadOnly),"missing independent oracle");
  check(expected.readAll().toStdString()==exact17(saved),"restart changed package identity");
  check(serialize(encodeCurves(*w.committedCurves()))==serialize(encodeCurves(saved.curves)),"GUI reopen lost curves");check(stateText(w.view.editor.state())==stateText(editorStateForIntegratedProject(saved)),"GUI reopen lost legacy state");
  for(auto& c:saved.curves.contours)check(w.view.nativeItemPath(c.key)==nativeCurvePath(c),"reopened path differs");
  pressKey(w,Qt::Key_S,Qt::ControlModifier);check(exact17(readIntegratedProject(output.toStdString()))==exact17(saved),"second GUI save changed package");
 }
 w.close();QApplication::processEvents();std::cout<<"PERSISTENCE "<<mode.toStdString()<<" PASS pid="<<QCoreApplication::applicationPid()<<"\n";
}
int main(int argc,char**argv){QApplication app(argc,argv);try{
 check(argc>=3,"usage --interaction output-dir | --write/--verify input output");
 if(QString(argv[1])=="--singleton"){
 Document d;d.nextId=2;d.contours.push_back(Contour{{1,0},{{1,{10,10}}},false});LabWindow w(d);w.show();QApplication::processEvents();checkRendered(w);w.view.setTransform(QTransform::fromScale(6,6));w.view.centerOn(10,10);QApplication::processEvents();clickScene(w,{10,10});check(w.view.active()==std::optional<Key>{{1,0}},"isolated node not selectable");checkRendered(w);std::cout<<"SINGLETON PASS\n";
 }else if(QString(argv[1])=="--interaction"){QDir().mkpath(argv[2]);interaction17(argv[2]);}else{check(argc==4,"missing file args");persistence17(argv[1],argv[2],argv[3]);}return 0;
}catch(const std::exception& e){std::cerr<<"TESTE17 FAIL "<<e.what()<<"\n";return 1;}}
