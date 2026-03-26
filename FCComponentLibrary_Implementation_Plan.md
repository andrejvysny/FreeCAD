# FCComponentLib — Implementation Plan

## Architecture: Logic-Free Widgets + Adapters

### Core Principle

**Widgets are pure Qt. All FreeCAD logic lives in adapters.**

```
src/Gui/Adapters/              ← FreeCAD logic (links FreeCADApp, FreeCADBase, FreeCADGui)
  {Name}Adapter.h/cpp         → bridges widget ↔ FreeCAD data/systems

src/Libs/FCComponentLib/       ← Pure Qt widgets (links Qt6 ONLY)
  Components/{Category}/       → logic-free, testable, reusable
  Tokens/                      → design token system
  Style/                       → component styling engine

src/Libs/FCComponentGallery/   ← Standalone gallery app (links FCComponentLib only)
  Stories/                     → interactive component demos
```

### Dependency Rule

```
FCComponentLib  → Qt6::Core  Qt6::Widgets  Qt6::Svg  (NOTHING ELSE)
FreeCADGui      → FCComponentLib + FreeCADApp + FreeCADBase + Coin3D
Adapters        → live inside FreeCADGui, bridge widget ↔ FreeCAD
```

### Why This Architecture

Analysis of `src/Gui/` revealed 4 types of non-UI logic baked into widgets:

| Logic Type | Example | Where it goes |
|-----------|---------|--------------|
| Expression binding | `ExpressionBinding::bind(ObjectIdentifier)` | `ExpressionAdapter` |
| Parameter persistence | `PrefWidget::savePreferences()` | `PrefAdapter` |
| Selection observation | `TreeWidget::onSelectionChanged()` | `SelectionAdapter` |
| Domain type conversion | `Base::Quantity` as Q_PROPERTY | `QuantityAdapter` |

Widgets expose pure Qt signals/slots. Adapters connect them to FreeCAD systems.

---

## Current State

### What exists now

**FCComponentLib** (`src/Libs/FCComponentLib/`) — 101 files, 20 extracted components:

| Category | Components |
|----------|-----------|
| Buttons/ | FcPushButton, FcSplitButton, FcToolButton, FcColorPicker, FcButtonGroup |
| Inputs/ | FcSpinBox, FcDoubleSpinBox, FcLineEdit, FcCheckBox, FcRadioButton, FcComboBox, FcAccelLineEdit, FcClearLineEdit, FcModifierLineEdit, FcLabelButton, FcLabelEditor, FcElideCheckBox, FcImageTextEdit |
| Display/ | FcUrlLabel, FcCompassDial, FcPropertyListEditor, FcSeparator |
| Containers/ | FcActionSelector, FcCollapsibleGroup |
| Feedback/ | FcColorButton |

**Infrastructure:** Tokens/ (token manager, parser, YAML themes), Style/ (component styling), ComponentRegistry, Gallery app with stories.

**Problem:** Current CMakeLists.txt links `FreeCADBase`:
```cmake
target_link_libraries(FCComponentLib
    PUBLIC Qt6::Core Qt6::Widgets Qt6::Svg FreeCADBase  ← REMOVE
    PRIVATE yaml-cpp::yaml-cpp fmt::fmt
)
```

### What needs to change

1. **Remove `FreeCADBase` dependency** from FCComponentLib
2. **Audit all 20 existing components** for any `Base/` or `App/` includes
3. **Create `src/Gui/Adapters/` directory** for adapter classes
4. **Move any FreeCAD-coupled code** from widgets into adapters
5. **Update CMakeLists.txt** to enforce pure Qt linking

---

## Setup Phase: Clean the Foundation

Before extracting new components, fix the existing setup.

### Task 1: Audit existing components for FreeCAD coupling

```bash
# Find any FreeCAD includes in component files
grep -rn '#include.*\(App/\|Base/\|Gui/\|Inventor/\)' \
  src/Libs/FCComponentLib/Components/
```

For each violation found:
- If the include is used → extract that logic to an adapter
- If the include is unused → remove it

### Task 2: Audit Tokens/ and Style/ for FreeCAD coupling

The Tokens system (TokenManager, Parser, Value) and Style system (ComponentStyle, FcStyle) may reference `Base::Color` or other FreeCAD types.

```bash
grep -rn '#include.*\(App/\|Base/\|Gui/\)' \
  src/Libs/FCComponentLib/Tokens/ \
  src/Libs/FCComponentLib/Style/
```

For each violation:
- Replace `Base::Color` → `QColor`
- Replace `Base::Exception` → `std::runtime_error` or `QString` error
- Remove any `App/` or `Gui/` includes entirely

### Task 3: Remove FreeCADBase from CMakeLists.txt

```cmake
# BEFORE
target_link_libraries(FCComponentLib
    PUBLIC Qt6::Core Qt6::Widgets Qt6::Svg FreeCADBase
    PRIVATE yaml-cpp::yaml-cpp fmt::fmt
)

# AFTER
target_link_libraries(FCComponentLib
    PUBLIC Qt6::Core Qt6::Widgets Qt6::Svg
    PRIVATE yaml-cpp::yaml-cpp fmt::fmt
)
```

### Task 4: Create adapter directory structure

```bash
mkdir -p src/Gui/Adapters
```

Create `src/Gui/Adapters/README.md`:
```markdown
# Gui::Adapters

Bridge classes connecting pure Qt widgets (FCComponentLib) to FreeCAD systems.

Each adapter:
- Lives in namespace `Gui::Adapters`
- Owns a pointer to an FCComponentLib widget
- Connects widget signals to FreeCAD data operations
- Converts FreeCAD types (Base::Quantity, App::Property) to Qt types (double, QString)

Adapters are allowed to include App/, Base/, Gui/, and Coin3D headers.
Widgets are NOT.
```

### Task 5: Verify gallery builds standalone

```bash
cmake --build build/debug --target FCComponentGallery 2>&1 | tail -20
```

The gallery MUST build with only Qt6 + FCComponentLib (no FreeCAD).

### Task 6: Update EXTRACTION_REPORT.md

Update the report to reflect the new architecture:
- Note which existing components are already clean (zero FreeCAD deps)
- Note which need adapter extraction
- Update the "Skipped" section with the new approach (they're no longer permanently skipped — they just need adapters)

---

## Extraction Approach: One Component at a Time

After the setup phase is clean, extract components individually using the `extract-component` skill (`.claude/skills/extract-component/SKILL.md`).

### Priority order (by impact × feasibility)

**Tier 1 — High-impact, need adapters** (unlocks the most downstream changes):

| # | Widget | Coupling | Adapter needed |
|---|--------|----------|---------------|
| 1 | QuantitySpinBox | `Base::Quantity` + `ExpressionBinding` | `QuantityAdapter` + `ExpressionAdapter` |
| 2 | PrefWidgets (14 classes) | `WindowParameter` + `Base::Observer` | `PrefAdapter` (generic, reusable) |
| 3 | StatefulLabel | `Base::Observer` + `ParameterGrp` | `ParameterObserverAdapter` |
| 4 | ProgressBar | `Base::SequencerBase` | `ProgressAdapter` |
| 5 | InputField | `ExpressionWidget` + `Base::Quantity` | Reuses `QuantityAdapter` |

**Tier 2 — Medium-impact, complex adapters:**

| # | Widget | Coupling | Adapter needed |
|---|--------|----------|---------------|
| 6 | TaskBox (QSint replacement) | `QSint::ActionGroup` | None (pure Qt already) |
| 7 | TaskPanel (QSint replacement) | `QSint::ActionPanel` | None |
| 8 | TreeWidget | `SelectionObserver` + `DocumentModel` | `TreeModelAdapter` + `SelectionAdapter` |
| 9 | NotificationArea | `Base::Observer` + logging | `NotificationAdapter` |
| 10 | OverlayWidgets | `OverlayManager` + parameter system | `OverlayAdapter` |

**Tier 3 — Lower impact, specialized:**

| # | Widget | Coupling | Notes |
|---|--------|----------|-------|
| 11 | VectorEditWidget | Uses `Gui::DoubleSpinBox` | Depends on FcDoubleSpinBox |
| 12 | CompassWidget | Uses `Gui::QuantitySpinBox` | Depends on QuantityAdapter |
| 13 | TextEdit/TextEditor | Python CallTips + WindowParameter | Complex adapter |
| 14 | ThemeSelectorWidget | `App::Application` + `Gui::Command` | `ThemeAdapter` |

### Per-component extraction workflow

For each component, invoke: `extract-component <ClassName>`

The skill handles:
1. Coupling analysis
2. Widget creation (pure Qt)
3. Adapter creation (if needed)
4. Gallery story
5. CMake updates
6. Verification

See `.claude/skills/extract-component/SKILL.md` for full process.

---

## Adapter Reference Designs

### QuantityAdapter (most important — used by 67+ .ui files)

```cpp
// src/Gui/Adapters/QuantityAdapter.h
class QuantityAdapter : public QObject
{
    Q_OBJECT
public:
    QuantityAdapter(FcComponents::FcFloatEditor* editor, App::PropertyFloat* prop);

private slots:
    void onWidgetValueChanged(double v);   // widget → property
    void onWidgetTextEdited(const QString& raw);  // parse "3mm + 2cm"
    void onPropertyChanged();              // property → widget

private:
    void updateDisplay();  // reads property, pushes double+suffix to widget
    FcComponents::FcFloatEditor* m_editor;
    App::PropertyFloat* m_property;
    double m_factor = 1.0;
    Base::Unit m_unit;
};
```

### PrefAdapter (generic — replaces 14 PrefWidget subclasses)

```cpp
// src/Gui/Adapters/PrefAdapter.h
class PrefAdapter : public QObject
{
    Q_OBJECT
public:
    // Works with ANY QWidget that has a "value" Q_PROPERTY
    PrefAdapter(QWidget* widget, const QByteArray& paramPath, const QByteArray& entryName);

private:
    void restore();  // reads Base::Parameter → sets widget value
    void save();     // reads widget value → writes Base::Parameter
    void onParameterChanged(const char* reason);

    QWidget* m_widget;
    ParameterGrp::handle m_paramGroup;
    QByteArray m_entryName;
};
```

### ExpressionAdapter (for formula-enabled inputs)

```cpp
// src/Gui/Adapters/ExpressionAdapter.h
class ExpressionAdapter : public QObject
{
    Q_OBJECT
public:
    ExpressionAdapter(QWidget* editor, App::DocumentObject* obj, const char* propName);

    bool hasExpression() const;
    void openExpressionDialog();

private:
    void onExpressionChanged();  // expression evaluated → push result to widget
    App::ObjectIdentifier m_path;
    std::shared_ptr<App::Expression> m_expression;
};
```

---

## Verification Checklist

After setup phase:
- [ ] `grep -rn 'Base/\|App/\|Gui/' src/Libs/FCComponentLib/` returns ZERO matches
- [ ] `FCComponentLib` CMakeLists.txt links only Qt6 (no FreeCADBase)
- [ ] `FCComponentGallery` builds standalone
- [ ] `FreeCADGui` still builds and links `FCComponentLib`
- [ ] `src/Gui/Adapters/` directory exists with README
- [ ] `EXTRACTION_REPORT.md` updated with new architecture notes

After each component extraction:
- [ ] Widget has zero FreeCAD includes
- [ ] Widget Q_PROPERTYs use only Qt types
- [ ] Adapter (if needed) compiles in FreeCADGui
- [ ] Gallery story added and works
- [ ] CMakeLists.txt updated for both libraries
- [ ] EXTRACTION_REPORT.md updated

---

## Risk Register

| Risk | Mitigation |
|------|-----------|
| Removing FreeCADBase breaks Token/Style system | Audit first, replace Base::Color → QColor before removing link |
| Adapters become too complex | Keep adapters thin — if adapter > widget in size, reconsider extraction |
| 341 .ui files reference old widget names | Use typedefs during transition: `using Gui::QuantitySpinBox = ...` |
| Gallery app diverges from FreeCAD appearance | Gallery loads same QSS themes as FreeCAD |
| Widget API doesn't expose enough for adapter | Add signals/slots to widget as needed — keep them Qt-typed |
