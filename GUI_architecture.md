# FreeCAD GUI Architecture

This document maps the `src/Gui/` layer: how it boots, how it hangs together, and how it interfaces with App/Base and workbenches. Use it to orient design changes without breaking the headless model.

## Layering and Dependencies
- **Base → App → Gui → Mod** (strict). Gui depends on App/Base; App/Base never depend on Gui/Qt.
- **Technologies:** Qt6 (widgets/docks/MDI), Coin3D + Quarter (Open Inventor scene graph in Qt), Python (commands/macros/bindings), PySide/Shiboken, PyCXX for bindings.

## Boot Sequence (GUI path)
1. `main()` in `src/Main/MainGui.cpp` picks GUI mode.
2. `App::Application::init()` registers App types, starts Python, loads `src/Mod/*/Init.py` (headless feature registration).
3. `Gui::Application::initTypes()` registers core GUI types (ViewProviders, commands, workbench base).
4. `Gui::Application::initApplication()` initializes Qt resources/icons/translations and Coin3D (`initOpenInventor`).
5. `FreeCADGuiInit.py` runs each module `InitGui.py`, registering workbenches/commands/preferences.
6. `Gui::MainWindow` is built; workbench activation populates menus/toolbars/docks.

## Core Runtime Objects
- **Gui::Application (`src/Gui/Application.{h,cpp}`):** Singleton (Instance). Manages Gui::Document set, view lifecycles, workbench activation (`activateWorkbench`), command registry, stylesheets, macro manager, and fastsignals for doc/object/view events (`signalNewDocument`, `signalInEdit`, `signalActivateWorkbench`, etc.).
- **Gui::MainWindow (`src/Gui/MainWindow.{h,cpp}`):** Qt main window. Hosts MDI area, docks (Tree, PropertyView, Python console, ReportView), toolbars/menus/status bar. Docks are rearrangeable; workbench swaps menus/toolbar content.
- **Gui::Document (`src/Gui/Document.{h,cpp}`):** GUI twin of `App::Document`. Owns ViewProviders for every `App::DocumentObject`; wires App signals (new/changed/deleted object) to GUI updates. Manages MDI views (`View3DInventor`, text editors, graphviz, etc.), edit mode, undo/redo UI commands, and scene-graph attach/detach for view providers.
- **Gui::Workbench (`src/Gui/Workbench.{h,cpp}`, `InitGui.py` in modules):** Defines menus/toolbars/context menus for a domain. `WorkbenchManager/Factory` loads by name; Python workbenches subclass `Gui::PythonWorkbench`.

## Command and Event Flow
- **Gui::Command (`src/Gui/Command.h` + Command*.cpp):** Each command carries metadata (module, group, text, icon, shortcut) and implements `activated()` / `isActive()`. Registered through `Application::commandManager().addCommand(...)` (C++) or `FreeCADGui.addCommand` (Python).
- **Macro & Undo integration:** Typical pattern uses `openCommand()`/`doCommand()`/`commitCommand()` so actions are macro-recorded and undoable. `doCommand(Doc, "Python..." )` executes in document context.
- **Menus/Toolbars:** Built from command names; workbench activation repopulates them.

## Document ↔ ViewProvider Bridge
- **Pairing:** Each `App::DocumentObject::getViewProviderName()` names a `Gui::ViewProvider` subclass. `Gui::Document` instantiates and tracks these.
- **Base classes:** `ViewProvider` → `ViewProviderDocumentObject` → specialized geometry/object/group providers. Extensions: `ViewProviderExtension`, `ViewProviderGeoFeatureGroupExtension`, `ViewProviderLink`, etc.
- **Responsibilities:** Build Coin3D subgraph in `attach()`, react to property changes in `updateData()`, expose display modes, icons, context menus, edit modes (`setEdit()/unsetEdit()`), tree behavior (`claimChildren`, drag/drop), visibility toggles (`show/hide`).
- **Scene graph:** Coin3D nodes (`SoSeparator`, `SoTransform`, `SoMaterial`, `SoIndexedFaceSet`, etc.) live under the viewer’s root; ordering matters (transform before geometry). Custom nodes live in `src/Gui/Inventor/` and `SoFC*` files.

## 3D View Stack
- **Viewer widget:** `View3DInventorViewer` + `Quarter/` glue embed Coin3D in Qt. Variants: `View3DInventor`, `SplitView3DInventor`, Rift/VR viewers.
- **Navigation styles:** `Navigation/` (CAD, Blender, gesture, SpaceMouse via `3Dconnexion/`). `NaviCube` overlays orientation.
- **Selection in 3D:** See selection system below; pick results map Coin3D paths to ViewProviders/DocObjects.

## Selection System
- **Location:** `src/Gui/Selection/Selection.{h,cpp}` (singleton `Gui::Selection()`).
- **Capabilities:** Add/clear selection, preselection (hover), sub-element addressing (Face1/Edge2), filters (`SelectionFilter`), observers (`SelectionObserver`).
- **Flow:** Viewer hit-tests → Selection singleton → notifies observers (Tree, PropertyView, commands) → commands query `getSelection()`/`getSelectionEx()`.

## Task Panels and Editing
- **Infrastructure:** `src/Gui/TaskView/` (`TaskDialog`, `TaskView`, `TaskBox`).
- **Usage:** `ViewProvider::setEdit()` opens a task dialog via `Gui::Control().showDialog(...)`; `unsetEdit()` closes. Task dialogs drive OK/Cancel and often start/commit undo transactions. Panels typically loaded from `.ui` via `Gui::UiLoader`.

## Property and Tree Presentation
- **Tree:** `src/Gui/Tree.cpp` (complex). Shows document hierarchy; ViewProviders influence labels/icons/children/drag-drop and highlight/expand signals.
- **Property editor:** `src/Gui/propertyeditor/` + `PropertyView.cpp`. Picks editors per property type; many editors map to property `getEditorName()` from App layer.
- **Property panels in combo view:** Combined tree + task view for workflow.

## Resources, Styles, Preferences
- **Icons:** `src/Gui/Icons/` and module resources; registered via `.qrc` and `BitmapFactory`.
- **Stylesheets:** `src/Gui/Stylesheets/` and runtime loader (`Application::setStyleSheet`, `FreeCADStyle`).
- **Preference pages:** `src/Gui/PreferencePages/` (Qt dialogs) plus module pages registered from `InitGui.py` or C++.
- **Style parameters:** `StyleParameters/ParameterManager` for theme variables.

## Python Integration
- Python bindings for GUI classes (`*Py.cpp`, `.pyi` shims) enable scripting commands, task panels, and custom ViewProviders. Always guard `if FreeCAD.GuiUp:` before importing `FreeCADGui`.

## Workbench Integration Points
- **InitGui.py per module:** Registers commands, menus/toolbars, workbench class, preference pages, resources.
- **App/Gui split per module:** `src/Mod/<WB>/App/` for headless features; `src/Mod/<WB>/Gui/` for ViewProviders/commands/task panels. CMake targets and Python init keep App/Gui binaries separate.

## Anti-Patterns and Safety
- Never include Gui/Qt headers in App/Base.
- Always recompute (`doc.recompute()`) after model edits from commands/task panels.
- Avoid global `App.ActiveDocument` inside App objects; use `obj.Document`. In GUI code, prefer document from context.
- No type suppression; no `<`/`>` in property descriptions (breaks XML).
- `View3DInventorViewer` is not thread-safe; modify scene graph on GUI thread.

## Subdirectory Map (high level)
- `Command*.cpp` — Command categories (Std, View, Doc, Link, Macro, Feat, Window, Test).
- `ViewProvider*.cpp/h` — Object visualization; extensions; geometry/group/link/origin/material/placement.
- `Selection/` — Selection singleton, filters, observers.
- `TaskView/` — Task dialog framework.
- `Dialogs/` — Qt dialogs (`Dlg*`) + `.ui` files (preferences, file ops, wizards).
- `propertyeditor/` — Property widgets and editor factory.
- `Navigation/` — Navigation styles, NaviCube.
- `Inventor/`, `SoFC*.cpp` — Custom Coin3D nodes.
- `Quarter/` — Qt/Coin integration helpers.
- `Stylesheets/`, `Icons/`, `Language/` — QSS, icon assets, translations.
- `PreferencePages/` — Preference dialogs/pages.
- `DAGView/` — Dependency graph view.
- `3Dconnexion/` — SpaceMouse support.
- `QSint/` — Vendored Qt widget components.
- `StyleParameters/` — Theme parameter manager.

## Cross-Cutting Hotspots (be cautious)
- `Tree.cpp` (large, shared by all workbenches).
- `Selection/Selection.cpp` (global selection semantics).
- `View3DInventorViewer.cpp` (viewer behavior/navigation/interaction).
- `Application.cpp` (signals, workbench lifecycle, stylesheets).

## References
- Developer guides: `guide/01-architecture-overview.md`, `guide/05-gui-viewproviders-commands.md`, `guide/06-workbench-development.md`, `guide/10-modifying-core-and-ui.md`, `guide/11-creating-new-workbench.md`.
- `src/Gui/AGENTS.md` — quick structural cheat sheet and anti-patterns.
