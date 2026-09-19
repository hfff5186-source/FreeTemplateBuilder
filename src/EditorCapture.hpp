#pragma once
#include <Geode/Geode.hpp>
#include "Template.hpp"

class EditorCapture {
public:
    static TemplateData captureFromEditor(
        EditLevelLayer* editor,
        std::string const& name
    );

    static bool applyToEditor(
        EditLevelLayer* editor,
        TemplateData const& data,
        float originX,
        float originY
    );
};
