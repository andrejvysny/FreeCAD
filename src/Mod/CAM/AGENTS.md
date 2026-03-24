# src/Mod/CAM/ — CNC Toolpath Generation

Computer-Aided Manufacturing workbench. 877 files. Generates G-code toolpaths from 3D models.

## STRUCTURE

```
CAM/
├── App/                    # 43 C++ files — core path data structures
│   ├── Area*.cpp           # Area operations (4264 lines in Area.cpp)
│   ├── Command.cpp/h       # G-code command representation
│   ├── Path.cpp/h          # Toolpath data model
│   └── Voronoi*.cpp        # Voronoi diagram operations
├── Gui/                    # GUI + simulator
│   └── Resources/panels/   # 53 task panel UI files
├── Path/                   # Python path operations
│   ├── Op/                 # Operations (Profile, Pocket, Drill, Adaptive...)
│   ├── Post/               # Postprocessors (G-code output formatters)
│   │   └── scripts/        # Legacy postprocessors (DEPRECATED)
│   ├── Dressup/            # Path modifiers (DogBone, DragKnife, RampEntry...)
│   ├── Main/               # Job, Stock definitions
│   └── Base/               # Base classes for operations
├── PathSimulator/          # Toolpath simulation
│   └── AppGL/              # OpenGL-based simulator
├── CAMTests/               # 90 test files
├── Tools/                  # Tool library (JSON tool definitions)
├── libarea/                # Vendored area calculation library
│   ├── clipper.cpp         # 5169 lines — polygon clipping
│   └── Adaptive.cpp        # 3412 lines — adaptive clearing algorithm
├── Machine/                # Machine definitions
└── DemoParts/              # Example parts for testing
```

## WHERE TO LOOK

| Task               | Location                                                  |
| ------------------ | --------------------------------------------------------- |
| Add operation type | `Path/Op/{OperationType}.py` — inherit from `PathOp` base |
| Add postprocessor  | `Path/Post/{name}_post.py`                                |
| Add path dressup   | `Path/Dressup/`                                           |
| Add tool type      | `Tools/` — JSON format                                    |
| Modify simulator   | `PathSimulator/`                                          |
| Run CAM tests      | `CAMTests/` — 90 test files                               |

## CONVENTIONS

- Operations inherit from `PathOp` base class
- Postprocessors convert internal path to machine-specific G-code
- `Path/Post/scripts/` contains DEPRECATED legacy postprocessors
- Tool library uses JSON format in `Tools/`
- Tapping operation is DEPRECATED — see `Path/Op/Tapping.py`

## NOTES

- `libarea/` is vendored — contains polygon clipping (clipper) and adaptive clearing
- `PathSimulator/AppGL/` uses OpenGL for 3D material removal simulation
- CAM was renamed from "Path" — some internal references still use "Path" naming
