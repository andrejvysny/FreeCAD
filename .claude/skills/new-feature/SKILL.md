---
name: new-feature
description: Scaffold a new C++ feature with DocumentObject + ViewProvider + Command + CMake registration
argument-hint: "<ModuleName> <FeatureName>"
allowed-tools: Read, Grep, Glob, Bash, Edit, Write
user-invocable: true
agent: cpp-developer
---

# Scaffold a New C++ Feature

Creates the complete file set for a new C++ feature in a FreeCAD workbench.

## Arguments

- `$0` -- ModuleName: The workbench (e.g., Part, Sketcher, Fem)
- `$1` -- FeatureName: PascalCase name (e.g., MyExtrusion)

## Steps

1. Read `src/Mod/$0/AGENTS.md` for module conventions
2. Study existing features: `ls src/Mod/$0/App/Feature*.h` to match patterns
3. Scaffold the following files:

### Files to Create

**`src/Mod/{Module}/App/Feature{Name}.h`** -- DocumentObject header
- SPDX header
- `#pragma once`
- Class inheriting from appropriate base (Part::Feature, App::DocumentObject, etc.)
- `PROPERTY_HEADER_WITH_OVERRIDE` macro
- Property declarations
- `execute()` override
- `getViewProviderName()` override

**`src/Mod/{Module}/App/Feature{Name}.cpp`** -- DocumentObject implementation
- SPDX header
- `#include "PreCompiled.h"` as FIRST include
- `PROPERTY_SOURCE` macro
- Constructor with `ADD_PROPERTY` calls
- `execute()` implementation

**`src/Mod/{Module}/Gui/ViewProvider{Name}.h`** -- ViewProvider header
- SPDX header
- Class inheriting from appropriate VP base
- `PROPERTY_HEADER_WITH_OVERRIDE` macro

**`src/Mod/{Module}/Gui/ViewProvider{Name}.cpp`** -- ViewProvider implementation
- SPDX header
- `#include "PreCompiled.h"` first
- `PROPERTY_SOURCE` macro

### Files to Update

- `src/Mod/{Module}/App/CMakeLists.txt` -- add source files
- `src/Mod/{Module}/Gui/CMakeLists.txt` -- add VP source files

## Convention Matching

Study at least 2 existing features in the same module to match:
- Include ordering
- Property initialization pattern
- Base class choice
- Namespace usage
