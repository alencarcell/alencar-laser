// Regressão reproduzível do codec .alaser v5 (curvas nativas).
// NÃO altera o esquema. Requer Boost.JSON + minizip (mesmo stack do laboratório).
//
// Compilação (ambiente original):
//   g++ -std=c++20 -O2 -I src -I$BOOST_INC -I$MINIZIP_INC \
//       src/test_alaser_v5_codec_regression.cpp src/boost_json_impl.cpp \
//       -lminizip -lz -o test_alaser_v5_codec_regression
//
// Ou via CMake target alencar_test_v5_codec_regression quando disponível.
//
// Este teste:
//  1) monta um CurveDocument sintético
//  2) grava IntegratedProject v5
//  3) relê e confere identidade de curvas + schemaVersion
//  4) remove âncora interna, regrava, relê e confere topologia
//  5) não toca em fabricação / GRBL
#include "bezier_package.hpp"
#include "bezier_interaction.hpp"
#include <filesystem>
#include <iostream>
#include <cstdlib>

using namespace alaser;
namespace fs = std::filesystem;

static int failed = 0;
#define CHECK(c,m) do{ if(!(c)){ std::cerr<<"FAIL: "<<m<<"\n"; ++failed; } }while(0)

static CurveDocument sampleCurves(){
  CurveDocument d;
  d.nextId = 10;
  CurveContour c;
  c.key = {42, 0};
  c.closed = false;
  c.nodes = {
    {1, {0,0}, {0,0}, {5,5}},
    {2, {20,0}, {15,5}, {25,5}},
    {3, {40,0}, {35,-5}, {45,-5}},
    {4, {60,0}, {55,0}, {60,0}},
  };
  c.edgeKind = {1, 1, 1};
  d.contours.push_back(c);
  validateCurve(d);
  return d;
}

static Document emptyPoly(){
  Document doc;
  doc.nextId = 1;
  return doc;
}

int main(int argc, char** argv){
  const fs::path out = argc > 1 ? fs::path(argv[1]) : fs::temp_directory_path() / "alencar_v5_codec_regression.alaser";
  std::cout << "=== test_alaser_v5_codec_regression ===\n";
  std::cout << "output: " << out << "\n";

  IntegratedProject project;
  project.version = 5;
  project.legacy = Package{emptyPoly(), boost::json::object{}, boost::json::object{{"fabricationEnabled", false}}, ""};
  project.curves = sampleCurves();

  writeIntegratedProject(out, project);
  CHECK(fs::exists(out), "v5 file written");

  auto round1 = readIntegratedProject(out);
  CHECK(round1.version == 5, "schemaVersion == 5 after read");
  CHECK(round1.curves.contours.size() == 1, "one curve contour");
  CHECK(round1.curves.contours[0].nodes.size() == 4, "4 nodes round-trip");
  CHECK(round1.curves.contours[0].edgeKind.size() == 3, "3 edges round-trip");
  CHECK(round1.curves.contours[0].nodes[1].id == 2, "node id 2 stable");
  CHECK(round1.curves.nextId == 10, "nextId stable");

  // Exclusão interna + regravação
  removeCurveNode(round1.curves, {42, 0}, 2);
  validateCurve(round1.curves);
  CHECK(round1.curves.contours[0].nodes.size() == 3, "3 nodes after delete");
  CHECK(round1.curves.contours[0].edgeKind.size() == 2, "2 edges after delete");
  CHECK(round1.curves.contours[0].edgeKind[0] == 0, "bridge is line");

  writeIntegratedProject(out, round1);
  auto round2 = readIntegratedProject(out);
  CHECK(round2.version == 5, "still v5 after rewrite");
  CHECK(round2.curves.contours[0].nodes.size() == 3, "deleted node persisted");
  CHECK(round2.curves.contours[0].nodes[0].id == 1, "id 1 remains");
  CHECK(round2.curves.contours[0].nodes[1].id == 3, "id 3 remains");
  CHECK(round2.curves.contours[0].nodes[2].id == 4, "id 4 remains");
  validateCurve(round2.curves);

  // Leitura de arquivo inexistente deve falhar sem crash de processo (exceção)
  bool threw = false;
  try { readIntegratedProject(out.string() + ".does_not_exist"); }
  catch (...) { threw = true; }
  CHECK(threw, "missing file throws");

  std::cout << (failed ? "FAILED" : "PASSED") << " failed=" << failed << "\n";
  return failed ? 1 : 0;
}
