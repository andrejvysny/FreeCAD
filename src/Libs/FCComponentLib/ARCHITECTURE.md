# FCComponentLib Architecture Boundary

## Purpose
FCComponentLib is the pure component library for FreeCAD UI reuse. It contains Qt widgets, component infrastructure, and reusable compound widgets. It does not contain FreeCAD binding logic.

## Dependency rule
Allowed direct dependencies:
- Qt6::Core
- Qt6::Widgets
- Qt6::Svg

Forbidden direct dependencies:
- App/
- Base/
- Gui/
- Inventor/
- Coin3D
- FreeCAD document, property, selection, or command APIs

If a widget needs FreeCAD data, document state, or GUI services, that logic belongs in `src/Gui/Adapters/`.

## What belongs here in v1
### IN
- Pure leaf widgets with no FreeCAD coupling
- Reusable compound widgets built only from Qt and other FCComponentLib widgets
- Library infrastructure, such as component metadata, registry, palette helpers, and other Qt-only support code

### OUT
- QuantitySpinBox
- InputField
- PrefWidgets
- Property editor widgets
- TaskView widgets
- QSint surfaces
- Any widget that needs App, Base, Gui, Inventor, or Coin3D to function

## Current tree baseline
The surviving library tree is intentionally small. The active source areas are:
- `Components/`
- `Components/Buttons/`
- `Style/`

That baseline is the repo truth for v1 planning. Earlier documents that describe a much larger extracted surface are historical only.

## Deferred surface list
These are out of v1 because they carry framework coupling, need adapter contracts first, or are too broad to freeze as pure Qt widgets yet.

| Deferred surface | Reason |
| --- | --- |
| QuantitySpinBox | Depends on FreeCAD quantity semantics and formatting rules, so it needs adapter design before it can be split cleanly. |
| InputField | Tied to FreeCAD input behavior and expression handling, so it is not a pure leaf widget. |
| PrefWidgets | Preference plumbing is FreeCAD-specific and pulls in app state. |
| Property editor widgets | They depend on property model and document integration, so they belong behind adapters first. |
| TaskView widgets | They are workflow heavy and depend on FreeCAD task infrastructure. |
| QSint surfaces | They are framework-style UI surfaces with broad coupling and need a separate extraction path. |

## Boundary rule for contributors
Before adding a widget, ask two questions:
1. Can it compile and run with Qt only?
2. Does it stay useful without knowing anything about FreeCAD documents or Gui services?

If either answer is no, it does not belong in FCComponentLib.
