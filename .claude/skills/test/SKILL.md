---
name: test
description: Run FreeCAD tests. Optional module filter. Usage /test [module] [--filter pattern]
argument-hint: "[module] [--filter gtest_pattern]"
allowed-tools: Bash, Read, Grep
user-invocable: true
---

# Run FreeCAD Tests

## Arguments

`$0` -- Module name (optional): Part, Sketcher, App, Base, Gui, etc.
`$1` -- Filter flag: `--filter` followed by gtest pattern

## Test Commands

**All tests:**
```bash
pixi run test
```

**Specific C++ module:**
```bash
build/debug/tests/{Module}_tests_run
```

**With filter:**
```bash
build/debug/tests/{Module}_tests_run --gtest_filter="{pattern}"
```

**Python tests:**
```bash
build/debug/bin/FreeCADCmd -t 0
```

## Steps

1. Parse arguments for module name and filter pattern
2. If no args: run `pixi run test`
3. If module specified: check `build/debug/tests/{Module}_tests_run` exists
4. Run appropriate test command
5. Report pass/fail counts and any failures

## Available C++ Test Suites

App, Base, Gui, Assembly, Material, Measure, Mesh, MeshPart, Part, PartDesign, Sketcher, Spreadsheet, Start, Zipios

## Executable Location

`build/debug/tests/{Module}_tests_run`
