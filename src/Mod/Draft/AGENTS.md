# src/Mod/Draft/ — 2D Drafting (Python)

2D drafting tools. 503 files. Mostly Python using FeaturePython pattern. Foundation for BIM workbench.

## STRUCTURE

```
Draft/
├── App/                    # Minimal C++ (DraftApp module init only)
├── draftobjects/           # Python DocumentObject proxies (Rectangle, Wire, Circle...)
├── draftviewproviders/     # Python ViewProviders for draft objects
├── draftmake/              # Factory functions: make_rectangle(), make_wire()...
├── draftfunctions/         # Operations: move, rotate, offset, fuse, upgrade...
├── draftgeoutils/          # 2D geometry utilities (intersections, offsets, arcs)
├── draftguitools/          # Interactive GUI tools (65 files)
├── drafttaskpanels/        # Task panel implementations
├── draftutils/             # Shared utilities (gui_utils, translate, params)
├── drafttests/             # Draft-specific tests
├── Resources/              # Icons (110), translations (93)
├── Init.py                 # Registers DXF, DWG, SVG, OCA import/export
└── InitGui.py              # Gui::PythonWorkbench
```

## WHERE TO LOOK

| Task                  | Location                                                                      |
| --------------------- | ----------------------------------------------------------------------------- |
| Add draft object      | `draftobjects/{type}.py` + `draftmake/make_{type}.py` + `draftviewproviders/` |
| Add GUI tool          | `draftguitools/gui_{tool}.py`                                                 |
| Add geometry utility  | `draftgeoutils/`                                                              |
| Add draft function    | `draftfunctions/{operation}.py`                                               |
| Modify DXF/SVG import | Check `Init.py` for registered importers                                      |

## CONVENTIONS

- FeaturePython pattern: `addObject("Part::Part2DObjectPython", name)` + proxy class
- Factory functions in `draftmake/` — always use these, never construct objects directly
- `draftutils/translate.py` for i18n — use `translate("Draft", "text")`
- Snap system in `draftguitools/gui_snapper.py`

## ANTI-PATTERNS

- Many deprecated functions in `draftfunctions/svg.py` — check before using
- Several bare `except:` violations in `draftutils/gui_utils.py`, `draftgeoutils/offsets.py`
- Some files still use `App.ActiveDocument` instead of `obj.Document`
- Draft is BIM's foundation — breaking Draft breaks BIM
