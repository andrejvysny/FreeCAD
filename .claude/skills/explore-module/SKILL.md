---
name: explore-module
description: Deep dive into a FreeCAD workbench module -- show structure, key classes, entry points, dependencies, complexity hotspots
argument-hint: "<ModuleName> (e.g., Part, Sketcher, Fem, Draft, BIM, CAM, TechDraw)"
allowed-tools: Read, Grep, Glob, Bash
user-invocable: true
---

# Explore FreeCAD Module

Perform a comprehensive analysis of a FreeCAD workbench module.

## Arguments

`$0` -- Module name (e.g., Part, Sketcher, Fem, Draft, BIM, CAM, TechDraw)

## Steps

1. **Read AGENTS.md** if it exists: `src/Mod/$0/AGENTS.md`

2. **Directory structure**: List top-level contents of `src/Mod/$0/`

3. **File counts**:
```bash
find src/Mod/$0 -name "*.cpp" | wc -l
find src/Mod/$0 -name "*.py" | wc -l
find src/Mod/$0 -name "*.ui" | wc -l
find src/Mod/$0 -name "*.h" | wc -l
```

4. **Entry points**:
   - Read `Init.py` -- headless initialization, registered importers
   - Read `InitGui.py` -- GUI init, workbench class, commands

5. **Key classes** (C++ modules):
   - List DocumentObject subclasses in App/
   - List ViewProviders in Gui/
   - List Command files

6. **Dependencies**:
   - CMakeLists.txt linked libraries
   - Python imports in Init.py/InitGui.py

7. **Complexity hotspots**: Files over 1000 lines

8. **Tests**: Check `tests/src/Mod/$0/` and module test directories

## Report Format

```
## Module: {Name}

### Overview
Type: [C++/Python/Mixed] | Files: [count] | Purpose: [description]

### Structure
[directory tree]

### Key Classes
[DocumentObjects, ViewProviders, Commands]

### Entry Points
[Init.py, InitGui.py summary]

### Dependencies
[upstream and downstream]

### Complexity Hotspots
[files > 1000 lines]

### Test Coverage
[test files/suites]

### Conventions
[module-specific patterns]
```
