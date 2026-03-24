# tests/ — Test Infrastructure

C++ (gtest) + Python (unittest). Mirrors `src/` layout for C++ tests.

## STRUCTURE

```
tests/
├── CMakeLists.txt              # Test build config, gtest discovery
├── lib/googletest/             # Bundled Google Test
└── src/
    ├── App/                    # Core App tests
    ├── Base/                   # Core Base tests
    ├── Gui/                    # GUI tests
    ├── Misc/                   # Miscellaneous tests
    ├── Mod/
    │   ├── Part/App/           # Part workbench tests
    │   ├── Sketcher/App/       # Sketcher tests
    │   ├── PartDesign/App/     # PartDesign tests
    │   ├── Assembly/App/       # Assembly tests
    │   ├── Material/App/       # Material tests
    │   ├── Measure/App/        # Measure tests
    │   ├── Mesh/App/           # Mesh tests
    │   ├── MeshPart/App/       # MeshPart tests
    │   ├── Spreadsheet/App/    # Spreadsheet tests
    │   └── Start/Gui/          # Start page tests
    └── zipios++/               # Zipios tests
```

Python tests live in `src/Mod/Test/` (not here).

## WHERE TO LOOK

| Task                 | Location                                                                  |
| -------------------- | ------------------------------------------------------------------------- |
| Add C++ test         | `tests/src/Mod/{Name}/App/{Feature}.cpp`                                  |
| Add test helper      | `tests/src/Mod/{Name}/App/{Name}TestHelpers.h`                            |
| Add test data        | `tests/src/Mod/{Name}/App/data/` — copied via `fc_target_copy_resource()` |
| Add Python test      | `src/Mod/Test/{Feature}Tests.py` (NOT in tests/)                          |
| Run all tests        | `pixi run test` or `ctest --test-dir build/debug`                         |
| Run single C++ suite | `build/debug/tests/{Name}_tests_run --gtest_filter="Test*"`               |

## CONVENTIONS

- Executable naming: `{Module}_tests_run` (e.g., `Part_tests_run`, `Sketcher_tests_run`)
- Test files: `{FeatureName}.cpp` — mirrors source file being tested
- Test helpers: `{Module}TestHelpers.{h,cpp}`
- Qt tests: `QT_QPA_PLATFORM=offscreen` required, use `setup_qt_test()` CMake function
- Test discovery: `gtest_discover_tests()` with `PRE_TEST` mode
- Available suites: App, Base, Gui, Assembly, Material, Measure, Mesh, MeshPart, Part, PartDesign, Sketcher, Spreadsheet, Start, Zipios
