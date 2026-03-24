<!--
FreeCAD Developer Guide 02
Type System and Properties

Audience: FreeCAD C++ and Python developers who already understand the layer split.
Scope: Only the runtime type system and the property framework.
-->

# 02. Type System and Properties

FreeCAD predates widespread use of C++ RTTI and uses its own runtime type system.
That same infrastructure is also used to create objects by type name and to power
the property framework that drives almost every DocumentObject.

This guide points you at the core headers and shows the registration flow in code.
Paths are workspace relative.

Key files:

- `src/Base/Type.h`
- `src/Base/BaseClass.h`
- `src/App/Property.h`
- `src/App/PropertyContainer.h`
- `src/App/PropertyStandard.h`
- `src/App/PropertyGeo.h`
- `src/App/PropertyLinks.h`


## 1. The Type System

FreeCAD has a custom RTTI system. Any class that wants to participate must:

- Include the required macros in its class definition.
- Provide the macro expansions in its `.cpp`.
- Ensure `init()` gets called during startup, in parent then child order.

This is why you see explicit init sequences in application startup.


### 1.1 Base::Type (`src/Base/Type.h`)

`Base::Type` is the handle that represents a runtime type.

Core ideas:

- Lightweight value type. Internally it is a 16-bit type identifier.
- Registry-backed. A `Base::Type` can resolve to metadata about the type.
- Supports safe derived checks and instance creation through registered factories.

Important API:

- `Base::Type fromName(const char* name)`
- `Base::Type getParent() const`
- `bool isDerivedFrom(const Base::Type& other) const`
- `void* createInstance() const`
- `static void* createInstanceByName(const char* name)`

The registry is static. The names vary by implementation detail, but conceptually
it consists of:

- A map from name to type id (often called `typemap`).
- A table of per-type metadata (often called `typedata`).

Example usage (typical patterns you see in C++ code):

```cpp
#include <Base/Type.h>

void exampleTypeQueries()
{
    Base::Type t = Base::Type::fromName("App::DocumentObject");

    if (t.isDerivedFrom(Base::Type::fromName("Base::BaseClass"))) {
        // All participating types derive from BaseClass.
    }

    Base::Type parent = t.getParent();
    (void)parent;
}

void* exampleFactory()
{
    // Uses the registered create() function for that type.
    return Base::Type::createInstanceByName("App::DocumentObject");
}
```

Notes:

- `createInstance()` and `createInstanceByName()` return `void*` because the type
  system is core and cannot assume any specific base class at the ABI level.
  In practice, participating classes inherit `Base::BaseClass`, so callers cast.
- `fromName()` fails if the type was not registered. That is a strong hint the
  corresponding `init()` never ran.


### 1.2 Base::BaseClass (`src/Base/BaseClass.h`)

`Base::BaseClass` is the root of the participating hierarchy.

It provides:

- `static Base::Type getClassTypeId(void)`
- `virtual Base::Type getTypeId(void) const`
- `bool isDerivedFrom(const Base::Type&) const`
- Template helpers like `isDerivedFrom<T>()` and `is<T>()`

Typical usage:

```cpp
#include <Base/BaseClass.h>

void acceptAnything(Base::BaseClass* obj)
{
    if (!obj) {
        return;
    }

    // Runtime check against a concrete class type id.
    if (obj->isDerivedFrom(Base::Type::fromName("App::DocumentObject"))) {
        // ...
    }

    // Template form.
    // if (obj->is<App::DocumentObject>()) { ... }
}
```


### 1.3 freecad_cast and why it exists

FreeCAD provides a cast helper that acts like `dynamic_cast` but uses FreeCAD's
type system.

Typical pattern:

```cpp
#include <Base/BaseClass.h>

void exampleCast(Base::BaseClass* base)
{
    // Works when RTTI is disabled, and is consistent with FreeCAD type ids.
    auto* asBase = freecad_cast<Base::BaseClass*>(base);
    (void)asBase;
}
```

In code you usually cast to an App or Gui class rather than to BaseClass itself.
The key point is the cast uses `getTypeId()` and `isDerivedFrom()` rather than
C++ RTTI.


### 1.4 TYPESYSTEM macros (exact)

The macros in `src/Base/BaseClass.h` are the standard way to make a class
participate in the type system.

Exact macros (as used by FreeCAD classes):

```cpp
// In header (.h):
#define TYPESYSTEM_HEADER() \
public: \
    static Base::Type getClassTypeId(void); \
    virtual Base::Type getTypeId(void) const; \
    static void init(void); \
    static void* create(void); \
private: \
    static Base::Type classTypeId

// In source (.cpp):
#define TYPESYSTEM_SOURCE(_class_, _parentclass_) \
    TYPESYSTEM_SOURCE_P(_class_) \
    void _class_::init(void) { \
        initSubclass(_class_::classTypeId, #_class_, #_parentclass_, &(_class_::create)); \
    }
```

`TYPESYSTEM_HEADER_WITH_OVERRIDE()` is a common variant for subclasses where
`getTypeId()` is declared as an `override`.

The macro has the same shape as `TYPESYSTEM_HEADER()` but changes the virtual
declaration to use `override`:

```cpp
// In header (.h):
#define TYPESYSTEM_HEADER_WITH_OVERRIDE() \
public: \
    static Base::Type getClassTypeId(void); \
    Base::Type getTypeId(void) const override; \
    static void init(void); \
    static void* create(void); \
private: \
    static Base::Type classTypeId
```

Why init matters:

- `init()` calls `initSubclass(...)`, which registers the name, parent, and
  factory function.
- Parent before child is required. A child init refers to the parent by name.
- FreeCAD enforces this by calling all relevant `init()` functions during
  application startup.

Where init happens:

- The init cascade is triggered from application startup, in
  `App::Application::init()`, which ensures the type registry is complete
  before any document object creation or file restore.


### 1.5 Minimal class skeleton using TYPESYSTEM macros

This is the shape for a class that participates in the type system but does not
use the property framework.

```cpp
// MyTypeOnly.h

#include <Base/BaseClass.h>

class MyTypeOnly : public Base::BaseClass
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

public:
    MyTypeOnly() = default;
};
```

```cpp
// MyTypeOnly.cpp

#include "PreCompiled.h"

#include "MyTypeOnly.h"

TYPESYSTEM_SOURCE(MyTypeOnly, Base::BaseClass);

void* MyTypeOnly::create(void)
{
    return new MyTypeOnly;
}
```

Notes:

- `create()` is the factory stored in the registry. It must return `void*`.
- `create()` is how `Base::Type::createInstance()` can construct instances.


### 1.6 isDerivedFrom, is<T>(), and getTypeId() in practice

At runtime there are two common check styles.

Type-id based check:

```cpp
bool isDocumentObject(Base::BaseClass* obj)
{
    if (!obj) {
        return false;
    }
    return obj->isDerivedFrom(Base::Type::fromName("App::DocumentObject"));
}
```

Template helper check:

```cpp
template <class T>
bool isExactly(Base::BaseClass* obj)
{
    return obj && obj->getTypeId() == T::getClassTypeId();
}
```

Both rely on the same registered type ids, so they behave consistently across
modules.


## 2. The Property System

If the type system answers "what am I?", the property system answers "what data
do I expose to the document, UI, and file format?".

Properties are:

- Serializable fields with metadata (name, group, type flags, documentation).
- Observable, with change notification and recompute behavior.
- Editable via the property editor, and accessible from Python.


### 2.1 App::Property (`src/App/Property.h`)

`App::Property` is the base class for all property types.

Key facts:

- Inherits `Base::Persistence`, so it can `Save()` and `Restore()`.
- Has container linkage. The container is usually an `App::PropertyContainer`
  (often a DocumentObject).
- Provides Copy/Paste for duplicating properties across objects.

Key methods you should recognize:

- `setValue(...)` and `getValue()` on concrete subclasses.
- `Property* Copy() const` and `void Paste(const Property& from)`.
- `void Save(Base::Writer& writer) const` and `void Restore(Base::XMLReader& reader)`.

Status bits control behavior. The exact names and combinations depend on the
property implementation, but common ones include:

- Touched: marks the property as modified.
- Immutable: value cannot change after initial setup.
- ReadOnly: exposed but not editable.
- Hidden: not shown in the property editor.
- Transient: not persisted, or persisted in a limited form.
- Output: value is output-only and does not touch the container.
- NoRecompute: changes do not trigger recompute.


### 2.2 Change notification flow

The property framework is designed to notify its container at the right time.
The shape is:

1. `aboutToSetValue()` is called before the value changes.
2. The value is updated.
3. `hasSetValue()` is called after the value changes.
4. The container is notified, typically through `onChanged()`.

This gives the container hooks to react consistently.

There is also a signal for observers:

- `signalChanged`

That signal is used by UI and other listeners that subscribe to property events.


### 2.3 Expression support

Properties can participate in expressions and path-based access.
The APIs you see include:

- `setPathValue(...)`
- `getPathValue(...)`
- `canonicalPath(...)`

This is what allows expressions to target nested parts of a property and what
enables stable addressing inside documents.


### 2.4 App::PropertyContainer (`src/App/PropertyContainer.h`)

`App::PropertyContainer` is the owner of properties.

Core responsibilities:

- Hold a property list, including static properties declared by the class and
  dynamic properties added at runtime.
- Map property names to instances.
- Provide change callbacks.

You should know these APIs:

- `Property* getPropertyByName(const char* name) const`
- `std::vector<Property*> getPropertyList() const` (exact return varies)
- `Property* addDynamicProperty(...)`

Callbacks (virtual hooks) used by subclasses:

- `onBeforeChange(const Property* prop)`
- `onEarlyChange(const Property* prop)`
- `onChanged(const Property* prop)`

The properties that are compiled into a class are described by a static
`PropertyData` structure.


### 2.6 Property status bits vs PropertyType flags

There are two overlapping concepts that are easy to mix up:

- Property internal status bits live on the Property instance. They track things
  like "touched" or "read-only" at runtime.
- `PropertyType` flags are metadata attached during registration. They drive UI
  and persistence rules for that property when owned by a container.

The `ADD_PROPERTY_TYPE(...)` macro is where the container-level flags get set.
Example patterns:

```cpp
ADD_PROPERTY_TYPE(Result, (0), "Base", App::Prop_Output, "Computed result");
ADD_PROPERTY_TYPE(Id, (0), "Base", App::Prop_ReadOnly, "Stable identifier");
ADD_PROPERTY_TYPE(Cache, (0), "Base", App::Prop_Transient, "Runtime cache");
ADD_PROPERTY_TYPE(Preview, (false), "Base", App::Prop_Hidden, "Internal toggle");
ADD_PROPERTY_TYPE(Seed, (0), "Base", App::Prop_NoRecompute, "No recompute on change");
```

This is how you keep a value visible but non-editable (`Prop_ReadOnly`), or how
you keep a cache out of the file (`Prop_Transient`), without writing custom
Save/Restore code.


### 2.7 Container callbacks example

Most real containers are subclasses of `App::DocumentObject`, but the callback
style is the same for any `App::PropertyContainer` subclass.

Typical implementation shape:

```cpp
void MyFeature::onChanged(const App::Property* prop)
{
    if (prop == &Count) {
        // React to Count changes.
        // Often you mark for recompute, invalidate a cache, or update a result.
    }

    App::PropertyContainer::onChanged(prop);
}
```

Key point:

- Always forward to the parent implementation unless you have a specific reason
  not to. The base class handles core bookkeeping.


### 2.5 PropertyType enum (exact)

The property "type" here is not the C++ type of the Property instance.
It is a bitmask that controls persistence and editing behavior.

Exact enum from `src/App/PropertyContainer.h`:

```cpp
enum PropertyType {
    Prop_None        = 0,
    Prop_ReadOnly    = 1,    // Grayed out in editor
    Prop_Transient   = 2,    // Not saved to file (but name/type saved)
    Prop_Hidden      = 4,    // Hidden in editor
    Prop_Output      = 8,    // Doesn't touch container
    Prop_NoRecompute = 16,   // No recompute on change
    Prop_NoPersist   = 32,   // Not saved at all
};
```

How to read this:

- `Prop_ReadOnly` is UI behavior. The value exists, but editing is disabled.
- `Prop_Transient` keeps the property definition but skips value persistence.
- `Prop_NoPersist` skips persistence completely.
- `Prop_Output` suppresses the usual touching and often recompute propagation.


## 3. PROPERTY_HEADER and PROPERTY_SOURCE macros

Properties are not only runtime objects, they are also reflected into a per-class
property table so the container can enumerate, serialize, and restore them.

The `PROPERTY_*` macros extend the type system macros by adding `PropertyData`.

Exact macros from `src/App/PropertyContainer.h`:

```cpp
#define PROPERTY_HEADER_WITH_OVERRIDE(_class_) \
  TYPESYSTEM_HEADER_WITH_OVERRIDE(); \
  // ... getClassName() consteval with compile-time checks ... \
protected: \
  static const App::PropertyData * getPropertyDataPtr(void); \
  const App::PropertyData &getPropertyData(void) const override; \
private: \
  static App::PropertyData propertyData

#define PROPERTY_SOURCE(_class_, _parentclass_) \
  TYPESYSTEM_SOURCE_P(_class_) \
  const App::PropertyData * _class_::getPropertyDataPtr(void){return &propertyData;} \
  const App::PropertyData & _class_::getPropertyData(void) const{return propertyData;} \
  App::PropertyData _class_::propertyData; \
  void _class_::init(void){ \
    initSubclass(_class_::classTypeId, #_class_, #_parentclass_, &(_class_::create)); \
    _class_::propertyData.parentPropertyData = _parentclass_::getPropertyDataPtr(); \
  }
```

What these do:

- They register the type (same `initSubclass(...)` call as the type system).
- They link the property chain by setting `propertyData.parentPropertyData`.
  That is how inherited properties show up in the list.


### 3.1 Minimal class skeleton using PROPERTY macros

This is the canonical shape of a class with built-in properties.

```cpp
// MyFeature.h

#include <App/PropertyStandard.h>
#include <App/PropertyContainer.h>

class MyFeature : public App::PropertyContainer
{
    PROPERTY_HEADER_WITH_OVERRIDE(MyFeature);

public:
    MyFeature();

    App::PropertyInteger Count;
    App::PropertyBool Enabled;
};
```

```cpp
// MyFeature.cpp

#include "PreCompiled.h"

#include "MyFeature.h"

PROPERTY_SOURCE(MyFeature, App::PropertyContainer);

MyFeature::MyFeature()
{
    ADD_PROPERTY_TYPE(Count, (0), "Base", App::Prop_None, "Number of items");
    ADD_PROPERTY_TYPE(Enabled, (true), "Base", App::Prop_None, "Enable behavior");
}
```

Two key points:

- `PROPERTY_SOURCE` wires the type id registration and the property chain.
- The constructor registers each property into `propertyData`.


## 4. ADD_PROPERTY and ADD_PROPERTY_TYPE macros

Registering properties into a class property table is done by `ADD_PROPERTY*`.
These macros also set the property container pointer.

Exact macros:

```cpp
#define ADD_PROPERTY(_prop_, _defaultval_) \
  do { \
    this->_prop_.setValue _defaultval_; \
    this->_prop_.setContainer(this); \
    propertyData.addProperty(static_cast<App::PropertyContainer*>(this), #_prop_, &this->_prop_); \
  } while (0)

#define ADD_PROPERTY_TYPE(_prop_, _defaultval_, _group_, _type_, _Docu_) \
  do { \
    this->_prop_.setValue _defaultval_; \
    this->_prop_.setContainer(this); \
    propertyData.addProperty(static_cast<App::PropertyContainer*>(this), #_prop_, &this->_prop_, (_group_), (_type_), (_Docu_)); \
  } while (0)
```

Implications:

- The property instance must exist as a member (or otherwise outlive the
  container). These macros store pointers.
- Default assignment happens before registration.
- `setContainer(this)` is crucial for notifications and for locating the owning
  object during Save/Restore.


## 5. Common Property Types

The concrete property classes live in a few major headers.

- Standard scalar and string types: `src/App/PropertyStandard.h`
- Geometry value types: `src/App/PropertyGeo.h`
- Links to other objects: `src/App/PropertyLinks.h`
- Unit-aware quantities: `src/App/PropertyUnits.h`

Comprehensive table of common property types:

| Type | Header | C++ value type | Python type | Usage |
|------|--------|---------------|-------------|-------|
| PropertyBool | PropertyStandard.h | bool | bool | Flags, toggles |
| PropertyInteger | PropertyStandard.h | long | int | Counts |
| PropertyFloat | PropertyStandard.h | double | float | General floating point |
| PropertyString | PropertyStandard.h | std::string | str | Names, labels |
| PropertyEnumeration | PropertyStandard.h | Enumeration | str/int | Dropdown choices |
| PropertyLength | PropertyUnits.h | double (mm) | Quantity | Length with units |
| PropertyAngle | PropertyUnits.h | double (deg) | Quantity | Angle with units |
| PropertyVector | PropertyGeo.h | Base::Vector3d | Vector | 3D point |
| PropertyPlacement | PropertyGeo.h | Base::Placement | Placement | Position + rotation |
| PropertyLink | PropertyLinks.h | DocumentObject* | object | Single link |
| PropertyLinkSub | PropertyLinks.h | (obj, subnames) | (obj, [str]) | Link + sub-elements |
| PropertyLinkList | PropertyLinks.h | vector<obj*> | [object] | Multiple links |
| PropertyXLink | PropertyLinks.h | DocumentObject* | object | Cross-document link |
| PropertyColor | PropertyStandard.h | Color | (r,g,b,a) | Color value |

Notes on a few categories:

- Links are central to the parametric graph. Link properties also participate in
  undo, recompute, and dependency tracking.
- Geometry properties often wrap Base types and are commonly used in App objects.
- Unit-aware properties expose Python Quantity objects and integrate with units.


## 6. Concrete Example: App::PropertyInteger

`App::PropertyInteger` is a good minimal example because it shows:

- A typed value (`long _lValue`).
- A GUI editor binding (`getEditorName()`).
- Python conversion hooks (`getPyObject()`, `setPyObject()`).
- Full persistence (`Save()`, `Restore()`).
- Copy semantics (`Copy()`, `Paste()`).

Excerpt from `src/App/PropertyStandard.h`:

```cpp
class AppExport PropertyInteger: public Property {
    TYPESYSTEM_HEADER_WITH_OVERRIDE();
public:
    void setValue(long);
    long getValue() const;
    const char* getEditorName() const override { return "Gui::PropertyEditor::PropertyIntegerItem"; }
    PyObject* getPyObject() override;
    void setPyObject(PyObject* py) override;
    void Save(Base::Writer& writer) const override;
    void Restore(Base::XMLReader& reader) override;
    Property* Copy() const override;
    void Paste(const Property& from) override;
protected:
    long _lValue;
};
```

What happens when you call `setValue(...)`:

- The property marks itself touched unless the change is output-only.
- The container is notified through the property change path.
- Undo/redo bookkeeping is triggered by the container (typical in DocumentObject).


## 7. Dynamic Properties (runtime addition)

Not every property is compiled into a class.
FreeCAD supports dynamic properties, which are added at runtime.

In Python you often see:

```python
obj.addProperty("App::PropertyLength", "Length", "Box", "Length of the box").Length = 1.0
```

The same concept exists in C++ through `App::PropertyContainer::addDynamicProperty()`.
Dynamic properties are stored separately from the static `PropertyData`.

Concrete C++ example shape:

```cpp
// Adds a property at runtime. The exact overload set depends on the container.
App::Property* p = this->addDynamicProperty(
    "App::PropertyLength", "Length", "Box", App::Prop_None, "Length of the box");
if (p) {
    // After addition, access by name or keep the returned pointer.
    auto* len = static_cast<App::PropertyLength*>(p);
    len->setValue(1.0);
}
```

Important implications:

- Dynamic properties are ideal for FeaturePython objects, add-on workbenches, or
  cases where the set of fields depends on runtime configuration.
- Persistence depends on the property flags. A dynamic property can still be
  persisted unless it is marked transient or no-persist.
- The container keeps a dedicated structure for them. In C++ this is stored in
  the `DynamicProperty dynamicProps` member.


## 8. AtomicPropertyChange pattern

Some properties represent a list or a compound value and may update multiple
internal fields. Triggering notifications for each sub-change can be expensive
and can cause intermediate inconsistent states.

The pattern is to guard the bulk update and emit a single notification:

```cpp
AtomicPropertyChange guard(*this);
_lValueList[0] = newVal1;
_lValueList[1] = newVal2;
guard.tryInvoke();  // Single notification
```

What this achieves:

- Observers see a single coherent change.
- The container runs `onChanged()` once.
- Recompute and transaction handling happen once.

Two practical rules:

- Use this when a single logical change touches multiple internal fields.
- Call `tryInvoke()` once when the compound update is complete.


## 9. Complete Registration Flow

This is the end-to-end trace from startup to change notification.

### 9.1 Startup registration

1. Application starts and calls `MyClass::init()`.
2. `MyClass::init()` calls `initSubclass(...)`.
3. `initSubclass(...)` registers the type name, parent name, and factory in the
   `Base::Type` registry.
4. If `MyClass` uses `PROPERTY_SOURCE`, `init()` also sets:
   `MyClass::propertyData.parentPropertyData = ParentClass::getPropertyDataPtr()`.
   This links the property chain used for enumeration.

Minimal example of the init part (as generated by `PROPERTY_SOURCE`):

```cpp
void MyClass::init(void)
{
    initSubclass(MyClass::classTypeId, "MyClass", "ParentClass", &(MyClass::create));
    MyClass::propertyData.parentPropertyData = ParentClass::getPropertyDataPtr();
}
```

### 9.2 Construction and property registration

5. The constructor runs.
6. Each `ADD_PROPERTY_TYPE(...)` call:

   - Sets a default value (`setValue`).
   - Binds the container (`setContainer`).
   - Registers the property metadata and pointer into `propertyData`.

Example:

```cpp
MyClass::MyClass()
{
    ADD_PROPERTY_TYPE(Count, (0), "Base", App::Prop_None, "Number of items");
}
```

### 9.3 A single property change

7. A caller changes a property value, for example `Count.setValue(42)`.
8. The property calls `aboutToSetValue()`.
9. The property updates its internal value.
10. The property calls `hasSetValue()`.
11. The container receives the notification and runs its hooks:

   - `onBeforeChange(prop)`
   - `onEarlyChange(prop)` (when used)
   - `onChanged(prop)`

12. The container can mark itself touched, record an undo transaction, and queue
    recompute depending on flags like `Prop_NoRecompute` and `Prop_Output`.

If you debug a recompute loop or missing recompute, this is the chain to follow.
