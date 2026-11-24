#include "curve_layer_bindings.hpp"
#include <iostream>
#include <stdexcept>

using namespace alaser;

static void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

static CurveContour makeContour(int object, int sub, int64_t firstId) {
    CurveContour c;
    c.key={object,sub};
    c.nodes={{firstId,{0,0},{0,0},{0,0}},
             {firstId+1,{10,0},{10,0},{10,0}}};
    c.edgeKind={0};
    return c;
}

int main() {
    try {
        EditorState editor;
        editor.layers.emplace(0,Layer{0,"Padrão",true,false});
        editor.layers.emplace(1,Layer{1,"Corte",true,false});
        Contour polygon;
        polygon.key={1,0};
        polygon.nodes={{1,{0,0}},{2,{5,0}}};
        editor.doc.contours.push_back(polygon);
        editor.doc.nextId=3;
        editor.objects.emplace(1,Meta{0,0});
        validate(editor);

        CurveDocument curves;
        curves.contours={makeContour(700,0,1),makeContour(700,1,3)};
        curves.nextId=5;
        validateCurve(curves);

        CurveLayerBindings bindings;
        bindings.layerByContour.emplace(Key{700,0},0);
        bindings.layerByContour.emplace(Key{700,1},1);
        validateCurveLayerBindings(curves,editor,bindings);

        const auto encoded=encodeCurveLayerBindings(bindings);
        const auto decoded=decodeCurveLayerBindings(encoded,curves,editor);
        require(decoded.layerByContour==bindings.layerByContour,
                "roundtrip changed native curve layer bindings");
        require(decoded.layerByContour.at(Key{700,0})==0,
                "first contour layer mismatch");
        require(decoded.layerByContour.at(Key{700,1})==1,
                "second contour layer mismatch");

        bool missingRejected=false;
        try {
            CurveLayerBindings missing;
            missing.layerByContour.emplace(Key{700,0},0);
            validateCurveLayerBindings(curves,editor,missing);
        } catch(const std::exception&) { missingRejected=true; }
        require(missingRejected,"missing binding was accepted");

        bool unknownLayerRejected=false;
        try {
            CurveLayerBindings invalid;
            invalid.layerByContour.emplace(Key{700,0},99);
            invalid.layerByContour.emplace(Key{700,1},1);
            validateCurveLayerBindings(curves,editor,invalid);
        } catch(const std::exception&) { unknownLayerRejected=true; }
        require(unknownLayerRejected,"unknown layer was accepted");

        bool orphanRejected=false;
        try {
            CurveLayerBindings invalid;
            invalid.layerByContour.emplace(Key{700,0},0);
            invalid.layerByContour.emplace(Key{700,1},1);
            invalid.layerByContour.emplace(Key{999,0},0);
            validateCurveLayerBindings(curves,editor,invalid);
        } catch(const std::exception&) { orphanRejected=true; }
        require(orphanRejected,"orphan binding was accepted");

        boost::json::array duplicateRows{
            boost::json::object{{"objectId",700},{"subpathId",0},{"layerId",0}},
            boost::json::object{{"objectId",700},{"subpathId",0},{"layerId",1}},
            boost::json::object{{"objectId",700},{"subpathId",1},{"layerId",1}}
        };
        bool duplicateRejected=false;
        try { (void)decodeCurveLayerBindings(duplicateRows,curves,editor); }
        catch(const std::exception&) { duplicateRejected=true; }
        require(duplicateRejected,"duplicate JSON binding was accepted");

        std::cout << "PASS: native Bezier layer binding contract\n";
        std::cout << "PASS: JSON roundtrip and negative validation cases\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
