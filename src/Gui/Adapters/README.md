# Gui Adapters

`src/Gui/Adapters/` is the FreeCAD-specific binding layer for FCComponentLib widgets.

## Allowed dependencies
Adapters may depend on:
- `Gui/`
- `App/`
- `Base/`
- `FCComponentLib`
- Qt

## What this layer does
- Attach FreeCAD behavior to pure widgets
- Bridge document, property, command, and selection state into the UI
- Own widget binding, lifecycle hooks, and registration glue
- Keep FreeCAD-specific code out of FCComponentLib

## What this layer does not do
- It does not define reusable widget primitives
- It does not move pure Qt widget logic out of FCComponentLib
- It does not introduce new public dependency paths from FCComponentLib back into FreeCAD
- It does not own business rules that should stay in App or Base

## Boundary rule
If a feature needs FreeCAD services, document state, or legacy Gui integration, implement it here. If it is pure widget behavior, keep it in FCComponentLib.

## Deferred surfaces
The following are deferred from v1 and should stay out of FCComponentLib until adapter contracts are settled:

- QuantitySpinBox, because quantity parsing and formatting need a FreeCAD-aware adapter.
- InputField, because expression and validation behavior depends on FreeCAD context.
- PrefWidgets, because preference wiring is FreeCAD-owned.
- Property editor widgets, because they depend on App property semantics and object editing flow.
- TaskView widgets, because they are coupled to task infrastructure and workflow state.
- QSint surfaces, because they require a broader GUI integration plan.

## Developer note
Adapters are the only place where FCComponentLib widgets should gain FreeCAD meaning. Keep the dependency direction one way: `FCComponentLib` first, `Gui/Adapters/` second.

## Registration and binder contract

- `AdapterRegistration` stores adapter hooks by widget class name.
- An adapter registration must provide `onAttach(QWidget&)`; `onDetach(QWidget&)` is optional.
- `WidgetBinder::attach(QWidget*)` is idempotent per widget instance. It sets `_fcGuiAdapterBound=true` after successful `onAttach`.
- `WidgetBinder::detach(QWidget*)` calls `onDetach` only for previously attached widgets.
- `WidgetFactory::createWidget()` calls `WidgetBinder::attach()` after widget creation.
- `UiLoader::createWidget()` calls `WidgetBinder::attach()` for both native Qt and factory-created widgets.
- Python `loadUi` uses `UiLoader`, so it inherits the same binder attachment behavior.

The binder only coordinates attachment. It does not resolve application services and does not own FreeCAD business logic.
