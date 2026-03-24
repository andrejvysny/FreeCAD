# src/Mod/Sketcher/ — 2D Constraint Sketching

Parametric 2D sketch editor with geometric constraint solver. 573 files. Requires Eigen3. Core dependency of PartDesign.

## STRUCTURE

```
Sketcher/
├── App/                        # 48 files — constraint solver, sketch object
│   ├── Sketch.cpp              # 5712 lines — main Sketch class, solver interface
│   ├── SketchObject*.cpp       # SketchObject (feature), operations, external geometry
│   ├── Constraint.cpp/h        # Constraint data model
│   ├── planegcs/               # Geometric Constraint Solver (GCS)
│   │   ├── GCS.cpp             # 5803 lines — THE solver core
│   │   └── Constraints.cpp     # 3218 lines — constraint equations
│   ├── GeometryFacade.cpp      # Facade over Part::Geometry with sketch metadata
│   └── PropertyConstraintList.cpp  # Custom property for constraint storage
├── Gui/                        # 141 files — interactive sketcher UI
│   ├── CommandConstraints.cpp  # 11040 lines — ALL constraint commands (biggest file)
│   ├── CommandCreateGeo.cpp    # Geometry creation commands
│   ├── ViewProviderSketch.cpp  # 5081 lines — interactive sketch editing
│   ├── DrawSketchHandler*.h    # 20+ handler files — one per drawing tool
│   ├── EditMode*Manager.cpp    # Coin3D scene management during edit
│   └── Resources/icons/geometry/  # 88 geometry tool icons
├── Init.py
└── InitGui.py                  # SketcherGui::Workbench (C++)
```

## WHERE TO LOOK

| Task                        | Location                                                      |
| --------------------------- | ------------------------------------------------------------- |
| Add constraint type         | `App/planegcs/Constraints.cpp` + `Gui/CommandConstraints.cpp` |
| Add geometry tool           | `Gui/DrawSketchHandler{Tool}.h` + `Gui/CommandCreateGeo.cpp`  |
| Modify constraint solver    | `App/planegcs/GCS.cpp` — extremely complex                    |
| Add external geometry       | `App/SketchObjectExternal.cpp` (2956 lines)                   |
| Modify sketch visualization | `Gui/ViewProviderSketch.cpp` + `EditMode*Manager.cpp`         |

## CONVENTIONS

- `DrawSketchHandler` pattern: one header per drawing tool, inherits `DrawSketchDefaultHandler`
- Geometry IDs: positive = user geometry, negative = external, -1/-2 = axes, -3 = origin
- `GeoEnum` defines special geometry indices
- Constraint solver uses Levenberg-Marquardt + DogLeg algorithms

## ANTI-PATTERNS

- Never modify `planegcs/GCS.cpp` without running `Sketcher_tests_run` — solver is fragile
- `CommandConstraints.cpp` at 11040 lines is the largest file in FreeCAD — avoid adding more to it
