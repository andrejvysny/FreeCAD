# src/Mod/Fem/ — Finite Element Analysis

FEA workbench. 1026 files across 16 subdirectories. Heavily Python — C++ core with Python solver wrappers, mesh handling, and examples.

## STRUCTURE

```
Fem/
├── App/                    # 96 C++ files — FEM objects (constraints, mesh, results)
│   ├── FemAnalysis.cpp     # Analysis container
│   ├── FemConstraint*.cpp  # 15+ constraint types (Force, Fixed, Displacement...)
│   ├── FemMesh*.cpp        # Mesh data, SMESH wrapper
│   └── FemResult*.cpp      # Result storage
├── Gui/                    # 218 files — ViewProviders, commands, task panels
│   ├── Command.cpp         # 3120 lines — all FEM commands
│   ├── ViewProviderFemMesh.cpp  # 3381 lines — mesh visualization
│   └── Resources/          # Icons (105), UI files (40)
├── femcommands/            # Python command implementations
├── femexamples/            # Example FEM setups with pre-built meshes
│   └── meshes/             # Pre-generated mesh data (huge Python files)
├── femguiobjects/          # Python GUI objects
├── femguiutils/            # GUI utility functions
├── feminout/               # Import/export: CalculiX, Z88, Elmer, Gmsh, VTK
├── femmesh/                # Mesh creation and manipulation
├── femobjects/             # Python FEM objects (FeaturePython pattern)
├── fempreferencepages/     # Settings UI
├── femresult/              # Result processing
├── femsolver/              # Solver interfaces (CalculiX, Elmer, Z88, Mystran)
│   ├── calculix/           # CalculiX integration
│   ├── elmer/              # Elmer integration
│   ├── z88/                # Z88 integration
│   └── mystran/            # Mystran integration
├── femtaskpanels/          # Task panel implementations
├── femtest/                # FEM-specific tests
├── femtools/               # Shared FEM utilities
├── femviewprovider/        # Python ViewProviders
├── ObjectsFem.py           # Factory: creates all FEM objects
└── coding_conventions.md   # FEM-specific coding standards
```

## WHERE TO LOOK

| Task                | Location                                                           |
| ------------------- | ------------------------------------------------------------------ |
| Add constraint type | `App/FemConstraint{Type}.cpp` + `femobjects/` + `femviewprovider/` |
| Add solver support  | `femsolver/{solver}/` — writer, tasks, equations                   |
| Add import/export   | `feminout/import{Format}.py`                                       |
| Add FEM example     | `femexamples/` — follow existing pattern                           |
| Add mesh operation  | `femmesh/`                                                         |
| Run FEM tests       | `femtest/` or `pixi run test` with FEM filter                      |

## CONVENTIONS

- `ObjectsFem.py` is the factory — ALL FEM object creation goes through here
- Solver integration: each solver has `writer.py` (input file), `tasks.py` (execution), `solver.py` (config)
- FEM has its own `coding_conventions.md` — read it before contributing
- Mesh data files in `femexamples/meshes/` are auto-generated — don't edit manually
- C++ constraints use `FemConstraint` base class with `PROPERTY_HEADER`

## NOTES

- External solvers (CalculiX, Elmer, Z88, Gmsh) must be installed separately
- VTK is optional but needed for result visualization
- `ViewProviderFemMesh.cpp` (3381 lines) handles all mesh rendering
