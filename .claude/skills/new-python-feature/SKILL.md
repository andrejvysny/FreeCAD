---
name: new-python-feature
description: Scaffold a new FeaturePython object with proxy, ViewProvider, and factory function
argument-hint: "<ModuleName> <FeatureName>"
allowed-tools: Read, Grep, Glob, Bash, Edit, Write
user-invocable: true
agent: python-developer
---

# Scaffold a New FeaturePython Object

Creates a complete FeaturePython object following FreeCAD patterns.

## Arguments

- `$0` -- ModuleName: The workbench (e.g., Draft, BIM)
- `$1` -- FeatureName: PascalCase name (e.g., MyWall)

## Steps

1. Read `src/Mod/$0/AGENTS.md` for module conventions
2. Study existing objects in the module to match directory patterns
3. Study `src/Mod/TemplatePyMod/FeaturePython.py` as canonical reference
4. Create the following files:

### Object Proxy

```python
# SPDX-License-Identifier: LGPL-2.1-or-later

import FreeCAD

class {FeatureName}:
    def __init__(self, obj):
        obj.addProperty("App::PropertyLength", "Height", "{Module}", "Height")
        obj.Proxy = self

    def execute(self, fp):
        pass  # implement

    def dumps(self):
        return None

    def loads(self, state):
        pass
```

### ViewProvider Proxy

```python
# SPDX-License-Identifier: LGPL-2.1-or-later

class ViewProvider{FeatureName}:
    def __init__(self, vobj):
        vobj.Proxy = self

    def attach(self, vobj):
        pass

    def getIcon(self):
        return ":/icons/{module}_{name}.svg"

    def dumps(self):
        return None

    def loads(self, state):
        pass
```

### Factory Function

```python
# SPDX-License-Identifier: LGPL-2.1-or-later

import FreeCAD

def make_{name}(name="{FeatureName}"):
    doc = FreeCAD.ActiveDocument
    obj = doc.addObject("Part::FeaturePython", name)
    {FeatureName}(obj)
    if FreeCAD.GuiUp:
        ViewProvider{FeatureName}(obj.ViewObject)
    doc.recompute()
    return obj
```

## Directory Patterns by Module

- **Draft**: `draftobjects/`, `draftviewproviders/`, `draftmake/`
- **BIM**: `bimcommands/`, objects inline
- **Fem**: `femobjects/`, `femviewprovider/`, `ObjectsFem.py` factory
- **Other Python modules**: Study existing structure
