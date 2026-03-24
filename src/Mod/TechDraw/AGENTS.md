# src/Mod/TechDraw/ — Technical Drawing

2D technical drawing generation from 3D models. 1098 files. Second largest workbench by file count.

## STRUCTURE

```
TechDraw/
├── App/                    # 193 files — drawing objects (pages, views, dimensions)
│   ├── Draw*.cpp           # DrawPage, DrawView, DrawViewPart, DrawViewSection...
│   ├── Dimension*.cpp      # Dimension formatting, auto-correct, geometry
│   ├── Cosmetic*.cpp       # Cosmetic edges, vertices, center lines
│   ├── CosmeticExtension.* # Extension for cosmetic annotations
│   └── Geometry.cpp        # Drawing-specific geometry helpers
├── Gui/                    # 318 files — UI, commands, task panels
│   ├── Command*.cpp        # 8 command files (views, dims, annotate, decorate...)
│   ├── QGI*.cpp            # Qt Graphics Items — 2D rendering primitives
│   ├── QGV*.cpp            # Qt Graphics Views — drawing page views
│   ├── ZVALUE.h            # Z-ordering constants for drawing layers
│   ├── Rez.h               # Resolution/scaling utilities
│   ├── DlgPrefs*.cpp       # 6 preference pages
│   └── Resources/          # Icons (111), UI files, translations
├── TechDrawTools/          # Python utility tools
├── Init.py
└── InitGui.py              # TechDrawGui::Workbench (C++)
```

## WHERE TO LOOK

| Task                   | Location                                                       |
| ---------------------- | -------------------------------------------------------------- |
| Add view type          | `App/DrawView{Type}.cpp` + `Gui/ViewProviderDrawing{Type}.cpp` |
| Add dimension type     | `App/DrawViewDimension.cpp` + `Gui/QGIViewDimension.cpp`       |
| Add command            | `Gui/Command{Category}.cpp`                                    |
| Modify 2D rendering    | `Gui/QGI*.cpp` — Qt Graphics Items                             |
| Add cosmetic element   | `App/Cosmetic*.cpp` + `CosmeticExtension.*`                    |
| Modify preference page | `Gui/DlgPrefs{Category}*.*`                                    |

## CONVENTIONS

- Commit prefix: `TD:` (not `TechDraw:`)
- `QGI*` classes = Qt Graphics Items for 2D page rendering
- `ZVALUE.h` defines layer ordering — respect Z-values when adding items
- `Rez.h` handles DPI/scaling conversion
- Drawing pages use SVG templates in `Resources/`

## NOTES

- TechDraw renders 3D → 2D projections using HLR (Hidden Line Removal) from OCCT
- Broken views (`DrawBrokenView`) use Boolean cuts internally
- Balloon/leader annotations have their own property enums (`BalloonPropEnum`, `ArrowPropEnum`)
