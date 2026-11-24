#include "editor_tools.hpp"
#include "bezier_native.hpp"
#include <iostream>
#include <stdexcept>

using namespace alaser;

static void require(bool ok,const char* message){
    if(!ok) throw std::runtime_error(message);
}

int main(){
    try {
        // Main's conventional editor state: one polygonal object on layer 0.
        EditorState state;
        state.layers.emplace(0,Layer{0,"Padrão",true,false});
        Contour polygon;
        polygon.key={1,0};
        polygon.nodes={{1,{0,0}},{2,{10,0}}};
        state.doc.contours.push_back(polygon);
        state.doc.nextId=3;
        state.objects.emplace(1,Meta{0,0});
        validate(state);

        // A native Bezier contour is stored in a separate CurveDocument.
        CurveDocument curves;
        CurveContour native;
        native.key={2,0};
        native.nodes={{1,{0,0},{0,0},{0,0}},{2,{10,0},{10,0},{10,0}}};
        native.edgeKind={0};
        curves.contours.push_back(native);
        curves.nextId=3;
        validateCurve(curves);

        // Hypothesis under review: register a Bezier-only object in EditorState.objects
        // so its layer can be resolved through Meta.layer.
        state.objects.emplace(2,Meta{0,0});
        bool rejected=false;
        try { validate(state); }
        catch(const std::exception&) { rejected=true; }
        require(rejected,
            "expected main EditorState validation to reject metadata without polygon geometry");

        // A matching object ID is technically accepted, but it aliases the polygon's
        // metadata rather than representing an independently owned native object.
        state.objects.erase(2);
        native.key={1,0};
        curves.contours={native};
        validateCurve(curves);
        validate(state);
        require(state.objects.at(1).layer==0,
            "shared numeric object ID must resolve only to existing polygon metadata");

        std::cout << "PASS: main layer metadata contract characterized\n";
        std::cout << "LIMIT: Bezier-only object metadata cannot be registered in EditorState\n";
        std::cout << "LIMIT: reusing a polygon object ID aliases its metadata; it is not an independent binding\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
