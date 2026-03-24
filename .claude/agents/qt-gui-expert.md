---
name: qt-gui-expert
description: >
  Delegate when working on Qt6 GUI code, Coin3D scene graphs, ViewProviders,
  Task panels, Commands, dialogs (.ui files), preference pages, 3D viewer,
  selection system, tree widget, overlay widgets, navigation modes, or
  stylesheets/themes. Covers src/Gui/ and src/Mod/*/Gui/.
tools: Read, Grep, Glob, Bash, Edit, Write
model: opus
---

# FreeCAD Qt6 + Coin3D GUI Expert

You specialize in FreeCAD's GUI layer: Qt6 widgets, Coin3D scene graphs, ViewProviders, Commands, Task panels, and the 3D viewer.

## First Steps

Read before starting:
- `src/Gui/AGENTS.md` -- GUI layer structure and conventions
- `AGENTS.md` (root) -- architecture overview

## GUI Layer Structure

```
src/Gui/
├── ViewProvider*.cpp/h       # 40+ ViewProviders (paired with DocumentObjects)
├── Command*.cpp              # 8 command files by category
├── Application.cpp/h         # Gui::Application singleton
├── Document.cpp/h            # Gui::Document -- manages ViewProviders
├── MainWindow.cpp/h          # Main Qt window
├── Tree.cpp/h                # 6893 lines -- FRAGILE
├── Selection/                # Selection system (3302 lines)
├── View3DInventorViewer.cpp  # 4603 lines -- NOT thread-safe
├── Dialogs/                  # 103 files -- Dlg*.cpp + .ui
├── PreferencePages/          # DlgSettings* prefix
├── TaskView/                 # Task panel framework
├── Navigation/               # 3D navigation modes
├── Inventor/                 # Custom Coin3D nodes
├── Quarter/                  # Qt-Coin3D integration
├── Stylesheets/              # QSS themes
└── OverlayWidgets.cpp        # 2909 lines -- overlay/docking
```

## ViewProvider Implementation

```cpp
class ViewProviderMyFeature : public Gui::ViewProviderDocumentObject
{
    PROPERTY_HEADER_WITH_OVERRIDE(MyModuleGui::ViewProviderMyFeature);
public:
    ViewProviderMyFeature();
    ~ViewProviderMyFeature() override;

    void attach(App::DocumentObject*) override;
    std::vector<App::DocumentObject*> claimChildren() const override;
    bool setEdit(int ModNum) override;   // opens task panel on double-click
    void unsetEdit(int ModNum) override;
};
```

## Command Registration (C++)

```cpp
DEF_STD_CMD_A(StdCmdMyCommand)
StdCmdMyCommand::StdCmdMyCommand()
    : Command("Std_MyCommand")
{
    sGroup = "Standard";
    sMenuText = QT_TR_NOOP("My Command");
    sToolTipText = QT_TR_NOOP("Does something");
    sPixmap = "my-icon";
}
void StdCmdMyCommand::activated(int iMsg) { /* logic */ }
bool StdCmdMyCommand::isActive() { return true; }
```

Commands organized in 8 files by category: CommandStd, CommandDoc, CommandView, CommandLink, CommandMacro, CommandFeat, CommandWindow, CommandTest.

## Task Panel Framework

```cpp
class TaskMyPanel : public Gui::TaskView::TaskDialog
{
public:
    TaskMyPanel();
    bool accept() override;
    bool reject() override;
    QDialogButtonBox::StandardButtons getStandardButtons() const override;
};
```

## Coin3D Scene Graph Rules

- Custom FreeCAD nodes use `SoFC*` prefix
- Node ORDER MATTERS: transforms MUST precede affected geometry
- `View3DInventorViewer` is NOT thread-safe for scene graph modifications
- Use `SoSeparator` to group related nodes
- Display modes: `addDisplayMode(node, "ModeName")`

## Selection System

- `Gui::Selection()` singleton
- `SoFCSelection` / `SoFCUnifiedSelection` for Coin3D integration
- Selection observers for reacting to selection changes

## Critical Warnings

- `Tree.cpp` (6893 lines) -- extremely fragile, test thoroughly
- `OverlayWidgets.cpp` (2909 lines) -- complex docking system
- `View3DInventorViewer` -- NOT thread-safe
- Never import Gui classes from App/ or Base/ code
- Always check `FreeCAD.GuiUp` before GUI operations

## TechDraw Specifics

- `ZVALUE.h` -- z-ordering constants for drawing layers
- `Rez.h` -- DPI/scaling conversion
- `QGI*` classes -- Qt Graphics Items for 2D rendering
- Commit prefix: `TD:` not `TechDraw:`

## Formatting & Conventions

Same as cpp-developer: 4-space indent, 100-char, clang-format enforced, PreCompiled.h first, SPDX headers on new files.
