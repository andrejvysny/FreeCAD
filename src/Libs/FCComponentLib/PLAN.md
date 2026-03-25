# FCComponentLib Extraction Plan

## Context

FreeCAD's GUI layer contains many reusable Qt widget classes scattered across `src/Gui/Widgets.h`,
`src/Gui/` standalone files, and `src/Mod/*/Gui/`. A component library already exists at
`src/Libs/FCComponentLib/` with 6 extracted components (FcPushButton, FcSpinBox, FcDoubleSpinBox,
FcLineEdit, FcComboBox, FcColorButton), plus a gallery app at `src/Libs/FCComponentGallery/`.

This plan identifies and extracts additional reusable widgets into the existing library, following
its established patterns: `FcComponents` namespace, `FCComponentLibExport` macro,
`FC_REGISTER_COMPONENT` registration, and gallery stories.

## Scan Summary

- Directories scanned: 23 (src/Gui/ + 22 module Gui/ dirs)
- Total widgets found: ~60
- Candidates passing threshold: 14
- Rejected (coupling too high): ~30
- Rejected (dialog/non-widget): ~6
- Already extracted: 6

## Existing Components

| Component | Category | Status |
|-----------|----------|--------|
| FcPushButton | Buttons/ | Exists |
| FcSpinBox | Inputs/ | Exists |
| FcDoubleSpinBox | Inputs/ | Exists |
| FcLineEdit | Inputs/ | Exists |
| FcComboBox | Inputs/ | Exists |
| FcColorButton | Feedback/ | Exists |

Existing infrastructure: Style/ (ComponentStyle, ComponentPalette), Tokens/ (TokenManager, Parser),
Components/ (ComponentMeta.h with FC_REGISTER_COMPONENT, ComponentRegistry).

## Extraction Candidates

### Tier 1 — Clean extraction (coupling 0-2)

| # | Widget | Source | Category | Files Used In | Coupling | Dependencies |
|---|--------|--------|----------|---------------|----------|--------------|
| 1 | SplitButton | src/Gui/SplitButton.h | Buttons/ | 3 | 0 | Qt only |
| 2 | ElideCheckBox | src/Gui/ElideCheckBox.h | Inputs/ | 2 (fundamental) | 0 | Qt only |
| 3 | UrlLabel | src/Gui/Widgets.h:297 | Display/ | 8 | 0 | Qt only |
| 4 | ActionSelector | src/Gui/Widgets.h:83 | Containers/ | 6 | 0 | Qt only |
| 5 | AccelLineEdit | src/Gui/Widgets.h:140 | Inputs/ | 8 | 0 | Qt only |
| 6 | ClearLineEdit | src/Gui/Widgets.h:174 | Inputs/ | 2 (fundamental) | 0 | Qt only |
| 7 | ModifierLineEdit | src/Gui/Widgets.h:157 | Inputs/ | 2 (fundamental) | 0 | Qt only |
| 8 | LabelButton | src/Gui/Widgets.h:425 | Inputs/ | 5 | 0 | Qt only |
| 9 | LabelEditor | src/Gui/Widgets.h:544 | Inputs/ | 3 | 0 | Qt only |
| 10 | ButtonGroup | src/Gui/Widgets.h:632 | Buttons/ | 8 | 0 | Qt only |
| 11 | PropertyListEditor | src/Gui/Widgets.h:520 | Display/ | 2 (fundamental) | 0 | Qt only |
| 12 | CompassDialWidget | src/Mod/TechDraw/Gui/Widgets/CompassDialWidget.h | Display/ | 4 | 1 | Qt + Base::Console (unused, remove) |

### Tier 2 — Minor refactoring (coupling 3-5)

| # | Widget | Source | Category | Files | Coupling | Refactoring Notes |
|---|--------|--------|----------|-------|----------|-------------------|
| 13 | QtColorPicker | src/Mod/Spreadsheet/Gui/qtcolorpicker.h | Buttons/ | 6 | 0 | Replace export macro; different license header (LGPL-2.1-only OR GPL-3.0-only, compatible) |
| 14 | MTextEdit | src/Mod/TechDraw/Gui/mtextedit.h | Inputs/ | 3 | 1 | Replace TechDrawGlobal.h with FCComponentLibGlobal.h |

### Rejected — Documented reasons

| Widget | Source | Reason |
|--------|--------|--------|
| StatefulLabel | src/Gui/Widgets.h | Base::Observer + ParameterGrp (coupling 6) |
| ExpLineEdit | src/Gui/Widgets.h | ExpressionWidget, App::Expression (coupling 8) |
| QuantitySpinBox | src/Gui/QuantitySpinBox.h | Base::Quantity, App::Expression, Gui/ deps (coupling 8) |
| InputField | src/Gui/InputField.h | ExpressionWidget, Base::Quantity, history (coupling 8) |
| All PrefWidgets (14) | src/Gui/PrefWidgets.h | WindowParameter, Base::Observer (coupling 6) |
| TextEdit/TextEditor | src/Gui/TextEdit.h | CallTips (Python), WindowParameter (coupling 6) |
| ProgressBar | src/Gui/ProgressBar.h | Base::SequencerBase (coupling 7) |
| TreeWidget | src/Gui/Tree.h | SelectionObserver, document model (coupling 9) |
| NotificationArea | src/Gui/NotificationArea.h | Base::Observer, logging system (coupling 7) |
| OverlayWidgets (9) | src/Gui/OverlayWidgets.h | OverlayManager, parameter system (coupling 8) |
| NewFileButton | src/Mod/Start/Gui/NewFileButton.h | App::GetApplication() in constructor (coupling 6) |
| ThemeSelectorWidget | src/Mod/Start/Gui/ThemeSelectorWidget.h | App::Application, Gui::Command (coupling 7) |
| VectorEditWidget | src/Mod/TechDraw/Gui/Widgets/ | Uses Gui::DoubleSpinBox (Gui/ dep blocker) |
| CompassWidget | src/Mod/TechDraw/Gui/Widgets/ | Uses Gui::QuantitySpinBox (Gui/ dep blocker) |
| ImageLabel | src/Mod/Material/Gui/ImageEdit.h | Header includes Mod/Material/App/Model.h (blocker) |
| MRichTextEdit | src/Mod/TechDraw/Gui/mrichtextedit.h | .ui file dependency + TechDrawGlobal.h (coupling 4) |
| CheckListDialog | src/Gui/Widgets.h | QDialog subclass (excluded by rules) |
| StatusWidget | src/Gui/Widgets.h | QDialog subclass (excluded by rules) |
| ToolTip | src/Gui/Widgets.h | QObject, not a QWidget |
| CommandIconView | src/Gui/Widgets.h | Internal customization dialog use only |
| SqueezeLabel | src/Gui/DownloadItem.h | Only used internally in DownloadItem |
| ToolBarGrip | src/Gui/ToolBarManager.h | Only used in ToolBarManager (2 files) |

## Dependency Graph

No inter-dependencies between candidates. All can be extracted independently.

## Recommended Extraction Order

Extract in parallel — no ordering constraints. Group by source file for efficiency:

**Batch 1 — Standalone files (trivial, self-contained .h/.cpp pairs):**
1. FcSplitButton (from src/Gui/SplitButton.h/.cpp)
2. FcElideCheckBox (from src/Gui/ElideCheckBox.h/.cpp)
3. FcCompassDial (from src/Mod/TechDraw/Gui/Widgets/CompassDialWidget.h/.cpp)
4. FcColorPicker (from src/Mod/Spreadsheet/Gui/qtcolorpicker.h/.cpp)
5. FcImageTextEdit (from src/Mod/TechDraw/Gui/mtextedit.h/.cpp)

**Batch 2 — Extracted from src/Gui/Widgets.h/.cpp (need surgical extraction):**
6. FcUrlLabel
7. FcActionSelector
8. FcAccelLineEdit
9. FcClearLineEdit
10. FcModifierLineEdit
11. FcLabelButton
12. FcLabelEditor
13. FcButtonGroup
14. FcPropertyListEditor

## Implementation Pattern (match existing code exactly)

### File structure per component
```
src/Libs/FCComponentLib/Components/{Category}/Fc{Name}.h
src/Libs/FCComponentLib/Components/{Category}/Fc{Name}.cpp
```

### Header template
```cpp
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association
#pragma once
#include <QBaseWidget>
#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{
/// @brief Doxygen description
class FCComponentLibExport FcWidgetName : public QBaseWidget
{
    Q_OBJECT
    Q_PROPERTY(...)
public:
    explicit FcWidgetName(QWidget* parent = nullptr);
    ...
};
}  // namespace FcComponents
```

### Source template
```cpp
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association
#include "FcWidgetName.h"
#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents { ... }

FC_REGISTER_COMPONENT(FcWidgetName, "Category", "Description")
```

### Key patterns from existing code
- **NO PreCompiled.h** — this is a standalone library
- Namespace: `FcComponents`
- Export macro: `FCComponentLibExport`
- Registration: `FC_REGISTER_COMPONENT(Class, Category, Desc)` at bottom of .cpp
- Gallery stories go in `src/Libs/FCComponentGallery/Stories/{Category}Stories.cpp`
- Stories use `FcGallery::StoryRegistry::instance().registerStories(...)` pattern

### component-meta.json
Generated alongside each component per the task spec, placed next to the .h/.cpp files.

## CMake Changes

Add new source files to `src/Libs/FCComponentLib/CMakeLists.txt` in the
`FCComponentLib_Widgets_SRCS` section. Add new story files to
`src/Libs/FCComponentGallery/CMakeLists.txt`.

No new `target_link_libraries` needed — all candidates are pure Qt.

## Verification

1. **Build**: `pixi run build` — verify FCComponentLib compiles with new components
2. **Format**: Run `check-format` skill on all new files
3. **Review**: Run `freecad-reviewer` on all new files for standards compliance
4. **Gallery**: Build FCComponentGallery — verify new components appear and stories work
5. **No external changes**: Confirm no files outside `src/Libs/` were modified

## Estimated Effort

- Tier 1 (12 components): ~12 .h + 12 .cpp + 12 component-meta.json + 4 story files
- Tier 2 (2 components): ~2 .h + 2 .cpp + 2 component-meta.json
- CMake updates: 2 files
- Total new files: ~46
