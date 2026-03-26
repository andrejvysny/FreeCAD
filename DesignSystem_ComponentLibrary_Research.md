# Figma-to-FreeCAD pipeline: feasible but requires a custom bridge

**A Figma-to-FreeCAD component pipeline is technically achievable — but no off-the-shelf solution exists.** FreeCAD's UI is built exclusively on Qt Widgets (C++/Python), and every existing Figma-to-Qt tool targets QML, not Widgets. The most practical path is a token-first pipeline: extract design tokens from Figma via Tokens Studio, transform them through Style Dictionary into QSS stylesheets and Jinja2-templated `.ui` files, and integrate those artifacts into FreeCAD's existing build system. This approach delivers roughly **60–70% of the value** — consistent theming, reduced manual translation, a single source of truth — at a fraction of the cost of full design-to-code automation. The remaining gap (complex layouts, custom widget behavior, interactive logic) must be bridged manually, a constraint confirmed by Qt's own engineers.

---

## FreeCAD runs on Qt Widgets exclusively — no QML anywhere

FreeCAD's GUI sits on a layered stack: **Qt Widgets** for all 2D interface elements, **Coin3D/Open Inventor** (via a custom-embedded Quarter library) for the 3D viewport, and **PySide2/PySide6** for Python-side UI creation. The codebase is migrating from Qt 5 to **Qt 6.4+ minimum** (announced January 2026), with Qt 5 support being dropped entirely for the development head targeting FreeCAD 1.2. The C++23 standard is now required.

The architecture follows a strict **App/Gui separation** (MVC pattern). The App layer has zero Qt dependency and communicates with the Gui layer through `boost::signals2`. Key C++ classes define the framework: `Gui::MainWindow` (QMainWindow subclass), `Gui::ControlSingleton` (task panel lifecycle manager), `Gui::DockWindowManager` (dock panel registry), `Gui::ViewProvider` hierarchy (Coin3D scene graph per object), and `Gui::Workbench` / `Gui::PythonWorkbench` (the workbench system). All UI panels, dialogs, and toolbars use Qt Widgets — there are **zero `.qml` files** in the FreeCAD source tree.

Task panels — FreeCAD's primary tool interaction surface — work through a specific pattern. A `Gui::TaskView::TaskDialog` base class (using the QSint collapsible panel library) hosts content widgets, typically loaded from `.ui` files. In C++, workbenches subclass `TaskDialog` and wire up signals. In Python, addons create a class with a `self.form` attribute (a QWidget, often loaded via `FreeCADGui.PySideUic.loadUi()`), then call `FreeCADGui.Control.showDialog()`. Only one task dialog per document can be active at any time.

The **Workbench system** uses lazy loading for fast startup. Each workbench (C++ or Python) registers via `InitGui.py`, defining toolbars, menus, and dock panels through `setupToolBars()` / `setupMenuBar()` (C++) or `appendToolbar()` / `appendMenu()` (Python). FreeCAD ships hundreds of `.ui` files across its workbenches — in `src/Mod/PartDesign/Gui/`, `src/Mod/Sketcher/Gui/`, `src/Mod/TechDraw/Gui/`, and similar directories — compiled via `uic` at build time (C++) or loaded at runtime (Python).

A 2023 forum thread explored QML adoption, but participants concluded it was impractical: QML cannot embed Qt Widgets inside it (only the reverse via `QQuickWidget`), QML/QtQuick packaging remains unreliable across FreeCAD's target platforms, and a full rewrite would be unusable until complete. FreeCAD's current path forward remains **Qt Widgets with incremental modernization**.

---

## No Figma-to-Qt Widgets path exists, but QML bridges are maturing

The research turned up a critical gap confirmed by a Qt employee on the Figma forum: **"Qt Widgets are done purely with C++... Translating UI design to Qt Widgets is a painful task."** Every Figma-to-Qt conversion tool generates QML, not Widgets. The complete inventory of relevant tools:

**Qt's official "Figma to Qt" plugin** entered public beta in December 2025. It generates clean, development-ready QML from Figma designs, translates Auto Layout to Qt's new FlexboxLayout, reads Figma local variables as design tokens, and includes live preview with code inspection. It requires **Qt 6.10+** for deployment and is free to use. This represents Qt's strategic investment in the Figma→QML path but explicitly excludes Widgets.

**FigmaQML** (github.com/mmertama/FigmaQML) is the sole open-source alternative — a standalone C++ application that reads Figma data via the REST API and generates QML for each page/canvas. It supports both desktop and MCU targets, allows injecting custom QML via `FigmaQmlSingleton::setSource`, and exports application code with CMake integration. The older **Qt Bridge for Figma** requires a Qt Design Studio Enterprise license (~€2,300/year) and is being gradually superseded by the new plugin.

No mainstream Figma-to-code tool supports Qt output at all. Anima, Locofy, Builder.io, and Figma Code Connect all target web/mobile frameworks exclusively. The **Figma REST API** provides comprehensive extraction — full node trees, geometry, fills, strokes, text properties, component properties, and (on Enterprise plans) variables — but converting this data to Qt Widgets requires a custom pipeline.

For **design tokens specifically**, a mature path exists. Tokens Studio for Figma manages tokens in DTCG format and syncs to Git. Style Dictionary (by Amazon) transforms these tokens into platform-specific output via custom formats. A working example at github.com/TilmanGriesel/style-dictionary-qml-example demonstrates QML token generation. Custom Style Dictionary formats can similarly output QSS or C++ constants. The **W3C Design Tokens specification reached its first stable version (2025.10)** in October 2025, making it a reliable intermediate format.

---

## O3DE's Blue Jay provides the closest "Storybook for Qt" reference

There is **no general-purpose, open-source "Storybook for Qt Widgets" tool**. The closest existing equivalent is **Open 3D Engine's (O3DE) Blue Jay Design System**, which includes a standalone Qt Control Gallery application (`O3DEQtControlGallery.exe`). This gallery features a dropdown to navigate through available components, interactive widget examples on the right panel, and sample code on the left — exactly the pattern a FreeCAD component catalog would follow.

O3DE's **AzQtComponents library** (in `Code/Framework/AzQtComponents/`) provides the architectural blueprint: custom widget classes in a `Components/Widgets/` directory, per-widget QSS files (`CheckBox.qss`, `PushButton.qss`), a `StyleManager` class installing a `QProxyStyle` at the application level, full API documentation, and component development guidelines. This pattern — **self-contained widgets with paired stylesheets and a gallery browser** — is directly applicable to FreeCAD.

FreeCAD itself lacks a formal design system but has active initiatives. A community-led **Design Guide** (documented at ondsel.com/blog/freecad-breaking-open-source-ux-curse/) establishes UX principles, UI zones, and task flow guidelines. The official **Developers Handbook roadmap** calls for a "UI/UX Style Book" and UI normalization. GitHub issue #13120 highlights the current problem: inconsistent OK/Cancel button placement across task panels — exactly the kind of inconsistency a component library solves.

FreeCAD's existing `Gui::WidgetFactory` (in `src/Gui/WidgetFactory.cpp`) provides a registry-based widget creation system that could serve as the foundation for a proper component library. Combined with Qt Designer plugins via `QDesignerCustomWidgetInterface`, custom FreeCAD widgets could be made available for visual editing in Qt Designer and browsable in a gallery application.

For the gallery/preview UI itself, **QQuickWidget** offers an interesting option: a modern QML-based component browser could embed within FreeCAD's Qt Widgets infrastructure without requiring the actual component library to use QML. Qt 6.4 significantly improved QQuickWidget with Vulkan, Metal, and Direct3D support beyond just OpenGL.

---

## The recommended architecture: a four-layer custom pipeline

Based on the research, the most practical Figma-to-FreeCAD architecture is a four-layer pipeline, implemented in phases:

**Layer 1 — Extraction**: Tokens Studio for Figma manages design tokens (colors, typography, spacing, border radii) and syncs them to a Git repository in DTCG JSON format. The Figma REST API supplements this with component structure data. For teams without Figma Enterprise, Tokens Studio's plugin API accesses variables on any paid plan, working around the Enterprise-only Variables REST API limitation.

**Layer 2 — Intermediate Representation**: W3C DTCG format (v2025.10) serves as the canonical token format. A separate JSON schema defines component specifications — mapping Figma components to Qt widget classes, specifying layout structures (QVBoxLayout, QHBoxLayout, QGridLayout), and declaring property bindings. This layer is where semantic translation happens: a Figma "Button" component maps to `QPushButton`, a Figma Auto Layout frame maps to a QBoxLayout with specific spacing tokens.

**Layer 3 — Code Generation**: Style Dictionary with custom formats transforms DTCG tokens into QSS stylesheets, QPalette initialization code, and C++ constants. Jinja2 templates (a pattern Qt itself uses in its Interface Framework) generate `.ui` XML files from component specifications. A Python generation script renders these templates, producing files compatible with FreeCAD's existing `uic` compilation workflow.

**Layer 4 — FreeCAD Integration**: Generated `.ui` files drop into FreeCAD's source tree alongside existing ones. Generated `.qss` files integrate with FreeCAD's theme system. The CMake build system compiles `.ui` files to C++ headers via `uic` as it already does for hundreds of existing files.

---

## Phased implementation and honest feasibility assessment

**Phase 1 (2–3 weeks): Design Token Pipeline.** Set up Figma with Tokens Studio, export DTCG tokens, create Style Dictionary config with custom QSS format, generate a `.qss` file for FreeCAD theming. This delivers consistent colors, typography, and spacing from a single source of truth — high value, low risk.

**Phase 2 (4–6 weeks): Component Scaffolding.** Define a JSON schema mapping Figma components to Qt widget types. Build Jinja2 templates for common FreeCAD patterns (task panels, preference pages, dialogs). A Python script reads specifications and generates `.ui` files. This cannot handle complex layouts or custom widgets automatically, but it eliminates boilerplate for **standard form-based panels** that make up the majority of FreeCAD's UI.

**Phase 3 (2–3 weeks): CI/CD Automation.** Figma webhooks trigger token extraction; Style Dictionary builds generate QSS and token files; the component generator produces `.ui` files; an automated PR lands in the FreeCAD repository. This closes the loop from design change to code artifact.

**Phase 4 (future, higher risk): QML Hybrid.** For new FreeCAD dialogs, use QML embedded via QQuickWidget. The official Figma to Qt plugin generates QML directly. Shared design tokens feed both the QSS path (existing widgets) and QML path (new components). The risk here is QML packaging across FreeCAD's diverse platform targets — AppImage, Conda, Flatpak — which remains a concern noted in forum discussions.

The hard limits should be stated plainly. **Full layout automation from Figma to Qt Widgets is not achievable** — the semantic gap between a visual design tool's absolute/auto-layout model and Qt's widget/layout system is too large. Complex interactions, state machines, data binding to C++ models, and custom widget behaviors (like FreeCAD's `Gui::PrefQuantitySpinBox` or `Gui::TaskView::TaskBox`) require manual implementation. The pipeline works best as a **scaffolding and consistency tool**, not a replacement for UI development expertise.

KDAB's analysis also warns that QSS carries **measurable performance costs** in widget instantiation and reparenting. For performance-critical UIs with many widgets, `QProxyStyle` subclassing is preferred over QSS. A production pipeline should generate QSS for simple dialogs and QProxyStyle code for complex, frequently-instantiated views.

---

## Conclusion

The Figma-to-FreeCAD pipeline occupies an underserved niche. FreeCAD's strict Qt Widgets architecture means the QML tools now maturing in the Qt ecosystem (particularly the official Figma to Qt plugin) cannot be used directly. But the foundational pieces exist: **DTCG tokens are standardized, Style Dictionary is extensible, `.ui` files are simple XML amenable to template generation, and FreeCAD's build system already processes hundreds of `.ui` files.** The missing piece is the custom glue — a component specification schema, Jinja2 templates for FreeCAD-specific patterns, and Style Dictionary formats for QSS output. O3DE's Blue Jay Design System proves that a Qt Widgets component gallery with a Storybook-like experience is achievable and provides a concrete architectural reference.

The strategic insight is that **QML adoption in FreeCAD, even incrementally via QQuickWidget, would unlock the entire Qt Figma ecosystem.** Until then, the token-first pipeline described here offers the best return on investment: a single source of truth in Figma, automated theming via QSS, scaffolded `.ui` files for standard panels, and a component gallery for developer consistency — bridging much of the gap between design intent and FreeCAD implementation.