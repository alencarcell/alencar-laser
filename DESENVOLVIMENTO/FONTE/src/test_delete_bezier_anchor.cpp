// Testes automatizados — exclusão estrutural de âncoras Bézier.
// Sem Qt, sem GRBL, sem fabricação. GCC/Clang + opcional UBSan.
#include "bezier_interaction.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>

using namespace alaser;

static int g_failed = 0;
static int g_passed = 0;

#define EXPECT(cond, msg) do { \
  if(!(cond)){ std::cerr << "FAIL: " << msg << "\n"; ++g_failed; } \
  else { ++g_passed; } \
} while(0)

static bool bitsEq(Pt a, Pt b){
  return std::memcmp(&a.x,&b.x,sizeof(double))==0 && std::memcmp(&a.y,&b.y,sizeof(double))==0;
}

static CurveDocument makeOpenCubic4(){
  // Contorno aberto com 4 nós e 3 arestas cúbicas.
  CurveDocument d;
  d.nextId = 10;
  CurveContour c;
  c.key = {100, 0};
  c.closed = false;
  c.nodes = {
    {1, {0,0}, {0,0}, {10,20}},
    {2, {40,0}, {20,20}, {60,20}},
    {3, {80,0}, {60,-20}, {100,-20}},
    {4, {120,0}, {100,20}, {120,0}},
  };
  c.edgeKind = {1, 1, 1};
  d.contours.push_back(c);
  validateCurve(d);
  return d;
}

static CurveDocument makeClosedCubic4(){
  CurveDocument d;
  d.nextId = 20;
  CurveContour c;
  c.key = {200, 0};
  c.closed = true;
  c.nodes = {
    {11, {0,0}, {-5,5}, {5,5}},
    {12, {50,0}, {45,5}, {55,5}},
    {13, {50,50}, {55,45}, {45,45}},
    {14, {0,50}, {5,45}, {-5,45}},
  };
  c.edgeKind = {1, 1, 1, 1};
  d.contours.push_back(c);
  validateCurve(d);
  return d;
}

static CurveDocument makeOpenLine3(){
  CurveDocument d;
  d.nextId = 5;
  CurveContour c;
  c.key = {300, 0};
  c.closed = false;
  c.nodes = {
    {1, {0,0}, {0,0}, {0,0}},
    {2, {10,0}, {10,0}, {10,0}},
    {3, {20,0}, {20,0}, {20,0}},
  };
  c.edgeKind = {0, 0};
  d.contours.push_back(c);
  validateCurve(d);
  return d;
}

static CurveDocument makeMultiContour(){
  // Dois contornos: alvo e vizinho sobreposto (mesmas coordenadas, key distinta).
  CurveDocument d = makeOpenCubic4();
  CurveContour neighbor = d.contours[0];
  neighbor.key = {100, 1};
  for(auto& n : neighbor.nodes){ n.id = d.nextId++; }
  d.contours.push_back(neighbor);
  validateCurve(d);
  return d;
}

static std::string fingerprint(const CurveDocument& d){
  std::ostringstream os;
  os << d.nextId << "|";
  for(const auto& c : d.contours){
    os << c.key.object << "." << c.key.sub << ":" << (c.closed?'C':'O') << ":";
    for(uint8_t e : c.edgeKind) os << int(e);
    os << ":";
    for(const auto& n : c.nodes)
      os << n.id << "@" << n.anchor.x << "," << n.anchor.y
         << "/" << n.handleIn.x << "," << n.handleIn.y
         << "/" << n.handleOut.x << "," << n.handleOut.y << ";";
    os << "#";
  }
  return os.str();
}

// --- casos ---

static void test_open_internal(){
  std::cout << "[test] open internal delete\n";
  auto d = makeOpenCubic4();
  // remove nó 2 (índice 1)
  removeCurveNode(d, {100,0}, 2);
  validateCurve(d);
  EXPECT(d.contours.size()==1, "one contour");
  EXPECT(d.contours[0].nodes.size()==3, "3 nodes remain");
  EXPECT(d.contours[0].edgeKind.size()==2, "2 edges for open 3-node");
  EXPECT(d.contours[0].nodes[0].id==1 && d.contours[0].nodes[1].id==3 && d.contours[0].nodes[2].id==4,
         "remaining IDs stable 1,3,4");
  // ponte sobre o removido é reta; aresta original 2->3 (agora entre nós 3 e 4) preserva cúbica
  EXPECT(d.contours[0].edgeKind[0]==0, "bridge over deleted is line");
  EXPECT(d.contours[0].edgeKind[1]==1, "unaffected trailing cubic preserved");
  // handles dos restantes intactos
  EXPECT(bitsEq(d.contours[0].nodes[0].handleOut, {10,20}), "node1 handleOut preserved");
  EXPECT(bitsEq(d.contours[0].nodes[1].handleIn, {60,-20}), "node3 handleIn preserved");
}

static void test_open_endpoints(){
  std::cout << "[test] open endpoint deletes\n";
  {
    auto d = makeOpenCubic4();
    removeCurveNode(d, {100,0}, 1); // first
    validateCurve(d);
    EXPECT(d.contours[0].nodes.size()==3, "after delete first: 3 nodes");
    EXPECT(d.contours[0].edgeKind.size()==2, "2 edges");
    EXPECT(d.contours[0].nodes[0].id==2, "new first is old second");
    EXPECT(d.contours[0].edgeKind[0]==1 && d.contours[0].edgeKind[1]==1, "remaining cubics kept");
  }
  {
    auto d = makeOpenCubic4();
    removeCurveNode(d, {100,0}, 4); // last
    validateCurve(d);
    EXPECT(d.contours[0].nodes.size()==3, "after delete last: 3 nodes");
    EXPECT(d.contours[0].edgeKind.size()==2, "2 edges");
    EXPECT(d.contours[0].nodes.back().id==3, "new last is old third");
    EXPECT(d.contours[0].edgeKind[0]==1 && d.contours[0].edgeKind[1]==1, "leading cubics kept");
  }
}

static void test_closed_delete(){
  std::cout << "[test] closed delete\n";
  auto d = makeClosedCubic4();
  removeCurveNode(d, {200,0}, 12);
  validateCurve(d);
  EXPECT(d.contours[0].closed, "still closed");
  EXPECT(d.contours[0].nodes.size()==3, "3 nodes");
  EXPECT(d.contours[0].edgeKind.size()==3, "3 edges");
  // IDs restantes
  std::vector<int64_t> ids;
  for(auto& n:d.contours[0].nodes) ids.push_back(n.id);
  EXPECT(ids.size()==3 && ids[0]==11 && ids[1]==13 && ids[2]==14, "IDs 11,13,14");
  // exatamente uma aresta nova (reta) — a que faz ponte sobre o removido
  int lines=0, cubics=0;
  for(auto e:d.contours[0].edgeKind){ if(e==0)++lines; else ++cubics; }
  EXPECT(lines==1 && cubics==2, "one bridge line, two preserved cubics");
}

static void test_min_topology(){
  std::cout << "[test] minimum topology guards\n";
  {
    auto d = makeOpenLine3();
    // open with 3 nodes: can delete one → 2 nodes OK
    removeCurveNode(d, {300,0}, 2);
    validateCurve(d);
    EXPECT(d.contours[0].nodes.size()==2, "open reduced to 2");
    EXPECT(d.contours[0].edgeKind.size()==1, "1 edge");
    // cannot delete further
    bool threw=false;
    try{ removeCurveNode(d, {300,0}, 1); }catch(...){ threw=true; }
    EXPECT(threw, "open 2-node refuses further delete");
    validateCurve(d); // document intact
  }
  {
    auto d = makeClosedCubic4();
    removeCurveNode(d,{200,0},12); // 4 → 3
    bool threw=false;
    try{ removeCurveNode(d,{200,0},13); }catch(...){ threw=true; }
    EXPECT(threw, "closed 3-node refuses further delete");
    validateCurve(d);
    EXPECT(d.contours[0].nodes.size()==3, "still 3 after refused delete");
  }
}

static void test_successive(){
  std::cout << "[test] successive deletes\n";
  auto d = makeOpenCubic4();
  removeCurveNode(d,{100,0},2);
  validateCurve(d);
  removeCurveNode(d,{100,0},3);
  validateCurve(d);
  EXPECT(d.contours[0].nodes.size()==2, "2 nodes after two deletes");
  EXPECT(d.contours[0].nodes[0].id==1 && d.contours[0].nodes[1].id==4, "ends remain");
  EXPECT(d.contours[0].edgeKind.size()==1, "single edge");
  EXPECT(d.contours[0].edgeKind[0]==0, "bridge is line");
}

static void test_invalid_no_corrupt(){
  std::cout << "[test] invalid ops do not corrupt\n";
  auto d = makeOpenCubic4();
  auto before = fingerprint(d);
  bool threw=false;
  try{ removeCurveNode(d,{100,0},999); }catch(...){ threw=true; }
  EXPECT(threw, "missing node throws");
  EXPECT(fingerprint(d)==before, "document unchanged after missing node");
  threw=false;
  try{ removeCurveNode(d,{999,0},1); }catch(...){ threw=true; }
  EXPECT(threw, "missing contour throws");
  EXPECT(fingerprint(d)==before, "document unchanged after missing contour");
}

static void test_undo_redo_session(){
  std::cout << "[test] undo/redo via CurveDragSession\n";
  auto d = makeOpenCubic4();
  auto original = fingerprint(d);
  CurveDragSession session(d);
  session.select({100,0});
  EXPECT(session.deleteAnchor(2), "delete internal");
  validateCurve(session.committed());
  EXPECT(session.committed().contours[0].nodes.size()==3, "3 after delete");
  EXPECT(session.undoCount()==1, "one undo entry");
  EXPECT(session.undo(), "undo");
  EXPECT(fingerprint(session.committed())==original, "undo restores exact geometry");
  EXPECT(session.redo(), "redo");
  EXPECT(session.committed().contours[0].nodes.size()==3, "redo re-applies delete");
  // multiple
  session.select({100,0});
  session.deleteAnchor(3);
  EXPECT(session.committed().contours[0].nodes.size()==2, "2 after second delete");
  session.undo();
  session.undo();
  EXPECT(fingerprint(session.committed())==original, "double undo restores original");
}

static void test_neighbors_preserved(){
  std::cout << "[test] neighbor contours preserved\n";
  auto d = makeMultiContour();
  auto neighborBefore = fingerprint(CurveDocument{std::vector<CurveContour>{d.contours[1]}, d.nextId});
  // delete from first contour only
  removeCurveNode(d,{100,0},2);
  validateCurve(d);
  EXPECT(d.contours.size()==2, "both contours remain");
  EXPECT(d.contours[0].nodes.size()==3, "target reduced");
  EXPECT(d.contours[1].nodes.size()==4, "neighbor node count intact");
  EXPECT(d.contours[1].key.object==100 && d.contours[1].key.sub==1, "neighbor key intact");
  // all neighbor node ids and geometry
  for(size_t i=0;i<4;i++){
    EXPECT(d.contours[1].nodes[i].id != 0, "neighbor ids present");
  }
  EXPECT(d.contours[1].edgeKind.size()==3, "neighbor edges intact");
  (void)neighborBefore;
}

static void test_session_refuses_min(){
  std::cout << "[test] session refuses min topology without corruption\n";
  auto d = makeOpenLine3();
  CurveDragSession session(d);
  session.select({300,0});
  session.deleteAnchor(2);
  auto mid = fingerprint(session.committed());
  bool threw=false;
  try{ session.deleteAnchor(1); }catch(...){ threw=true; }
  EXPECT(threw, "session throws on min topology");
  EXPECT(fingerprint(session.committed())==mid, "no corruption on refused delete");
}

int main(){
  std::cout << "=== test_delete_bezier_anchor ===\n";
  test_open_internal();
  test_open_endpoints();
  test_closed_delete();
  test_min_topology();
  test_successive();
  test_invalid_no_corrupt();
  test_undo_redo_session();
  test_neighbors_preserved();
  test_session_refuses_min();
  std::cout << "passed=" << g_passed << " failed=" << g_failed << "\n";
  return g_failed ? 1 : 0;
}
