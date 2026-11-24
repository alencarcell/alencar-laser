#include "integrated_project_edit.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>

using namespace alaser;

static void require(bool ok,const char* message) {
    if(!ok) throw std::runtime_error(message);
}

int main() {
    std::filesystem::path file;
    try {
        EditorState state;
        state.layers.emplace(0,Layer{0,"Padrão",true,false});
        state.layers.emplace(1,Layer{1,"Gravação",true,false});
        Contour polygon;
        polygon.key={1,0};
        polygon.nodes={{1,{0,0}},{2,{5,0}}};
        state.doc.contours.push_back(polygon);
        state.doc.nextId=3;
        state.objects.emplace(1,Meta{0,0});
        validate(state);

        CurveDocument curves;
        CurveContour c;
        c.key={700,0};
        c.nodes={{1,{1,1},{1,1},{1,1}},{2,{9,1},{9,1},{9,1}}};
        c.edgeKind={0};
        curves.contours.push_back(c);
        curves.nextId=3;
        validateCurve(curves);

        CurveLayerBindings bindings;
        bindings.layerByContour.emplace(Key{700,0},1);

        IntegratedProject project=buildEditedIntegratedProjectWithCurveLayers(
            state,curves,bindings,nullptr);
        auto stored=curveLayerBindingsForIntegratedProject(project,state);
        require(stored.layerByContour==bindings.layerByContour,
                "binding was not attached to editor metadata");

        const auto stamp=std::chrono::steady_clock::now().time_since_epoch().count();
        file=std::filesystem::temp_directory_path()/
            ("alencar_curve_layer_roundtrip_"+std::to_string(stamp)+".alaser");
        writeIntegratedProject(file,project);
        IntegratedProject reopened=readIntegratedProject(file);
        EditorState reopenedState=editorStateForIntegratedProject(reopened);
        auto reopenedBindings=curveLayerBindingsForIntegratedProject(reopened,reopenedState);
        require(reopened.version==5,"project version changed");
        require(reopened.curves.contours.size()==1,"Bezier contour count changed");
        require(reopenedBindings.layerByContour==bindings.layerByContour,
                "native curve layer binding changed after package roundtrip");
        require(reopenedState.layers.at(1).name=="Gravação",
                "target layer metadata changed after package roundtrip");

        // Missing binding must be rejected rather than silently assigning layer 0.
        IntegratedProject damaged=reopened;
        damaged.legacy.layers.as_object().erase(kNativeCurveLayerBindingsKey);
        bool missingRejected=false;
        try { (void)curveLayerBindingsForIntegratedProject(damaged,reopenedState); }
        catch(const std::exception&) { missingRejected=true; }
        require(missingRejected,"missing native curve binding was accepted");

        std::filesystem::remove(file);
        std::cout << "PASS: native Bezier layer binding .alaser v5 roundtrip\n";
        std::cout << "PASS: missing binding rejected on reopen\n";
        return 0;
    } catch(const std::exception& e) {
        if(!file.empty()) {
            std::error_code ec;
            std::filesystem::remove(file,ec);
        }
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
