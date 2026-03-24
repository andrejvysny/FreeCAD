# src/Mod/BIM/ — Building Information Modeling

BIM/architecture workbench. 563 files. Pure Python. Replaces legacy Arch module. Depends on Draft and Part.

## STRUCTURE

```
BIM/
├── bimcommands/            # 83 Python command files — one per command
├── nativeifc/              # Native IFC support (Industry Foundation Classes)
│   ├── ifc_tools.py        # Core IFC operations
│   ├── ifc_import.py       # IFC file import
│   └── ifc_objects.py      # IFC-backed document objects
├── importers/              # File importers (IFC, DAE, 3DS, SAT, OBJ)
├── geometry/               # BIM geometry helpers
├── utils/                  # Shared utilities
├── bimtests/               # BIM-specific tests
├── Dice3DS/                # Vendored 3DS file parser
├── Presets/                # Building element presets
├── Resources/              # Icons (148), UI files (49), translations
├── Init.py                 # Registers IFC, DAE, 3DS, OBJ import/export
└── InitGui.py              # Gui::PythonWorkbench — 843 lines, massive toolbar config
```

## WHERE TO LOOK

| Task                | Location                                          |
| ------------------- | ------------------------------------------------- |
| Add BIM command     | `bimcommands/Bim{Name}.py` — one file per command |
| Modify IFC support  | `nativeifc/` — ifc_tools.py is the core           |
| Add file importer   | `importers/` + register in `Init.py`              |
| Add building preset | `Presets/`                                        |
| Add BIM test        | `bimtests/`                                       |

## CONVENTIONS

- One command per file in `bimcommands/` — keeps things modular
- IFC objects use `nativeifc/` for native IFC4 support (not legacy Arch)
- BIM aggregates Draft tools — many toolbar items are Draft commands
- FeaturePython pattern: `addObject("Part::FeaturePython", ...)` or `App::FeaturePython`

## ANTI-PATTERNS

- `nativeifc/ifc_tools.py` has 5 bare `except:` blocks — catch specific exceptions
- BIM heavily depends on Draft — test both when modifying shared code
- Legacy "Arch" command names preserved for compatibility (e.g., `Arch_Wall`)

## NOTES

- BIM replaced the old Arch workbench — some internal names still reference "Arch"
- Requires IfcOpenShell for IFC support (external dependency)
- `InitGui.py` is 843 lines due to massive toolbar/menu configuration
