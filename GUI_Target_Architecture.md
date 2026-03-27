# GUI Target Architecture for FreeCAD Gui

## Summary

This document defines the target architecture for FreeCAD's `Gui` layer as a
problems-first, design-system-driven architecture. It is intentionally limited
to `Gui` and to the design-system integration points that directly affect `Gui`.
It does not redesign `Base`, `App`, or workbench `App` logic.

The main architectural goal is not "more abstraction" or "more components". The
goal is to make the `Gui` layer coherent enough that FreeCAD can converge on a
documented, reusable, token-driven design system without breaking realistic
adoption constraints:

- standard Qt widgets are already used widely in core FreeCAD
- `.ui` files are a real and persistent part of the codebase
- external workbenches and addons cannot be forced to rewrite their UI around
  custom widget subclasses
- some legacy widgets and 3rd-party integrations cannot be removed on demand

The target state therefore combines:

- token-driven theming through `QStyle`, not custom QSS as the primary path
- pure reusable Qt components in `FCComponentLib`
- FreeCAD-specific behavior in `Gui`, primarily through adapters, controllers,
  and runtime services
- optional custom widgets only where they provide genuinely new capability
- workflow standardization for repeated patterns such as geometry selection
- an incremental migration model that works for core FreeCAD and for external
  consumers

The target architecture is governed by Kadet's process constraints:

- problems and use cases must be defined before solution proposals
- documentation is architectural truth; exploratory implementation is not
- token structure and `QStyle` foundations come before advanced components
- architecture must reduce the spectrum of allowed UI choices
- overlapping parallel architecture tracks should be avoided

## Scope And Hard Constraints

This document assumes the following scope and constraints:

- Scope is strictly `Gui` plus design-system integration points that directly
  affect `Gui`
- No redesign of `Base` or `App`
- No requirement that external workbenches or addons adopt new custom widgets
- No assumption that `.ui` files will disappear
- No assumption that existing standard Qt widgets can be replaced wholesale

This document also assumes the working rule used for this session:

- the only artifact created from this work is `GUI_Target_Architecture.md`

## Current State: Problems And Needs

### Current-State Problems

FreeCAD `Gui` has accumulated significant capability, but the implementation is
architecturally inconsistent. The main current-state problems are:

1. `Gui` is a patchwork of changes performed in different places without a
   sufficiently clear whole-picture architecture.
2. Many widgets and panels mix rendering, workflow logic, state management,
   persistence, and FreeCAD integration in the same class.
3. Repeated workflows such as geometry or object selection have too many local
   implementations.
4. Custom QSS and hardcoded metrics create inconsistency and poor portability.
5. Existing standard Qt widgets, `.ui` files, and addon code impose real
   adoption constraints that many idealized solutions ignore.
6. The current codebase uses standard Qt widgets in many places, so any
   strategy that depends on mandatory `FC*` replacements would fail in
   practice.
7. Legacy widgets and 3rd-party pieces exist and cannot always be removed
   outright.

### Problem Areas In The Current Codebase

#### 1. Shell runtime is over-centralized

`Gui::Application` currently acts as a runtime hub for bootstrap, Python
exposure, workbench lifecycle, command creation, theme/style services, and
document tracking. `MainWindow` is also oversized and mixes shell hosting with
startup, recovery, and session policy.

This creates:

- global reach-through
- blurred ownership
- difficult lifecycle control
- weak separation between shell concerns and document/view concerns

#### 2. Workbench and command ownership are not cleanly modeled

Workbench activation and shell composition are still largely imperative and
rebuild-oriented. Command infrastructure is global and mixed with Python
dispatch, macro logging, shortcut concerns, and enablement logic.

This creates:

- split ownership of workbench truth
- shell rebuild logic tied to runtime side effects
- weak composition boundaries
- hard-to-reason-about enablement and execution paths

#### 3. Document/view runtime is over-fused

`Gui::Document`, `View3DInventorViewer`, `Selection/Selection.cpp`, and a large
part of the `ViewProvider*` layer combine responsibilities that should be owned
by distinct runtime services.

Examples:

- `Gui::Document` handles provider ownership, scene wiring, edit state,
  persistence, and document/UI lifecycle
- `View3DInventorViewer` combines scene hosting, navigation, selection wiring,
  edit overlays, dimensions, drag-and-drop, and viewer policy
- selection is global enough to be convenient but coupled enough to be
  architecture-hostile
- view providers often reach into workflow or shell concerns that do not belong
  in presentation objects

This creates:

- fragile scene ownership
- edit-mode complexity
- poor testability
- direct coupling between viewer logic, selection logic, and UI behavior

#### 4. Panels are widget-centric instead of runtime-centric

The tree, property editor, and task panel layers are classic examples of
widget-centric architecture:

- `Tree.cpp`
- `propertyeditor/PropertyView.*`
- `propertyeditor/PropertyEditor.*`
- `propertyeditor/PropertyItem.*`
- `TaskView/*`

These areas often combine:

- model projection
- state synchronization
- editor creation
- transaction logic
- workflow control
- direct interaction with global services

This creates:

- oversized classes
- duplicated synchronization code
- brittle selection coupling
- difficult migration toward design-system rules

#### 5. Styling and component responsibilities are mixed

FreeCAD currently contains:

- standard Qt widgets
- `.ui`-driven dialogs and panels
- logic-heavy custom widgets
- QSS usage
- newer token/style experiments

The result is that styling, state, workflow behavior, and component semantics
are frequently interleaved. This directly conflicts with a design-system-first
architecture.

#### 6. Repeated workflow patterns are not standardized

Geometry selection is the flagship example, but it is not the only one. Other
examples include:

- task-panel layout patterns
- form layout patterns
- toolbar checkable controls
- notification and banner behavior
- property editing workflows

Without canonical workflow standards, the design system cannot become a real
source of truth. It would become a bag of component visuals with many local
exceptions.

### Needs The Target Architecture Must Satisfy

The new architecture must satisfy all of the following:

- consistent theming without custom QSS as the primary mechanism
- clear separation between pure presentation components and FreeCAD logic
- compatibility with standard Qt widgets
- compatibility with `.ui`-driven workflows
- a realistic adoption path for core modules and external workbenches/addons
- optional rather than mandatory custom widgets, except where genuinely new
  capabilities are needed
- explicit migration and validation rules
- a design-system process where specification precedes implementation

## Non-Negotiable Stakeholder Constraints

These constraints are treated as requirements, not preferences:

- The design system is documentation-first.
- Architecture must follow defined problems and use cases, not lead them.
- Implementation is allowed as research, but research code is not
  architectural truth.
- The target must reduce the number of allowed UI choices to a finite,
  documented set.
- `FCComponentLib` should contain pure reusable Qt components only.
- FreeCAD-specific logic should live in `Gui`, mainly through adapters,
  controllers, and services.
- Overlapping parallel architecture tracks should be avoided.
- Token structure and `QStyle` foundations come before advanced or specialized
  components.
- Geometry selection is the canonical example of "specify first, design second,
  implement third".

## Architectural Principles

### 1. Problems Before Solution

Architecture is only valid if the problems, requirements, constraints, and
migration path are explicit. A preferred pattern is not enough.

### 2. Constrained Choice

A small, documented set of approved variants is preferable to arbitrary local
configuration. The design system should define what is adjustable and what is
intentionally fixed.

### 3. Pure Components, Impure Adapters

Reusable components belong in `FCComponentLib` and should remain logic-light,
deterministic, and testable in isolation. FreeCAD document state, selection
state, commands, and workflow integration belong in `Gui`.

### 4. Standard Qt First

The architecture must work well with existing Qt widgets. It cannot depend on
everyone switching from `QPushButton` to a nearly identical `FCPushButton`.

### 5. Optional Custom Widgets Only

Custom widgets are justified when they provide new capability, such as a split
button or a specialized selection control. They are not the baseline mechanism
for ordinary controls that already exist in Qt.

### 6. Documentation Is Part Of The Architecture

Component specs, workflow specs, layout guidance, usage rules, and migration
guidance are not optional documentation. They are core architectural artifacts.

### 7. Migration Is Part Of The Design

Architecture that cannot be incrementally adopted in core FreeCAD, `.ui`
workflows, and external addons is not an acceptable target architecture.

## Canonical Layering

The target dependency direction is:

```text
Problems / Requirements / Use Cases
  -> Foundations / Tokens / Palette Model
  -> Theme Runtime / QStyle
  -> FCComponentLib
  -> Playground / Gallery / Documentation Host
  -> Gui/Adapters
  -> Gui Controllers / Context Services / Session Services
  -> Shell / Workbench / Document / Panel Runtime
```

This layering means:

- the playground/gallery is useful, but it is not the source of truth by
  itself
- pure components must remain usable in isolation
- FreeCAD-specific logic stays out of the component library
- the shipped `Gui` runtime owns workflow logic and document integration

## Existing Building Blocks Worth Preserving

The current codebase is messy, but it already contains some useful seeds that
should be preserved and formalized instead of replaced blindly:

- `Gui/Adapters/AdapterRegistration.*`
- `Gui/Adapters/WidgetBinder.*`
- `Gui/WidgetFactory.cpp`
- `Gui/UiLoader.cpp`
- menu and toolbar descriptor trees
- preference page registration/factory patterns
- task dialog and watcher extension points
- the existing token and `QStyle` direction now under development

The target architecture should build on these seeds where they align with the
documented direction.

## Target Architecture

### Overview

The target state is a layered modular monolith inside `FreeCADGui`. It is not a
microservice-style split and not a rewrite around a brand new widget toolkit.
It is a runtime architecture that clarifies ownership and reduces coupling.

At a high level:

```text
GuiBootstrap
  -> GuiRuntime
     -> ThemeRuntime
     -> ShellWorkspace
     -> WorkbenchRuntime
     -> CommandRuntime
     -> GuiContextService
     -> DocumentPresentationSession[*]
        -> ViewProviderRepository
        -> SceneCompositionService
        -> EditSessionService
        -> SelectionService
        -> ViewerRuntime
     -> PanelRuntime
        -> Tree / Property / Task / Preferences controllers
     -> Gui/Adapters
```

### 1. GuiBootstrap

`GuiBootstrap` owns:

- Qt application startup for the GUI process
- resource initialization
- GUI type registration
- Python GUI exposure setup
- startup plumbing that is fundamentally bootstrap-specific

It should be the only layer allowed to own low-level startup sequencing.

### 2. GuiRuntime

`GuiRuntime` is the top-level owner of `Gui` services and runtime composition.
It replaces accidental singleton ownership with deliberate runtime-owned
services.

During migration, legacy global accessors may remain as compatibility facades,
but authoritative state should move into runtime-owned services.

### 3. ThemeRuntime

`ThemeRuntime` is a required architectural subsystem. It owns:

- token resolution
- derived-token computation
- palette stepping and palette access
- `QStyle` integration
- theme switching
- rendering caches
- compatibility bridges for theme editor and playground/gallery usage

It is the runtime owner of the theming model, not merely a stylesheet helper.

#### Token Model

The architecture should recognize at least these token categories:

- base tokens
- semantic tokens
- component tokens
- state tokens
- palette concepts

The document intentionally does not freeze one exact palette encoding. Required
capabilities matter more than one final representation.

The following rules are target-state rules:

- there may be many tokens but few independent values
- tokens represent concepts, not merely raw literals
- semantic distinction matters even when default values coincide
- most component values should be derived from a relatively small base set
- visually correct results are more important than mathematically neat
  derivations
- control sizing should not be directly derived from system font metrics

### 4. ShellWorkspace

`ShellWorkspace` owns live shell surfaces:

- `MainWindow`
- dock hosts
- MDI/view area hosting
- overlay containers
- status and message surfaces
- notification surfaces
- banner surfaces
- shell chrome persistence hooks

It should not own startup policy, workbench discovery, or document-specific
state.

### 5. WorkbenchRuntime

`WorkbenchRuntime` owns:

- workbench catalog
- workbench activation/deactivation lifecycle
- active workbench state
- layout application for workbench-scoped shell surfaces
- persistence of last active workbench where appropriate

It should remove split ownership between multiple registries by becoming the
single authoritative workbench runtime.

### 6. CommandRuntime

`CommandRuntime` owns:

- command registry
- action creation
- shortcut binding
- enablement evaluation
- execution dispatch
- macro logging integration

It should separate command metadata from execution behavior and stop command
objects from being universal dumping grounds for unrelated concerns.

### 7. GuiContextService

`GuiContextService` provides typed snapshots and change notifications for UI
consumers. At minimum, it should cover:

- active document context
- active view context
- selection context
- workbench context
- command context
- preference and theme context

Widgets should consume context through controllers or adapters, not by reaching
directly into global singletons.

### 8. DocumentPresentationSession

Each open GUI document should have a `DocumentPresentationSession`. It owns the
document-scoped presentation runtime:

- adaptation of App-level document events into GUI presentation state
- provider repository ownership
- viewer/session persistence hooks
- document-scoped event fan-out for GUI services

It should replace the current over-fused `Gui::Document` responsibility set
with explicit document-scoped services.

### 9. ViewProviderRepository

`ViewProviderRepository` is the authoritative mapping between document objects,
view providers, provider identity, and viewer-facing bindings.

This repository should become the single owner of provider registration and
lookup in the document presentation layer.

### 10. SceneCompositionService

`SceneCompositionService` owns the rules for scene graph composition:

- normal scene content
- claimed child providers
- annotation layers
- edit overlays
- dimension layers
- selection-related scene contributions

Its purpose is to remove scene ownership surgery from unrelated classes and make
normal mode and edit mode scene composition explicit and testable.

### 11. EditSessionService

`EditSessionService` owns:

- edit mode identity
- edited object/provider/subelement state
- edit lifecycle transitions
- restoration rules when edit state changes
- association with document/view context

Viewer overlays should render edit state. They should not own edit truth.

### 12. SelectionService

`SelectionService` should be re-centered around a document-scoped core, with
global compatibility facades only where needed for legacy APIs.

It owns:

- selection state
- preselection state
- selection history
- picked element data
- typed selection events
- selection policy and filtering hooks

It must not directly drive tree widgets, task widgets, or shell behavior.

### 13. ViewerRuntime

`ViewerRuntime` should be a composed runtime owned by the document presentation
session. `View3DInventorViewer` remains the Qt/Coin host, but its
responsibilities should be delegated to smaller subsystems.

Recommended internal seams:

- `SceneHost`
- `NavigationController`
- `SelectionOverlayController`
- `EditOverlayController`
- `DimensionOverlayController`

This keeps Qt/Coin hosting separate from workflow logic and visual overlays.

### 14. PanelRuntime

`PanelRuntime` owns panel descriptors, panel controllers, panel attachment
policy, and panel lifecycle rules. It should host panels through a stable
controller contract instead of relying on self-owning god widgets.

Recommended shared contracts:

- `PanelDescriptor`
- `IPanelController`

#### Panel rules

- widgets render state and user input
- controllers own synchronization and runtime integration
- panel widgets should not directly depend on `Application::Instance`,
  `Selection()`, `Control()`, or `getMainWindow()`

## Component-System Architecture

The design system should define components through explicit specs. Every
component spec should define:

- purpose
- allowed variants
- fixed properties
- adjustable properties
- states
- token hooks
- layout behavior
- accessibility and usability constraints
- DOs and DON'Ts

Target-state rules:

- transient `pressed` state and persistent `checked`/`toggled` state remain
  distinct
- size and spacing choices are finite
- extra variation should not be introduced without clear benefit

This is particularly important for controls such as:

- push buttons
- tool buttons
- checkboxes
- toggle buttons
- spinboxes
- form controls
- panel layout structures

## Component Library And Playground

### Role

A design-system component library with a playground/gallery is desirable and
useful. In the target architecture it should serve as:

- a host for isolated component rendering and testing
- a preview surface for tokens, variants, states, and themes
- a tool that fits theme-editor workflows
- a place that can host or export design-system documentation and guidelines

### Limits

The playground/gallery is not an architecture track of its own. It must not:

- become the source of truth independent of the design-system spec
- contain FreeCAD business or workflow logic
- bypass the `Gui/Adapters` boundary
- drive architecture ahead of documented requirements

The correct relationship is:

- documentation and component contracts define the system
- the playground/gallery demonstrates and validates it
- `Gui` integrates it into FreeCAD runtime behavior

## Adapter Boundary

`FCComponentLib` should remain pure and reusable. `Gui/Adapters` is the
official bridge into FreeCAD runtime behavior.

Adapters are the right place to bind:

- document state
- selection state
- command wiring
- contextual enablement
- layout/session behavior when tied to FreeCAD runtime

However, adapters alone do not solve adoption. The architecture must also work
for:

- standard Qt widgets styled through `QStyle`
- `.ui` files
- legacy widgets that can only be improved or wrapped incrementally
- 3rd-party workbenches and addons that will not adopt a fully custom widget
  stack

## Standard Qt Widgets, `.ui` Files, And Legacy Widgets

This is a first-class constraint, not an implementation detail.

### Standard Qt widgets

The design system must style and harmonize standard Qt widgets such as:

- `QPushButton`
- `QToolButton`
- `QCheckBox`
- `QComboBox`
- `QSpinBox`
- `QLineEdit`
- standard item views and delegates

It must do so without requiring custom replacement subclasses as a baseline
strategy.

### `.ui` files

`.ui` files remain a real constraint and must be supported. When extra semantic
configuration is needed, the architecture should prefer:

- lightweight metadata
- widget properties
- post-processing
- adapter attachment
- documented conventions

It should not require every `.ui` author to replace ordinary Qt widgets with
FreeCAD-only subclasses.

### Legacy and 3rd-party widgets

Existing widgets such as QSint-based collapsible areas are not ideal, but they
cannot simply be wished away. The target rule is:

- improve them where practical
- wrap them when necessary
- replace them only when the migration cost is justified

### Custom widgets

Custom widgets are appropriate when they add genuinely new capability. Examples:

- split button
- advanced selection control
- richer design-system-specific compound control

Custom widgets are not appropriate as mandatory replacements for standard
controls that already meet the required capability.

## Gui Runtime By Subsystem

### Shell and workbench layer

Current major issues:

- `Gui::Application` is overloaded
- `MainWindow` mixes shell hosting with policy
- workbench composition is too imperative
- command runtime concerns are over-fused

Target response:

- move to `GuiBootstrap`, `GuiRuntime`, `ShellWorkspace`, `WorkbenchRuntime`,
  and `CommandRuntime`
- preserve compatibility facades during migration
- reduce global reach-through

### Document and viewer layer

Current major issues:

- `Gui::Document` owns too much
- scene ownership is fragile
- edit mode is too invasive
- `View3DInventorViewer` hosts too many concerns
- selection is globally convenient but architecturally over-coupled

Target response:

- introduce `DocumentPresentationSession`, `ViewProviderRepository`,
  `SceneCompositionService`, `EditSessionService`, `SelectionService`, and
  `ViewerRuntime`
- keep the viewer host thin relative to the runtime services around it

### Panel layer

Current major issues:

- tree/property/task layers are widget-centric
- synchronization is scattered
- transaction and workflow logic are embedded in widgets
- panel widgets reach into global services directly

Target response:

- move to `PanelRuntime`, `PanelDescriptor`, and `IPanelController`
- make widgets renderers and controllers the integration point

## Workflow-Level Standards

The design system must include a workflow or pattern layer above atoms and basic
components. This is essential because a large part of FreeCAD inconsistency
comes from repeated workflows being reimplemented locally.

Priority workflow candidates:

- geometry or object selection
- task-panel layout patterns
- form layout patterns
- toolbar checkable-control patterns
- notifications, banners, and attention patterns

For each workflow pattern, the target process is:

1. inventory current implementations
2. extract requirements and use cases
3. define one canonical standard or one replacement architecture
4. define a reusable implementation path
5. define an adoption path for core and, where realistic, for addons

## Geometry Selection As The Flagship Example

Geometry selection should be treated as the flagship example of the required
method.

The problem is not merely that a better widget might be missing. The actual
problem is that selection-related workflows are duplicated and inconsistent
across the GUI.

The target method is:

1. document existing implementations and where they are used
2. extract use cases and requirements
3. define the target selection architecture
4. only then introduce reusable components, adapters, or widgets if they are
   needed

Target-state rules for geometry selection:

- one approved architecture, not many local reinventions
- selection behavior belongs to services and policies, not to isolated widgets
- migration must consider existing workflows and addon impact

## Notifications And Attention Patterns

Notifications are a design-system and runtime concern, not only a visual one.
The target architecture should distinguish:

- blocking dialogs
- persistent banners
- transient notifications or toasts
- a notification area or feed

The architecture should prefer non-blocking surfaces where the user's task does
not need to be interrupted. This should become a reusable workflow standard,
not a local UI decision repeated differently across the application.

## 3D Gizmos And OVP Scope

The design system should cover the visual language of 3D gizmos and similar
overlayed controls where they touch `Gui` visuals. It should not assume that
all gizmo behavior is part of the same architecture effort.

For object view panels and similar controls:

- ordinary controls such as spinboxes inherit normal component rules
- bespoke 3D manipulation behavior remains outside the immediate design-system
  scope unless explicitly standardized as a workflow

## Migration And Compatibility Rules

Migration must be explicit. The target architecture should define three
adoption paths.

### 1. Passive adoption

Standard Qt widgets improve automatically through token-driven `QStyle` and
theming, with little or no code change.

Use this for:

- ordinary buttons
- standard inputs
- standard view delegates where token/styling changes are sufficient

### 2. Guided adoption

Core modules or addons use documented properties, roles, helpers, adapters, or
layout conventions to opt into richer behavior.

Use this for:

- `.ui` files that need semantic configuration
- panels that can attach to adapters after load
- screens that need documented layout semantics

### 3. Optional specialized adoption

Custom widgets are available for genuinely new capability, but they are not the
baseline requirement for ordinary controls.

Use this for:

- split buttons
- specialized selection controls
- design-system-specific compound widgets

### Migration rules

The final architecture must enforce the following rules:

- external workbenches and addons are first-class compatibility concerns
- broad mandatory rewrites are not acceptable
- migration should be incremental, likely workbench-by-workbench or
  subsystem-by-subsystem
- any proposed abstraction must be evaluated for optionality, adoptability, and
  total migration cost

## Recommended Migration Sequence

1. Finish and stabilize foundations documentation.
2. Formalize token taxonomy, palette capabilities, and `ThemeRuntime`.
3. Stabilize token-driven `QStyle` integration as the baseline theming engine.
4. Formalize the component spec template and documentation format.
5. Keep `FCComponentLib` pure and formalize the adapter boundary in `Gui`.
6. Define the playground/gallery as a support tool for docs, theming, and
   component validation.
7. Extract workflow-level standards, starting with geometry selection and
   notification patterns.
8. Introduce specialized custom widgets only when the specification proves they
   are needed.
9. Migrate incrementally through core subsystems and workbenches while keeping
   passive adoption available to standard Qt usage.

## Validation Criteria

The target architecture is acceptable only if all of the following are true:

- standard Qt controls remain visually consistent under the token/`QStyle`
  system without mandatory replacement classes
- pure components render correctly in isolation in the playground/gallery
- documented variants and states match between the playground and the shipped
  `Gui`
- `.ui`-driven interfaces remain compatible with the target approach
- legacy and 3rd-party widgets can be improved or wrapped incrementally rather
  than requiring immediate removal
- `Gui` adapters bind pure components to FreeCAD logic without contaminating
  `FCComponentLib`
- geometry selection is standardized through requirements extraction and one
  canonical architecture
- external workbenches and addons have a plausible low-friction adoption path
- shell concerns remain above document/view concerns
- document/view services remain separate from panel/widget rendering

## Anti-Goals

The target architecture explicitly rejects the following:

- mandatory `FC*` replacements for standard Qt widgets as the baseline strategy
- a design system defined only by visuals without workflow standards
- custom QSS as the long-term primary theming mechanism
- a component gallery that becomes a competing architecture track
- architecture proposals that do not first define the problems they solve
- removal of legacy widgets without migration-cost justification
- shell widgets owning document/view truth
- panel widgets acting as direct owners of global application state

## Assumptions And Defaults

- Kadet's problem-first, documentation-first process is authoritative for this
  target architecture
- the current built-in theme remains the baseline visual identity
- the design system should improve consistency and usability without an
  unnecessary visual restyle
- the component library and playground are useful, but secondary to the
  specification and token/component contracts
- advanced custom components come after token structure and `QStyle`
  foundations stabilize
- palette and token encoding details are intentionally left flexible where
  capabilities matter more than one specific syntax
- compatibility with external workbenches and addons is a design requirement,
  not an afterthought

## Final Recommendation

FreeCAD `Gui` should move toward a design-system-first modular monolith in
which:

- theming is centralized in a token-driven `ThemeRuntime`
- standard Qt widgets remain first-class citizens
- pure reusable components live in `FCComponentLib`
- FreeCAD-specific integration lives in `Gui/Adapters`, controllers, and
  session services
- shell, document/view, and panel responsibilities are explicitly separated
- repeated workflows are standardized through documented specs before new
  implementation is introduced

That is the most realistic target architecture for improving FreeCAD `Gui`
without repeating the current pattern of ad hoc changes in ad hoc places.
