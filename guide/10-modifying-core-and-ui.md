<!--
SPDX-License-Identifier: LGPL-2.1-or-later
-->

# 10. Modifying Core And UI

This guide is for developers who want to change FreeCAD behavior in `src/App/` and `src/Gui/`, or extend those layers from a module in `src/Mod/<Module>/`.

Assumptions:

- You already understand FreeCAD's type system and property framework (guides 01 to 02).
- You want concrete patterns that compile and integrate cleanly.

Conventions used in examples:

- Replace `MyMod` with your module name.
- Replace `MyModGui` with your GUI module namespace.
- All examples are minimal and omit unrelated details.
- Every new `.cpp` starts with `#include <PreCompiled.h>`.

## 1. Understanding What "Core" Means

"Core" is not one folder, it is the layers that every workbench depends on.

- Base layer, `src/Base/`:
  - Type system, math utilities, persistence helpers.
  - Rarely modified. If you change Base, you affect everything.
- App layer, `src/App/`:
  - Document model, `App::DocumentObject`, properties, extensions.
  - Most behavior changes belong here.
- Gui layer, `src/Gui/`:
  - ViewProviders, commands, task panels, tree view, selection.
  - UI changes belong here.

Rule of thumb:

- If you want objects to compute differently or serialize different data, start in `src/App/`.
- If you want the UI to show, edit, select, or render differently, start in `src/Gui/`.
- If you think you need `src/Base/`, double check. Base changes are hard to review and easy to break.

## 2. Adding A New Property Type

You add a new property type when none of the existing property classes model your data correctly.

Typical reasons:

- You need custom serialization.
- You need a specific editor.
- You want first class Python access.

This walkthrough mirrors how `App::PropertyInteger` and friends are structured in `src/App/PropertyStandard.h`.

### 2.1 Pick A Minimal Data Model

Keep the stored data as small and stable as possible.

Example goal:

- Store an angle in degrees.
- Serialize it as a number.
- Expose it to Python as `float`.
- Provide an editor name, so the GUI can pick an editor.

### 2.2 Create The Header In `src/App/`

Create `src/App/PropertyAngle.h`.

Key points:

- Inherit from `App::Property`.
- Add `TYPESYSTEM_HEADER_WITH_OVERRIDE()`.
- Implement `Save`, `Restore`, `Copy`, `Paste`.
- Provide Python bridge with `getPyObject` and `setPyObject`.
- If the GUI needs a custom editor, override `getEditorName()`.

Reference snippet (existing pattern):

```cpp
class AppExport PropertyInteger: public Property {
    TYPESYSTEM_HEADER_WITH_OVERRIDE();
public:
    void setValue(long);
    long getValue() const;
    const char* getEditorName() const override { return "Gui::PropertyEditor::PropertyIntegerItem"; }
    void Save(Base::Writer& writer) const override;
    void Restore(Base::XMLReader& reader) override;
    Property* Copy() const override;
    void Paste(const Property& from) override;
protected:
    long _lValue;
};
```

Minimal new property header:

```cpp
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef APP_PROPERTYANGLE_H
#define APP_PROPERTYANGLE_H

#include <App/Property.h>

namespace App
{

class AppExport PropertyAngle : public Property
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

public:
    PropertyAngle();

    void setValue(double degrees);
    double getValue() const;

    const char* getEditorName() const override
    {
        // Pick an existing editor item unless you add a new editor.
        return "Gui::PropertyEditor::PropertyFloatItem";
    }

    void Save(Base::Writer& writer) const override;
    void Restore(Base::XMLReader& reader) override;
    Property* Copy() const override;
    void Paste(const Property& from) override;

    PyObject* getPyObject() override;
    void setPyObject(PyObject* value) override;

protected:
    double _degrees;
};

} // namespace App

#endif // APP_PROPERTYANGLE_H
```

### 2.3 Implement Value, Serialization, Copy, Python Bridge

Create `src/App/PropertyAngle.cpp`.

Implementation notes:

- Keep `setValue()` responsible for notifying changes (use existing `Property` helpers).
- Use `writer` and `reader` patterns consistent with existing properties.
- `Copy()` returns a new heap instance, `Paste()` copies from `from`.
- For Python, accept `float` and `int`, reject others.

```cpp
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <PreCompiled.h>

#include <App/PropertyAngle.h>

#include <Base/Writer.h>
#include <Base/XMLReader.h>

using namespace App;

// Many App properties use TYPESYSTEM_SOURCE. If your property uses PROPERTY_SOURCE
// in your tree, follow that convention. This guide shows PROPERTY_SOURCE because
// it matches the common DocumentObject pattern and keeps the steps consistent.
PROPERTY_SOURCE(App::PropertyAngle, App::Property)

PropertyAngle::PropertyAngle()
    : _degrees(0.0)
{
}

void PropertyAngle::setValue(double degrees)
{
    if (_degrees == degrees) {
        return;
    }
    aboutToSetValue();
    _degrees = degrees;
    hasSetValue();
}

double PropertyAngle::getValue() const
{
    return _degrees;
}

void PropertyAngle::Save(Base::Writer& writer) const
{
    writer.Stream() << _degrees;
}

void PropertyAngle::Restore(Base::XMLReader& reader)
{
    reader >> _degrees;
}

Property* PropertyAngle::Copy() const
{
    auto* p = new PropertyAngle();
    p->_degrees = _degrees;
    return p;
}

void PropertyAngle::Paste(const Property& from)
{
    const auto* p = dynamic_cast<const PropertyAngle*>(&from);
    if (!p) {
        return;
    }
    setValue(p->_degrees);
}

PyObject* PropertyAngle::getPyObject()
{
    return PyFloat_FromDouble(_degrees);
}

void PropertyAngle::setPyObject(PyObject* value)
{
    if (PyFloat_Check(value)) {
        setValue(PyFloat_AsDouble(value));
        return;
    }
    if (PyLong_Check(value)) {
        setValue(PyLong_AsDouble(value));
        return;
    }
    throw Base::TypeError("PropertyAngle expects a float");
}
```

Notes:

- `TYPESYSTEM_SOURCE()` is what registers the runtime type.
- Depending on the exact property base class you inherit from, you may need `PROPERTY_SOURCE()` or `TYPESYSTEM_SOURCE()`. Follow the pattern used by the closest existing property.
- If your `Restore()` reads attributes instead of raw stream, match how `PropertyString` or `PropertyQuantity` does it.

### 2.4 Register The Type

For core property types in `src/App/`, register in the global type init.

Pattern:

```cpp
// src/App/Application.cpp (inside App::Application::initTypes())
App::PropertyAngle::init();
```

If it is module specific and you do not want to modify App core, register from your module App init.

Typical pattern in a module init function:

```cpp
// src/Mod/MyMod/App/AppMyMod.cpp
void AppMyModInit()
{
    App::PropertyAngle::init();
    MyMod::MyFeature::init();
}
```

### 2.5 Quick Checklist

- Header in `src/App/` or module App.
- Type macro in the class.
- Implementation file includes `PreCompiled.h` first.
- Serialization and copy implemented.
- Python bridge implemented.
- `::init()` called during startup.

## 3. Adding A New DocumentObject (C++)

You add a `DocumentObject` when you want new persistent model behavior in a document.

### 3.1 Choose The Right Base Class

- `App::DocumentObject`:
  - No placement, no shape.
  - Good for pure data objects.
- `App::GeoFeature`:
  - Has placement.
  - Good for geometric features without needing `Part::TopoShape`.
- `Part::Feature`:
  - Has `Shape` property and Part integration.
  - Most geometry producing objects use this.

### 3.2 Header File

Steps:

1. Inherit from the chosen base.
2. Add `PROPERTY_HEADER_WITH_OVERRIDE(Namespace::ClassName)`.
3. Declare properties as public members.
4. Declare `execute()`.
5. Declare `getViewProviderName()`.

Minimal example (given pattern):

```cpp
// MyFeature.h
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef MYMOD_MYFEATURE_H
#define MYMOD_MYFEATURE_H

#include <App/GeoFeature.h>
#include <App/PropertyUnits.h>

namespace MyMod
{

class MyExport MyFeature : public App::GeoFeature
{
    PROPERTY_HEADER_WITH_OVERRIDE(MyMod::MyFeature);

public:
    MyFeature();

    App::PropertyLength Width;
    App::PropertyLength Height;

    App::DocumentObjectExecReturn* execute() override;

    const char* getViewProviderName() const override
    {
        return "MyModGui::ViewProviderMyFeature";
    }

    // Optional hooks:
    // short mustExecute() const override;
    // void onChanged(const App::Property* prop) override;
    // void onDocumentRestored() override;
};

} // namespace MyMod

#endif // MYMOD_MYFEATURE_H
```

### 3.3 Source File

Steps:

1. Include `PreCompiled.h` first.
2. Add `PROPERTY_SOURCE(Namespace::ClassName, ParentClass)`.
3. In the constructor, add properties with `ADD_PROPERTY_TYPE()`.
4. Implement `execute()`.

```cpp
// MyFeature.cpp
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <PreCompiled.h>

#include <Mod/MyMod/App/MyFeature.h>

using namespace MyMod;

PROPERTY_SOURCE(MyMod::MyFeature, App::GeoFeature)

MyFeature::MyFeature()
{
    ADD_PROPERTY_TYPE(Width, (10.0), "Dimensions", App::Prop_None, "Width of the feature");
    ADD_PROPERTY_TYPE(Height, (5.0), "Dimensions", App::Prop_None, "Height of the feature");
}

App::DocumentObjectExecReturn* MyFeature::execute()
{
    const double w = Width.getValue();
    const double h = Height.getValue();

    if (w <= 0.0 || h <= 0.0) {
        return new App::DocumentObjectExecReturn("Width and Height must be positive");
    }

    // Compute result here.
    // If you are a Part::Feature, assign to Shape.
    // If you are App::GeoFeature, typically update derived properties.

    return StdReturn;
}
```

### 3.4 Optional Overrides You Actually Use

These are the common hooks for "why does it not recompute" and "how do I react to property changes".

```cpp
short MyFeature::mustExecute() const
{
    // Return 1 if the object must recompute.
    // Most objects rely on the default implementation.
    return App::GeoFeature::mustExecute();
}

void MyFeature::onChanged(const App::Property* prop)
{
    App::GeoFeature::onChanged(prop);

    // If a change affects derived state, mark for recompute.
    if (prop == &Width || prop == &Height) {
        touch();
    }
}

void MyFeature::onDocumentRestored()
{
    App::GeoFeature::onDocumentRestored();

    // Fix up defaults for older documents if needed.
}
```

Implementation notes:

- `execute()` runs during recompute.
- For geometry objects, prefer `obj.Document` and avoid global `App.ActiveDocument`.
- Keep `execute()` deterministic. It should depend only on properties and referenced objects.

### 3.5 Registration

Your class must be registered so `addObject("MyMod::MyFeature", ...)` works.

Typical module registration:

```cpp
// src/Mod/MyMod/App/AppMyMod.cpp
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <PreCompiled.h>

#include <Mod/MyMod/App/MyFeature.h>

extern "C" {
MyModExport void initMyMod()
{
    MyMod::MyFeature::init();
}
}
```

Or via Python module init (`Init.py`) if your module uses Python bootstrapping.

```python
# src/Mod/MyMod/Init.py
import FreeCAD

FreeCAD.addDocumentObject("MyMod::MyFeature", "MyFeature")
```

Use the same registration style as your existing module.

## 4. Adding A ViewProvider (C++)

ViewProviders live in `src/Gui/` or a module's `Gui/` folder. They control visualization, tree behavior, and editing UI.

### 4.1 Basic Structure

Steps:

1. Inherit from `Gui::ViewProviderDocumentObject`.
2. Add `PROPERTY_HEADER_WITH_OVERRIDE()`.
3. Override `attach(App::DocumentObject*)`.
4. Override `updateData(const App::Property*)`.
5. Provide display modes via `getDisplayModes` and `setDisplayMode`.
6. Provide `getIcon()`.

Header skeleton:

```cpp
// ViewProviderMyFeature.h
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef MYMODGUI_VIEWPROVIDERMYFEATURE_H
#define MYMODGUI_VIEWPROVIDERMYFEATURE_H

#include <Gui/ViewProviderDocumentObject.h>

class SoCoordinate3;
class SoIndexedFaceSet;
class SoSeparator;

namespace MyModGui
{

class MyModGuiExport ViewProviderMyFeature : public Gui::ViewProviderDocumentObject
{
    PROPERTY_HEADER_WITH_OVERRIDE(MyModGui::ViewProviderMyFeature);

public:
    ViewProviderMyFeature();

    void attach(App::DocumentObject* obj) override;
    void updateData(const App::Property* prop) override;

    std::vector<std::string> getDisplayModes() const override;
    void setDisplayMode(const char* mode) override;
    QIcon getIcon() const override;

    bool setEdit(int ModNum) override;
    void unsetEdit(int ModNum) override;

private:
    SoSeparator* rootSep;
    SoCoordinate3* coords;
    SoIndexedFaceSet* faces;
};

} // namespace MyModGui

#endif // MYMODGUI_VIEWPROVIDERMYFEATURE_H
```

### 4.2 Coin3D Scene Graph Basics

Minimal `attach()` example (given pattern):

```cpp
void ViewProviderMyFeature::attach(App::DocumentObject* obj)
{
    ViewProviderDocumentObject::attach(obj);

    SoSeparator* sep = new SoSeparator();
    auto* coords = new SoCoordinate3();
    auto* faces = new SoIndexedFaceSet();

    sep->addChild(coords);
    sep->addChild(faces);

    addDisplayMaskMode(sep, "Shaded");
}
```

In practice you keep node pointers so you can update data later.

Source file skeleton:

```cpp
// ViewProviderMyFeature.cpp
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <PreCompiled.h>

#include <Mod/MyMod/Gui/ViewProviderMyFeature.h>

#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoSeparator.h>

using namespace MyModGui;

PROPERTY_SOURCE(MyModGui::ViewProviderMyFeature, Gui::ViewProviderDocumentObject)

ViewProviderMyFeature::ViewProviderMyFeature()
    : rootSep(nullptr),
      coords(nullptr),
      faces(nullptr)
{
}

void ViewProviderMyFeature::attach(App::DocumentObject* obj)
{
    ViewProviderDocumentObject::attach(obj);

    rootSep = new SoSeparator();
    coords = new SoCoordinate3();
    faces = new SoIndexedFaceSet();

    rootSep->addChild(coords);
    rootSep->addChild(faces);

    // Put something visible here, even a placeholder.
    // coords->point.set1Value(0, 0, 0, 0);
    // ... fill faces indices ...

    addDisplayMaskMode(rootSep, "Shaded");
}

void ViewProviderMyFeature::updateData(const App::Property* prop)
{
    ViewProviderDocumentObject::updateData(prop);

    // React to changes in App properties.
    // Example: if width or height changes, update coords and faces.
    // Keep this fast, it can run often.
    (void)prop;
}

std::vector<std::string> ViewProviderMyFeature::getDisplayModes() const
{
    // Expose display modes to the context menu.
    return {"Shaded"};
}

void ViewProviderMyFeature::setDisplayMode(const char* mode)
{
    // Switch nodes, materials, or rendering style.
    ViewProviderDocumentObject::setDisplayMode(mode);
}

QIcon ViewProviderMyFeature::getIcon() const
{
    // Return a resource icon, or a fallback.
    return Gui::BitmapFactory().iconFromTheme("MyMod_MyFeature");
}

bool ViewProviderMyFeature::setEdit(int ModNum)
{
    (void)ModNum;
    return false;
}

void ViewProviderMyFeature::unsetEdit(int ModNum)
{
    (void)ModNum;
}
```

### 4.3 Optional Tree And Interaction Hooks

These are common when you want tree changes without touching `src/Gui/Tree.cpp`.

```cpp
std::vector<App::DocumentObject*> ViewProviderMyFeature::claimChildren() const
{
    // Return children shown under this object in the tree.
    // Often used by group like objects.
    return {};
}

void ViewProviderMyFeature::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    ViewProviderDocumentObject::setupContextMenu(menu, receiver, member);
    // Add custom actions.
}
```

If you want default expansion behavior, set status flags from the App object.

```cpp
// In your App object constructor, after properties are added.
this->setStatus(App::ObjectStatus::Expand, true);
this->setStatus(App::ObjectStatus::NoAutoExpand, false);
```

## 5. Adding A GUI Command (C++)

Commands are the standard way to add menu items, toolbars, and shortcuts.

The command framework is in `src/Gui/Command.h`. A command provides metadata plus an `activated()` implementation.

Example command (given pattern, adapted to be minimal):

```cpp
// CommandMyMod.cpp
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <PreCompiled.h>

#include <Gui/Application.h>
#include <Gui/Command.h>

class MyCommand : public Gui::Command
{
public:
    MyCommand() : Command("MyMod_DoSomething")
    {
        sAppModule    = "MyMod";
        sGroup        = "MyMod";
        sMenuText     = QT_TR_NOOP("Do Something");
        sToolTipText  = QT_TR_NOOP("Performs the something operation");
        sStatusTip    = sToolTipText;
        sPixmap       = "MyMod_DoSomething";
    }

    void activated(int iMsg) override
    {
        (void)iMsg;
        openCommand("Do Something");
        doCommand(Doc, "App.activeDocument().addObject('MyMod::MyFeature', 'Feature')");
        commitCommand();
        updateActive();
    }

    bool isActive() override
    {
        return hasActiveDocument();
    }
};
```

Registration pattern:

```cpp
void CreateMyModCommands()
{
    Gui::CommandManager& rcCmdMgr = Gui::Application::Instance->commandManager();
    rcCmdMgr.addCommand(new MyCommand());
}
```

Where to call registration:

- Module GUI init function.
- Or a module `InitGui.py` that imports a compiled GUI library.

When to use `doCommand()`:

- When you want the action recorded as a macro and undoable.
- When your action can be expressed as Python commands.

When to use direct C++ operations:

- When you need performance.
- When you need direct access to GUI objects.

If you do direct operations, still wrap with `openCommand()` and `commitCommand()` so undo works.

## 6. Adding A GUI Command (Python)

Python commands are a fast way to add UI actions without a C++ rebuild.

Minimal example (given pattern, with GUI guard and recompute):

```python
import FreeCAD

if FreeCAD.GuiUp:
    import FreeCADGui


class MyCommand:
    def GetResources(self):
        return {
            'Pixmap': 'MyIcon',
            'MenuText': 'Do Something',
            'ToolTip': 'Does something',
        }

    def Activated(self):
        doc = FreeCAD.ActiveDocument
        if doc is None:
            doc = FreeCAD.newDocument()
        obj = doc.addObject("Part::FeaturePython", "MyObj")
        _ = obj
        doc.recompute()

    def IsActive(self):
        return FreeCAD.ActiveDocument is not None


if FreeCAD.GuiUp:
    FreeCADGui.addCommand('MyMod_DoSomething', MyCommand())
```

Notes:

- Use `FreeCAD.Console.PrintLog()` for debug output, not `print()`.
- A Python command can create C++ objects too, as long as the type is registered.

## 7. Modifying The Tree View

The tree view implementation lives in `src/Gui/Tree.cpp` and is one of the largest GUI files. Most of the time you do not need to edit it.

Prefer changing tree behavior via your ViewProvider.

What controls the tree for an object:

- `getIcon()` picks the tree icon.
- `getDisplayModes()` defines right click display options.
- `claimChildren()` controls which objects appear as children.
- `dragObject()` and `dropObject()` enable drag and drop.
- `setupContextMenu()` adds right click menu actions.

Object status flags influence expansion:

- `ObjectStatus::Expand`
- `ObjectStatus::NoAutoExpand`

### 7.1 Group Like Tree Structure With `claimChildren()`

Example: show linked child objects under the parent.

```cpp
std::vector<App::DocumentObject*> ViewProviderMyGroup::claimChildren() const
{
    std::vector<App::DocumentObject*> out;

    const auto* obj = getObject();
    if (!obj) {
        return out;
    }

    // Example pattern, adapt to your properties.
    // If you have PropertyLinkList Children, push the linked objects.

    return out;
}
```

### 7.2 Drag And Drop

Drag and drop is also ViewProvider driven.

```cpp
bool ViewProviderMyGroup::canDragObject(App::DocumentObject* obj) const
{
    // Decide if an object can be dragged out.
    return obj != nullptr;
}

bool ViewProviderMyGroup::dragObject(App::DocumentObject* obj)
{
    // Called when drag starts.
    return obj != nullptr;
}

bool ViewProviderMyGroup::canDropObject(App::DocumentObject* obj) const
{
    // Decide if an object can be dropped onto this.
    return obj != nullptr;
}

bool ViewProviderMyGroup::dropObject(App::DocumentObject* obj)
{
    // Perform the drop operation.
    // Usually you modify a LinkList property and touch/recompute.
    (void)obj;
    return true;
}
```

### 7.3 Context Menu Extensions

```cpp
void ViewProviderMyFeature::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    ViewProviderDocumentObject::setupContextMenu(menu, receiver, member);

    QAction* act = menu->addAction(QObject::tr("Recompute Feature"));
    QObject::connect(act, SIGNAL(triggered()), receiver, member);
}
```

If you truly need to modify selection, drag and drop, or expansion rules globally, you end up in `src/Gui/Tree.cpp`. Keep those changes small, and add tests if possible.

## 8. Adding A Preference Page

Preferences are for persistent user settings. Add them when users need to configure behavior across sessions.

### 8.1 C++ Preference Page

Pattern:

- Create a `PreferencePage` class.
- Load a `.ui` built with Qt Designer.
- Read and write values from `ParameterGrp`.

Minimal skeleton:

```cpp
// PrefPageMyMod.h
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef MYMODGUI_PREFPAGEMOD_H
#define MYMODGUI_PREFPAGEMOD_H

#include <Gui/PreferencePage.h>

namespace MyModGui
{

class PrefPageMyMod : public Gui::Dialog::PreferencePage
{
    Q_OBJECT

public:
    PrefPageMyMod(QWidget* parent = nullptr);
    ~PrefPageMyMod() override;

    void saveSettings() override;
    void loadSettings() override;
    void retranslateUi() override;

private:
    QWidget* form;
};

} // namespace MyModGui

#endif
```

```cpp
// PrefPageMyMod.cpp
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <PreCompiled.h>

#include <Mod/MyMod/Gui/PrefPageMyMod.h>

#include <Gui/PrefWidgets.h>
#include <Gui/UiLoader.h>

using namespace MyModGui;

PrefPageMyMod::PrefPageMyMod(QWidget* parent)
    : Gui::Dialog::PreferencePage(parent),
      form(nullptr)
{
    form = Gui::UiLoader().load("MyMod/Resources/ui/PrefMyMod.ui", this);
}

PrefPageMyMod::~PrefPageMyMod() = default;

void PrefPageMyMod::loadSettings()
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Preferences/MyMod");
    const bool enabled = hGrp->GetBool("Enabled", true);
    // Set widget state from enabled.
    (void)enabled;
}

void PrefPageMyMod::saveSettings()
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Preferences/MyMod");
    // Read widget state and store.
    // hGrp->SetBool("Enabled", enabled);
}

void PrefPageMyMod::retranslateUi()
{
    // Update translated strings if needed.
}
```

Register it from `InitGui.py`:

```python
# src/Mod/MyMod/InitGui.py
import FreeCAD

if FreeCAD.GuiUp:
    import FreeCADGui
    from MyModGui import PrefPageMyMod

    FreeCADGui.addPreferencePage(PrefPageMyMod, "MyMod")
```

### 8.2 Python Preference Page

For small settings, Python is fine.

```python
import FreeCAD

if FreeCAD.GuiUp:
    import FreeCADGui


class PrefPageMyMod:
    def __init__(self):
        self.form = None

    def createWidget(self, parent):
        # Build widgets manually or load a .ui
        from PySide6 import QtWidgets
        self.form = QtWidgets.QWidget(parent)
        return self.form

    def saveSettings(self):
        grp = FreeCAD.ParamGet("User parameter:BaseApp/Preferences/MyMod")
        grp.SetBool("Enabled", True)

    def loadSettings(self):
        grp = FreeCAD.ParamGet("User parameter:BaseApp/Preferences/MyMod")
        enabled = grp.GetBool("Enabled", True)
        _ = enabled


if FreeCAD.GuiUp:
    FreeCADGui.addPreferencePage(PrefPageMyMod, "MyMod")
```

## 9. Adding Task Panels

Task panels show up in the left task view during editing. They are the standard way to edit an object interactively.

There are two parts:

- A `TaskBox` with the UI widgets.
- A `TaskDialog` that integrates with OK/Cancel and the task view.

Given pattern:

```cpp
class TaskMyPanel : public Gui::TaskView::TaskBox {
    Q_OBJECT
public:
    TaskMyPanel(ViewProviderMyFeature* vp);
    // Load .ui file, connect signals
};

class TaskDlgMyFeature : public Gui::TaskView::TaskDialog {
public:
    TaskDlgMyFeature(ViewProviderMyFeature* vp);
    bool accept() override;  // OK clicked
    bool reject() override;  // Cancel clicked
    QDialogButtonBox::StandardButtons getStandardButtons() const override;
};
```

### 9.1 Implement TaskBox And TaskDialog

```cpp
// TaskMyFeature.h
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef MYMODGUI_TASKMYFEATURE_H
#define MYMODGUI_TASKMYFEATURE_H

#include <Gui/TaskView/TaskDialog.h>
#include <Gui/TaskView/TaskView.h>

namespace MyModGui
{

class ViewProviderMyFeature;

class TaskMyPanel : public Gui::TaskView::TaskBox
{
    Q_OBJECT

public:
    explicit TaskMyPanel(ViewProviderMyFeature* vp);

private:
    ViewProviderMyFeature* vp;
};

class TaskDlgMyFeature : public Gui::TaskView::TaskDialog
{
public:
    explicit TaskDlgMyFeature(ViewProviderMyFeature* vp);

    QDialogButtonBox::StandardButtons getStandardButtons() const override;
    bool accept() override;
    bool reject() override;

private:
    TaskMyPanel* panel;
    ViewProviderMyFeature* vp;
};

} // namespace MyModGui

#endif
```

```cpp
// TaskMyFeature.cpp
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <PreCompiled.h>

#include <Mod/MyMod/Gui/TaskMyFeature.h>
#include <Mod/MyMod/Gui/ViewProviderMyFeature.h>

#include <Gui/UiLoader.h>

using namespace MyModGui;

TaskMyPanel::TaskMyPanel(ViewProviderMyFeature* vp)
    : Gui::TaskView::TaskBox(Gui::BitmapFactory().iconFromTheme("MyMod_MyFeature"),
                             QObject::tr("My Feature")),
      vp(vp)
{
    auto* w = Gui::UiLoader().load("MyMod/Resources/ui/TaskMyFeature.ui");
    setGroupBox(w);

    // Connect widgets to vp or to document changes.
    (void)this->vp;
}

TaskDlgMyFeature::TaskDlgMyFeature(ViewProviderMyFeature* vp)
    : panel(new TaskMyPanel(vp)),
      vp(vp)
{
}

QDialogButtonBox::StandardButtons TaskDlgMyFeature::getStandardButtons() const
{
    return QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
}

bool TaskDlgMyFeature::accept()
{
    // Apply changes to properties.
    // Use vp->getObject() to access the App object.
    (void)vp;
    return true;
}

bool TaskDlgMyFeature::reject()
{
    // Revert temporary UI state if needed.
    (void)vp;
    return true;
}
```

### 9.2 Connect Task Dialog From ViewProvider

Connection from ViewProvider (given pattern):

```cpp
bool ViewProviderMyFeature::setEdit(int ModNum)
{
    (void)ModNum;
    Gui::Control().showDialog(new MyModGui::TaskDlgMyFeature(this));
    return true;
}

void ViewProviderMyFeature::unsetEdit(int ModNum)
{
    (void)ModNum;
    Gui::Control().closeDialog();
}
```

Practical tips:

- Keep UI widgets as a view over properties.
- Write to properties in `accept()`.
- Trigger recompute with `obj->getDocument()->recompute()` or Python macro commands.

If you want changes to be undoable while the task panel is open, you typically:

- Start an undo command when entering edit.
- Apply changes via `openCommand()` and `commitCommand()`.
- On cancel, either undo or restore cached values.

## 10. Modifying The Selection System

Selection is in the GUI layer and exposed via `Gui::Selection()`.

Key concepts:

- Pre-selection is hover highlight.
- Selection is click and persists until cleared.

### 10.1 Common API Calls

Examples (C++):

```cpp
#include <Gui/Selection.h>

void clearAllSelection()
{
    Gui::Selection().clearSelection();
}

std::vector<Gui::SelectionObject> currentSelection()
{
    return Gui::Selection().getSelection();
}

std::vector<Gui::SelectionObjectEx> currentSelectionEx()
{
    return Gui::Selection().getSelectionEx();
}
```

### 10.2 Observing Selection Changes

Use an observer instead of polling.

```cpp
// MySelectionObserver.h
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef MYMODGUI_MYSELECTIONOBSERVER_H
#define MYMODGUI_MYSELECTIONOBSERVER_H

#include <Gui/Selection.h>

namespace MyModGui
{

class MySelectionObserver : public Gui::SelectionObserver
{
public:
    void addSelection(const Gui::SelectionChanges& msg) override;
    void removeSelection(const Gui::SelectionChanges& msg) override;
    void setPreselection(const Gui::SelectionChanges& msg) override;
    void clearSelection(const Gui::SelectionChanges& msg) override;
};

} // namespace MyModGui

#endif
```

```cpp
// MySelectionObserver.cpp
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <PreCompiled.h>

#include <Mod/MyMod/Gui/MySelectionObserver.h>

#include <Base/Console.h>

using namespace MyModGui;

void MySelectionObserver::addSelection(const Gui::SelectionChanges& msg)
{
    Base::Console().Log("Selection add: %s\n", msg.pObjectName);
}

void MySelectionObserver::removeSelection(const Gui::SelectionChanges& msg)
{
    Base::Console().Log("Selection remove: %s\n", msg.pObjectName);
}

void MySelectionObserver::setPreselection(const Gui::SelectionChanges& msg)
{
    Base::Console().Log("Preselect: %s\n", msg.pObjectName);
}

void MySelectionObserver::clearSelection(const Gui::SelectionChanges& msg)
{
    (void)msg;
    Base::Console().Log("Selection cleared\n");
}
```

Register and unregister:

```cpp
static MySelectionObserver gObserver;

void installObserver()
{
    Gui::Selection().addObserver(&gObserver);
}

void removeObserver()
{
    Gui::Selection().removeObserver(&gObserver);
}
```

Keep observer callbacks fast and avoid heavy document operations inside them.

## 11. Adding Import And Export Handlers

Import and export is usually module owned and configured in `Init.py`.

Register formats:

```python
# src/Mod/MyMod/Init.py
import FreeCAD

FreeCAD.addImportType("My Format (*.myf)", "MyMod.ImportMyFormat")
FreeCAD.addExportType("My Format (*.myf)", "MyMod.ExportMyFormat")
```

Handlers must provide functions with known names.

### 11.1 Import Handler

Minimal `open()` and `insert()`:

```python
# src/Mod/MyMod/ImportMyFormat.py
import FreeCAD


def open(filename):
    doc = FreeCAD.newDocument()
    insert(filename, doc.Name)
    return doc


def insert(filename, docname):
    doc = FreeCAD.getDocument(docname)
    FreeCAD.Console.PrintLog(f"Importing {filename} into {docname}\n")

    # Parse file, create objects.
    obj = doc.addObject("App::DocumentObjectGroup", "Imported")
    _ = obj

    doc.recompute()
```

### 11.2 Export Handler

Minimal `export()`:

```python
# src/Mod/MyMod/ExportMyFormat.py
import FreeCAD


def export(objects, filename):
    FreeCAD.Console.PrintLog(f"Exporting {len(objects)} objects to {filename}\n")
    with open(filename, "w", encoding="utf-8") as f:
        for obj in objects:
            f.write(f"{obj.Name}\n")
```

If your format needs full document context, prefer `insert()` for import and pass `docname`.

```python
def insert(filename, docname):
    doc = FreeCAD.getDocument(docname)
    # Use doc for object creation.
    doc.recompute()
```

Notes:

- Always call `doc.recompute()` after creating objects during import.
- Prefer storing stable identifiers in your file format.

## 12. CMake Integration

For C++ modules, CMake is what pulls your sources into the build.

Common patterns in a module `CMakeLists.txt`:

```cmake
SET(MyMod_SRCS
    AppMyMod.cpp
    MyFeature.cpp
    MyFeature.h
)

SET(MyModGui_SRCS
    CommandMyMod.cpp
    ViewProviderMyFeature.cpp
    ViewProviderMyFeature.h
    TaskMyFeature.cpp
    TaskMyFeature.h
)

SET(MyModGui_UIC_SRCS
    Resources/ui/TaskMyFeature.ui
    Resources/ui/PrefMyMod.ui
)
```

For linking, follow the neighbor module pattern.

```cmake
TARGET_LINK_LIBRARIES(MyMod
    FreeCADApp
)

TARGET_LINK_LIBRARIES(MyModGui
    FreeCADGui
    MyMod
)
```

Typical module toggle:

```cmake
OPTION(BUILD_MYMOD "Build MyMod module" ON)
IF(BUILD_MYMOD)
    # add_subdirectory, target_link_libraries, etc.
ENDIF()
```

Practical notes:

- Keep App and Gui targets separate.
- Add `.ui` files so they are compiled into the UI resource pipeline.
- If you add resources (icons, translations), follow the pattern used by sibling modules.

## 13. Important Anti Patterns To Avoid

These show up in reviews because they work in small tests and then break in real documents.

### 13.1 Document Access

Bad in object code:

```cpp
// BAD: global document access
auto* doc = App::GetApplication().getActiveDocument();
```

Good:

```cpp
// Good: use the owning document
auto* doc = this->getDocument();
```

For Python proxies, prefer `obj.Document`.

```python
def execute(self, obj):
    doc = obj.Document
    (void, doc)
```

### 13.2 Logging And Output

Bad:

```python
print("debug")
```

Good:

```python
import FreeCAD
FreeCAD.Console.PrintLog("debug\n")
FreeCAD.Console.PrintMessage("info\n")
FreeCAD.Console.PrintError("error\n")
```

### 13.3 Property Descriptions

Never put `<` or `>` in property descriptions.

Bad:

```cpp
ADD_PROPERTY_TYPE(MyProp, (0), "Group", App::Prop_None, "Angle <deg>");
```

Good:

```cpp
ADD_PROPERTY_TYPE(MyProp, (0), "Group", App::Prop_None, "Angle in degrees");
```

### 13.4 Recompute Discipline

If you create or modify document objects from scripts or commands, recompute.

Bad:

```python
obj = doc.addObject("Part::Box", "Box")
# no recompute
```

Good:

```python
obj = doc.addObject("Part::Box", "Box")
doc.recompute()
```

### 13.5 GUI Guards

Any GUI code must guard `GuiUp`.

Bad:

```python
import FreeCADGui
```

Good:

```python
import FreeCAD

if FreeCAD.GuiUp:
    import FreeCADGui
```

### 13.6 Control Statement Braces

In C++ code, always use braces.

Bad:

```cpp
if (cond)
    doThing();
```

Good:

```cpp
if (cond) {
    doThing();
}
```

### 13.7 `PreCompiled.h` First

### 13.8 No Type Suppression

FreeCAD has strong patterns for types and properties. Avoid hiding problems.

- Do not add "ignore" style pragmas to bypass compiler warnings.
- Do not cast away types just to satisfy a signature.

If you need a downcast, use `dynamic_cast` and handle failure.

```cpp
auto* p = dynamic_cast<MyMod::MyFeature*>(obj);
if (!p) {
    return;
}
```

Every `.cpp` file should include `PreCompiled.h` first.

Bad:

```cpp
#include <App/Document.h>
#include <PreCompiled.h>
```

Good:

```cpp
#include <PreCompiled.h>

#include <App/Document.h>
```

## 14. Debugging Tips

### 14.1 Logging

Use `FreeCAD.Console.PrintLog()` in Python.

```python
import FreeCAD
FreeCAD.Console.PrintLog("MyMod: start\n")
```

Use `Base::Console().Log()` in C++.

```cpp
#include <Base/Console.h>
Base::Console().Log("MyMod: execute()\n");
```

### 14.2 Inspect Properties From Python

`dumpPropertyContent()` is a quick way to see current property state.

```python
obj = FreeCAD.ActiveDocument.getObject("MyFeature")
obj.dumpPropertyContent()
```

If you are debugging expressions or derived properties, check the property editor view and the expression engine state.

### 14.3 Macro Recording

When you do UI actions, FreeCAD can record the equivalent Python. This is often the fastest way to learn the correct command sequence for an operation.

Use the recorded macro as:

- A prototype of your `Command::activated()` logic.
- A reference for correct object creation and parameter setting.

### 14.4 Run Debug Build

Launch the GUI from the debug build:

```bash
pixi run freecad
```

### 14.5 Tests

Run C++ gtests for one module:

```bash
build/debug/tests/ModuleName_tests_run --gtest_filter="TestName*"
```

Run Python tests in CLI:

```bash
build/debug/bin/FreeCADCmd -t 0
```

### 14.6 Fast Failure Patterns

If your object does not recompute:

- Confirm `execute()` is called by checking logs.
- Check `mustExecute()` if you override it.
- Confirm you did not accidentally return a failure `DocumentObjectExecReturn`.

If your ViewProvider does not show:

- Confirm `getViewProviderName()` returns the correct registered name.
- Confirm your GUI library is loaded (module `InitGui.py`).
- Confirm `attach()` is reached.

If your command does nothing:

- Confirm `isActive()` returns `true`.
- Confirm the command name matches what you register.
- If you use `doCommand()`, verify the Python string is valid.

If your new property does not show in the editor:

- Confirm the property is added with `ADD_PROPERTY_TYPE()` or `addProperty()`.
- Confirm the property has a stable group name.
- If you set a custom editor name, confirm the editor class exists.
