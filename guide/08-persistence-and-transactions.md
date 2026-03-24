<!--
SPDX-License-Identifier: LGPL-2.1-or-later

FreeCAD Developer Guide 08
Persistence and Transactions

Audience: developers who already know DocumentObjects, properties, and extensions.
Scope: how FreeCAD saves and restores documents (.FCStd), plus undo/redo transactions.
Non-goals: DocumentObject basics, recompute basics, build instructions.
-->

# 08. Persistence and Transactions

Persistence in FreeCAD is split into two kinds of data:

1. Structured XML for the document graph and most properties.
2. Separate archive entries for large payloads (BRep, meshes, thumbnails, caches).

Undo/redo is implemented with transactions. Most edits happen inside a transaction; property changes and object lifecycle changes are recorded; undo applies the inverse.

Key files:

- `src/Base/Persistence.h`
- `src/Base/Writer.h`
- `src/Base/Reader.h` (contains `Base::XMLReader`)
- `src/App/Property.h`
- `src/App/Document.h`
- `src/App/Transactions.h`
- `src/App/TransactionalObject.h`
- `src/Gui/Command.h` and `src/Gui/Command*.cpp`


## 1. The .FCStd File Format

`.FCStd` is a ZIP archive. FreeCAD writes multiple files into it, not a single monolithic blob.

Common entries:

- `Document.xml` (App document and object data)
- `GuiDocument.xml` (GUI state)
- `Thumbnail.png` (preview image)
- `*.brp` (BRep payloads and other persisted payload files)
- Additional files requested by persisted objects via `Base::Writer::addFile(...)`

Open as a ZIP to inspect contents:

```bash
unzip -l MyModel.FCStd
```

Python inspection is convenient when you want to print or extract a single entry:

```python
import zipfile

path = "MyModel.FCStd"
with zipfile.ZipFile(path, "r") as z:
    for info in z.infolist():
        print(f"{info.file_size:10d}  {info.filename}")

    doc_xml = z.read("Document.xml").decode("utf-8", errors="replace")
    print(doc_xml[:500])
```


## 2. Document.xml Structure

`Document.xml` is the core serialization. It contains a document-level property table, an object registry, and per-object property payloads.

You should recognize this shape when you open a saved file:

```xml
<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<Document SchemaVersion="4">
    <Properties Count="5">
        <Property name="Comment" type="App::PropertyString">...</Property>
    </Properties>
    <Objects Count="3">
        <Object type="App::Part" name="Part" id="1"/>
        <Object type="Part::Box" name="Box" id="2"/>
    </Objects>
    <ObjectData Count="3">
        <Object name="Box">
            <Properties Count="3">
                <Property name="Length" type="App::PropertyLength" status="1">10</Property>
            </Properties>
        </Object>
    </ObjectData>
</Document>
```

What matters for persistence work:

- `SchemaVersion` is a file schema marker. It lets restore code adapt to older XML.
- `<Objects>` declares which objects exist, with type names used for factory construction.
- `<ObjectData>` restores properties after objects exist, so links and expressions can resolve.


## 3. Persistence Base Class

Everything that persists derives (directly or indirectly) from `Base::Persistence`. It defines the Save and Restore contract and also supports external payload files inside the archive.

From `src/Base/Persistence.h`:

```cpp
class BaseExport Persistence: public BaseClass
{
    TYPESYSTEM_HEADER();

public:
    virtual unsigned int getMemSize() const = 0;
    virtual void Save(Writer& /*writer*/) const = 0;
    virtual void Restore(XMLReader& /*reader*/) = 0;
    virtual void SaveDocFile(Writer& /*writer*/) const;
    virtual void RestoreDocFile(Reader& /*reader*/);
};
```

Rule of thumb: `Save` and `Restore` stream XML, `SaveDocFile` and `RestoreDocFile` handle large data stored as separate archive entries. You opt into that by using `writer.addFile(...)` in `Save` and `reader.addFile(...)` in `Restore` (see the examples in `src/Base/Persistence.h` comments).


## 4. Property Serialization (Save/Restore)

Properties are persisted objects. `App::Property` inherits `Base::Persistence`, so each concrete property type implements `Save` and `Restore`. For transactions, properties also implement Copy and Paste.

A minimal interface model looks like this:

```cpp
class Property : public Base::Persistence {
public:
    virtual void Save(Base::Writer& writer) const = 0;
    virtual void Restore(Base::XMLReader& reader) = 0;
    virtual void Copy(const Property& from) = 0;
    virtual void Paste(const Property& from) = 0;
};
```

In this tree, `Save` and `Restore` are inherited from `Base::Persistence`, and the transaction-facing API is expressed as `Copy()` (allocate a snapshot) plus `Paste(...)` (apply a snapshot), shown next.

From `src/App/Property.h`:

```cpp
virtual Property* Copy() const = 0;
virtual void Paste(const Property& from) = 0;
```

Minimal Save and Restore example for a scalar property:

```cpp
void PropertyInteger::Save(Base::Writer& writer) const
{
    writer.Stream() << "<Integer value=\"" << _lValue << "\"/>";
}

void PropertyInteger::Restore(Base::XMLReader& reader)
{
    reader.readElement("Integer");
    _lValue = reader.getAttribute<long>("value");
}
```

The container writes the wrapper (`<Property name=... type=...> ... </Property>`). The property writes the inner representation and reads it back.

The in-tree documentation in `src/Base/Persistence.h` shows the same pattern for a vector property. It is worth reading because it is the canonical contract used across modules:

```cpp
void PropertyVector::Save (Writer &writer) const
{
   writer << writer.ind() << "<PropertyVector valueX=\"" <<  _cVec.x <<
                                            "\" valueY=\"" <<  _cVec.y <<
                                            "\" valueZ=\"" <<  _cVec.z <<"\"/>" << endl;
}

void PropertyVector::Restore(Base::XMLReader &reader)
{
  reader.readElement("PropertyVector");
  _cVec.x = reader.getAttribute<double>("valueX");
  _cVec.y = reader.getAttribute<double>("valueY");
  _cVec.z = reader.getAttribute<double>("valueZ");
}
```


## 5. Writer and XMLReader

FreeCAD streams XML for speed and memory. `Base::Writer` provides indentation helpers and an output stream abstraction; `Base::XMLReader` is SAX-based and reads elements and typed attributes.

From `src/Base/Writer.h`:

```cpp
class BaseExport Writer
{
public:
    void setForceXML(bool on);
    bool isForceXML() const;

    virtual void putNextEntry(const char* filename, const char* objName = nullptr);

    std::string addFile(const char* Name, const Base::Persistence* Object);
    virtual void writeFiles() = 0;

    const char* ind() const;
    void incInd();
    void decInd();

    virtual std::ostream& Stream() = 0;
};
```

Writing example:

```cpp
Base::ZipWriter writer("MyModel.FCStd");
writer.putNextEntry("Document.xml");
writer.Stream() << "<MyData value=\"" << 42 << "\"/>";
```

You will also see a shorthand style in examples and older code comments:

```cpp
// Writing
Base::Writer writer;
writer << "<MyData>";
writer << value;
writer << "</MyData>";

// Reading
reader.readElement("MyData");
const char* strVal = reader.getAttribute("value");
int intVal = reader.getAttributeAsInteger("value");
double dblVal = reader.getAttributeAsFloat("value");
```

In this repository, map those reads to `reader.getAttribute<const char*>(...)`, `reader.getAttribute<int>(...)`, and `reader.getAttribute<double>(...)` (see `src/Base/Reader.h`). For writing, map them to `writer.Stream() << ...`.

Requesting an extra archive entry for large data:

```cpp
void MyBigProperty::Save(Base::Writer& writer) const
{
    if (writer.isForceXML()) {
        writer.Stream() << "<MyBigProperty mode=\"xml\"/>";
        return;
    }

    const std::string file = writer.addFile("MyBigProperty.bin", this);
    writer.Stream() << "<MyBigProperty file=\"" << file << "\"/>";
}

void MyBigProperty::SaveDocFile(Base::Writer& writer) const
{
    // Write payload bytes to writer.Stream() (now points at the payload entry).
}
```

`Base::XMLReader` lives in `src/Base/Reader.h` in this tree (some docs refer to it as `XMLReader.h`):

```cpp
class BaseExport XMLReader: public XERCES_CPP_NAMESPACE::DefaultHandler
{
public:
    void readElement(const char* ElementName = nullptr);
    void readEndElement(const char* ElementName = nullptr, int level = -1);
    bool hasAttribute(const char* AttrName) const;

    template<typename T>
    T getAttribute(const char* AttrName) const;
};
```

## 6. DocumentObject Serialization Flow

Save is orchestrated by `App::Document`. The public entry points are visible in `src/App/Document.h`:

```cpp
bool save();
bool saveAs(const char* file);
void restore(const char* filename = nullptr,
             bool delaySignal = false,
             const std::vector<std::string>& objNames = {});

void Save(Base::Writer& writer) const override;
void Restore(Base::XMLReader& reader) override;
```

High-level save flow:

1. `Document::save()` or `Document::saveAs()` selects a target `.FCStd`.
2. A `Base::ZipWriter` writes `Document.xml` and then all requested payload files.
3. The document iterates objects, writes type and name info, then asks each object to persist its properties.
4. Properties can request extra files for large data (geometry, meshes) using `writer.addFile(...)` plus `SaveDocFile`.

This is why you often see `*.brp` entries: they are payload files referenced from XML, not inline XML blobs.


## 7. Transaction System (Undo/Redo)

Undo and redo are transactions. A transaction records object creation, deletion, and property changes. Committing pushes to the undo stack. Undo and redo apply recorded actions backward or forward.

The real transaction API in this tree is in `src/App/Transactions.h`:

```cpp
class AppExport Transaction: public Base::Persistence
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

public:
    explicit Transaction(int id = NullTransaction);
    void apply(Document& Doc, bool forward);

    void renameProperty(TransactionalObject* Obj, const Property* pcProp, const char* oldName);
    void addOrRemoveProperty(TransactionalObject* Obj, const Property* pcProp, bool add);
    void addObjectNew(TransactionalObject* Obj);
    void addObjectDel(const TransactionalObject* Obj);
    void addObjectChange(const TransactionalObject* Obj, const Property* Prop);
};
```

The conceptual interface is often presented like this:

```cpp
class Transaction {
public:
    void apply(Document& doc, bool forward);
    void addObject(DocumentObject* obj);
    void removeObject(DocumentObject* obj);
    void addPropertyChange(DocumentObject* obj, const Property* prop);
};
```

FreeCAD's concrete method names differ, but the operations are the same.

If you prefer a simpler mental model, the core operations are still: apply, add new object, delete object, and record property change.


## 8. TransactionalObject

Transactions need stable targets. Objects that participate in undo/redo inherit `App::TransactionalObject`.

From `src/App/TransactionalObject.h`:

```cpp
class AppExport TransactionalObject: public App::ExtensionContainer
{
    PROPERTY_HEADER_WITH_OVERRIDE(App::TransactionalObject);

public:
    virtual bool isAttachedToDocument() const;
    virtual const char* detachFromDocument();

protected:
    void onBeforeChangeProperty(Document* doc, const Property* prop);
};
```

The important part is the property-change notification path. Before the property changes, the document can record a snapshot for undo. That snapshot uses `Property::Copy()` and is replayed using `Property::Paste(...)`.


## 9. Open Transaction Pattern

Python-level transaction pattern:

```python
import FreeCAD as App

doc = App.ActiveDocument
doc.openTransaction("Create Box")
box = doc.addObject("Part::Box", "Box")
box.Length = 10
doc.commitTransaction()
```

Abort cancels pending changes:

```python
doc.openTransaction("Try")
box.Length = 999
doc.abortTransaction()
```

The corresponding `App::Document` APIs are in `src/App/Document.h`:

```cpp
int openTransaction(TransactionName name, int tid = 0);
int openTransaction(std::string name, int tid = 0);
void commitTransaction();
void abortTransaction() const;
bool undo(int id = 0);
bool redo(int id = 0);
```


## 10. Undo/Redo in GUI

GUI commands should wrap model edits so undo and macro recording behave. The usual pattern in `Command*.cpp` is: open a command, run edits, commit.

```cpp
void MyCommand::activated(int iMsg)
{
    openCommand("My Command");
    doCommand(Doc, "obj = App.ActiveDocument().addObject('Part::Box', 'Box')");
    doCommand(Doc, "obj.Length = 10");
    commitCommand();
    updateActive();
}
```

Even if you apply changes in C++ directly (not via `doCommand`), still wrap them in the command transaction.


## 11. The Restore/Recreate Flow

Restore is a multi-step pipeline. The object registry must be reconstructed before property values (especially links) can be restored.

High-level open flow:

1. Unzip `.FCStd` and obtain `Document.xml`.
2. Parse `<Objects>` and create instances from type names.
3. Parse `<ObjectData>` and restore properties.
4. Call `onDocumentRestored()` hooks (per-object fixups).
5. Recompute to rebuild derived state.
6. Restore GUI state from `GuiDocument.xml`.

The instantiate step uses the runtime type registry. A typical shape is:

```cpp
void* raw = Base::Type::createInstanceByName("Part::Box");
auto* obj = static_cast<App::DocumentObject*>(raw);
```

If a type name in `Document.xml` is not registered at restore time, FreeCAD cannot instantiate that object. That usually means module init did not run, or a type was renamed without migration.


## 12. Version Handling in Restore

FreeCAD handles older files by keeping restore code tolerant and by migrating during restore.

At the file level, `Document.xml` includes `SchemaVersion`. At the API level, the document can also report the program version a file was created with (`Document::getProgramVersion()`).

Migration pattern for a feature restore:

```cpp
void MyFeature::Restore(Base::XMLReader& reader)
{
    reader.readElement("MyFeature");

    if (reader.hasAttribute("OldPropertyName")) {
        migrateOldProperty(reader);
    }

    App::DocumentObject::Restore(reader);
}
```

Practical migration advice:

- Prefer adding new properties over renaming old ones.
- Use defaults with `getAttribute(..., defaultValue)` when older files may omit new attributes.
- Keep migration code localized, and remove it only when you are sure you can drop compatibility.


## 13. Copy/Paste (Clipboard)

This section is property copy and paste, used heavily by transactions and also by duplication and clipboard-style operations.

From `src/App/Property.h`:

```cpp
virtual Property* Copy() const = 0;
virtual void Paste(const Property& from) = 0;
```

How it is used:

- Transaction recording snapshots a property by calling `Copy()`.
- Undo and redo replay the snapshot by calling `Paste(...)` on the live property.

Implement Paste through the property setter, not by copying private fields directly. Otherwise you can bypass notifications and transaction bookkeeping.


## 14. Thumbnail and Metadata

Thumbnail:

- `Thumbnail.png` is stored as an archive entry in `.FCStd`.

Metadata:

- Document properties such as `CreatedBy`, `Company`, `Comment`, timestamps, and license fields are persisted in `Document.xml`.
- GUI state such as view configuration and visibility is stored in `GuiDocument.xml`.

You will see the metadata persisted as regular properties, for example:

```xml
<Property name="Comment" type="App::PropertyString">...</Property>
```
