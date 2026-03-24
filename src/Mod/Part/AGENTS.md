# src/Mod/Part/ — OpenCASCADE Geometry Wrapper

Core geometry workbench. 732 files. Wraps OpenCASCADE (OCCT) for parametric solid modeling. Nearly every other workbench depends on Part.

## STRUCTURE

```
Part/
├── App/                    # 274 files — OCCT wrappers, geometry classes
│   ├── Geometry.cpp        # 7748 lines — all OCCT geometry types
│   ├── TopoShape*.cpp      # Shape operations (TopoShapeExpansion = 6147 lines)
│   ├── Feature*.cpp        # Part::Feature base, FeaturePartBoolean, FeatureExtrusion...
│   ├── Attacher.cpp        # Attachment engine (3130 lines) — used by PartDesign, Sketcher
│   ├── *PyImp.cpp          # Python bindings (TopoShapePyImp = 2990 lines)
│   ├── *.pyi               # Python type stubs
│   ├── BRepFeat/           # Boolean/feature operations
│   └── BodyBase.cpp        # Base class for PartDesign bodies
├── Gui/                    # 186 files — ViewProviders, commands, dialogs
│   ├── Command*.cpp        # 4 command files (basic, parametric, filter, simple)
│   ├── ViewProvider*.cpp   # Part shape visualization
│   ├── Dlg*.cpp + .ui      # Export/import dialogs, fillet, extrusion
│   └── TaskDialogs/        # Task panel widgets
├── Init.py                 # Registers STEP, IGES, BREP import/export
├── InitGui.py              # PartGui::Workbench (C++ class)
├── BOPTools/               # Python Boolean operations tools
├── CompoundTools/          # Compound manipulation utilities
└── Resources/              # Icons, translations
```

## WHERE TO LOOK

| Task                     | Location                                                           |
| ------------------------ | ------------------------------------------------------------------ |
| Add geometry type        | `App/Geometry.cpp` + corresponding `*Py.xml` + `*PyImp.cpp`        |
| Add shape operation      | `App/TopoShape*.cpp` or `App/modelRefine.cpp`                      |
| Add Part command         | `Gui/Command*.cpp` — pick by category                              |
| Modify attachment        | `App/Attacher.cpp` (3130 lines) — shared with Sketcher, PartDesign |
| Add import/export format | `Init.py` — `FreeCAD.addImportType()`                              |
| Modify shape display     | `Gui/ViewProviderExt.cpp`                                          |

## CONVENTIONS

- `Part::Feature` is the base class for all shape-holding objects (has `Shape` property)
- `Part::TopoShape` wraps `TopoDS_Shape` from OCCT — never use OCCT types directly in headers
- Python shape API: `Part.makeBox()`, `Part.makeCylinder()`, `Part.show(shape)`
- Element mapping: `TopoShapeExpansion.cpp` handles topological naming (TNP)

## COMPLEXITY HOTSPOTS

| File                         | Lines | Why                        |
| ---------------------------- | ----- | -------------------------- |
| `App/Geometry.cpp`           | 7748  | All OCCT geometry wrappers |
| `App/TopoShapeExpansion.cpp` | 6147  | Element mapping / TNP      |
| `App/TopoShape.cpp`          | 4583  | Core shape operations      |
| `App/Attacher.cpp`           | 3130  | Attachment engine          |
| `App/TopoShapePyImp.cpp`     | 2990  | Python bindings for shapes |
| `App/WireJoiner.cpp`         | 3214  | Wire joining algorithms    |
