# src/Mod/ — Workbench Ecosystem

34 pluggable workbenches. Each gated by `BUILD_{NAME}` CMake toggle. Mix of C++ and Python-only modules.

## STRUCTURE

Every workbench follows this pattern (variations noted below):

```
Mod/{Name}/
├── App/            # C++ DocumentObject subclasses (if C++ workbench)
├── Gui/            # C++ ViewProviders + commands (if C++ workbench)
├── Init.py         # Headless init — import/export types, test registration
├── InitGui.py      # GUI init — Workbench class, command/toolbar registration
├── Resources/      # Icons, translations, .ui files
└── CMakeLists.txt
```

## WORKBENCH CLASSIFICATION

### C++ Workbenches (compiled App/ + Gui/)

| Module          | Files | Key Dependency | Domain                    |
| --------------- | ----- | -------------- | ------------------------- |
| **Part**        | 732   | OpenCASCADE    | Geometry kernel wrapper   |
| **TechDraw**    | 1098  | Part           | Technical drawing         |
| **Fem**         | 1026  | Part, Mesh     | Finite element analysis   |
| **Sketcher**    | 573   | Part, Eigen3   | 2D constraint sketching   |
| **PartDesign**  | 438   | Part, Sketcher | Parametric solid modeling |
| **Mesh**        | 517   | —              | Mesh operations           |
| **CAM**         | 877   | Part           | CNC toolpath generation   |
| **Assembly**    | 164   | Part           | Assembly constraints      |
| **Material**    | 492   | —              | Material database         |
| **Spreadsheet** | 143   | —              | Spreadsheet engine        |
| **Robot**       | 303   | Part, Eigen3   | Robot simulation          |
| **Surface**     | 111   | Part           | Surface modeling          |
| **Import**      | 102   | Part           | STEP/IGES import          |

### Python-Only Workbenches (no compiled C++)

| Module            | Files | Key Dependency | Domain                          |
| ----------------- | ----- | -------------- | ------------------------------- |
| **Draft**         | 503   | Part           | 2D drafting (FeaturePython)     |
| **BIM**           | 563   | Draft, Part    | Building information modeling   |
| **AddonManager**  | 248   | —              | Package manager (git submodule) |
| **OpenSCAD**      | 143   | Part           | OpenSCAD interop                |
| **Start**         | 91    | —              | Start page                      |
| **Help**          | 85    | —              | Help browser                    |
| **Show**          | 16    | —              | Scene visualization             |
| **TemplatePyMod** | 15    | —              | Canonical FeaturePython example |

## WHERE TO LOOK

| Task                   | Location                                                       |
| ---------------------- | -------------------------------------------------------------- |
| Add new workbench      | Copy `TemplatePyMod/` pattern, create `Init.py` + `InitGui.py` |
| Add C++ feature        | `Mod/{Name}/App/Feature{Type}.cpp` + TYPESYSTEM macros         |
| Add Python feature     | Use FeaturePython: `addObject("Part::FeaturePython", ...)`     |
| Register import/export | Module's `Init.py` — `FreeCAD.addImportType()`                 |
| Add workbench command  | `InitGui.py` or `Gui/Command*.cpp`                             |
| Toggle module in build | `CMakeLists.txt` — `BUILD_{NAME}` variable                     |

## DEPENDENCY GRAPH

```
Part ←── Sketcher ←── PartDesign
 ↑          ↑
 ├── Draft ←┘
 │    ↑
 │    └── BIM
 ├── Mesh ←── Fem
 ├── TechDraw
 ├── CAM
 └── Assembly
```

**Part is the foundation** — almost every workbench depends on it.

## CONVENTIONS

- `Init.py` runs at ALL startups (headless + GUI) — keep lightweight
- `InitGui.py` runs only in GUI mode — register Workbench class here
- C++ workbenches: `GetClassName()` returns `"{Name}Gui::Workbench"`
- Python workbenches: `GetClassName()` returns `"Gui::PythonWorkbench"`
- AddonManager is a git submodule — `pixi run initialize` to clone
