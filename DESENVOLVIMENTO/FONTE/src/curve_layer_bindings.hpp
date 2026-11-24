#pragma once
// Binding metadata for native Bezier contours. Kept separate from polygon object IDs.
#include "bezier_native.hpp"
#include "editor_tools.hpp"
#include <boost/json.hpp>
#include <map>
#include <set>
#include <stdexcept>

namespace alaser {

struct CurveLayerBindings {
    std::map<Key,int> layerByContour;
};

inline void validateCurveLayerBindings(
    const CurveDocument& curves,
    const EditorState& editor,
    const CurveLayerBindings& bindings)
{
    validateCurve(curves);
    validate(editor);
    std::set<Key> curveKeys;
    for (const auto& contour : curves.contours) {
        curveKeys.insert(contour.key);
        auto binding = bindings.layerByContour.find(contour.key);
        if (binding == bindings.layerByContour.end())
            throw std::runtime_error("native curve contour has no layer binding");
        auto layer = editor.layers.find(binding->second);
        if (layer == editor.layers.end())
            throw std::runtime_error("native curve binding references unknown layer");
    }
    if (bindings.layerByContour.size() != curveKeys.size())
        throw std::runtime_error("orphan or duplicate native curve layer binding");
    for (const auto& [key, layer] : bindings.layerByContour) {
        (void)layer;
        if (!curveKeys.count(key))
            throw std::runtime_error("native curve layer binding references missing contour");
    }
}

inline boost::json::array encodeCurveLayerBindings(const CurveLayerBindings& bindings) {
    boost::json::array rows;
    for (const auto& [key, layer] : bindings.layerByContour) {
        rows.push_back(boost::json::object{
            {"objectId", key.object},
            {"subpathId", key.sub},
            {"layerId", layer}
        });
    }
    return rows;
}

inline CurveLayerBindings decodeCurveLayerBindings(
    const boost::json::value& value,
    const CurveDocument& curves,
    const EditorState& editor)
{
    if (!value.is_array())
        throw std::runtime_error("native curve layer bindings must be an array");
    CurveLayerBindings result;
    for (const auto& rowValue : value.as_array()) {
        if (!rowValue.is_object())
            throw std::runtime_error("native curve layer binding must be an object");
        const auto& row = rowValue.as_object();
        Key key{
            static_cast<int>(row.at("objectId").as_int64()),
            static_cast<int>(row.at("subpathId").as_int64())
        };
        const int layer = static_cast<int>(row.at("layerId").as_int64());
        if (!result.layerByContour.emplace(key, layer).second)
            throw std::runtime_error("duplicate native curve layer binding");
    }
    validateCurveLayerBindings(curves, editor, result);
    return result;
}

} // namespace alaser
