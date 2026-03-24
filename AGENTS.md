# PROJECT KNOWLEDGE BASE

**Generated:** 2026-03-23
**Commit:** 5a1b6600e7
**Branch:** main

## OVERVIEW

FreeCAD is an open-source parametric 3D modeler (2.4M lines, C++/Python). Three-layer architecture: Base (math/types) → App (headless document model) → Gui (Qt6/Coin3D visualization) → 34 pluggable workbenches in `src/Mod/`. OpenCASCADE geometry kernel, Python scripting API, LGPL-2.1-or-later.

## STRUCTURE

```
FreeCAD/
├── src/
│   ├── Base/           # Type system, math, persistence, console (NO Qt)
│   ├── App/            # Document model, properties, extensions (NO Qt)
│   ├── Gui/            # ViewProviders, Coin3D scene, Qt UI, selection
│   ├── Mod/            # 34 workbenches (Part, Sketcher, Fem, Draft, BIM...)
│   ├── Main/           # Entry points: MainGui.cpp, MainCmd.cpp, MainPy.cpp
│   ├── 3rdParty/       # Vendored: OndselSolver, salomesmesh, PyCXX, zipios++
│   └── Ext/freecad/    # Python utility shims (part.py, sketcher.py, utils.py)
├── tests/              # C++ (gtest) + Python (unittest) — mirrors src/ layout
├── tools/              # build/, lint/, profile/ scripts
├── cMake/              # CMake modules and find scripts
├── package/            # Packaging: rattler-build, conda
└── data/               # Desktop files, MIME types, AppStream metadata
```

## WHERE TO LOOK

| Task                       | Location                                      | Notes                                        |
| -------------------------- | --------------------------------------------- | -------------------------------------------- |
| Add workbench feature      | `src/Mod/{Name}/App/` (C++) or Python files   | C++ objects need TYPESYSTEM macros           |
| Add GUI command            | `src/Mod/{Name}/Gui/Command*.cpp` or Python   | Register via `addCommand()`                  |
| Add Python-only object     | Use FeaturePython pattern                     | See `src/Mod/TemplatePyMod/FeaturePython.py` |
| Add ViewProvider           | `src/Mod/{Name}/Gui/ViewProvider*.cpp`        | Paired with DocumentObject                   |
| Add property type          | `src/App/Property*.h`                         | 100+ existing types                          |
| Modify core document model | `src/App/Document*.cpp`                       | Careful — affects everything                 |
| Add/modify UI dialogs      | `src/Gui/Dialogs/Dlg*.cpp` + `.ui`            | Qt Designer files                            |
| Add preference page        | `src/Gui/PreferencePages/` or module's `Gui/` | Register in InitGui.py                       |
| Add file import/export     | Module's `Init.py`                            | `FreeCAD.addImportType()`                    |
| Python bindings            | `ClassPy.xml` + `ClassPyImp.cpp`              | Auto-generates `ClassPy.cpp/.h`              |
| Translations               | `*/Resources/translations/`                   | Use `QT_TRANSLATE_NOOP` for static strings   |
| Tests (C++)                | `tests/src/Mod/{Name}/App/`                   | gtest, `{Name}_tests_run` executable         |
| Tests (Python)             | `src/Mod/Test/`                               | unittest, run via `FreeCADCmd -t 0`          |

## ARCHITECTURE

### Layer Dependencies (strict)

```
Base  ←──  App  ←──  Gui  ←──  Mod/*
 │          │         │
 │          │         └─ Qt6, Coin3D, Quarter
 │          └─ OpenCASCADE (via Part)
 └─ Boost, Python, XML (no Qt!)
```

**App has ZERO Qt dependency.** This enables headless/CLI operation via `FreeCADCmd`.

### Core Class Hierarchy

```
Base::BaseClass (TYPESYSTEM_HEADER)
  └─ Base::Persistence
       └─ App::PropertyContainer (PROPERTY_HEADER)
            ├─ App::TransactionalObject
            │    ├─ App::DocumentObject        ←── all features inherit this
            │    │    └─ App::GeoFeature
            │    │         └─ Part::Feature     ←── has Shape property
            │    └─ Gui::ViewProvider           ←── 3D/tree representation
            └─ App::Document
```

### Key Patterns

**DocumentObject ↔ ViewProvider pairing**: Every `App::DocumentObject` has a paired `Gui::ViewProvider`. The App object owns data/logic, the ViewProvider owns visualization. Connected via `getViewProviderName()`.

**Property system**: 100+ types (`App::PropertyLength`, `App::PropertyLink`, `App::PropertyEnumeration`...). Type-safe with serialization, change notifications, expression support. Dynamic addition via `addProperty()`.

**Extension system**: Composition over inheritance. `GroupExtension`, `GeoFeatureGroupExtension`, `LinkBaseExtension`, `SuppressibleExtension`. Attached to `ExtensionContainer` (= DocumentObject).

**Type system macros**:

- `TYPESYSTEM_HEADER()` in `.h` — declares `getClassTypeId()`, `getTypeId()`, `init()`
- `TYPESYSTEM_SOURCE(Class, Parent)` in `.cpp` — implements type registration
- `PROPERTY_HEADER(Class)` — TYPESYSTEM + property data methods
- `EXTENSION_TYPESYSTEM_HEADER()` / `EXTENSION_PROPERTY_HEADER()` — for extensions

**Python binding pipeline**: `ClassPy.xml` (interface description) + `ClassPyImp.cpp` (implementation) → CMake `generate_from_xml` → auto-generates `ClassPy.cpp`/`ClassPy.h`. `.pyi` stubs in `src/App/`, `src/Gui/`.

### Workbench Types

| Type             | GetClassName             | Example                  | How objects work                   |
| ---------------- | ------------------------ | ------------------------ | ---------------------------------- |
| C++ workbench    | `"PartGui::Workbench"`   | Part, Sketcher, Fem      | Compiled DocumentObject subclasses |
| Python workbench | `"Gui::PythonWorkbench"` | Draft, BIM, AddonManager | FeaturePython proxy pattern        |

### Bootstrap Sequence

1. `main()` in `src/Main/MainGui.cpp` (or `MainCmd.cpp` for CLI)
2. `App::Application::init()` — type registration, config, Python interpreter
3. `src/App/FreeCADInit.py` — scans `Mod/` directories, runs each `Init.py`
4. `Gui::Application::initApplication()` — ViewProvider types, Qt resources
5. `src/Gui/FreeCADGuiInit.py` — loads `InitGui.py` per module, registers workbenches

## CONVENTIONS

- **Commit messages**: `Module: Brief description` — `Gui:`, `Part:`, `Sketcher:`, `TD:`, `Fem:`, `Core:`, `Material:`, `Start:`
- **SPDX header required**: `// SPDX-License-Identifier: LGPL-2.1-or-later` in every new file
- **PreCompiled.h**: Must be first include in every `.cpp` file
- **Include ordering**: Manual — never auto-sort
- **C++ naming**: PascalCase classes, camelCase methods/variables
- **Python logging**: `FreeCAD.Console.PrintMessage()` / `.PrintError()` / `.PrintLog()` — never `print()`
- **GUI guards**: `if FreeCAD.GuiUp:` before ViewProvider instantiation
- **Import order (Python)**: stdlib → third-party → `FreeCAD`/`FreeCADGui` → module imports → GUI imports (guarded)
- **Lazy imports**: Load modules only when needed (startup performance)
- **i18n**: `FreeCAD.Qt.translate("Context", "text")` or `QT_TRANSLATE_NOOP("Context", "text")`
- **CMake toggles**: `BUILD_{MODULE_NAME}` (e.g., `BUILD_FEM`, `BUILD_SKETCHER`)
- **AI policy**: Raw AI output NOT accepted. Must review, validate, explain decisions.

## ANTI-PATTERNS (THIS PROJECT)

- **Never** bare `except:` — always catch specific exceptions
- **Never** `from x import *` — explicit imports only
- **Never** `as any` / `@ts-ignore` equivalent type suppression
- **Never** `App.ActiveDocument` in object code — use `obj.Document` for robustness
- **Never** `<` or `>` in property descriptions — breaks XML serialization in .FCStd
- **Never** `print()` — use `FreeCAD.Console.*`
- **Never** `__getstate__`/`__setstate__` — use `dumps()`/`loads()` for serialization
- **Always** call `doc.recompute()` after creating/modifying objects
- **Always** check `FreeCAD.GuiUp` before GUI operations
- **Always** include braces on control statements (even single-line bodies)

## COMMANDS

```bash
# Build (preferred)
pixi run configure            # Debug build → build/debug/
pixi run build                # Compile debug
pixi run configure-release    # Release build → build/release/
pixi run build-release        # Compile release
pixi run freecad              # Run GUI

# Test
pixi run test                 # All CTest
build/debug/tests/Part_tests_run --gtest_filter="TestName*"  # Single C++ suite
build/debug/bin/FreeCADCmd -t 0    # Python tests (CLI)

# Lint
# Pre-commit hooks: clang-format (C++, 100 chars, 4-space), Black (Python, 100 chars)
```

## COMPLEXITY HOTSPOTS

| File                                  | Lines | Why                         |
| ------------------------------------- | ----- | --------------------------- |
| `Sketcher/Gui/CommandConstraints.cpp` | 11040 | All constraint commands     |
| `Part/App/Geometry.cpp`               | 7748  | OCCT geometry wrappers      |
| `Gui/Tree.cpp`                        | 6893  | Model tree widget           |
| `Part/App/TopoShapeExpansion.cpp`     | 6147  | Element mapping             |
| `App/PropertyLinks.cpp`               | 6020  | Link property system        |
| `Sketcher/App/planegcs/GCS.cpp`       | 5803  | Geometric constraint solver |

## NOTES

- `.FCStd` = ZIP archive containing `Document.xml`, `GuiDocument.xml`, `.brp` shape files
- Macro files use `.FCMacro` extension (not `.py`)
- Coin3D scene graph node order matters — transforms must precede geometry
- Robot and Sketcher require Eigen3
- AddonManager is a git submodule — run `pixi run initialize` first
- Current version: 1.2.0dev
- TemplatePyMod = canonical FeaturePython example (study before writing Python objects)
