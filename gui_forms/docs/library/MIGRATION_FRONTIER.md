# GUI.Forms source and atlas migration frontier

Status: **OBSERVED inventory with a CANDIDATE execution order**.

The grand-architect direction to isolate controls, core mechanisms, and state
machines is **GIVEN**. The bundle sequence below is not an architecture
decision; it is a reversible work order chosen to keep each change buildable,
testable, and visually reviewable. Accepted ADRs and contract registries remain
authoritative throughout the migration.

## Bundle status

| Bundle | Status | Cohesive scope |
| --- | --- | --- |
| 001 | **OBSERVED complete** | `Card`, `ReviewCard`, and `MasterDetailView`; hierarchical headers/sources, compatibility umbrella, activation-selection policy, focused tests, M4 capture crops, and reviewed atlas pages |
| 002 | **OBSERVED complete** | Basic retained primitives: `Panel`, `Label`, `ButtonBase`, `Button`, `CheckBox`, `RadioButton`, `LinkLabel`, `GroupBox`, and `PictureBox`; nine isolated translation units, shared private rendering helpers, bounded customization, focused M4 tests, native showcase crops, and reviewed atlas pages |
| 003 | **CANDIDATE next** | Containers and scrolling: `ContainerControl`, layout panels, tab controls, splitter controls, scaled containers, and scroll state |
| 004 | **CANDIDATE** | Input/range/date controls: text, list, combo, checked list, numeric input, date/time picker, progress, track, and scroll bars |
| 005 | **CANDIDATE** | Collections and transient surfaces: tree/object/correspondence views, menus, popups, tooltips, help, and error guidance |
| 006 | **CANDIDATE** | Inspection and specialized visual surfaces: property editors, instrument controls, diagnostics, raster canvas, and material panels |
| 007 | **CANDIDATE** | Window/host lifecycle state machines: `Window`, scheduler, dispatcher, host session/services, and private platform hosts |
| 008 | **CANDIDATE** | Presentation and high-throughput buffers: `LiveSurface`, write leases, snapshots, wakes, frame scheduling, display chunks, and device damage |
| 009 | **CANDIDATE** | Drawing, text shaping, resources, themes, materials, and semantic projection mechanisms |
| 010 | **CANDIDATE** | Binding, command, component, timer, metrics, feedback, and C ABI isolation |

## Bundle completion law

A bundle is complete only when all applicable checks have evidence:

1. Each migrated reusable type has a mirrored hierarchical declaration and
   definition path. A former family header remains only as a thin compatibility
   umbrella while downstream code migrates.
2. Public behavior and ownership laws are preserved. New customization is
   admitted as explicit validated state with ordering, invalidation, semantic,
   and lifetime tests; unmeasured performance claims are not added.
3. The final Neo source is synchronized and built on the M4 Mini. The quiet
   verifier emits one success line or a bounded diagnostic tail.
4. Every visual type is exercised in a real retained demoboard surface through
   Screen Sharing. The atlas crop is taken from that observation, not from a
   mockup.
5. `manual.json` supplies reviewed method explanations and evidence notes. The
   generated HTML and Markdown mirrors pass structural validation.

The generator's remaining pages intentionally say **detailed review pending**.
An inventory page is not evidence that a type has been migrated, visually
verified, or accepted as a final design.
