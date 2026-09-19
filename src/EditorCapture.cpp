#include "EditorCapture.hpp"

using namespace geode::prelude;

/*
 * This is the adapter layer for GD 2.2081.
 *
 * The important design is:
 * 1. Read the editor's selected GameObjects.
 * 2. Serialize their object IDs and transforms into TemplateData.
 * 3. On Build, clone/create objects and apply the stored transforms relative
 *    to the requested origin.
 *
 * BINDING NOTE: this file assumes the "standard" GD editor shape used by
 * most 2.2 Geode mods:
 *   - EditLevelLayer::m_editorLayer  -> LevelEditorLayer*  (holds m_objects)
 *   - EditLevelLayer::m_editorUI     -> EditorUI*          (holds selection +
 *                                        createObject(...))
 *   - EditorUI::m_selectedObjects    -> CCArray* of currently selected
 *                                        GameObject*
 *   - EditorUI::createObject(int id, CCPoint pos, bool snap = false)
 *                                     -> GameObject* (already added to the
 *                                        editor and registered with undo)
 *
 * If your installed Geode bindings name these differently, only the calls
 * marked "BINDING:" below need to change -- the public API/behavior of
 * EditorCapture stays the same.
 */

TemplateData EditorCapture::captureFromEditor(EditLevelLayer* editor, std::string const& name) {
    TemplateData out;
    out.name = name;
    out.algorithm = "PT";

    if (!editor || !editor->m_editorUI)
        return out;

    CCArray* selected = editor->m_editorUI->m_selectedObjects;
    if (!selected || selected->count() == 0)
        return out;

    float originX = 0.f, originY = 0.f;
    {
        auto* first = static_cast<GameObject*>(selected->objectAtIndex(0));
        if (first) {
            originX = first->getPositionX();
            originY = first->getPositionY();
        }
    }

    CCARRAY_FOREACH_B_TYPE(selected, obj, GameObject) {
        if (!obj) continue;

        TemplateObject t;
        t.id        = obj->m_objectID;
        t.x         = obj->getPositionX() - originX;
        t.y         = obj->getPositionY() - originY;
        t.rotation  = obj->getRotation();
        t.scaleX    = obj->getScaleX();
        t.scaleY    = obj->getScaleY();
        t.zOrder    = obj->getZOrder();

        out.objects.push_back(t);
    }

    return out;
}

bool EditorCapture::applyToEditor(
    EditLevelLayer* editor,
    TemplateData const& data,
    float originX,
    float originY
) {
    if (!editor || !editor->m_editorUI)
        return false;

    EditorUI* ui = editor->m_editorUI;
    bool placedAny = false;

    for (auto const& t : data.objects) {
        if (t.id <= 0)
            continue;

        CCPoint pos = { originX + t.x, originY + t.y };

        GameObject* created = ui->createObject(t.id, pos, false);
        if (!created)
            continue;

        created->setPosition(pos);
        created->setRotation(t.rotation);
        created->setScaleX(t.scaleX);
        created->setScaleY(t.scaleY);
        created->setZOrder(t.zOrder);

        placedAny = true;
    }

    if (placedAny) {
        ui->deselectAll();
    }

    return placedAny;
}
