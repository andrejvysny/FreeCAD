# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

FreeCAD uses CMake + Ninja with Pixi for dependency management.

```bash
# Preferred: Pixi workflow
pixi run initialize           # Init git submodules (first time)
pixi run configure            # Configure debug build (build/debug/)
pixi run configure-release    # Configure release build (build/release/)
pixi run build                # Build debug
pixi run build-release        # Build release
pixi run install              # Install debug build
pixi run freecad              # Run FreeCAD GUI

# Manual CMake
cmake --preset debug          # or: cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug -j$(nproc)
```

Build presets defined in `CMakePresets.json`: `debug`, `release`, `conda-{linux,macos,windows}-{debug,release}`.

## Testing

```bash
# All tests
pixi run test                 # CTest on debug build
ctest --test-dir build/debug

# Single C++ test suite (gtest)
build/debug/tests/App_tests_run
build/debug/tests/Sketcher_tests_run --gtest_filter="TestName*"

# Python tests
build/debug/bin/FreeCADCmd -t 0          # CLI tests
xvfb-run build/debug/bin/FreeCAD -t 0    # GUI tests (Linux)
```

C++ test suites: `App`, `Base`, `Gui`, `Assembly`, `Material`, `Measure`, `Mesh`, `MeshPart`, `Part`, `PartDesign`, `Sketcher`, `Spreadsheet`, `Start`, `Zipios` — all under `tests/` as `{Name}_tests_run`.

## Linting

Pre-commit hooks enforce formatting:

- **C++**: clang-format (LLVM-based, 100 char limit, 4-space indent)
- **Python**: Black (100 char limit)
- clang-tidy checks enabled (bugprone, performance, readability, modernize, cppcoreguidelines)

## Contribution Rules

See `CONTRIBUTING.md` for full process. Key rules:

- **Commit messages**: `Module: Brief description` — e.g., `Gui:`, `Part:`, `Sketcher:`, `TD:`, `Build:`, `Fem:`, `Core:`, `Material:`, `Start:`
- One PR = one problem. Each commit must compile independently. Squash checkpoint commits.
- UI changes require before/after screenshots in PR body
- Python API breaks: minimize, document migration path, list affected addons
- **AI policy**: Raw AI output NOT accepted. AI may assist, but contributor must review, validate, and explain all design/code decisions to reviewers
- **License**: LGPL-2.1-or-later. SPDX header (`// SPDX-License-Identifier: LGPL-2.1-or-later`) required in every new file
- `[skip ci]` in commit message to bypass CI when appropriate

## C++ Standards

Enforced by `.clang-format` (LLVM-based with Qt modifications). Key rules for code generation:

- 4-space indent, 100 char column limit, tabs never
- **Braces**: new line after class, struct, function, namespace, enum; **same line** for control statements (`if`, `for`, `while`)
- Always use braces on control statements (even single-line bodies)
- Break before binary operators; constructor initializers break before comma
- Pointer/reference alignment left: `int* ptr`, `const std::string& ref`
- No short functions, blocks, or enums on a single line
- No indentation inside namespaces
- Includes never auto-sorted — maintain manual ordering
- Max 2 consecutive empty lines
- **Naming**: PascalCase classes, camelCase methods/variables
- `TYPESYSTEM_HEADER()` / `TYPESYSTEM_SOURCE()` macros for runtime type reflection
- `PreCompiled.h` must be the first include in every `.cpp` file
- Cognitive complexity threshold: 25 (clang-tidy)
- C++ Python bindings: `ClassPy.xml` (description) + `ClassPyImp.cpp` (implementation) → auto-generates `ClassPy.cpp`/`ClassPy.h` via `generate_from_xml` CMake macro

## Python Standards

Enforced by Black (100 chars) + pre-commit hooks. PEP8 compliant.

- **Import order** (blank line between groups): stdlib → third-party → `FreeCAD`/`FreeCADGui` → FreeCAD modules → GUI imports
- GUI imports must be guarded: `if FreeCAD.GuiUp:`
- Use `FreeCAD.Console.PrintMessage()`, `.PrintError()`, `.PrintLog()` — not `print()`
- Never bare `except:` — always specify exception type
- No star imports (`from x import *`)
- Prefer `obj.Document` over `App.ActiveDocument` for robustness
- Load modules only when needed (lazy imports for startup performance)

## Python Scripting Patterns

**Critical**: Always call `doc.recompute()` after creating or modifying objects — the scene won't update without it.

**FeaturePython pattern** (canonical example: `src/Mod/TemplatePyMod/FeaturePython.py`):

```python
class MyFeature:
    def __init__(self, obj):
        obj.addProperty("App::PropertyLength", "Height", "MyGroup", "Description")
        obj.Proxy = self                    # mandatory

    def execute(self, fp):                  # mandatory — called on recompute
        fp.Shape = Part.makeBox(fp.Height, fp.Height, fp.Height)

    def dumps(self):   return None          # serialization (NOT __getstate__)
    def loads(self, state):   pass          # deserialization (NOT __setstate__)

class ViewProviderMyFeature:
    def __init__(self, vobj):
        vobj.Proxy = self                   # mandatory

    def attach(self, vobj):                 # mandatory
        pass

    def getIcon(self):
        return "path/to/icon.svg"

    def dumps(self):   return None
    def loads(self, state):   pass

# Creation pattern
obj = doc.addObject("Part::FeaturePython", "MyObj")
MyFeature(obj)
if FreeCAD.GuiUp:
    ViewProviderMyFeature(obj.ViewObject)
doc.recompute()
```

**Pitfalls**:

- Don't use `<` or `>` in property descriptions — breaks XML serialization in .FCStd files
- Property types use `App::Property` prefix: `App::PropertyLength`, `App::PropertyVector`, `App::PropertyBool`, `App::PropertyLink`, `App::PropertyEnumeration`, etc. (100+ types)
- Always check `FreeCAD.GuiUp` before instantiating ViewProviders

**Command pattern**:

- Class with `GetResources()`, `Activated()`, `IsActive()` methods
- Register: `FreeCADGui.addCommand("ModuleName_CommandName", MyCommand())`

**Workbench class**: Requires `MenuText`, `ToolTip`, `Icon`, `Initialize()`, `GetClassName()` returning `"Gui::PythonWorkbench"`

**PySide**: `from PySide import QtCore, QtGui, QtWidgets` — unified shim for Qt4/5/6 compatibility (in `Ext/PySide`)

## Internationalization

All user-facing strings must be translatable:

- Dynamic: `FreeCAD.Qt.translate("Context", "My text")` — context = module/addon name (case-sensitive)
- Static (menus, tooltips): `QT_TRANSLATE_NOOP("Context", "My text")`
- Only string literals are extracted by `lupdate` — variables are ignored
- Translations directory: `Resources/translations/`

## Architecture

### Layer Model

```
Base/    → Type system, persistence, math primitives, console (no Qt dependency)
App/     → Document model, property system, extensions, transactions (headless core)
Gui/     → ViewProviders, 3D scene (Coin3D), Qt UI, selection, task panels
Mod/     → 33 pluggable workbenches (Part, Sketcher, PartDesign, FEM, Draft, etc.)
```

App has zero Qt dependency — clean separation from Gui.

### Core Patterns

**Document/Object model**: `App::Application` (singleton) → `App::Document` → `App::DocumentObject` (base for all features). Each DocumentObject has a paired `Gui::ViewProvider` for 3D/tree representation.

**Property system** (`src/App/Property*.h`): Type-safe data binding with serialization, change notifications, expression support, and dynamic addition. PropertyLinks handle inter-object references.

**Extension system** (`src/App/Extension.h`): Composition over inheritance — GroupExtension, GeoFeatureGroupExtension, LinkBaseExtension, SuppressibleExtension add capabilities without deep hierarchies.

**Transaction system**: Atomic undo/redo across property changes.

### Workbench Structure

Each workbench in `src/Mod/{Name}/` follows:

```
App/          # C++ DocumentObject subclasses (features)
Gui/          # ViewProviders, commands, task panels
Init.py       # Core registration (importers, tests) — runs at startup
InitGui.py    # GUI init, Workbench class, command registration — runs in GUI mode
```

### Python Bindings

- XML-based: `ClassPy.xml` + `ClassPyImp.cpp` → auto-generates `ClassPy.cpp`/`ClassPy.h`
- `.pyi` stub files in `src/App/` and `src/Gui/` for IDE support
- Python modules: `FreeCAD` (core), `FreeCADGui` (GUI)

### Graphics Stack

Coin3D (Open Inventor) scene graph → Quarter (Qt integration) → OpenGL. ViewProviders manage SoNode subtrees. Node order matters — transformations must precede affected geometry.

### Key Dependencies

OpenCASCADE (geometry kernel), Coin3D (3D rendering), Qt6 (GUI), Python 3.11 (scripting), Boost, Eigen, VTK, pybind11, SWIG.

## Conventions

- **CMake module toggles**: `BUILD_{MODULE_NAME}` (e.g., `BUILD_FEM`, `BUILD_SKETCHER`)
- **File format**: `.FCStd` = ZIP archive containing `Document.xml`, `GuiDocument.xml`, `.brp` shape files
- **Macro files**: `.FCMacro` extension (not `.py`)
- Current version: 1.2.0dev

## Subagent Orchestration

Delegate domain-specific tasks to specialized subagents for higher quality output.

| Domain        | Agent               | When                                                                |
| ------------- | ------------------- | ------------------------------------------------------------------- |
| C++ code      | `cpp-developer`     | Any C++ in src/Base, src/App, src/Gui, src/Mod/_/App, src/Mod/_/Gui |
| Python code   | `python-developer`  | FeaturePython, Commands, Workbenches, Init\*.py                     |
| Qt/Coin3D GUI | `qt-gui-expert`     | Dialogs, task panels, ViewProvider scenes, .ui files, themes        |
| Code review   | `freecad-reviewer`  | PR review, standards compliance checks                              |
| Architecture  | `freecad-architect` | Cross-module design, layer analysis, new workbench planning         |
| OCCT geometry | `occt-geometry`     | TopoShape, Part::Geometry, element mapping, BRep ops                |
| Tests         | `test-developer`    | gtest or unittest test writing                                      |
| Build system  | `cmake-build`       | CMakeLists, build config, module toggles                            |
| i18n          | `i18n-checker`      | Translation string validation                                       |
| Migration     | `migration-helper`  | Legacy pattern updates (Arch->BIM, Path->CAM, etc.)                 |
| UI/UX review  | `ui-ux-reviewer`    | Screenshot analysis, dialog optimization                            |

For complex features spanning multiple domains, use the architect first to plan, then delegate implementation to domain agents in parallel.
