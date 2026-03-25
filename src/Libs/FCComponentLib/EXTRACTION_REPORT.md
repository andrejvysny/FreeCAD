# FCComponentLib Extraction Report

## Summary

- Components extracted: 14 (new) + 6 (pre-existing) = 20 total
- Components skipped (could not decouple): ~30
- Build verification: Pending (human to run `pixi run build`)
- Format check: Pending (human to run format checks)

## Extracted Components (New)

| Component | Category | Original Source | Coupling Changes | Stories | Status |
|-----------|----------|----------------|------------------|---------|--------|
| FcSplitButton | Buttons/ | src/Gui/SplitButton.h | Namespace only | 3 | Done |
| FcButtonGroup | Buttons/ | src/Gui/Widgets.h | Namespace, no registration (QObject) | 0 | Done |
| FcColorPicker | Buttons/ | src/Mod/Spreadsheet/Gui/qtcolorpicker.h | Re-licensed, split to private .cpp | 4 | Done |
| FcElideCheckBox | Inputs/ | src/Gui/ElideCheckBox.h | Namespace, removed FCGlobal.h | 3 | Done |
| FcAccelLineEdit | Inputs/ | src/Gui/Widgets.h | Extracted from monolithic file | 3 | Done |
| FcClearLineEdit | Inputs/ | src/Gui/Widgets.h | Extracted from monolithic file | 2 | Done |
| FcModifierLineEdit | Inputs/ | src/Gui/Widgets.h | Extracted from monolithic file | 1 | Done |
| FcLabelButton | Inputs/ | src/Gui/Widgets.h | Extracted from monolithic file | 3 | Done |
| FcLabelEditor | Inputs/ | src/Gui/Widgets.h | Extracted from monolithic file | 3 | Done |
| FcImageTextEdit | Inputs/ | src/Mod/TechDraw/Gui/mtextedit.h | Removed TechDrawGlobal.h | 1 | Done |
| FcUrlLabel | Display/ | src/Gui/Widgets.h | Extracted from monolithic file | 3 | Done |
| FcCompassDial | Display/ | src/Mod/TechDraw/Gui/Widgets/CompassDialWidget.h | Removed Base/Console, TechDrawGlobal | 3 | Done |
| FcPropertyListEditor | Display/ | src/Gui/Widgets.h | Extracted from monolithic file | 2 | Done |
| FcActionSelector | Containers/ | src/Gui/Widgets.h | Extracted from monolithic file | 3 | Done |

## Pre-Existing Components (Unchanged)

| Component | Category |
|-----------|----------|
| FcPushButton | Buttons/ |
| FcSpinBox | Inputs/ |
| FcDoubleSpinBox | Inputs/ |
| FcLineEdit | Inputs/ |
| FcComboBox | Inputs/ |
| FcColorButton | Feedback/ |

## Skipped Components (could not decouple cleanly)

| Component | Reason | What would be needed |
|-----------|--------|---------------------|
| StatefulLabel | Base::Observer + ParameterGrp coupling | Abstract observer interface |
| ExpLineEdit | App::Expression system | Expression abstraction layer |
| QuantitySpinBox | Base::Quantity + App::Expression + Gui deps | Major refactoring |
| InputField | Expression + Quantity + parameter history | Major refactoring |
| PrefWidgets (14) | WindowParameter + Base::Observer | Parameter abstraction layer |
| TextEdit/TextEditor | Python CallTips + WindowParameter | Decouple from Python |
| ProgressBar | Base::SequencerBase | Progress abstraction |
| VectorEditWidget | Uses Gui::DoubleSpinBox | Depend on FcDoubleSpinBox first |
| CompassWidget | Uses Gui::QuantitySpinBox | Depends on quantity system |
| NewFileButton | App::GetApplication() in constructor | Parameterize config |
| ThemeSelectorWidget | App + Gui::Command + PreferencePackManager | Major decoupling |
| MRichTextEdit | .ui file dependency | Extract .ui + decouple |

## Directory Structure

```
src/Libs/FCComponentLib/
├── CMakeLists.txt
├── EXTRACTION_REPORT.md
├── FCComponentLibGlobal.h
├── PLAN.md
├── Components/
│   ├── ComponentMeta.h
│   ├── ComponentRegistry.h / .cpp
│   ├── Buttons/
│   │   ├── FcButtonGroup.h / .cpp
│   │   ├── FcButtonGroup/component-meta.json
│   │   ├── FcColorPicker.h / .cpp
│   │   ├── FcColorPickerPrivate.h / .cpp
│   │   ├── FcColorPicker/component-meta.json
│   │   ├── FcPushButton.h / .cpp  (pre-existing)
│   │   ├── FcSplitButton.h / .cpp
│   │   └── FcSplitButton/component-meta.json
│   ├── Containers/
│   │   ├── FcActionSelector.h / .cpp
│   │   └── FcActionSelector/component-meta.json
│   ├── Display/
│   │   ├── FcCompassDial.h / .cpp
│   │   ├── FcCompassDial/component-meta.json
│   │   ├── FcPropertyListEditor.h / .cpp
│   │   ├── FcPropertyListEditor/component-meta.json
│   │   ├── FcUrlLabel.h / .cpp
│   │   └── FcUrlLabel/component-meta.json
│   ├── Feedback/
│   │   └── FcColorButton.h / .cpp  (pre-existing)
│   └── Inputs/
│       ├── FcAccelLineEdit.h / .cpp
│       ├── FcAccelLineEdit/component-meta.json
│       ├── FcClearLineEdit.h / .cpp
│       ├── FcClearLineEdit/component-meta.json
│       ├── FcComboBox.h / .cpp  (pre-existing)
│       ├── FcDoubleSpinBox.h / .cpp  (pre-existing)
│       ├── FcElideCheckBox.h / .cpp
│       ├── FcElideCheckBox/component-meta.json
│       ├── FcImageTextEdit.h / .cpp
│       ├── FcImageTextEdit/component-meta.json
│       ├── FcLabelButton.h / .cpp
│       ├── FcLabelButton/component-meta.json
│       ├── FcLabelEditor.h / .cpp
│       ├── FcLabelEditor/component-meta.json
│       ├── FcLineEdit.h / .cpp  (pre-existing)
│       ├── FcModifierLineEdit.h / .cpp
│       ├── FcModifierLineEdit/component-meta.json
│       └── FcSpinBox.h / .cpp  (pre-existing)
├── Style/
│   ├── ComponentPalette.h / .cpp
│   └── ComponentStyle.h / .cpp
└── Tokens/
    ├── Parser.h / .cpp
    ├── TokenManager.h / .cpp
    ├── TokenSource.h / .cpp
    ├── Value.h / .cpp
    └── tokens/ (YAML files)
```

## Validation Results

- No `#include` from `App/`, `Gui/`, or `Mod/` in any new component
- No `PreCompiled.h` includes
- All files have `// SPDX-License-Identifier: LGPL-2.1-or-later` header
- All files under 500 lines (except FcColorPickerPrivate.cpp at 575 — 3 private classes)

## Next Steps

- [ ] Run `pixi run build` to verify compilation
- [ ] Run format checks on new files
- [ ] Human review of extracted components
- [ ] Replace original usages with FCComponentLib imports (separate session)
- [ ] Components needing further refactoring before extraction:
  - VectorEditWidget (after FcDoubleSpinBox integration)
  - CompassWidget (after QuantitySpinBox alternative)
  - MRichTextEdit (extract .ui file)
  - NewFileButton (parameterize App::GetApplication)
