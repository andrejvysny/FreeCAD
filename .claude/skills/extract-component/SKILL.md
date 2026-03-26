---
name: extract-component
description: Extract a single Qt widget from src/Gui/ into FCComponentLib as a logic-free component, creating an adapter in src/Gui/Adapters/ if the original has FreeCAD coupling
argument-hint: "<OriginalClassName> [--source <path>]"
allowed-tools: Read, Grep, Glob, Bash, Edit, Write
user-invocable: true
agent: qt-gui-expert
---

# Extract Component to FCComponentLib

Extracts ONE widget at a time from `src/Gui/` (or `src/Mod/*/Gui/`) into `src/Libs/FCComponentLib/` as a **logic-free pure Qt widget**, creating an adapter in `src/Gui/Adapters/` when FreeCAD coupling exists.

## Arguments

- `$0` -- OriginalClassName: The class to extract (e.g., `StatefulLabel`, `QuantitySpinBox`, `ProgressBar`)
- `--source <path>` -- optional explicit source file path

## Hard Rules

1. **The extracted widget MUST have zero FreeCAD dependencies.** It links only `Qt6::Core`, `Qt6::Widgets`, `Qt6::Gui`, `Qt6::Svg`.
2. **No `#include` from `App/`, `Base/`, `Gui/`, or Coin3D** in any widget file.
3. **No FreeCAD types in Q_PROPERTY** — use `double`, `QString`, `QColor`, `int`, `bool`, `QVariant`.
4. **All FreeCAD logic goes into an adapter** in `src/Gui/Adapters/`.
5. **Every widget gets a gallery story** in `src/Libs/FCComponentGallery/Stories/`.
6. **Follow existing conventions**: `FcComponents` namespace, `FCComponentLibExport` macro, `FC_REGISTER_COMPONENT`.

## Step-by-Step Process

### Step 1: Analyze the original

Read the original widget source and classify every member as one of:

| Category | Stays in widget? | Examples |
|----------|-----------------|---------|
| **PURE UI** — layout, painting, animation, QSS | YES | `paintEvent()`, `sizeHint()`, stylesheets |
| **DATA DISPLAY** — showing values from Qt types | YES | `setValue(double)`, `setText(QString)` |
| **USER INPUT** — capturing user actions as Qt signals | YES | `valueChanged(double)`, `textEdited(QString)` |
| **BUSINESS LOGIC** — domain computation, parsing, validation | NO → adapter | `Base::Quantity::parse()`, unit conversion |
| **FRAMEWORK COUPLING** — FreeCAD systems | NO → adapter | `ExpressionBinding`, `PrefWidget`, `SelectionObserver` |
| **DATA BINDING** — connecting to FreeCAD data | NO → adapter | `App::Property` access, `ParameterGrp` |

Create a coupling report:

```
## Coupling Analysis: <ClassName>
Source: <file path>
FreeCAD includes: <count>
  - <header>: used for <what>
  - ...
Verdict: <CLEAN | NEEDS_ADAPTER | CANNOT_EXTRACT>
Adapter needed: <yes/no>
Adapter responsibilities: <list>
```

### Step 2: Determine target category

Map the widget to one of the existing FCComponentLib categories:

| Category | For |
|----------|-----|
| `Buttons/` | Clickable actions: push buttons, split buttons, tool buttons, toggles |
| `Inputs/` | Data entry: spinboxes, line edits, checkboxes, radio buttons, combos |
| `Display/` | Read-only display: separators, status indicators, dials |
| `Containers/` | Grouping: collapsible sections, action panels, tab containers |
| `Feedback/` | User feedback: color pickers, progress bars, notifications |
| `Labels/` | Text display: url labels, stateful labels, formatted labels |
| `Selectors/` | Selection widgets: action selectors, list editors |

### Step 3: Create the pure Qt widget

Create files in `src/Libs/FCComponentLib/Components/{Category}/`:

**Header** (`Fc{Name}.h`):
```cpp
// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once

#include <QWidget>  // or appropriate Qt base

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

class FCComponentLibExport Fc{Name} : public Q{Base}
{
    Q_OBJECT
    // Q_PROPERTY using ONLY Qt types: double, int, bool, QString, QColor, QVariant
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)

public:
    explicit Fc{Name}(QWidget* parent = nullptr);

    // Pure Qt API — no FreeCAD types in signatures
    double value() const;
    void setValue(double v);

signals:
    void valueChanged(double newValue);
    // Expose raw user input for adapters that need to parse it
    void textEdited(const QString& rawInput);

protected:
    // UI-only overrides
    void paintEvent(QPaintEvent* event) override;

private:
    // Only Qt types as members
    double m_value = 0.0;
};

}  // namespace FcComponents
```

**Implementation** (`Fc{Name}.cpp`):
```cpp
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "Fc{Name}.h"

#include <FCComponentLib/Components/ComponentMeta.h>

FC_REGISTER_COMPONENT(Fc{Name}, "{Category}", "Brief description")

namespace FcComponents
{
// Implementation with ZERO FreeCAD includes
}
```

**Key refactoring patterns:**

| Original pattern | Widget replacement |
|-----------------|-------------------|
| `Base::Quantity value` Q_PROPERTY | `double value` + `QString suffix` Q_PROPERTY |
| `ExpressionBinding::bind()` | `void setExpressionIndicator(bool, QString)` — visual only |
| `PrefWidget::restorePreferences()` | Remove — adapter calls `setValue()` after reading prefs |
| `Base::Observer<const char*>` | Remove — adapter observes and pushes changes |
| `Gui::Selection` observation | Remove — adapter observes and updates model |
| `App::Property` access | Remove — adapter reads property, calls `setValue()` |
| `Base::Console::PrintMessage()` | Remove or use `qDebug()` for debug only |

### Step 4: Create adapter (if needed)

If the coupling analysis found FreeCAD dependencies, create an adapter in `src/Gui/Adapters/`:

**Header** (`src/Gui/Adapters/{Name}Adapter.h`):
```cpp
// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once

#include <QObject>

// FreeCAD includes are allowed HERE
#include <App/Property.h>
#include <Base/Quantity.h>

namespace FcComponents { class Fc{Name}; }

namespace Gui::Adapters
{

class GuiExport {Name}Adapter : public QObject
{
    Q_OBJECT

public:
    {Name}Adapter(FcComponents::Fc{Name}* widget, /* FreeCAD data source */);

private slots:
    void onWidgetValueChanged(double newValue);  // widget → FreeCAD
    void onPropertyChanged();                     // FreeCAD → widget

private:
    FcComponents::Fc{Name}* m_widget;
    // FreeCAD-specific members
};

}  // namespace Gui::Adapters
```

**Adapter responsibilities checklist:**
- [ ] Convert FreeCAD types → Qt types for widget display
- [ ] Convert Qt types → FreeCAD types for data write-back
- [ ] Handle expression binding (if applicable)
- [ ] Handle parameter persistence (if applicable)
- [ ] Handle selection observation (if applicable)
- [ ] Handle validation and parsing (if applicable)

### Step 5: Create gallery story

Add a story to the appropriate file in `src/Libs/FCComponentGallery/Stories/`:

```cpp
// In the appropriate *Stories.cpp file
static void register{Name}Stories()
{
    StoryRegistry::instance().add({
        "Fc{Name}",
        "{Category}",
        "Default",
        "Shows Fc{Name} in default configuration",
        [](QWidget* parent) -> QWidget* {
            auto* w = new FcComponents::Fc{Name}(parent);
            // Configure with representative data
            return w;
        }
    });

    // Add variant stories (disabled state, different configs, etc.)
}
```

### Step 6: Update CMakeLists.txt

**FCComponentLib** (`src/Libs/FCComponentLib/CMakeLists.txt`):
```cmake
# Add to FCComponentLib_Widgets_SRCS
Components/{Category}/Fc{Name}.h
Components/{Category}/Fc{Name}.cpp
```

**FreeCADGui** (`src/Gui/CMakeLists.txt`) — only if adapter created:
```cmake
# Add to appropriate source list
Adapters/{Name}Adapter.h
Adapters/{Name}Adapter.cpp
```

### Step 7: Verify

Run these checks:

1. **Dependency check** — confirm no FreeCAD includes in widget:
   ```bash
   grep -rn '#include.*\(App/\|Base/\|Gui/\|Inventor/\|Coin3D\)' \
     src/Libs/FCComponentLib/Components/{Category}/Fc{Name}.*
   ```
   Expected: zero matches.

2. **Build check** — if build is configured:
   ```bash
   cmake --build build/debug --target FCComponentLib 2>&1 | tail -20
   ```

3. **Gallery check** — gallery app still builds:
   ```bash
   cmake --build build/debug --target FCComponentGallery 2>&1 | tail -20
   ```

4. **FreeCAD check** — if adapter was created, full build:
   ```bash
   cmake --build build/debug --target FreeCADGui 2>&1 | tail -20
   ```

### Step 8: Update tracking documents

Add the component to `src/Libs/FCComponentLib/EXTRACTION_REPORT.md`:

```markdown
| Fc{Name} | {Category}/ | src/Gui/{Original}.h | {coupling changes} | {story count} | Done |
```

If an adapter was created, note it:
```markdown
Adapter: src/Gui/Adapters/{Name}Adapter.h/cpp
```

## Decision Guide: When NOT to extract

Do not extract if:
- Widget is a dialog/window (not a reusable component)
- Widget is Coin3D-only (`SoFC*` nodes, `View3DInventor*`)
- Coupling is so deep the adapter would be larger than the widget
- Widget is only used in one place and is highly specialized

## Reference: Existing Components

Check `src/Libs/FCComponentLib/EXTRACTION_REPORT.md` for what's already extracted.
Check `src/Libs/FCComponentLib/PLAN.md` for candidates with coupling scores.

## Reference: Coupling Scores by Widget

| Widget | Source | Coupling | Blocker |
|--------|--------|----------|---------|
| QSint widgets | src/Gui/QSint/ | 0 | None |
| SplitButton, ElideCheckBox | src/Gui/ | 0 | None |
| UrlLabel, ActionSelector | src/Gui/Widgets.h | 0 | None |
| StatefulLabel | src/Gui/Widgets.h | 6 | `Base::Observer` + `ParameterGrp` |
| QuantitySpinBox | src/Gui/QuantitySpinBox.h | 8 | `Base::Quantity` + `ExpressionBinding` |
| PrefWidgets (14) | src/Gui/PrefWidgets.h | 6 | `WindowParameter` + `Base::Observer` |
| TreeWidget | src/Gui/Tree.h | 9 | `SelectionObserver` + document model |
| ProgressBar | src/Gui/ProgressBar.h | 7 | `Base::SequencerBase` |
| InputField | src/Gui/InputField.h | 8 | `ExpressionWidget` + `Base::Quantity` |
