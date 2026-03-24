<!--
FreeCAD Developer Guide 04
FeaturePython and Python Bindings

Audience: FreeCAD C++ and Python developers who already read guides 01-03.
Scope: FeaturePython proxy pattern (App and Part), plus the bindings generation pipeline.
No build instructions here.
-->

# 04. FeaturePython Pattern and Python Bindings Pipeline

This guide shows how FreeCAD lets you create new DocumentObjects in Python without writing a new C++ subclass. The core idea is simple: create a C++ FeaturePython object, attach a Python proxy, and recompute.

Paths are workspace relative.


## 1. What is FeaturePython?

FeaturePython is a family of C++ DocumentObject types that are designed to delegate logic to Python.
You still create a normal C++ object in the document, but you attach a Python object as a proxy.

Practical consequences:

- Storage, properties, undo/redo, recompute scheduling, and file IO still live in C++.
- Feature logic (and optionally UI behavior) can live in Python.
- You do not write a new C++ subclass for the feature itself.

In Python you typically instantiate one of these base types:

- `App::FeaturePython` for generic App features.
- `Part::FeaturePython` (via `"Part::FeaturePython"` in `addObject`) when you want a `Shape` and Part behavior.

The feature becomes "Python driven" as soon as you assign `obj.Proxy`.
You can see the C++ side setting up the Proxy property in `src/App/FeaturePython.h`:

```cpp
template<class FeatureT>
class FeaturePythonT: public FeatureT
{
public:
    FeaturePythonT()
    {
        ADD_PROPERTY(Proxy, (Py::Object()));
        imp = new FeaturePythonImp(this);
    }

    DocumentObjectExecReturn* execute() override
    {
        bool handled = imp->execute();
        if (!handled) {
            return FeatureT::execute();
        }
        return DocumentObject::StdReturn;
    }
};
```

Source: `src/App/FeaturePython.h`

Read that as: recompute calls C++ `execute()`, which tries the Python proxy first and falls back to the normal C++ implementation if the proxy does not handle it.


## 2. The Python Proxy Pattern

### 2.1 The minimal rule

The proxy pattern is one line:

```python
obj.Proxy = self
```

If you do not do that, FreeCAD has no Python behavior to call.

In `src/Mod/TemplatePyMod/FeaturePython.py`, the base helper class does exactly that:

```python
class PartFeature:
    def __init__(self, obj):
        obj.Proxy = self
```

Source: `src/Mod/TemplatePyMod/FeaturePython.py`

### 2.2 A real FeaturePython proxy from TemplatePyMod

This is the actual `Box` feature proxy shipped as the canonical example:

```python
class Box(PartFeature):
    def __init__(self, obj):
        PartFeature.__init__(self, obj)
        ''' Add some custom properties to our box feature '''
        obj.addProperty("App::PropertyLength","Length","Box","Length of the box", locked=True).Length=1.0
        obj.addProperty("App::PropertyLength","Width","Box","Width of the box", locked=True).Width=1.0
        obj.addProperty("App::PropertyLength","Height","Box", "Height of the box", locked=True).Height=1.0

    def onChanged(self, fp, prop):
        ''' Print the name of the property that has changed '''
        FreeCAD.Console.PrintMessage("Change property: " + str(prop) + "\n")

    def execute(self, fp):
        ''' Print a short message when doing a recomputation, this method is mandatory '''
        FreeCAD.Console.PrintMessage("Recompute Python Box feature\n")
        fp.Shape = Part.makeBox(fp.Length,fp.Width,fp.Height)
```

Source: `src/Mod/TemplatePyMod/FeaturePython.py`

Important details:

- `obj.addProperty(...)` adds C++ properties to a C++ object. The proxy is not the storage.
- `execute(self, fp)` receives the real C++ object instance (`fp`), not the proxy.
- For `Part::FeaturePython`, setting `fp.Shape` is the usual output.

Optional advanced proxy style: if your Python proxy exposes a `__object__` attribute pointing at the C++ instance, FreeCAD can call some proxy methods without passing `fp`. The check lives in `src/App/FeaturePython.cpp` (`PyObject_HasAttrString(pyobj, "__object__")`). Most FeaturePython code uses the simpler `execute(self, fp)` signature.


## 3. Creating FeaturePython Objects (Python)

There are two steps:

1. Create a C++ object of a FeaturePython base type.
2. Attach your Python proxy (and optionally a view provider proxy).

### 3.1 The canonical example from TemplatePyMod

TemplatePyMod uses this exact workflow:

```python
def makeBox():
    doc=FreeCAD.newDocument()
    a=FreeCAD.ActiveDocument.addObject("Part::FeaturePython","Box")
    Box(a)
    ViewProviderBox(a.ViewObject)
    doc.recompute()
```

Source: `src/Mod/TemplatePyMod/FeaturePython.py`

### 3.2 A reusable function that works in existing documents

Same idea, but using the active document and guarding GUI-only code:

```python
import FreeCAD as App

def createBox():
    doc = App.ActiveDocument
    obj = doc.addObject("Part::FeaturePython", "MyBox")
    Box(obj)
    if App.GuiUp:
        ViewProviderBox(obj.ViewObject)
    doc.recompute()
```


## 4. ViewProvider Proxy (Python)

If FreeCAD runs with the GUI up, every document object may have a `ViewObject` with a view provider.
For FeaturePython objects you can also provide the view provider logic in Python.

The proxy rule is the same:

```python
vobj.Proxy = self
```

### 4.1 Minimal ViewProvider proxy from TemplatePyMod

TemplatePyMod includes a simple box view provider that mostly demonstrates the call points:

```python
class ViewProviderBox:
    def __init__(self, obj):
        obj.Proxy = self

    def attach(self, obj):
        return

    def updateData(self, fp, prop):
        return

    def getDefaultDisplayMode(self):
        return "Shaded"
```

Source: `src/Mod/TemplatePyMod/FeaturePython.py`

That class intentionally keeps `attach()` empty.
For a real custom scene graph, look at the octahedron example.

### 4.2 A real Coin3D scene graph setup (octahedron)

The octahedron view provider shows how to register display modes and react to `Shape` changes:

```python
class ViewProviderOctahedron:
    def __init__(self, obj):
        obj.addProperty("App::PropertyColor","Color","Octahedron","Color of the octahedron", locked=True).Color=(1.0,0.0,0.0)
        obj.Proxy = self

    def attach(self, obj):
        self.shaded = coin.SoGroup()
        obj.addDisplayMode(self.shaded, "Shaded")
        self.wireframe = coin.SoGroup()
        obj.addDisplayMode(self.wireframe, "Wireframe")

    def updateData(self, fp, prop):
        if prop == "Shape":
            s = fp.getPropertyByName("Shape")
            # Update Coin nodes from s.Vertexes...

    def getDisplayModes(self, obj):
        return ["Shaded", "Wireframe"]
```

Source: `src/Mod/TemplatePyMod/FeaturePython.py`

Key takeaways:

- `attach(self, vobj)` is where you build Coin nodes and call `vobj.addDisplayMode(...)`.
- `updateData(self, fp, prop)` is called when the feature's properties change.
- `onChanged(self, vobj, prop)` is called when view provider properties change.


## 5. Mandatory vs Optional Methods

FreeCAD calls into your proxies by name.

Mandatory:

- Feature proxy: `execute(self, fp)`
- View provider proxy (GUI): `attach(self, vobj)`

Common optional hooks:

- Feature proxy: `onChanged(self, fp, prop)`, `onDocumentRestored(self, fp)`
- View provider proxy: `updateData(self, fp, prop)`, `onChanged(self, vobj, prop)`, `getIcon(self)`, `setupContextMenu(self, vobj, menu)`

TemplatePyMod implements `dumps()`/`loads()` on view providers because Coin nodes are not picklable:

```python
def dumps(self):
    return None

def loads(self,state):
    return None
```

Source: `src/Mod/TemplatePyMod/FeaturePython.py`


## 6. ClassPy.xml Python Binding Pipeline

FreeCAD's Python API for core C++ classes is generated.
Historically this was driven by XML (often referred to as `*Py.xml` / "ClassPy.xml" in older docs). In this checkout, most definitions are expressed as `.pyi` stubs, but the generator still supports `.xml` inputs.

### 6.1 Where binding definitions live

- Newer style: `.pyi` files next to the C++ sources, for example `src/App/DocumentObject.pyi`.
- Legacy style: `.xml` files, handled by the same generator.

### 6.2 Legacy XML shape (example)

Legacy `*Py.xml` files described exported Python classes, methods, and attributes. Example structure:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<FrozenObject SchemaVersion="1">
  <PythonExport Name="DocumentObjectPy" Include="App/DocumentObject.h" Father="ExtensionContainerPy" Twin="DocumentObject" Namespace="App" FatherInclude="App/ExtensionContainerPy.h" FatherNamespace="App" TwinPointer="DocumentObject">
    <Documentation>...</Documentation>
    <Methode Name="addObject">
      <Documentation>
        <UserDocu>Add an object of given type</UserDocu>
      </Documentation>
    </Methode>
    <Attribute Name="Name" ReadOnly="true">
      <Get>...</Get>
    </Attribute>
  </PythonExport>
</FrozenObject>
```

### 6.3 The generator entrypoint

The generator is `src/Tools/bindings/generate.py`.
It parses either `.xml` or `.pyi`:

```python
def generate_model(filename):
    if filename.endswith(".xml"):
        return model.generateModel_Module.parse(filename)
    elif filename.endswith(".pyi"):
        return model.generateModel_Python.parse(filename)
    raise ValueError("invalid file extension")
```

Source: `src/Tools/bindings/generate.py`

### 6.3 How CMake triggers generation

The hooks live in `cMake/FreeCadMacros.cmake`. They both call `src/Tools/bindings/generate.py` and register the generated wrapper sources as build outputs.

Legacy `.xml` input:

```cmake
macro(generate_from_xml BASE_NAME)
  COMMAND ${Python3_EXECUTABLE} "${TOOL_NATIVE_PATH}" --outputPath "${OUTPUT_NATIVE_PATH}" ${BASE_NAME}.xml
  MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${BASE_NAME}.xml"
endmacro()
```

Current `.pyi` input:

```cmake
macro(generate_from_py_impl BASE_NAME SUFFIX)
  COMMAND ${Python3_EXECUTABLE} "${TOOL_NATIVE_PATH}" --outputPath "${OUTPUT_NATIVE_PATH}" ${BASE_NAME}.pyi
  MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${BASE_NAME}.pyi"
endmacro()
```

### 6.4 Output files

The generator produces C++ wrapper sources (`*Py*.cpp` and `*Py*.h`) that are compiled into FreeCAD.
The `.pyi` inputs also act as developer-facing API documentation and as type information for IDEs.


## 7. Property Access from Python

Properties are C++ objects, but they appear as normal Python attributes.

### 7.1 Adding properties

The TemplatePyMod `Box` adds properties like this:

```python
obj.addProperty("App::PropertyLength","Length","Box","Length of the box", locked=True).Length=1.0
```

Source: `src/Mod/TemplatePyMod/FeaturePython.py`

### 7.2 Reading and writing values

Once a property exists, you read and write it like a normal attribute:

```python
obj.Length = 10.0
val = obj.Length
```

Dynamic properties work the same way:

```python
obj.addProperty("App::PropertyString", "Description", "Base", "Description")
obj.Description = "My object"
```

Notes:

- Changing a property usually touches the object, and it will recompute on the next recompute pass.
- For feature code, rely on the `fp` argument passed into `execute(fp)` instead of global state.


## 8. Python-only Workbench Objects

FeaturePython is the basis for many Python-heavy workbenches.
You can ship an entire workbench in Python, register commands, and create FeaturePython objects from those commands.

TemplatePyMod shows the key calls.

### 8.1 Registering the workbench

`src/Mod/TemplatePyMod/InitGui.py` defines a `Workbench` subclass and registers it:

```python
class TemplatePyModWorkbench ( Workbench ):
    MenuText = "Python sandbox"
    def Initialize(self):
        import Commands

Gui.addWorkbench(TemplatePyModWorkbench)
```

Source: `src/Mod/TemplatePyMod/InitGui.py`

### 8.2 Registering commands

TemplatePyMod uses `FreeCADGui.addCommand()` directly, and also provides a helper that captures command source for the macro recorder:

```python
def addCommand(name,cmdObject):
    (list,num) = inspect.getsourcelines(cmdObject.Activated)
    ...
    FreeCADGui.addCommand(name,cmdObject,source)
```

And it registers a command that creates the FeaturePython box:

```python
class TemplatePyMod_Cmd6:
    def Activated(self):
        import FeaturePython
        FeaturePython.makeBox()

FreeCADGui.addCommand('TemplatePyMod_Cmd6', TemplatePyMod_Cmd6())
```

Source: `src/Mod/TemplatePyMod/Commands.py`


## 9. Python Bindings Architecture

### 9.1 PyCXX and the wrapper types

Bindings use the PyCXX `Py::` wrappers; example from `src/App/FeaturePython.cpp`:

```cpp
Py::Tuple args(1);
args.setItem(0, Py::Object(object->getPyObject(), true));
Py::Object res = Base::pyCall(py_execute.ptr(), args.ptr());
```

Source: `src/App/FeaturePython.cpp`

### 9.2 DocumentObject wrappers and property descriptors

At runtime, C++ `App::DocumentObject` instances have Python wrapper objects; property access is implemented on that wrapper. FeaturePython uses `object->getPyObject()` to pass the real object into proxy calls.


## 10. Complete Minimal Example (FeaturePython + Command)

Paste into the Python console (or a module) to create a working `Part::FeaturePython` and register a GUI command.

```python
import FreeCAD as App
class SimpleCylinder:
    def __init__(self, obj):
        obj.Proxy = self
        obj.addProperty("App::PropertyLength", "Radius", "Cylinder", "Radius")
        obj.addProperty("App::PropertyLength", "Height", "Cylinder", "Height")
        obj.Radius = 5.0
        obj.Height = 10.0
    
    def execute(self, fp):
        import Part
        fp.Shape = Part.makeCylinder(fp.Radius, fp.Height)
    def dumps(self):
        return None
    def loads(self, state):
        return None
class ViewProviderCylinder:
    def __init__(self, vobj):
        vobj.Proxy = self
    def getIcon(self):
        return ""
    def dumps(self):
        return None
    def loads(self, state):
        return None
def create_cylinder(name: str = "Cylinder"):
    doc = App.ActiveDocument
    if doc is None:
        doc = App.newDocument()
    
    obj = doc.addObject("Part::FeaturePython", name)
    SimpleCylinder(obj)
    
    if App.GuiUp:
        ViewProviderCylinder(obj.ViewObject)
    
    doc.recompute()
    return obj
if App.GuiUp:
    import FreeCADGui as Gui

    class CmdCreateCylinder:
        def GetResources(self):
            return {
                'MenuText': 'Create Cylinder',
                'ToolTip': 'Create a simple cylinder (FeaturePython)',
                'Pixmap': 'Part_Box',
            }

        def Activated(self):
            create_cylinder()

        def IsActive(self):
            return App.ActiveDocument is not None

    Gui.addCommand('Simple_CreateCylinder', CmdCreateCylinder())
```

If you want this command to live in a Python workbench, register it in your module's `InitGui.py` (like TemplatePyMod does), then add it to toolbars and menus from the workbench's `Initialize()`.
