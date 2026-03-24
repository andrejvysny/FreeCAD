# src/Gui/ — GUI Framework Layer

Qt6 + Coin3D visualization layer. 460+ files, 17 subdirectories. Depends on Base and App — never imported by them.

## STRUCTURE

```
Gui/
├── ViewProvider*.cpp/h    # 40+ ViewProviders — 3D/tree representation of DocumentObjects
├── Command*.cpp           # 8 command files (Doc, View, Std, Link, Macro, Feat, Window, Test)
├── Application.cpp/h      # Gui::Application singleton — startup, workbench management
├── Document.cpp/h         # Gui::Document — manages ViewProviders for App::Document
├── MainWindow.cpp/h       # Main Qt window, menus, toolbars, status bar
├── Tree.cpp/h             # Model tree widget (6893 lines — complexity hotspot)
├── Selection/             # Selection system (Selection.cpp = 3302 lines)
├── Dialogs/               # 103 files — Dlg*.cpp + .ui (Qt Designer)
├── PreferencePages/       # Settings pages
├── propertyeditor/        # Property panel widgets
├── TaskView/              # Task panel framework
├── Navigation/            # 3D navigation modes
├── Inventor/              # Custom Coin3D nodes
├── Quarter/               # Qt-Coin3D integration
├── Stylesheets/           # QSS themes + icon assets
├── Icons/                 # SVG/PNG application icons
├── Language/              # Translation .ts files
├── DAGView/               # DAG visualization (experimental)
├── QSint/                 # Vendored Qt widget library
├── 3Dconnexion/           # SpaceMouse support
└── StyleParameters/       # Theme parameters
```

## WHERE TO LOOK

| Task                     | Location                                                  |
| ------------------------ | --------------------------------------------------------- |
| Add menu/toolbar command | `Command*.cpp` — one of 8 files by category               |
| Add dialog               | `Dialogs/Dlg{Name}.cpp` + `.ui` file                      |
| Add preference page      | `PreferencePages/Dlg{Settings}{Name}.*`                   |
| Add ViewProvider         | `ViewProvider{Type}.cpp/h` — pair with App DocumentObject |
| Modify 3D viewer         | `View3DInventorViewer.cpp` (4603 lines)                   |
| Modify tree widget       | `Tree.cpp` (6893 lines) — careful, fragile                |
| Modify selection         | `Selection/Selection.cpp` (3302 lines)                    |
| Add task panel           | `TaskView/TaskDialog*.cpp`                                |
| Modify overlay/docking   | `OverlayWidgets.cpp` (2909 lines)                         |
| Add Coin3D custom node   | `SoFC*.cpp` or `Inventor/`                                |
| Modify navigation        | `Navigation/` directory                                   |

## CONVENTIONS

- ViewProvider class names mirror DocumentObject: `Part::Feature` → `PartGui::ViewProviderPartExt`
- Commands: class with `GetResources()`, `Activated()`, `IsActive()`, registered via `addCommand()`
- All `.ui` files use Qt Designer format — edit with Designer or manually
- Coin3D node order matters: transforms MUST precede affected geometry nodes
- `SoFC*` prefix for custom Coin3D nodes (FreeCAD-specific)

## ANTI-PATTERNS

- Never import Gui classes from App/ or Base/ code
- Never access `Gui::Application` without checking `FreeCAD.GuiUp` first
- `View3DInventorViewer` is NOT thread-safe for scene graph modifications
