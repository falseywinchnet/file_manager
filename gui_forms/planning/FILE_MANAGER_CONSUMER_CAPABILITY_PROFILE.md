# File Manager consumer capability profile

Date: 2026-08-05

Status: **GIVEN consumer demand plus dependency-ordered GUI.Forms planning;
not an implementation authorization and not an ABI freeze**.

This is the deliberately generous shopping list for the native File Manager
frontend. It names reusable behavior GUI.Forms should grow before the frontend
needs it. It does not put File Manager code, filesystem objects, Engine
contracts, Orchestrator contracts, or plugin authority inside `gui_forms/`.

The companion owner verdicts and investigation are in
`../../planning/gui_forms/GUI_FORMS_ACCESSIBILITY_AND_TEXT_ROUND_001.md`. The
visual consumer constitution remains under `../../frontend/planning/visual/`.

## 1. Admission rule

File Manager asks GUI.Forms for a capability when at least one is true:

1. two or more product surfaces need the same stateful behavior;
2. the behavior crosses host, accessibility, focus, text, drag/drop, scale, or
   renderer boundaries;
3. reproducing it in the frontend would create a shadow widget framework;
4. it belongs in a reusable control or compositional primitive;
5. it needs headless traces, resource limits, or platform conformance below the
   application layer.

File Manager retains domain policy. GUI.Forms does not decide what a path
means, which command is allowed, how a file operation works, why a result
matched, what a plugin may do, or which property is authoritative.

## 2. Authoring and runtime doctrine

- The ordinary API remains imperative and stateful: create controls, compose
  them, set properties, attach commands/events, and mutate synchronously.
- The runtime is retained. Paint callbacks record bounded retained chunks; they
  do not reconstruct the application every frame.
- DML may describe and generate the same retained objects. It is not a runtime
  document database or an immediate-mode diff loop.
- Application models own files, folders, results, criteria, properties,
  operations, and history. Controls own presentation state such as focus,
  selection, expansion, scrolling, editing, popup lifetime, and accessibility
  identity.
- Custom-control seams declare measure, arrange, paint, hit-test, text,
  semantic, and accessibility invalidation effects.
- Every capability below is usable through C++ without C#, Skia, AppKit,
  Win32, GTK, or a browser type entering the public seam.

## 3. Priority and landmark map

| Class | Meaning |
|---|---|
| **FM0** | Required in the named GUI.Forms snapshot before Frontend 001 can compose the complete shell without private headers or shadow widgets |
| **FM1** | Required for ordinary daily file-management behavior after shell composition is proven |
| **FM2** | Required for search, criteria, complex operations, rich inspection, and plugin-fed facts |
| **FMX** | Strategic extension requested now so ABI, DML, drawing, and host seams do not close it out; may land after initial dogfood |

| Landmark | GUI.Forms work | Product evidence gained |
|---|---|---|
| **FM-L0 — owned typography** | M4 shaping/editing plus M9 font packs: HarfBuzz, FreeType, bundled role fonts, deterministic metrics, scale/accommodation inputs | a native specimen replaces browser typography as design evidence |
| **FM-L1 — shell mechanics** | M1/M3/M5/M6: window participation, custom-chrome zones, responsive layout, panes, scrolling, focus, headless traces | one-window tree/content/selection shell can be composed honestly |
| **FM-L2 — command and navigation instruments** | M6/M7: command model, command shelf/ribbon, menus, breadcrumb editor, autocomplete, anchored matrix | probable path and complete vocabulary use one command authority |
| **FM-L3 — object field and inspection** | M5/M7: virtual tree/list/icon/details views, selection, label edit, property list, preview host | large folders remain bounded and selected objects inspectable |
| **FM-L4 — transient and operation surfaces** | M5–M9: overlays, popovers, expandable rows, toast stack, drawer, progress, validation, bounded motion/sound | search, criteria, failures, and mergers gain coherent surfaces |
| **FM-L5 — transfer and host integration** | M3/M12: outbound/inbound drag/drop, clipboard, external targets, monitor/scale, window chrome, dialogs | previews, tree rows, objects, windows, and other programs interoperate |
| **FM-L6 — native accessibility** | M8 plus host publishers: default adapters, virtual children, text ranges, actions, accommodations | Narrator, VoiceOver, Orca, keyboard, large text, high contrast, reduced motion, and sound-off pass |
| **FM-L7 — authored material** | M2/GUI.Drawing/M9: paths, texture/gradient paints, masks, shadows, icons, color roles, inspection | House Composite depth is expressible without backend escape calls |
| **FM-L8 — stable consumption** | M10/M11: DML/schema coverage, C ABI, C++ wrappers, install target, deterministic fixtures | frontend receives a named dependency snapshot |

## 4. Foundation, window, and input

| ID | Priority | GUI.Forms obligation | File Manager use |
|---|---|---|---|
| FM-W01 | FM0 | Top-level/owned-window lifecycle, close cancellation/reason, placement, min/max size, restoration, monitor movement, deterministic shutdown | location and secondary windows |
| FM-W02 | FM0 | Distinguish key, secondary-participating, drag-proximate, and genuinely deactivated state without equating non-key with disabled | multi-window transfer and unavailable boards |
| FM-W03 | FM0 | Custom-chrome content zones with host caption controls, drag regions, resize borders, system menu/snap behavior, and native window identity | Watercolor title fresco without breaking host behavior |
| FM-W04 | FM0 | Logical/device transforms, per-monitor scale, topology events, occlusion, and pixel-snapping policy | crisp material and monitor moves |
| FM-W05 | FM0 | Pointer, keyboard, wheel, multi-click, capture, cursor, mnemonic/access-key, command-key, and focus-scope contracts | ordinary desktop behavior |
| FM-W06 | FM0 | Keyboard focus independent from selection plus restoration across popup, collapse, modal, and virtualization | object field, ribbon, path, inspector |
| FM-W07 | FM0 | Hit targets separable from visible geometry; proximity state without changing focus or selection | hairline seams and quiet grips |
| FM-W08 | FM1 | Hover-intent/dwell service with cancellation, movement tolerance, focus equivalent, deterministic clock | rich inspection and tooltips |
| FM-W09 | FM1 | One command dispatch path shared by controls, menus, shortcuts, accessibility, and automation | consistent actions |
| FM-W10 | FMX | Power/session/display-preference notification and safe suspend/resume invalidation | long-running desktop use |

## 5. Layout, scrolling, and responsive composition

| ID | Priority | GUI.Forms obligation | File Manager use |
|---|---|---|---|
| FM-LY01 | FM0 | Flex-like top composition plus dock/anchor, grid/table, flow, stack, canvas, split, explicit custom layout | shell and dense subpanels without CSS |
| FM-LY02 | FM0 | Min/preferred/max, margins, padding, baseline, intrinsic measurement, bounded reentrant layout | dense but usable controls |
| FM-LY03 | FM0 | Content-aware shrink followed by authored priority collapse of regions, groups, labels, and detail | responsive shell down to 150 x 150 |
| FM-LY04 | FM0 | Accessibility text scale as a distinct measurement input; reflow/grow before its own collapse order | large text without clipping |
| FM-LY05 | FM0 | Split panes with wide invisible hit zones, min sizes, collapse/restore, allocation, keyboard operation, persisted identity | tree/content/selection |
| FM-LY06 | FM0 | One-scroll-plane composition with nested non-owning sections, scroll anchoring, and focus reveal | preview plus properties |
| FM-LY07 | FM1 | Scroll-to-item, drag/selection autoscroll, edge velocity, stable anchor across mutation | tree and object field |
| FM-LY08 | FM1 | Variable-height virtual rows with overscan, height correction, pinning, semantic logical children | expanding search correspondence |
| FM-LY09 | FM1 | Anchored overlay placement with screen-edge avoidance and reposition on layout/scale | menus, suggestions, path matrix, inspection |
| FM-LY10 | FMX | Interruptible retained size/position/opacity/disclosure transitions with reduced-motion substitution and bounded damage | grips, drawers, panes, expansion; no perpetual frame loop |

## 6. Commands, menus, ribbon, and popups

| ID | Priority | GUI.Forms obligation | File Manager use |
|---|---|---|---|
| FM-C01 | FM0 | Stable command identity: label, description, icon, shortcut, mnemonic/key tip, enabled, visible, checked, default, destructive, availability reason | ribbon/menu/context/accessibility unity |
| FM-C02 | FM0 | Many presentations bind one command without duplicating execution or state | complete menu plus selective shelf |
| FM-C03 | FM0 | First-party command shelf/ribbon: category tabs, groups/captions, large/small labelled items, split/dropdown items, hosted fields, contextual visibility, priority overflow/collapse | Office Pearl command geography |
| FM-C04 | FM0 | Keyboard traversal and key tips for ribbon categories/groups/items | non-pointer use |
| FM-C05 | FM0 | Menu bar/context menus with nesting, separators, checks/radio, icons, shortcuts, scrolling, edge placement, cancellation, focus restoration | complete vocabulary and context authority |
| FM-C06 | FM1 | Menu population from immutable command snapshot before open; bounded replacement/cancellation for late plugin facts | domesticated plugin commands |
| FM-C07 | FM0 | Generic popup/dropdown ownership, click-away/Escape, capture, focus scope, nesting limit, semantic parentage | menus, combo, suggestions, matrix |
| FM-C08 | FM0 | Tooltip separate from accessible name/help: plain, icon/text, bounded factual key/value; hover and focus paths | commands and object facts |
| FM-C09 | FM1 | Pointer-neutral inspection popover and interactive pinned mode | rich hover without authority change |
| FM-C10 | FM1 | Disclosure row/stack with hover preview, focus/click pinning, variable height, keyboard commands, expanded semantics | search and operation rows |
| FM-C11 | FM2 | Persistent edge drawer with header, close/collapse, nested sections, resizable extent, focus containment, virtual operation list | directory mergers |
| FM-C12 | FM2 | In-window toast layer with deduplication, bounded queue, timeout, pause, actions, accessible announcement | invisible outcomes only; not search-result structure |

## 7. Navigation and text instruments

| ID | Priority | GUI.Forms obligation | File Manager use |
|---|---|---|---|
| FM-N01 | FM0 | Segmented breadcrumb with stable segment IDs, keyboard traversal, overflow, activation, semantic path relations | current location |
| FM-N02 | FM0 | In-place breadcrumb-to-editor conversion without replacing identity or losing focus history | manual path entry |
| FM-N03 | FM0 | Async suggestion model with generation IDs, cancellation, highlight, accepted value, noncommitting preview | terminal-like completion |
| FM-N04 | FM0 | Anchored multi-row overlay composed from virtual rows and segmented stacks | terminal `./` recent matrix |
| FM-N05 | FM0 | Editable-text prefix/suffix/action slots with distinct focus and hit semantics | search scope, clear, terminal actuator |
| FM-N06 | FM1 | Pending/resolved/ambiguous/invalid/unavailable validation with non-color cues | paths and properties |
| FM-N07 | FM1 | Cluster-safe ellipsis, reveal-on-focus/inspection, selection/copy, logical hit mapping | names, paths, excerpts |
| FM-N08 | FM2 | Structured factual spans and matched-substring ranges without full rich-document editing | evidence text |

## 8. Virtual collections and object surfaces

| ID | Priority | GUI.Forms obligation | File Manager use |
|---|---|---|---|
| FM-O01 | FM0 | Virtual TreeView with stable IDs, lazy children, expand, selection, keyboard, label edit, icons/badges, drag targets, hover-expand, semantic children | folder hierarchy |
| FM-O02 | FM0 | One stable virtual item model with small/large icon, list, details/table, and tile presentations | folder and virtual-folder field |
| FM-O03 | FM0 | Selection independent from realization/sort: single, toggle, range, admitted marquee, select-all, anchor/focus | large folders and view changes |
| FM-O04 | FM0 | Deterministic spatial arrow navigation in icon modes and row/column navigation in details | keyboard operation |
| FM-O05 | FM0 | Inline label editing with commit/cancel/validation, scroll/focus preservation, generation conflicts | rename/direct edit |
| FM-O06 | FM1 | Column model with resize/reorder policy, sort, alignment, accessibility, persistence IDs | details view |
| FM-O07 | FM1 | Decoration slots for material icon, thumbnail, overlays, coarse badge, status, labels, focus, selection | object identity without cards |
| FM-O08 | FM1 | Type-to-select independent from the separate search field | conventional behavior |
| FM-O09 | FM1 | Mutation transactions preserve selection, focus, edit, scroll anchor, accessibility identity | live model changes |
| FM-O10 | FM1 | Calm empty/loading/unavailable/rebuilding collection states | honest status |
| FM-O11 | FM2 | Lightweight PropertyList: groups, name/value relation, read-only/editable rows, custom editors, reset/default, validation, async commit | selection pane |
| FM-O12 | FM2 | Preview host for trusted controls or isolated snapshots with copy/drag, focus, accessibility delegate, cancellation, unavailable state | preview without plugin chrome authority |

## 9. Search, criteria, progress, and error composition

These are frontend compositions. GUI.Forms supplies primitives without
freezing “file search” into a control type.

| ID | Priority | GUI.Forms obligation | File Manager composition |
|---|---|---|---|
| FM-S01 | FM1 | Expandable virtual correspondence row with excerpt, trailing information lane, value/percentage, pin/hover/focus | search result |
| FM-S02 | FM2 | Reorderable compact rack with labelled fields, combo/suggestion editors, enable/remove, validation, responsive wrap | criteria builder |
| FM-S03 | FM2 | Represent cheap-live versus staged-apply state through commands/validation, not business logic | criteria cost policy |
| FM-S04 | FM1 | Determinate/indeterminate progress with stage, amount, cancel, pause/retry, accessible value/status | copy, index, preview, search |
| FM-S05 | FM2 | Hierarchical operation list with child progress, faults, retry/skip/cancel, bounded trace | directory mergers/batches |
| FM-S06 | FM1 | Material task dialog with command roles, details, focus trap, default/cancel, owner restore, headless result adapter | collision/cross-volume/blocking error |
| FM-S07 | FM1 | Inline validation/error association and persistent local status independent of tooltip timing | edit faults |
| FM-S08 | FM2 | Undo/reversal presentation hook tied to command/result identity, without implementing filesystem undo | reversible operations |

## 10. Clipboard, drag/drop, and external interoperability

| ID | Priority | GUI.Forms obligation | File Manager use |
|---|---|---|---|
| FM-D01 | FM1 | Typed inbound/outbound sessions: text, file references, promised/lazy files where supported, bounded media bytes | cross-window/application transfer |
| FM-D02 | FM1 | Source advertisement and target negotiation for copy/move/link/none, with host modifier mapping | conventional grammar |
| FM-D03 | FM1 | Host-conventional drag ghost/hotspot, operation cursor, target/insertion/rejection cues | feedback without theater |
| FM-D04 | FM1 | Sources from item/preview; targets on tree row, folder object, background, pane, other window | declared transfer surface |
| FM-D05 | FM1 | Edge autoscroll and bounded hover-expand with cancellation | deep targets |
| FM-D06 | FM1 | Session across participating non-key windows without stealing key status merely on proximity | multi-window behavior |
| FM-D07 | FM1 | Security-scoped/pasteboard lifetime stays in adapters; portable payload exposes expiry/availability | host correctness |
| FM-D08 | FM1 | Typed clipboard formats, ownership changes, cut/copy state hook, external interoperability | ordinary operations |
| FM-D09 | FM2 | Progress/conflict handoff after drop, separate from transfer negotiation | collision/cross-volume |
| FM-D10 | FMX | Conformance traces and synthetic external peers on every host | nondestructive regression |

## 11. Accessibility and accommodations

| ID | Priority | GUI.Forms obligation |
|---|---|---|
| FM-A01 | FM0 | Semantic graph exists for supported controls regardless of attached assistive client |
| FM-A02 | FM0 | Every stock control has default role/name/value/state/action semantics; authored metadata enriches rather than enables |
| FM-A03 | FM0 | Custom controls provide an adapter or explicitly declare presentational status for File Manager conformance |
| FM-A04 | FM0 | Explicit task order independent from draw and accidental child order |
| FM-A05 | FM0 | Stable virtual children, focus/selection, set position/level, bounds-on-demand, lazy native proxies |
| FM-A06 | FM0 | Editable/navigable text ranges with caret, selection, attributes, composition, cluster geometry, actions |
| FM-A07 | FM0 | VoiceOver, UI Automation, and AT-SPI2 publishers with ordered/coalesced notifications |
| FM-A08 | FM0 | High contrast, text scale, display scale, reduced motion, focus visibility, and relevant host preferences are authoritative inputs |
| FM-A09 | FM0 | No meaning solely in color, motion, sound, hover, or pointer geometry; keyboard/focus equivalents mandatory |
| FM-A10 | FM1 | Assistive action, keyboard command, and pointer activation reach one command/state transition |
| FM-A11 | FM1 | Quiet live-status policy for location, progress, result, validation, and error changes with deduplication |
| FM-A12 | FM1 | Headless snapshots plus Narrator/VoiceOver/Orca/keyboard/accommodation primary-scenario gates |

## 12. Typography and font-pack contract

### Owner-selected direction

- **GIVEN (parsed from `T1B`; owner may correct the parse):** HarfBuzz is the
  common shaper on every host.
- **GIVEN (`T2C`):** FreeType is the common glyph loader, hinter, and rasterizer
  on every host.
- **GIVEN:** File Manager UI text does not depend on host-installed fonts.
- **GIVEN:** Portsmouth Rapids is the preferred title/control face, subject to a
  production redistribution-rights gate.
- **GIVEN:** a bundled humanist body face in the Tahoma/Calibri family serves
  content, object labels, evidence, properties, and editing.
- **GIVEN (`T4B`, revised for all-bundled roles):** layout targets exact geometry
  for a pinned font-pack version and shaping/raster profile. Output still
  responds to scale, contrast, and selected antialias policy.

### First body-face specimen set

| Candidate | Status | Why admitted | Concern |
|---|---|---|---|
| **Carlito** | **CANDIDATE, lead specimen** | Official project describes it as Calibri-metric-compatible; Latin/Cyrillic; SIL OFL 1.1; four-face family | arbitrary filename scripts remain open; small-size UI hinting must be measured |
| **IBM Plex Sans** | CANDIDATE comparison | OFL; designed for UI; broad separately packaged scripts; industrial voice | less Tahoma/Calibri-like; broader asset family |
| **Liberation Sans** | CANDIDATE control | OFL; mature Arial-metric-compatible baseline | less humanist and less period-specific |
| **Selawik** | **REJECTED as lead** | open Microsoft Segoe-oriented reference | upstream README acknowledges missing matching kerning and need for improved hinting |

Upstream references: [Carlito](https://github.com/googlefonts/carlito),
[IBM Plex](https://github.com/IBM/plex),
[Liberation Fonts](https://github.com/liberationfonts/liberation-fonts), and
[Selawik](https://github.com/microsoft/Selawik).

### Minimal and safe packaging

| ID | Priority | Requirement |
|---|---|---|
| FM-T01 | FM0 | Pin exact bytes, hashes, license/provenance, face index, allowed tables, coverage, and metrics in the built-in manifest |
| FM-T02 | FM0 | Do not enumerate or silently fall back to arbitrary host fonts for product UI; missing coverage is a deterministic pack fault |
| FM-T03 | FM0 | Fallback per indivisible cluster from a bounded ordered pack; never restyle the whole control for one missing cluster |
| FM-T04 | FM0 | Define mandatory core filename/path coverage plus optional signed script packs; minimal builds declare coverage honestly |
| FM-T05 | FM0 | Build HarfBuzz/FreeType with only admitted modules; inventory symbols/parser modules, fuzz exact builds, bound font/table/glyph counts |
| FM-T06 | FM0 | Font-pack revision is a metric generation; changes invalidate caches and require geometry/raster review |
| FM-T07 | FM0 | Preserve typed grapheme, shaped-cluster, scalar, UTF-8/UTF-16, line, caret, document positions |
| FM-T08 | FM0 | Fractional glyph positions, controlled baseline snapping, grayscale/LCD profiles, gamma/contrast, scale-qualified caches |
| FM-T09 | FM1 | Host antialias/text preference signals may select among audited FreeType profiles without changing font identity or layout unexpectedly |
| FM-T10 | FMX | Untrusted document/font preview stays outside the ordinary UI font-pack path behind a separate threat boundary |

Reduced vulnerability surface remains a **HYPOTHESIS** until the module
inventory, corpus, fuzzing, and platform-stack comparison are recorded. Pinning
a file removes environmental variance; it does not make font parsing safe.

## 13. Drawing and material vocabulary

These operations are promoted from GUI.Drawing's FMD oversight by named File
Manager workloads. API spelling and renderer realization remain gated.

| ID | Priority | Required drawing capability | Workload |
|---|---|---|---|
| FM-R01 | FM0 | Rectangles, rounded rectangles, ellipses, lines, polylines, polygons, curves, arcs, open/closed paths, fill rules | controls, focus, icons, title fresco |
| FM-R02 | FM0 | Stroke width/alignment, cap, join, miter, dash, device-pixel hairline, deterministic snapping | seams, etching, icon detail |
| FM-R03 | FM0 | Solid, multi-stop linear/radial, texture/image, repeat/mirror, opacity-mask paints | pearl, Watercolor, graphite, Studio materials |
| FM-R04 | FM0 | Save/restore, affine transforms, rectangular/path clips, required region algebra, logical/device conversion | scrolling, overlays, icons, damage |
| FM-R05 | FM0 | Porter-Duff compositing, isolated alpha groups, masks, bounded shadows, small audited filter set | physical depth and overlap |
| FM-R06 | FM0 | Nine-patch/cap-inset material with scale-aware borders and center policy | crisp pearl/chassis/control stock |
| FM-R07 | FM0 | Positioned glyph runs, decoration, cluster hit geometry, ellipsis, selection, caret, baselines | text-bearing controls |
| FM-R08 | FM0 | PNG, CPU surfaces, premultiplication, quality sampling, fit/fill/crop/mask/tint/snapshots | icons, previews, thumbnails |
| FM-R09 | FM0 | Precompiled trusted vector/path assets; SVG may be build-time input but is not a required runtime parser | material icons with smaller parser surface |
| FM-R10 | FM1 | Partial/tiled updates, readback, deterministic cache keys, cancellation, generation ownership | thumbnail/preview/active surfaces |
| FM-R11 | FM0 | Stable command trace, bounds/cost inspection, semantic pixel probes, profile-qualified PNG export | design QA and comparison |
| FM-R12 | FM1 | Complexity ceilings for paths, clips, gradients, layers, filters, images, mapped bytes, command depth | resource/plugin containment |

No drawing call creates immediate-mode control lifetime. Paint records a
retained chunk or mutates an explicitly owned image surface.

## 14. Style, assets, motion, and sound

| ID | Priority | GUI.Forms obligation |
|---|---|---|
| FM-V01 | FM0 | Named foreground/background pairs, material roles, depth, global light, density, scale, contrast, and state matrices |
| FM-V02 | FM0 | Normal/hot/pressed/focused/default/selected/secondary/disabled/unavailable/drop-target states without color-only meaning |
| FM-V03 | FM0 | Size-specific icon resources and command/object/status slots; no font-icon dependency required |
| FM-V04 | FM0 | Data-only resource packs with built-in fallback, deterministic compiler, bounds, hashes, provenance, signature policy |
| FM-V05 | FM1 | Reduced-motion-aware bounded transitions with interrupt/reverse/coalesce and no idle animation loop |
| FM-V06 | FM1 | Semantic sound-cue service or callback with cue IDs, enable/volume, coalescing, cancellation, tiny audited PCM assets |
| FM-V07 | FM1 | Complete visual state with sound off; button/menu/hover/focus alone remain silent |
| FM-V08 | FMX | Theme laboratory enumerates roles/states/materials and reports fallback without private inspection |

## 15. DML, imperative API, inspection, and tests

| ID | Priority | GUI.Forms obligation |
|---|---|---|
| FM-X01 | FM0 | C++ composition for every admitted primitive; no DML-only feature or private-header reach-through |
| FM-X02 | FM0 | DML coverage for controls, layout, commands, popup ownership, style/resource roles, semantic hooks, collapse priority, invalidation |
| FM-X03 | FM0 | Stable IDs for controls, hosted items, models, commands, panes, popups, semantic nodes |
| FM-X04 | FM0 | Headless fixtures drive input/commands/clock/host results and inspect layout, focus, semantics, damage, chunks |
| FM-X05 | FM0 | Capability report says supported, unavailable, simulated, or incompatible; no empty success |
| FM-X06 | FM0 | Diagnostics for layout, realization, shaping, glyph cache, popup, drag, semantics, damage, deadlines, resource fallback |
| FM-X07 | FM0 | Clean install/export target and version negotiation for C ABI/C++ wrapper; examples are ordinary consumers |
| FM-X08 | FM1 | Hot reload preserves compatible identities and reports destructive migrations |
| FM-X09 | FMX | Deterministic visual/semantic exporter produces native review corpus without a browser engine |

**MEASURED PARTIAL (2026-08-13, FM-R11/FM-X04/FM-X06):** the public,
renderer-neutral `Window::visual_inspection_snapshot` now captures bounded final
control geometry, absolute and effective ancestor clips, live focus/hover/
press/capture state, authored material layers, presentation inputs, committed
effective `FontSpec` values, display-chunk freshness, retained draw operations,
and non-destructive damage. Text is redacted unless a local caller explicitly
opts in. `VisualInspectorView` and its pointer-transparent overlay consume only
that public snapshot, and the native Visual Inspector Lab plus headless tests
exercise material, typography, state, clipping, resize, accessibility, capture
bounds, and deterministic JSON. The effective font record remains the
committed GUI.Forms request; when a terminal installs an exact public
`TextMetricsProvider`, a separate optional record identifies the primary
bundled face, actual shaped fallback runs, and logical metrics without leaking
backend identity types. Provider absence stays explicitly estimated. Semantic
pixel probes, cost attribution, and profile-qualified PNG export remain open. Evidence:
`../experiments/M12P34_VISUAL_STATE_INSPECTION.md`.

**MEASURED PARTIAL (2026-08-13, FM-L7/FM-R02/FM-R03/FM-R05/FM-R06/FM-R11/
FM-R12/FM-V01/FM-V02/FM-V08/FM-X04/FM-X06):** retained `SurfaceMaterial` now
adds up to eight ordered edge keylines, each with physical edge, width, color,
and inward inset, closing the dark-outer-plus-inset-specular gap without nested
controls. Existing ordered pad/repeat/reflect/radial/image/nine-patch fills and
inset/outset shadows were sufficient for bounded Watercolor, Office Pearl,
Workshop Graphite, and Studio 2003 specimens. Construction rejects atomically
beyond the hard limit; recording and inspection JSON preserve authored order;
Skia/CoreGraphics semantic probes differ by at most 5 summed RGB levels; and
the native Layered Material Fidelity Lab exercises five visual states,
inspection, contrast, 100/125/150% text scale, active/inactive, and resize.
Arbitrary masks, isolated groups, blend modes, color-management policy,
nine-patch center tiling, profile-qualified PNG export, and Windows native
review remain open rather than simulated. Evidence:
`../experiments/M12P28_LAYERED_MATERIAL_FIDELITY.md`.

**MEASURED PARTIAL (2026-08-13, FM-W07/FM-LY05/FM-R02/FM-V02/FM-X04/FM-X06):**
`SplitContainer` now has an explicit bounded physical-seam contract: logical or
one-device-pixel visible thickness, independent symmetric/asymmetric logical hit
extents, a declared minimum hit target, inspectable idle/near/hot/drag/focus/
disabled/collapsed states, Escape cancellation, and numeric semantic actions.
At 1×/2× the hairline trace remains exactly one device pixel while its declared
logical hit target does not shrink; overlap resolves to the seam without a dead
crack; hover does not move layout or focus; disable/detach revoke capture; and
invalid geometry is rejected atomically. The native macOS and Win32-cross-built
Physical Seam + Proximity Lab supplies vertical/horizontal, collapsed,
disabled, scale, minimum, keyboard and inspection specimens. The approximately
90 ms reference interpolation, persisted extent store, DML/C ABI spelling and
host accessibility conformance remain open. Evidence:
`../experiments/M12P32_PHYSICAL_SEAMS_AND_PROXIMITY.md`.

**MEASURED PARTIAL (2026-08-13, FM-W04/FM-LY04/FM-T01–T08/FM-R07/FM-X04/FM-X06):**
the portable typography-resolution seam now distinguishes effective `FontSpec`,
actual bundled primary and shaped fallback run families, logical line metrics,
and final device-pixel baseline snapping. Skia/HarfBuzz reports the face tied
to each actual run; a complete private-pack Win32 provider verifies the family
selected by GDI; missing providers and invalid/missing bundles fail or remain
labelled rather than inventing host resolution. The Typography + Scale Lab,
headless injected provider, pinned-font checks, mixed Carlito/Noto corpus, and
100/125/150/200% reflow tests are green on the M4 Mac mini. Profile-qualified
native raster review and the rest of FM-R07 remain open. Evidence:
`../experiments/M12P30_DETERMINISTIC_TYPOGRAPHY_SCALE.md`.

**MEASURED PARTIAL (2026-08-13, FM-W03/FM-W04/FM-X04/FM-X06):** the macOS
host can now place an ordinary retained GUI.Forms identity surface at client
`y=0` continuously through the native title region while keeping genuine
AppKit caption controls. A public, renderer-neutral chrome-hit contract admits
one or more stable drag-backdrop identities, rejects empty/duplicate/unresolved
configuration, and preserves interactive descendants through exact retained
hit testing. Activation is reflected into the portable `Window` visual state.
The Custom Chrome Lab and deterministic tests cover active/inactive material,
transparent decoration, interactive exclusion, drag decisions, 1x/2x logical
geometry, and host lifecycle. Win32 still uses its standard native title bar;
full-client nonclient composition there remains an explicit platform gap.
Evidence: `../experiments/M12P29_NATIVE_CUSTOM_CHROME.md`.

**MEASURED PARTIAL (2026-08-13, FM-C03/FM-R02/FM-R04/FM-V01/FM-V02/FM-X04/
FM-X06):** explicit `ConnectedControlTopology` now lets separate `ButtonBase`
controls paint as one horizontal or vertical physical instrument without
coordinate inference. Expanded clipped material painting removes joined-edge
corners and duplicate borders; every non-leading control owns one deterministic
seam while retaining separate hit testing, keyboard focus, semantic identity,
checked/default/disabled state, and split/menu disclosure boundaries. Invalid
axes, sizes, indices, duplicates, mixed parents/windows, and mixed-axis requests
are rejected before mutation. Deterministic traces cover exact seam ownership,
state precedence, focus, semantics, and paint operation cardinality; the native
Connected Control Painting Lab covers isolated and 2/3/5-member horizontal and
three-member vertical sets, mixed/disabled-interior states, disclosure, resize,
inactive/active presentation, and the public visual inspector. Command-shelf
layout/overflow, product policy, Windows native capture, and accessibility
adapter publication remain open. Evidence:
`../experiments/M12P31_CONNECTED_CONTROL_PAINTING.md`.

**MEASURED PARTIAL (2026-08-13, FM-W04/FM-LY01–LY04/FM-X04/FM-X06):** the
renderer-neutral `ResponsiveTrackPanel` now nests fixed, content-measured, and
weighted remaining tracks with finite min/preferred/max, relational gaps,
authored hidden state, explicit unique priority collapse, focus protection/
fallback, reveal, cumulative device-boundary snapping, committed revision, and
complete resolution diagnostics. Automatic layout collapse remains distinct
from authored visibility and removes subtrees from paint, input, focus, and
semantics while preserving retained identity. The Responsive Tracks Lab and
headless matrix reproduce 40/23/66/40/remaining/24 reference bands,
218+3/fluid/3+288 grouped columns, deterministic Selection/Tree thresholds,
100/125/150/200% reflow-before-collapse, stable resize cycles, and a bounded
150×150 primary field. Native macOS dogfood exercised every size/text-scale
preset, explicit reveal/auto restoration, live inspector/overlay, and
inactive/active presentation; it also corrected a fixture-only host/preset
preview mismatch before acceptance. Command-group overflow, FM-LY06
scroll-plane focus reveal, Windows native review, and profile-qualified raster
evidence remain open. Evidence:
`../experiments/M12P33_FIXED_RESPONSIVE_TRACKS.md`.

## 16. Promotion summary

For the File Manager lane, TreeView, ListView/object modes, SplitContainer,
command/menu/status families, tooltip, property-list substrate, custom controls,
overlays, and outbound drag/drop are not optional P3 curiosities.
Accessibility publication is required; authored metadata remains optional for
construction. HarfBuzz, FreeType, bundled fonts, and deterministic role metrics
replace the platform-system-font candidate. FMD0–FMD3 and FMD5–FMD8 have named
workloads above. Runtime SVG parsing, arbitrary media codecs, printing, desktop
capture, ActiveX, browser hosting, MDI, and GPU paths remain excluded.

`MASTER_IMPLEMENTATION_PLAN.md` remains delivery-order authority and
`CONTROL_COMPLETENESS_MATRIX.md` remains behavioral accounting authority. This
profile is their consumer overlay, not a second plan.

## 17. Frontend go-ahead slice

**MEASURED PARTIAL (2026-08-05, FM-LY05):** the public C++ control library now
contains a retained two-panel split composition with stable panel/seam IDs,
separate thin paint and enlarged hit geometry, minimum constraints, live
pointer capture, keyboard resizing, focus transfer, collapse/restore, and
fixed-panel resize behavior. The compatible .NET projection passes the same
bounded behavior on host and physical Wine. Automatic versus user collapse,
extent persistence, collapse-tab proximity, DML/C ABI spelling, and native
semantic publication remain open. Evidence:
`experiments/M11H_RETAINED_SPLIT_CONTAINER.md`.

**MEASURED PARTIAL (2026-08-05, FM-W05/FM-W06/FM-C07 substrate):** the
renderer-free `Window` now owns bounded nested focus scopes, contained focus,
stable Tab/Shift+Tab traversal, explicit and out-of-order restoration,
owner-unavailable cleanup, typed changes, and structured diagnostics. This is
the reusable focus half of popup behavior. Popup ownership, click-away/Escape,
capture transfer, placement, semantic parentage, native accessibility
publication, DML, and C ABI/binding projection remain open. Evidence:
`experiments/M11H_RETAINED_FOCUS_SCOPES.md`.

The first snapshot need not finish every FM1/FM2/FMX row. It must provide FM0
behavior sufficient to compose:

1. a custom-chrome window with ribbon shelf, menu, path/search row, thin split
   panes, tree, virtual object field, selection pane, and status line;
2. bundled HarfBuzz/FreeType text using pinned control/body faces;
3. deterministic resize, text scale, focus, selection, collapse, popup, and
   shutdown traces;
4. default semantic adapters and headless semantic snapshot;
5. declared drawing/material subset and data-only resource pack;
6. clean CMake install/consume targets and versioned C++/experimental C ABI.

Anything omitted is a named unavailable capability and the frontend narrows its
fixture. It never substitutes HTML, immediate mode, private renderer calls, or
an application-local widget framework.
