# Free Template Builder v0.2

Standalone implementation for Geode Android / Geometry Dash 2.2081.

## Workflow

Editor:
1. Open Template Builder (`TPL`).
2. Save -> serializes selected editor objects to `my_template.tblib`.
3. Build -> loads the template and sends it to the editor creation adapter.

## Important

The UI and `.tblib` serialization are implemented. The two binding-specific functions in
`src/EditorCapture.cpp` are intentionally isolated because the exact selected-object and
object-creation APIs must match the installed Geode 2.2081 bindings.

Do not copy the paid mod's DLL or private template data into this project.
