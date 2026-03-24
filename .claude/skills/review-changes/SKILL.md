---
name: review-changes
description: Review current uncommitted or staged changes against FreeCAD coding standards (logic, conventions, anti-patterns -- not formatting)
argument-hint: "[--staged|--all] (default: all changes)"
allowed-tools: Read, Grep, Glob, Bash
user-invocable: true
agent: freecad-reviewer
---

# Review Current Changes

Review uncommitted changes against FreeCAD coding standards. Focuses on logic, conventions, and anti-patterns. For formatting checks, use `/check-format`.

## Arguments

`$ARGUMENTS` -- `--staged` for staged only, `--all` (default) for all changes.

## Steps

1. Get changes:
```bash
git diff                # unstaged
git diff --cached       # staged
git status              # overview
```

2. For each changed file, apply the full FreeCAD review checklist.

### C++ Checks
- SPDX header on new files
- PreCompiled.h as first include in .cpp
- Include order not auto-sorted
- Braces on all control statements
- PascalCase/camelCase naming
- No dynamic_cast (use freecad_cast)
- No OCCT types in public headers
- Layer violations (no Gui from App/Base)
- Cognitive complexity under 25
- enum class for new enums

### Python Checks
- SPDX header on new files
- No print() -- use FreeCAD.Console.*
- No bare except
- No star imports
- No App.ActiveDocument in object code
- dumps()/loads() not __getstate__/__setstate__
- No < > in property descriptions
- Correct import order
- GuiUp guards present
- doc.recompute() after modifications
- obj.Proxy = self in FeaturePython __init__

### General
- No debug remnants
- No TODO without context
- One problem per change set
- AI policy: no raw AI output indicators

3. Produce structured report with file:line references and severity levels (CRITICAL, WARNING, SUGGESTION).
