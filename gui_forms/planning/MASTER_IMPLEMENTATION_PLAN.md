# GUI.Forms master implementation program

Status: **execution-ready candidate program; not an accepted architecture
decision**. Date: 2026-08-03.

This document decomposes the route from the current proving slice to a usable,
portable GUI.Forms 1.0. It does not override the repository's pre-architecture
rules. Items marked **GIVEN** may constrain implementation. Items marked
**CANDIDATE** require the named gate and, where reversal cost is high, a numbered
ADR with grand-architect approval before they become **DECIDED**.

**GIVEN refinement, 2026-08-10:** the sibling `../../web_forms/` project now
owns the proposed authoritative round-trippable authoring source as bounded
browser-valid HTML/CSS. Its no-authored-JavaScript profile and nested ambient
layout/paint/state landscape are approved Web.Forms boundaries. The provisional Gallery DML remains
evidence, and older “DML” rows below continue to name authoring-schema coverage
debt until migrated; they do not authorize a second compiler/source language
inside GUI.Forms.

The program is deliberately sliceable. “Implement GUI.Forms” is not one task;
it is a chain of independently testable contracts. Every milestone must leave
the lower `gui_forms/` build working without reading File Manager sources.

## 1. Evidence baseline

### 1.1 GIVEN constraints

- GUI.Forms is a retained/stateful custom-rendered framework. It is not an
  immediate-mode tree rebuilt each frame.
- GUI.Forms and File Manager do not depend on .NET, Java, Godot, or a browser
  engine. C++20 is admitted. Objective-C++ remains a host-adapter detail.
- GUI.Forms has no GPU capability. It must not compile, link, expose,
  initialize, or probe GPU APIs. OS composition of a finished CPU bitmap is
  outside this prohibition.
- The renderer is private. No Skia or platform object crosses the public C,
  C++, C#, Web.Forms authoring, control, plugin, or test seam.
- PNG is the sole renderer/resource-core decoder. Other media decoders belong
  in isolated plugins.
- Application objects own semantic application state. GUI.Forms retains the
  presentation state needed for behavior: identity, parentage, requested and
  arranged geometry, focus/capture, editing state, caches, style/resource
  generations, semantic hooks, damage, and clocks for explicitly active work.
- Mutations are synchronous and typed. Development builds fail on undeclared
  invalidation; production conservatively invalidates the affected subtree and
  records a diagnostic.
- Nestable update/layout scopes coalesce at the outermost boundary.
- Top-level composition uses a GUI.Forms flex algorithm. Forms/sub-panel layout
  uses explicit anchor/dock, grid/table, flow, stack, split, canvas, and virtual
  control-specific families. “Flex” does not mean HTML or CSS.
- Web.Forms source is the proposed authoritative round-trippable authoring
  surface; generated code is disposable. Production retains compiled schema,
  stable IDs, and inspection metadata. Exact source/compiler semantics remain
  gated in the sibling project.
- Tree ownership and reference counting are combined: parent-to-child strong,
  child-to-parent weak, detached objects retained only by external strong
  references. Event subscriptions are tokenized and weak by default where a
  strong edge would create a cycle.
- A stable C ABI is the authoritative eventual binary seam. C++ is a thin RAII
  wrapper; the optional C# facade calls the native library and does not make the
  native library depend on .NET.
- Compatibility means familiar calls and expected outcomes, not strict WinForms
  bug preservation. Native ordering may deviate toward documented correctness.
- The compatible call surface used by the authoritative current retired compatibility specimen specimen
  and its admitted plugins is a **GIVEN** bridge target. Observed calls are
  presumptively admitted unless they conflict with a hard boundary or receive an
  explicit exclusion and migration path. ActiveX, browser hosting, printing,
  MDI, obsolete control families, and undocumented HWND/WndProc accidents are
  not admitted by that rule.
- Only `/Users/quentinkuttenkuler/Downloads/retired compatibility specimen-x64-next (6)` and its
  bundled plugins admit retired compatibility specimen compatibility surface. The older debug distribution
  admits nothing. Compatibility stops at calls into the Forms-shaped facade;
  third-party docking/map APIs and internals are not GUI.Forms obligations.
- retired compatibility specimen custom controls that depend on Windows handles/messages are ported to
  GUI.Forms drawing and input while preserving admitted public behavior. General
  HWND/message emulation is not the bridge strategy (ADR-001).
- retired compatibility specimen source is unavailable. Implement the captured interface-level facade
  first; a later unchanged-binary load under Wine is a best-effort compatibility
  experiment, not an earlier milestone gate (ADR-002).
- GUI.Forms publishes one deterministic event/property order selected for its
  retained model. Captured retired compatibility specimen sequences are checked pragmatically for blocking
  incompatibilities; alternate historical orders are not supported. Any
  authoring compiler, including Web.Forms, preserves authored initialization
  assignment order (ADR-003).
- Accessibility, tooltips, guidance, automation, and AI-readable inspection use
  a retained semantic-hook graph. Controls can exist without authored hook
  metadata, but supported stock controls provide default semantic adapters.
  File Manager requires native publication and has no accessibility off switch.
- Ordinary controls use GUI.Forms-authored material recipes and named
  foreground/background roles. A style-only backplane may be composited below
  controls but cannot intercept input or impersonate controls.
- **GIVEN owner revision:** HarfBuzz shapes and FreeType loads, hints, and
  rasterizes GUI.Forms text on every host. Product UI fonts come from pinned
  bundled packs rather than host-installed `system-ui`. Control labels prefer
  the supplied Latin-only Portsmouth Rapids face; a separately selected bundled
  humanist face serves body/content roles; uncovered clusters resolve through
  bounded bundled fallback packs.
- Theme and language replacements always fall back safely to built-ins.
- The lower project builds GUI.Forms and its demonstrations only and never
  reaches upward into File Manager.

### 1.2 OBSERVED current proving slice

- `include/gui_forms/` exposes a portable C++ spike: `Control`, `Window`, typed
  dirty flags, events, geometry, `Painter`, metrics, and a minimal static tree.
- Parent/child storage currently uses `std::shared_ptr`; stable IDs are unique
  within a `Window`; runtime IDs are monotonic, not generational ABI handles.
- `Window` implements bounded multi-pass layout, update scopes, focus, pointer
  capture, preview/target/bubble routing, hit testing, damage, and retained
  painting.
- The gallery's visible widgets are not reusable framework controls. One
  demo-private final `GalleryControl` switches over DML node kinds and owns
  hard-coded gallery layout/painting behavior.
- `gallery.dml` has a hand-maintained compiled header. There is no parser,
  compiler, binary schema, code generator, round-trip editor, or designer.
- AppKit and Skia are private to their adapters, but no portable host-service
  protocol or headless host implementation exists yet.
- Text proves committed UTF-8 insertion and an AppKit composition bridge, not a
  complete editor, shaping stack, bidi algorithm, cluster indexing, selection,
  clipboard, undo, or accessible text ranges.
- There is an experimental C ABI 0.x, reusable component/event kernel, and a
  bounded Win32 Gallery host. There is no C# binding, stable ABI, resource pack,
  language catalogue, theme compiler, configuration registry, accessibility
  publisher, Linux host, packaging, or compatibility runner.

### 1.3 MEASURED current baseline

On the recorded macOS 14.8.7 arm64 host, Apple Clang 16 and the pinned Skia
`2a9b593...` configuration:

- four of four current CTest gates pass (rechecked 2026-08-03 in 0.24 seconds);
- the PNG-only Skia smoke checksum is
  `17289292855132215351` for a 320x192 logical fixture at 2x;
- `libskia.a` is 5,758,736 bytes and the unsigned gallery executable was
  measured at 3,270,304 bytes;
- the linked gallery uses no Metal or OpenGL framework;
- the Skia source patch removing unconditional BMP/WBMP/ICO decoder objects is
  a recorded maintenance burden.

These results prove only the named fixture. They do not establish production
startup, memory, text correctness, accessibility, portability, ABI stability,
or renderer selection.

## 2. Meaning of “complete”

GUI.Forms 1.0 is complete when all of the following are true:

1. Every row in `CONTROL_COMPLETENESS_MATRIX.md` is marked `supported`,
   `supported by named optional package`, or `explicitly excluded`, with tests
   and documentation matching the status. A constructor alone is not support.
2. The retained kernel, public authoring/capability schema consumed by
   Web.Forms, C ABI, C++ wrapper, renderer vocabulary,
   host protocol, text model, semantic-hook graph, style/resource model, and
   event order are versioned contracts with accepted ADRs.
3. macOS, Windows, Wayland, and X11 hosts pass the shared headless/event-trace
   conformance suite plus platform-native IME, clipboard, drag/drop,
   accessibility, monitor/scale, lifecycle, and dialog tests. A documented
   release may stage X11 or Wayland only if the architect explicitly changes
   the platform gate.
4. The framework has no idle redraw loop and meets approved launch, memory,
   input, layout, raster, allocation, and package-size budgets on named reference
   machines and workloads.
5. The approved Web.Forms subset parses, validates, compiles reproducibly,
   round-trips losslessly, emits native bindings, and can be inspected without
   running application code.
6. Theme and language packs are deterministic data-only assemblies with bounded
   parsers, safe fallback, provenance, compatibility ranges, and fuzz coverage.
7. C and C++ applications link without Skia/platform types crossing the ABI.
   A C# control-gallery application consumes the same native library through
   generated bindings, with native tests proving lifetime and callback cleanup.
8. The Modern.Forms-style gallery, material laboratory, text/semantic
   laboratory, virtualization laboratory, and SDR-style retained bitmap
   laboratory are packaged examples, not privileged framework code.
9. Sanitizers, fuzzers, long-running stress tests, ABI checks, reproducible-build
   checks, dependency/license inventory, signing, installation, upgrade, and
   uninstall tests pass for release artifacts.

“Complete” does not mean every third-party WinForms control, every HWND/WndProc
accident, ActiveX, a browser, MDI, a report viewer, printing, or a full Visual
Studio clone. Such objects are packages, adapters, or exclusions unless an
approved compatibility corpus admits them.

## 3. Explicit non-goals

- No File Manager implementation inside this subtree.
- No GPU API, GPU fallback, shader/effect language, or GPU resource type.
- No web renderer, HTML/CSS layout, web component model, or network-aware theme
  or language pack.
- No invisible native text-control overlay.
- No immediate-mode compatibility facade and no frame-driven tree rebuild.
- No strict preservation of incorrect WinForms behavior.
- No Skia scene graph, Skia objects in controls, or public “Skia mode.”
- No executable code in theme or language assemblies.
- No arbitrary in-process third-party plugin authority. Custom controls compiled
  into a trusted application and isolated plugin UI are different threat
  classes and need different contracts.
- No claim of “lightweight,” “fast,” or “native” without its named workload.

## 4. Target architecture and unresolved decision gates

This is the architecture the milestones are organized around. **CANDIDATE**
choices remain reversible behind named seams until their gates accept them.

```text
bounded Web.Forms source ----> validator / compiler / typed IR
       |                                  |
       |                     binary schema + generated handles
       |                                  |
imperative C++ wrapper ---> versioned C ABI / handle registry <--- C# facade
                                      |
                         retained UI-thread presentation graph
                         /        |          |          \
                    layout    events/text  semantics   style/resources
                         \        |          |          /
                       display chunks + damage scheduler
                                      |
                           private renderer vocabulary
                                      |
                         CPU raster adapter (Skia candidate)
                                      |
                         narrow native host service protocol
                         /          |          |       \
                      AppKit      Win32      Wayland   X11
```

### 4.1 Renderer boundary

The engine emits immutable or generation-addressed display chunks containing
clipped fills/strokes, paths, positioned glyph runs, PNG-backed images,
nine-patch/material operations, shadows within approved CPU bounds, compositing
groups, and cached layer/tile references. Chunks carry logical bounds, pixel
snapping policy, opacity, resource generations, and damage dependencies.

**CANDIDATE R-SKIA:** retain the pinned CPU/PNG-only Skia adapter as production
backend. Gain: broad, mature geometry/blending/color behavior already proven to
link privately. Loss: source/build weight, patch maintenance, large policy
surface, and risk of leaking its object model upward.

**CANDIDATE R-HYBRID:** a narrow GUI rectangle/bevel/glyph-blit kernel plus a
replaceable vector/path module. Gain: smaller hot path and controlled semantics.
Loss: two raster paths, compositing equivalence work, and ownership of more pixel
correctness.

**CANDIDATE R-OTHER:** Blend2D, Cairo, CoreGraphics reference, PlutoVG/AGG, or
ThorVG software in bounded roles. None may be selected by reputation.

**Gate R1:** render identical control-heavy, text-heavy, 1/10/100-percent damage,
large-PNG, vector-icon, translucent-backplane, and 30 Hz bitmap-band fixtures.
Record cold/warm start, linked size and dependencies, RSS, allocations, p50/p95/
p99/worst render and present latency, damaged pixels, determinism, text quality,
and implementation complexity. Accept an ADR only after comparing the pinned
Skia baseline with at least one credible lower-surface alternative. Until then,
new framework behavior targets the renderer vocabulary, not Skia APIs.

**GIVEN GUI.Drawing revision:** portable drawing semantics are owned by
GUI.Drawing. The managed `System.Drawing` compatibility surface maps admitted
calls to that native subsystem; the installed .NET drawing service is only a
temporary compatibility scaffold and differential oracle. Imperative drawing
records bounded commands into retained display chunks or targets an owned CPU
image surface. It does not change the retained architecture into an
immediate-mode GUI. Skia and platform objects remain private adapters.

The authoritative retired compatibility specimen floor is 353 drawing rows: 307 required static-IL rows
across 34 types and 46 deferred metadata-only rows. File Manager's broader
vector-icon, thumbnail, preview, color, text/glyph, compositing, and diagnostic
needs remain a separate consumer-promotion ledger so the retired compatibility specimen closure number
cannot hide broader product work. Named File Manager workloads now promote the
minimums recorded there; exact API spelling, renderer realization, and limits
remain gated. Exact families, status vocabulary, platform surface-lease
boundary, and M11e–M11h stages are in `planning/GUI_DRAWING_REVISION_PLAN.md`.

### 4.2 Host boundary

**CANDIDATE H6, recommended:** native OS event loops implement a narrow
GUI.Forms-owned service protocol. The protocol covers lifecycle, scheduled
wakeup/dispatch, monitors/scaling/coordinates/occlusion, normalized input,
cursor/capture, IME sessions and candidate geometry, clipboard, typed drag/drop,
dialogs/menus, text-raster/accommodation preference signals, native
accessibility publication, session/power/display changes, and native surface
handles required for isolated composition. Bundled GUI.Forms packs, not the
host, own product font selection/fallback. The deterministic headless
implementation is the behavioral reference.

Gain: every desktop hook remains reachable and portable code stays testable.
Loss: four real adapters, conformance drift, and honest Wayland/X11 differences.

Alternatives remain SDL3 as a bring-up host, direct AppKit with later extraction,
or a complete framework's platform layer. Each either duplicates the native
escape path or makes later removal expensive.

**Gate H1:** accept the protocol only after the current AppKit gallery and a
headless trace runner use it without AppKit types entering core headers. Record
event, focus, capture, resize/scale, timer, dispatch, IME, modal, and shutdown
traces. **Gate H2:** decide separately whether normal application menus are
custom GUI.Forms controls with only the macOS global menu exported natively, or
native on all platforms.

### 4.3 Retained state, ownership, and thread domain

The authoritative engine domain is one UI thread per application context.
Controls and UI resources use non-atomic reference counts within that domain;
cross-domain immutable resources may use atomic ownership. Opaque ABI handles
contain an index and generation. Destroyed generations can never resolve.

The component graph and visual tree are separate: a component container can own
timers, image lists, bindings, tooltips, dialogs, and controls without visually
parenting them. Disposal is synchronous and idempotent: detach from visual and
semantic trees, cancel timers/dispatch, release capture/focus, revoke callbacks,
and invalidate handles before user teardown callbacks can observe a half-live
object.

Events are subscription-token based. The internal route is preview, target,
bubble; ordinary bindings expose Forms-like events. Exceptions are caught at
the ABI/user-callback boundary and translated to structured errors/diagnostics.

**Gate K1:** executable traces must cover reparenting, detached lifetime,
duplicate IDs, cycle rejection, handler-driven disposal, weak subscription,
focus/capture revocation, queued work after disposal, and stale handles. Adopt
the corrected event order only through an ADR after comparison with the
declared compatibility corpus.

### 4.4 Mutation, layout, damage, and scheduling

Typed dirtiness expands beyond the spike's four flags to distinct property,
measure, arrange, hit-test, paint-chunk, text-shape, style, resource, semantic,
and accessibility-publication generations. Metadata declares bounded effects.
Development failure and production conservative-subtree fallback implement the
GIVEN undeclared-mutation rule.

**CANDIDATE L4, recommended:** setters synchronously update requested state;
arranged-geometry reads, hit tests, paint, position-dependent input, and semantic
bounds create a minimal read barrier outside update scopes. Inside a scope,
ordinary reads see committed geometry until `PerformLayout` explicitly flushes.
Layout callbacks cannot recursively reenter layout; consequent mutations enter
a bounded later pass.

Gain: immediate Forms-like state plus automatic coalescing. Loss: apparently
cheap geometry getters can perform work and require diagnostics.

Top-level flex and explicit sub-panel algorithms share immutable constraints,
min/preferred/max, margin/padding, baselines, logical scale, pixel snapping, and
cached measure/arrange records. Virtual containers own their own realization
and extent algorithms instead of measuring all data items.

The scheduler wakes only for damage, queued UI work, caret/timer deadlines, or a
declared active custom surface. It never runs a perpetual render loop.

**Gate L1:** golden geometry and pass-count traces for nested flex, dock/anchor,
grid/table, flow, stack, split, canvas, hidden children, font/scale changes,
reparenting, and read barriers. **Gate L2:** one million logical list items with
bounded realized controls and no item-count-proportional paint/layout work.

### 4.5 Input, focus, commands, and modal behavior

Host events normalize physical key, text/composition, pointer identity/buttons,
wheel precision, click count, modifiers, scale, and timestamp without erasing
platform-specific payloads. Text is never inferred from key-down.

The kernel owns focus scopes, active control, tab traversal, mnemonics, default/
cancel commands, validation, capture, hover/pressed state, drag threshold, and
modal ownership. Commands provide stable identities and enabled/checked/text/
shortcut state shared by menus, strips, buttons, and accessibility actions.

**Gate I1:** trace pointer press/release/activation, cancellation, double click,
capture loss, wheel, tab, mnemonic, default/cancel, validation rejection,
handler-driven disabling/disposal, nested modal loops, and focus restoration.
Compatibility deviations must be documented and tested, not accidental.

### 4.6 Text and editing

The public ABI uses length-delimited UTF-8, but positions are typed: byte offset,
Unicode scalar, grapheme cluster, shaped cluster, line, and document position
are never interchangeable integers. The editor stores text, selection,
composition, undo, paragraph direction, attributes, and line index independently
of the renderer.

**GIVEN T-HB/FT owner direction:** HarfBuzz shapes script/language/direction
runs on every host. FreeType is the common glyph loader, hinter, and rasterizer.
The renderer consumes positioned glyph runs and glyph masks without exposing
either dependency in public controls or the ABI. Platform text stacks remain
comparison or host-input references, not product font/shaping authority.

Portsmouth Rapids selection is by typography role and cluster coverage. A
pinned bundled humanist face serves content/body roles. Missing coverage
resolves per indivisible cluster through a bounded bundled fallback order. A
missing face or script pack is a labelled resource/coverage fault; GUI.Forms
does not silently change the whole control family or search arbitrary host
fonts.

Staged delivery is allowed: Latin/dead-key editing first, then grapheme-safe
selection/clipboard/undo, bidi, and representative multi-stage CJK IMEs. The
data model and ABI may not encode Latin-only assumptions at any stage.

**Gate T1:** validate the owner-selected stack after a corpus of Latin, combining marks,
emoji sequences, Arabic/Hebrew bidi, Indic scripts, fallback runs, malformed
UTF-8 rejection, and scale/font changes. **Gate T2:** platform editor tests cover
composition updates, replacement ranges, candidate positioning, commit/cancel,
selection, navigation, clipboard, undo/redo, password policy, multiline, and
accessible text ranges.

### 4.7 Semantic graph, accessibility, help, and inspection

A semantic node attaches by stable control identity and may state role, name,
description, value/state, actions, relationships, logical order, bounds policy,
text ranges, tooltip, and help-overlay anchors. Absence of authored augmentation
is valid; supported stock controls still project default semantics. Platform
publishers, test automation, tooltips, and the greaseboard help overlay consume
the graph independently; disabling one optional consumer does not erase the
graph or disable native accessibility publication.

The help overlay is a separate per-panel display plane. It may dim, highlight,
draw arrows, and label anchors but does not rebuild the control plane or receive
ordinary application input.

**Gate S1:** headless semantic snapshots plus VoiceOver, UI Automation, and
AT-SPI tests for roles, focus, actions, selection, values, virtual children,
geometry, and text ranges. File Manager requires this gate; another independent
consumer may omit a publisher package, but GUI.Forms stock semantics remain the
same execution path.

### 4.8 Style, theme, resources, localization, and configuration

Style resolution produces immutable generation-addressed records from named
foreground/background pair roles, OKLCH-derived seed families, material/depth
recipes, state, light direction, density, scale, contrast, and typography role.
Raw color is restricted to custom drawing/backplane work. Theme change advances
one style/resource generation inside an update scope.

Theme and language assemblies are deterministic, indexed, data-only archives.
Each has a versioned manifest, sorted index, per-entry type/offset/length/hash,
bounded independent payloads, compatibility interval, provenance, and optional
signature. The built-in pack uses the same logical lookup path and cannot be
removed. External failure falls back per entry, not by blanking the UI.

Language controls retain message IDs and typed arguments. A replacement entry
must match the built-in argument signature; a bounded built-in formatter handles
plural/select forms. Locale resolution and live-vs-restart replacement remain
decision gates. Bundled-face fallback occurs per cluster.

Mutable configuration is separate from immutable packs and high-churn session
state. **CANDIDATE:** a schema-validated flat namespaced textual map with atomic
temp/write/sync/replace and one known-good predecessor; a transactional store
holds session/hive data. Secrets and grants use OS-protected storage.

**Gate RSC1:** fuzz pack manifests, indexes, decompression, PNG dimensions/
stride/color metadata, message programs, and fallback chains. **Gate RSC2:**
prove byte-reproducible compilation and recovery from truncated, corrupt,
incompatible, oversized, and partially translated packs. Decide signatures,
parent-locale chaining, and live replacement in ADRs.

### 4.9 Web.Forms compiler edge, generated code, and designer

The Web.Forms toolchain is a sibling compiler project, not a runtime HTML/CSS
parser embedded in GUI.Forms or every application:

1. lossless source lexer/parser with comments and source spans;
2. schema/type registry for controls, components, properties, events, layouts,
   styles, resources, messages, semantic hooks, and invalidation effects;
3. name/stable-ID resolution and component-vs-visual ownership validation;
4. typed normalized IR with explicit initialization/update scopes and ordered
   side effects;
5. deterministic compact binary schema plus debug/source map;
6. generated strongly typed C++ handles and C# facade classes;
7. inspect/diff/lint/format/decompile tools;
8. development hot reload by stable-ID transaction, kept out of release runtime;
9. round-trip edit model preserving unknown compatible fields and comments;
10. visual designer using the same compiler APIs and producing Web.Forms
    HTML/CSS, never treating generated C++/C# as authoritative.

The compiler rejects unknown required controls/properties, duplicate or unstable
IDs, invalid ownership, undeclared invalidation, illegal backplane input,
resource/type mismatch, event signature mismatch, and unsafe limits. Production
does not execute arbitrary expressions from markup; imperative
post-construction logic is application C++ attached to generated typed handles,
not a serialized programming language.

**Gate W1:** GUI.Forms publishes/accepts the public authoring capability manifest
proposed in `../../web_forms/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`.
**Gate W2:** compile an admitted Web.Forms Gallery source and delete the
hand-maintained header only after deterministic output, diagnostics, source
maps, malformed-input tests, and fidelity gates pass. **Gate W3:**
parse-format-parse and designer round trips preserve semantic IR and authored
trivia. **Gate W4:** schema migration supports the two previous minor versions
or gives a deterministic upgrade error and tool.

### 4.10 C ABI, C++ API, and C# bridge

The stable C ABI uses versioned size-prefixed function tables, opaque
generational handles, fixed-width integers, length-delimited strings/spans,
explicit borrowed/owned lifetimes, caller-supplied allocator hooks for returned
bulk data, result codes plus structured errors, callback/context pairs, and
subscription tokens. Exceptions, RTTI, STL, C++ object layout, Objective-C,
Skia, and platform handles never cross except through explicitly typed optional
native-host escape records.

The C++ layer provides move-aware RAII strong/weak handles, typed properties,
scoped subscriptions, update guards, spans/string views with documented call
lifetime, and explicit status-to-exception or status-return policies. It does
not add a second ownership system.

The generated C# facade uses `SafeHandle`/equivalent deterministic wrappers,
P/Invoke, pinned or copied spans under explicit rules, callback trampolines,
thread dispatch, and source-shaped Forms events/properties. The C# package may
depend on .NET; the native library and File Manager may not. “Mostly compatible”
is documented by the completeness matrix and executable corpus.

**Gate A1:** C11 and C++20 consumers compile/link against an exported-symbol
allowlist; stale handles, retain/release, reparenting, callback disposal, errors,
allocator mismatch, thread violations, and version negotiation pass under ASan/
UBSan/TSan where applicable. **Gate A2:** the C# gallery and a bounded
SDR-style plugin sample run on the native library and leak no callbacks or
handles across repeated load/unload. ABI 1.0 freezes only after two independent
consumers survive a breaking-change audit.

### 4.11 Custom controls and plugin surfaces

Trusted application custom controls subclass/compose portable GUI.Forms APIs,
declare property metadata and invalidation effects, emit renderer-neutral
display chunks, expose optional semantic children, and participate in DML via a
registered schema descriptor. Native-message emulation is not the normal custom
control API.

Out-of-process or hostile plugins do not receive a raw control pointer, native
window procedure, renderer, theme override, or host event stream. They provide
bounded data and/or a separately designed remote surface protocol through a
capability broker. File Manager's rule that plugins cannot override core
controls or style remains intact. Docking, maps, charts, printing, browser hosts,
and ActiveX are optional packages/adapters, not base GUI.Forms controls.

**Gate P1:** fault-inject custom control construction, paint, event, timer,
dispose, and hot reload. **Gate P2:** any remote plugin surface gets a separate
threat model, process containment test, resource quotas, protocol fuzzing, and
ADR; it is not admitted by the base control implementation.

## 5. Dependency-ordered milestones

Each milestone is independently reviewable and ends with a green lower build.
Parallel work is allowed only where input contracts are pinned.

### M0 — evidence lock and decision scaffolding

Deliver:

- machine-readable current-baseline manifest and benchmark environment;
- proposed ADRs for host protocol, renderer experiment, layout observability,
  event order, text stack, ABI, pack format, and compatibility promise;
- control and behavioral-contract manifest generated from the research ledger;
- CI presets for warnings-as-errors, sanitizers, coverage, release, and
  dependency/symbol audits;
- negative-results directory and experiment template.

Exit: no candidate is called selected; the architect identifies which ADRs may
proceed to experiments. Existing four tests remain green from a clean lower
build.

### M1 — reusable retained kernel and deterministic headless host

Deliver:

- component ownership separate from the visual tree;
- non-atomic UI-thread strong/weak ownership abstraction ready for later ABI
  handles;
- deterministic disposal and tokenized event subscriptions;
- visible/enabled/focusability inheritance, z-order, name lookup, reparenting,
  tab/focus scopes, capture revocation, and UI-thread enforcement;
- expanded typed dirtiness and undeclared-invalidation diagnostics;
- deterministic headless host/clock/dispatcher and event-trace format;
- after the M1a lifetime contract is measured, an experimental C ABI 0.x
  handle registry for lifetime, identity, errors, property access, and event
  tokens, plus a thin C++ wrapper and exported-symbol allowlist; this remains
  explicitly unstable and does not yet include the complete control catalogue;
- current gallery migrated without visual regression.

Exit: lifecycle/focus/event/transaction traces pass under sanitizers; a C11
consumer passes stale-handle, retain/release, callback-revocation, error, and
version-negotiation tests; no Skia or AppKit dependency enters the headless
tests; no idle work occurs. ABI 0.x is implementation scaffolding, not 1.0.

### M2 — display chunks, damage scheduler, and CPU renderer hardening

Deliver:

- stable internal display-chunk vocabulary and cache generations;
- exact damage-region operations with complexity guards and old/new bounds;
- style backplane/control/overlay planes with independent damage;
- bounded scheduler for damage, deadlines, and active custom surfaces;
- PNG resource registry with memory/dimension/color bounds and fuzz target;
- Skia adapter migrated to chunks, renderer benchmark harness, symbol/dependency
  audit, and at least one credible comparison lane.
- File Manager-promoted GUI.Drawing work from
  `FILE_MANAGER_CONSUMER_CAPABILITY_PROFILE.md`: path/curve geometry, stroke
  semantics, transforms/clips, gradient/texture/opacity-mask paints, nine-patch
  materials, isolated alpha groups, bounded shadows, positioned glyph runs,
  owned CPU images, and command inspection.

Exit: 1/10/100-percent damage and 30 Hz bitmap-band workloads record complete
metrics; idle renders remain zero; renderer ADR may be accepted or Skia remains
replaceable.

### M3 — host protocol and complete AppKit adapter

**GIVEN portability constraint:** the portable engine and protocol serve
Windows, macOS, and Linux equally. AppKit is the first complete production
adapter, not the semantic reference. Platform-specific renderer/Skia extensions
remain private and capability-scoped.

Deliver:

- portable host service interface and headless reference implementation;
- AppKit lifecycle, monitor/scale, occlusion, scheduling, pointer/keyboard,
  capture/cursors, clipboard, typed drag/drop, dialogs, menus per decided policy,
  text-raster/accommodation preferences, composition geometry, the VoiceOver
  publisher seam/probe, and clean shutdown;
- host conformance/event replay suite and platform capability reporting.
- File Manager host extensions: custom-chrome drag/resize/system zones,
  key/secondary-participating/deactivated state, outbound drag-source sessions,
  cross-window drag proximity, typed clipboard ownership, host accommodation
  signals, and a bounded semantic sound-cue presentation service or callback.

Exit: AppKit appears only under `src/host/macos`; the gallery and laboratories
  run exclusively through the protocol; nested modal and IME traces pass.

Progress through M3e:

- **OBSERVED M3a:** normalized lifecycle/input events, deterministic replay,
  close cancellation, occlusion state, and clean shutdown are implemented.
- **OBSERVED M3b:** portable/headless/AppKit monitor geometry, inherited cursors,
  and bounded UTF-8 clipboard text are implemented with structured service
  snapshots and boundary tests.
- **OBSERVED M3c:** retained pointer capture is synchronously mirrored into the
  host service and released on pointer-up, eligibility revocation, close, or
  shutdown; display topology changes are sequenced host events; and temporary
  occlusion suppresses deadlines without revoking active-surface tokens or
  replaying missed frames. AppKit capture is explicitly limited to the native
  implicit drag sequence rather than a global event tap.
- **OBSERVED M3d:** protocol 0.3 defines platform-neutral typed requests/results
  for message, open, save, folder, and color dialogs; a deterministic modal
  stack suppresses owner input, releases capture, preserves focus, rejects
  duplicate IDs, bounds nesting to eight, and unwinds on cancellation, adapter
  failure, or shutdown. Headless is the conformance oracle; AppKit maps the same
  variants to native panels without entering the public contract.
- **OBSERVED M3e (inbound destination):** protocol 0.4 carries bounded variants
  for UTF-8 text, UTF-8 file lists, and media-typed bytes; retained controls opt
  into drop targeting; and the kernel owns one deterministic
  leave/enter/over/drop order plus eligibility, close, and shutdown cleanup.
  Headless is the payload/order oracle and AppKit translates pasteboard values
  only inside its adapter. The Gallery collection is a live retained consumer.
- **OPEN:** outbound drag-source initiation, owned-form/sheet integration and
  native nested-modal focus restoration, menu policy, font preference/profile
  mapping, complete IME geometry,
  VoiceOver publication, Windows/Linux adapters, and session/power events. Gate
  H1 remains open.

### M4 — Unicode text, typography, and editing

Deliver in sub-slices:

1. typed Unicode positions, rope/gap/piece candidate experiment, line index,
   text/style spans, shaping/fallback service, glyph-run cache;
2. Latin/dead-key single-line editor with caret, selection, clipboard, undo;
3. grapheme navigation, multiline layout, password/read-only modes;
4. bidi, representative complex scripts, multi-stage IME, accessible ranges;
5. HarfBuzz shaping plus FreeType loading/hinting/rasterization;
6. Portsmouth Rapids control pack plus the selected bundled body face and
   bounded per-cluster script fallback packs;
7. pinned font/raster profile, exact metric generations, parser-module audit,
   corpus/fuzzing, and scale/antialias profile tests.

Progress through the M4b grapheme/shaping-seam slice:

- **OBSERVED:** renderer-neutral `TextStore` now validates strict UTF-8, uses
  distinct UTF-8/UTF-16/scalar/line position types, performs bounded atomic
  replacement, maintains Unicode separator line ranges, transforms opaque
  nonoverlapping style spans, and publishes structured revision/rebuild/
  rejection counters.
- **MEASURED:** malformed-sequence classes, scalar/surrogate boundary rejection,
  atomic failure, six line-break forms, style transformation, and 2,000 seeded
  mixed-script edits pass against a scalar reference model on native arm64 and
  as a strict MinGW PE32+ x86-64 executable under Wine.
- **CANDIDATE:** contiguous UTF-8 is the reversible baseline, not a selected
  large-document representation. Full metadata rebuilds are counted for later
  comparison with gap and piece candidates.
- **OBSERVED:** `TextStore` now exposes a distinct grapheme position domain and
  implements Unicode 17.0.0 UAX #29 extended cluster boundaries; shaping and
  fallback use renderer-neutral opaque font/glyph IDs and absolute UTF-8
  cluster maps.
- **MEASURED:** all 766 official Unicode grapheme conformance cases pass;
  renderer-free strict and sanitizer builds pass; static MinGW PE32+ focused
  tests also pass under Wine.
- **GIVEN owner direction:** `TextShaper` and `FontFallbackResolver` will be
  backed by HarfBuzz, FreeType, and bundled font packs; exact adapters, cache,
  subset/coverage, and antialias profiles remain implementation gates.
- **MEASURED PARTIAL:** experimental ABI 0.15 retained fields preserve distinct
  UTF-8 anchor/caret positions, validate grapheme boundaries through
  `TextStore`, use renderer-supplied text metrics for pointer hit-testing and a
  horizontally maintained viewport, clip selection/caret paint, navigate and
  delete combining/emoji graphemes atomically, and route bounded UTF-8
  clipboard text through headless, AppKit, and Win32/Wine host adapters. Native
  selection-aware replacement is now the authoritative mutation path and owns
  bounded undo/redo snapshots that restore text and directional selection.
  TextBox, editable ComboBox, and accepted NumericUpDown edits project through
  the same transaction contract. The C11 ABI contract, 28 native tests,
  generated facade build, and managed behavior probes pass on arm64 macOS and
  Wine.
- **OPEN:** history coalescing, multiline layout, bidi/culture, multi-stage IME
  composition/candidate geometry, accessible editable text ranges, and the M4
  exit gate remain open. HarfBuzz/FreeType shaping/fallback, pragmatic
  word/line navigation, protected-value policy, clipboard, and retained caret
  timing are implemented and measured. Evidence:
  `experiments/M4A_UNICODE_TEXT_STORE.md`,
  `experiments/M4B_GRAPHEME_SHAPING_SEAM.md`, and the ABI 0.11–0.15 C/managed
  field contracts.

Exit: text/IME corpus passes on each supported host and no renderer-specific
text object appears in public controls or ABI.

### M5 — layout families, scrolling, and virtualization

Deliver:

- top-level flex plus dock/anchor, stack, flow, table/grid, split, canvas;
- min/preferred/max, margin/padding, baseline, autosize, visibility, scale,
  pixel snapping, scroll extents, scrollbar policy;
- read-barrier/update-scope contract and pass diagnostics;
- virtual item realization/recycling with stable model IDs and semantic virtual
  children.
- File Manager extensions: authored priority collapse, a separate large-text
  collapse order, one-scroll-plane inspector composition, variable-height
  virtual rows, stable scroll anchoring, drag autoscroll/hover-expand, and
  anchored overlay placement.

Exit: golden geometry corpus passes across scale/font/platform reference hosts;
million-item list/tree fixtures keep realized controls, layout, and paint bounded
to viewport/overscan.

### M6 — reusable basic Forms control foundation

Deliver reusable public controls, not demo node switches:

- `Control`, `ScrollableControl`, `ContainerControl`, `UserControl`, `Form`;
- panel/group/label/link label, button/check/radio;
- text box base/text box, combo/list basics, numeric/domain up-down;
- scrollbar/track/progress, picture box;
- timer, tooltip/help/error provider, image list, command model;
- property metadata, initialization scopes, owner draw, style/semantic hooks,
  keyboard/mnemonic/default/cancel behavior.
- File Manager-promoted primitives: disclosure/expander, collapsible pane,
  command presentation binding, rich factual tooltip, popup ownership, text
  adornment slots, inline validation, and lightweight property name/value rows.

Exit: gallery is built solely from reusable controls; basic control behavior and
event traces pass through headless, AppKit, C++, and early C ABI lanes.

Progress through M6a:

- **OBSERVED:** `gui_forms_controls` exports renderer-neutral `Panel`,
  `GroupBox`, `Label`, `ButtonBase`, `Button`, `CheckBox`, `RadioButton`, and
  `LinkLabel` with UI-thread property guards, retained invalidation, tokenized
  events, deterministic activation/state order, and provisional Windows 7/10
  rendering values.
- **OBSERVED:** the Gallery consumes these types for its label/button/check/radio
  families and visibly exercises indeterminate and visited-link state. AppKit
  translates common hardware positions to the shared USB HID key vocabulary.
- **OPEN:** Gallery-private containers, editor, range, list, category, and
  instrument controls; every other M6 family; C ABI exposure; and the M6 exit
  gate. Evidence: `experiments/M6A_REUSABLE_BASIC_CONTROLS.md`.

Progress through M6b:

- **OBSERVED:** public `ContainerControl` and `UserControl` identities provide
  logical focus-containment operations without claiming M5 scrolling or an
  unimplemented load lifecycle.
- **OBSERVED:** renderer-neutral `RangeControl`, `TrackBar`, and determinate
  `ProgressBar` provide finite range state, horizontal/vertical rendering,
  pointer capture, normalized keyboard/wheel input, and the declared user order
  `scroll` then `value_changed`. The Gallery slider/progress families now use
  these public controls.
- **OPEN:** scrolling containers and scrollbars, full stock `TrackBar`
  compatibility, marquee progress, container validation/scaling/dialog-key and
  load lifecycle, remaining Gallery-private families, and the M6 exit gate.
  Evidence: `experiments/M6B_CONTAINER_RANGE_CONTROLS.md`.

Progress through M6c:

- **OBSERVED:** `Control` provides nested initialization scopes. M12-P27 closes
  construction-time native re-entry: stock state events coalesce by event
  identity and publish final payloads only after the outer dirty commit;
  focus/capture/input are revoked or blocked until that commit.
- **OBSERVED:** retained tree attachment binds the complete subtree before
  parent-first hooks; detachment unbinds the complete subtree before child-first
  hooks. Throwing attachment rolls back bindings and stable IDs, and lifecycle
  callbacks cannot structurally mutate or dispose the transitioning tree.
- **OBSERVED:** `UserControl` publishes a tokenized one-shot lifetime `loaded`
  event plus successful whole-subtree attachment state/count. The compiled DML
  Gallery consumes a real composed `UserControl` with two reusable label
  children and a visible nested initialization/load trace.
- **OPEN:** typed property/default/reset/serialization metadata, validation,
  scaling, scrolling, dialog routing, designer tooling, remaining
  Gallery-private families, and the M6 exit gate. Evidence:
  `experiments/M6C_INITIALIZATION_LIFECYCLE.md`.

Sequencing note after M6c:

- **GIVEN (2026-08-04):** the grand architect advances a bounded Windows host
  proving round next. This was permitted because host protocol version 4 and its
  headless/AppKit traces are already normative enough to act as the translation
  oracle.
- **MEASURED:** W0-W4 pass for the bounded Win32/Wine Gallery slice: PE64
  cross-build, renderer-free portable libraries, protocol-v4 event translation,
  GDI/DIB CPU presentation, WIC PNG, private Gallery fonts, stable-ID control
  automation, framebuffer capture, clean close, strict GCC build, and portable
  boundary/import audit. Evidence: `docs/WINDOWS_WINE_HOST.md`.
- **OPEN:** this completed elevation does not pull M11 managed-facade work, retired compatibility specimen
  unchanged-binary loading, TSF, OLE, UIA/MSAA, Windows packaging, physical
  Windows dogfood, or the full M12 exit gate forward implicitly.
- **MEASURED ADDITIVE HOST UPDATE (2026-08-05):** protocol v5 preserves that
  event-translation baseline and adds only the bounded semantic sound-cue
  capability. Headless trace/counter and AppKit playback gates pass; a Windows
  sound adapter and physical Windows/Wine cue gate remain open.

### M7 — collection, command, modal, data, and advanced controls

Deliver in independent packages/slices:

- virtual TreeView and ListView with item models, columns/groups/checks/editing;
- split and tab containers;
- ToolStrip/MenuStrip/ContextMenuStrip/StatusStrip and hosted item model;
- native common-dialog request/result adapters and message/task dialogs;
- data binding/currency/format-parse pipeline;
- DataGridView model, virtualization, selection, styles, editing hosts, custom
  cells/columns;
- PropertyGrid after descriptor/editor contracts are proven; the bounded native
  metadata/stock-editor slice is measured in M12-P17, while managed projection,
  nested collections, custom editors, and designer breadth remain;
- SDR custom-control porting laboratory. GUI.Forms-native docking or map
  equivalents remain separately admitted parent-use candidates, not M7 or retired compatibility specimen
  bridge exit requirements.
- File Manager product lane: promote TreeView, ListView/object-view modes,
  SplitContainer, menu/context/status models, and the required custom-control
  substrate; add first-party command shelf/ribbon composition, segmented
  breadcrumb/editor, suggestion popup, anchored recent-path overlay,
  lightweight PropertyList, preview-host seam, expandable correspondence rows,
  in-window toast layer, operation drawer, and hierarchical progress list.

Exit: each matrix row has behavior tests; calendar grid edit lifecycle and
SDR-style theme/invalidation/plugin-subtree fixtures pass. Legacy/browser/
printing/MDI families remain explicit exclusions or separately approved work.

### M8 — semantic graph, accessibility, help, and automation

Deliver:

- retained semantic graph, default stock adapters, and stable snapshot/
  inspection protocol;
- tooltip and keyboard-help consumers;
- per-panel greaseboard overlay plane;
- VoiceOver, UI Automation, and AT-SPI publishers;
- semantic virtual children and editable text ranges;
- keyboard-only and assistive-technology conformance suites.

Exit: every supported stock control has a tested default semantic adapter, while
controls still instantiate without authored metadata. Native accessibility
publication is always available for File Manager and is not a user-facing
toggle. Disabling tooltips/help/inspection does not disable accessibility or
erase the semantic graph.

### M9 — style, assets, localization, and configuration

Deliver:

- relational OKLCH color roles, material/depth recipes, immutable style records;
- built-in theme and language packs plus deterministic compilers;
- external data-only discovery, validation, compatibility, coverage, fallback,
  and optional signature policy;
- PNG and font assets, nine-patch/material definitions, high contrast and
  non-color state cues;
- typed message formatter/catalogue and live/restart language policy;
- schema-validated atomic runtime configuration and separate session state.
- pinned built-in Portsmouth/body/fallback font packs with coverage and license
  manifests; size-specific icon/precompiled-vector resources; bounded
  transition recipes; and tiny audited sound-cue assets/policy.

Exit: corrupt/partial packs cannot make the interface unusable; reproducible
pack output and fallback/fuzz suites pass; theme/language switch is one bounded
transaction.

### M10 — GUI.Forms authoring manifest and Web.Forms conformance

GUI.Forms delivers the versioned public control/property/event/layout/style/
resource/semantic capability manifest and conformance fixtures required by
section 4.9. The sibling Web.Forms project owns parser, HTML/CSS profile,
lowering, generated C++, source maps, round trip, and any designer. Replacing
`demo/gallery_dml.hpp` waits for Web.Forms' independent opening gates and a
converted admitted source fixture. C# generation may be exercised as a later
toolchain preview, but it is not distributed or promised until M11.

Exit: the public manifest truthfully covers shipped examples, Web.Forms
conformance output is reproducible and marked, production needs no source
parser, and round-trip tests preserve authored intent.

File Manager schema coverage includes collapse priority, command binding,
hosted command items, popup ownership, breadcrumb segments, virtual item-model
bindings, property rows, semantic task order, material roles, transition/sound
cue IDs, and declared invalidation. These are reusable schema concepts, not
serialized filesystem policy.

### M11 — ABI freeze and supported language bridges

Deliver:

- breaking-change audit and migration of the M1 experimental C ABI to the
  proposed 1.0 function tables and symbol allowlist;
- supported thin C++ public wrapper with no exposed `shared_ptr` ownership;
- generated C# facade and control gallery;
- compatibility analyzer, retired compatibility specimen reflection/usage capture, and documented
  deviation manifest;
- package metadata for native and managed consumers.

Capture-0 measurement pulled forward after the bounded Windows host round:

- **MEASURED:** the opt-in, non-executing compatibility scanner reads PE/CLI
  metadata, static IL operands, inheritance, native imports, and .NET
  single-file bundle version 6 without entering the native build. Its synthetic
  target proves non-execution, redaction, determinism, and hash rejection.
- **MEASURED:** the authoritative 1922 specimen plus its two current managed
  plugins yields 26 inspected managed assemblies, 1,436 tracked public
  compatibility API rows, 1,165 rows present in static IL, 107 redacted
  Forms-derived consumer types, and 428 native imports with zero capture
  diagnostics. Evidence: `experiments/CAPTURE_0_STATIC_COMPATIBILITY_MANIFEST.md`.
- **MEASURED:** Capture-1 classifies all 1,952 API/custom-control/native-import
  evidence rows with zero unclassified entries. The 1,436 public API rows yield
  1,160 required and 276 deferred rows; 796 required rows across 130 Forms types
  belong to the future managed facade. Private derived controls and direct
  native imports remain application-side.
- **MEASURED:** a synthetic consumer compiled against Microsoft's strong-named
  .NET 10 Forms reference loads through a private context against an unsigned
  same-name experimental facade on host .NET and Wine. Independent builds are
  byte-identical and the form/button probe passes.
- **MEASURED:** M11a mechanically resolves and emits all 796 required
  GUI.Forms-owned rows as 190 closure-complete types. Both replacement
  assemblies compile cleanly; a compiled-assembly verifier reports 796/796
  identities present; source, manifest, and repeated release builds are
  deterministic.
- **MEASURED:** additive ABI 0.2 preserves the 0.1 prefix and connects kinded
  construction, name/text, enabled/visible, bounds, parenting, and subtree
  disposal to the generated `Control` hot path. The same managed tree/property/
  event/disposal trace passes on host .NET and Wine.
- **MEASURED:** M11b additive ABI 0.3 projects generated `Form` trees into the
  selected native host. `Application.Run(Form)` passes a deterministic headless
  lifecycle on host .NET and a real Win32 DIB create/show/paint/close cycle
  under Wine. The captured 960 x 640 demonstration has 17 managed-origin
  retained controls; the host reports 17 measured/arranged/painted nodes.
- **MEASURED:** M11c additive ABI 0.4 carries retained button activation into
  managed `Click`, contains and counts a deliberate managed exception, performs
  live retained mutation through queued UI dispatch, requests a cancelable
  close, and emits `FormClosed` and `ApplicationContext.ThreadExit` once. The
  exact callback/lifecycle counts pass in deterministic headless runs and in
  the Win32 DIB host under Wine. AppKit compiles against the same platform-
  neutral wake/close/dispatch seam; the renderer-neutral suite passes 19/19.
- **OPEN:** behavioral implementation for the remaining generated calls,
  broader typed mouse/key/paint/timer coverage, generated-facade `Invoke`
  projection and sustained producer stress, exact close reasons, accessibility publication,
  default/single-file binding, dynamic usage traces, ABI freeze, and unchanged-
  binary execution remain M11/M12 gates. Evidence:
  `experiments/M11A_GENERATED_SURFACE_AND_ABI_0_2.md` and
  `experiments/M11B_MANAGED_HOST_SURFACE_AND_ABI_0_3.md` and
  `experiments/M11C_MANAGED_CALLBACKS_AND_LOOP_ABI_0_4.md`.
- **MEASURED:** M11d widens high-frequency managed behavior and privately binds
  the pinned unchanged retired compatibility specimen specimen to the generated facade under Wine. The
  resulting Win32 host published a 46-control/46-ID snapshot, painted 34
  controls, accepted five host events with zero rejection, and closed through
  automation without a managed or native exception. See
  `experiments/M11D_BEHAVIORAL_FACADE_AND_PRIVATE_LOAD.md`.
- **MEASURED:** the retained-surface continuation reaches a 165-control live
  tree under Wine and visibly projects retired compatibility specimen resource-backed toolbar buttons,
  numeric fields, radio/check controls, dock panes, and range controls. ABI 0.6
  owner-pointer and raster-button probes pass, as do 797/797 surface verification
  and 25/25 native tests. The source-configuration click reaches its map/control
  assembly path without the prior stale-child repaint fault. **OPEN:** active
  receiver streaming, secondary-window/popup composition, editing/accessibility,
  and sustained paint performance remain compatibility gates; the dark spectrum
  is not evidence of a running source.
- **OBSERVED revision:** the visible M11d continuation combines native retained
  GUI.Forms behavior with a managed drawing passthrough. Owner paint currently
  executes through .NET 10 `System.Drawing`, encodes an intermediate PNG, and
  projects that raster into the native tree. The compatibility catalogue now
  assigns all 353 `System.Drawing*` rows to `gui_drawing_compat_facade`; 307 are
  required. They are no longer counted as runtime-owned or implied by the
  797/797 Forms surface score.
- **MEASURED:** M11e adds an independent renderer-free GUI.Drawing semantic
  core and experimental one-symbol drawing ABI. C++20 and C11 consumers produce
  the same golden command trace; value/resource/state, path and logical image
  handles, deterministic disposal, stale/wrong-kind/wrong-thread faults, and
  renderer isolation pass 5/5 focused tests; the Skia-disabled/AppKit-disabled
  native configuration passes 24/24. Pixel
  storage, raster execution, facade mapping, and the M11d passthrough cutover
  remain open. Evidence: `experiments/M11E_GUI_DRAWING_CORE_AND_ABI.md`.
- **MEASURED M11f:** GUI.Drawing now owns bounded COW CPU bitmaps,
  tokenized locks, PNG, paths/regions/gradients, a private CPU-only Skia raster
  service, and a generated 307/307 Drawing facade. The combined verifier reports
  1,104/1,104. PE64 C and generated .NET 10 probes pass HBITMAP, HDC, HWND,
  capture/present, and tokenized GetHdc/ReleaseHdc behavior under Wine while the
  renderer-free boundary remains clean. A Win64-GNU CPU-only Skia archive now
  links `gui_drawing_raster0.dll`; the PE64 command/PNG round trip and Windows
  surface adapter probe pass under Wine. The unchanged zero-passthrough
  specimen run remains M11g. Evidence:
  `experiments/M11F_RENDERING_RASTER_STORAGE_AND_DRAWING_FACADE.md`.
- **MEASURED PARTIAL M11h:** experimental ABI 0.18 adds form-level key preview
  ahead of focused-descendant routing. The generated Forms facade now provides
  scoped `ActiveControl`, stable recursive tab traversal, functional
  `IButtonControl` accept/cancel dispatch, owned-form cycle rejection,
  cancellable close reasons, and modal owner suppression with focus restoration.
  Host and Wine headless gates pass, and a physical Wine Enter/Escape probe
  crosses the Win32 host and ABI preview path. Modeless secondary forms remain
  retained-hosted rather than independent native top-level windows; validation,
  mnemonics, and full popup focus scopes remain open. Evidence:
  `experiments/M11H_FORM_FOCUS_AND_DIALOG_KEYS.md`.
- **MEASURED PARTIAL M11h / Forms breadth:** experimental ABI 0.19 projects
  inherited portable cursor roles into the retained tree. Captured `Cursors`
  singletons now carry real resize/hand/wait/forbidden identities, and
  `Control.Cursor` updates native hit-target cursor selection. The legacy
  `ScrollableControl.DockPadding` wrapper now projects into retained `Padding`
  and immediately relayouts docked children. Native, host, and Wine facade gates
  cover round trip, inheritance reset, fill insets, and mutation relayout.
- **MEASURED PARTIAL M12-P10 / scrolling:** experimental ABI 0.20 projects the
  native retained two-axis `ScrollableControl` model: automatic viewport,
  margin, minimum-size and position state; manual `ScrollProperties` axes;
  per-control reveal offsets; display/viewport rectangles; and
  `ScrollControlIntoView`. Panel and ContainerControl share non-destructive
  authored geometry, clipped scroll chrome, captured thumb input, nested wheel
  fallback, and virtual scrollbar semantics. Real scroll input retains one
  revisioned event record and crosses ABI 0.20 into the generated standard
  `Scroll` event without callback-time layout reentry. Native, C11,
  generated-facade, renderer-free, sanitizer, Win64, and Wine gates pass.
  RTL/scaling, drag-edge autoscroll, virtualization anchoring, and
  exhaustive independent WinForms event-order comparison remain open. Evidence:
  `experiments/M12P10_RETAINED_SCROLLING_SUBSTRATE.md`.
- **MEASURED PARTIAL M12-P11 / layout transactions:** `Control` now owns
  renderer-neutral nested suspension and deferred/requested/committed layout
  revisions. Suspended subtrees retain committed geometry without blocking
  runnable siblings; final resume, explicit perform, and post-scope read
  barriers share the bounded `Window` scheduler. Failed native or managed
  layout callbacks restore dirty work and release re-entry guards. Experimental
  ABI 0.21 projects the transaction, while the generated facade implements both
  `PerformLayout` and `ResumeLayout` overloads plus exact `LayoutEventArgs`
  affected-object/property payloads and an eight-pass re-entry bound. Large
  designer graphs, mutation/disposal within layout, exhaustive independent
  WinForms ordering, and DML reorder policy remain open. Evidence:
  `experiments/M12P11_LAYOUT_TRANSACTIONS.md`.
- **MEASURED PARTIAL M12-P12 / mutation-safe layout:** layout traversal now
  snapshots strong child identities and revalidates live parent/window
  membership before callbacks and slot commits. Window recursion, base
  AutoSize/Dock/Anchor, Flow, Table, scrolling extent, Card, and MasterDetail
  therefore tolerate removal, same-window reparenting, callback-target
  disposal, and callback-driven addition without invalid iterators or stale
  geometry. A 1,024-leaf suspended construction corpus commits in at most two
  arrange passes with no bounded-pass hit. Randomized mixed-tree mutation,
  public convergence availability, independent ordering, and DML reorder
  policy remain open. Evidence:
  `experiments/M12P12_MUTATION_SAFE_LAYOUT.md`.
- **MEASURED PARTIAL M12-P13 / callback arbitration:** paint, hit testing,
  semantic projection/action routing, popup-root arbitration, and bulk
  validation now snapshot strong identities and revalidate live parent/window
  membership around application callbacks. Semantic projection discards a
  mixed-generation candidate and retries at most four times; hit testing uses
  the same stabilization bound. Metrics expose arbitration retry and limit-hit
  counts. Focused removal, addition, self-disposal, oscillation, underlying-hit,
  paint-lease, popup, semantic, damage, and validation gates pass. Randomized
  mixed callback mutation, frame-request mutation, managed protected callback
  projection, and physical-host reentry remain open. Evidence:
  `experiments/M12P13_CALLBACK_ARBITRATION.md`.
- **MEASURED PARTIAL M12-P14 / frame polling:** the existing strong request
  snapshot already made timer stop/restart/dispose mutation safe. A new
  exception-safe poll guard now prevents an `OnFrame` or Timer callback from
  recursively delivering peers or callback-created already-due work. Nested
  polls return a deferred result; new work remains connected for the next host
  turn. Public result state and metrics expose reentrant deferral. Focused
  scheduler, timer, range, animation, dispatcher, and showcase interaction
  gates pass; the exact scheduler/timer/animation center also passes
  renderer-free and ASan/UBSan, and affected libraries cross-compile for
  Win64. Physical-host nested-message-loop reentry remains.
  Evidence: `experiments/M12P14_FRAME_POLL_ARBITRATION.md`.
- **MEASURED PARTIAL M12-P15 / property metadata center:** one native
  renderer-neutral registration now serves binding and public property
  inspection without exposing executable callbacks. Value-only descriptors
  publish authored names, scalar kinds/defaults, categories/descriptions,
  browse/bind/serialize/reset/change capabilities, and exact declared dirty
  effects. Generic conversion, get/set, default or authored reset,
  authored/fallback `ShouldSerialize`, deterministic enumeration, tokenized
  change observation, live/UI-thread enforcement, and existing nested
  initialization batching pass across normal, renderer-free, ASan/UBSan, and
  Win64 gates. Current bindable stock properties plus AutoSize and
  CausesValidation are migrated. Compound/resource types, ambient/inherited
  origin, framework-wide coverage, atomic rollback, DML/managed projection,
  localization, and undeclared-effect diagnostics remain. Evidence:
  `experiments/M12P15_PROPERTY_METADATA_CENTER.md`.
- **MEASURED PARTIAL M12-P16 / compound property values:** the same native
  property and binding value domain now retains Point, Size, Rect, Insets,
  Color, FontSpec, generational ImageId, and finite named/flags enum values.
  Shared immutable enum schemas provide type identity and bounded choices;
  descriptor-aware conversion normalizes names/flags and rejects nonfinite
  geometry, unknown names/bits, and mismatched enum types before calling a
  setter. Base geometry/spacing/Dock/Anchor/AutoSizeMode/traversal/drag/
  accessibility plus visible Label/PictureBox/Button font/color/image
  properties use real retained setters and deterministic reset. Inherited
  Label appearance serializes only local overrides. No diagnostic spelling is
  a DML grammar. Normal, renderer-free, warnings-as-errors ASan/UBSan, and
  Win64 gates pass. Collection/content values, origin metadata, truthful
  change events for all properties, atomic rollback, complete stock coverage,
  DML/managed projection, localization, and class-level descriptor storage
  measurement remain. Evidence:
  `experiments/M12P16_COMPOUND_PROPERTY_VALUES.md`.
- **MEASURED PARTIAL M12-P17 / metadata-driven PropertyGrid:** property
  registrations now report exact defaulted/local/inherited/ambient/computed
  value origins independently of `ShouldSerialize`; inherited Label appearance
  proves override/reset authorship. A public native PropertyGrid weakly selects
  any retained Control, projects deterministic categorized/alphabetical rows,
  routes Boolean/scalar/finite-enum values to stock editors, refreshes truthful
  change notifications, preserves targets on invalid edits, and uses actual
  reset contracts. The Complete Showcase edits a live NumericUpDown through
  the public grid. Hostile tests prove selected-object retirement and
  synchronous inspector disposal during a target callback; compound controls
  now finish through the base mutation-free child disposal path. Normal,
  renderer-free, warnings-as-errors ASan/UBSan, and Win64 gates pass.
  Collection/content/nested values, custom editor factories, visible reset and
  context-menu UI, multiple selection, DML, and managed PropertyGrid projection
  remain. Evidence: `experiments/M12P17_METADATA_DRIVEN_PROPERTY_GRID.md`.
- **MEASURED PARTIAL M12-P18 / expandable compound properties:** native
  PropertyGrid now expands Point, Size, Rect, Insets, Color, and FontSpec into
  stable retained child paths. Leaf commits convert one bounded field,
  reconstruct the typed parent, and invoke the real registered setter; invalid
  width/channel/font edits preserve the complete live value and report the
  precise path. PropertyList owns validated row hierarchy, descendant
  visibility/layout, disclosure hit testing, and expand/collapse semantics.
  Each resettable parent now owns a stock retained Reset button whose enablement
  follows ShouldSerialize and whose activation resynchronizes the parent and
  every child. Native macOS accessibility dogfood edited Bounds.X, reset Value,
  and reset Bounds; normal, renderer-free, warnings-as-errors ASan/UBSan, and
  Win64 gates pass. Arbitrary nested objects/collections, editor/type-converter
  factories, flags/resource specializations, multiple selection, modal editors,
  DML, and managed PropertyGrid projection remain. Evidence:
  `experiments/M12P18_EXPANDABLE_COMPOUND_PROPERTIES.md`.
- **MEASURED PARTIAL M12-P19 / nested values and collections:** the property
  value domain now includes structurally comparable immutable object and
  homogeneous collection snapshots with explicit UTF-8, uniqueness, depth,
  member/item, and total-node bounds. Native PropertyGrid recursively projects
  typed member/index paths, reconstructs every immutable ancestor through the
  owning setter, preserves read-only members, refreshes same-shape edits without
  retiring the active editor, and atomically inserts/removes/moves collection
  items. Stock `ComboBox.Items` is a content-serialized, resettable,
  change-observed collection property and the Complete Showcase dogfoods its
  expansion and insertion. Normal, renderer-free, warnings-as-errors
  ASan/UBSan, Win64, public-showcase-policy, and native accessibility gates pass.
  Arbitrary editor/type-converter factories, dictionaries/heterogeneous values,
  nullable specializations, multiple selection, modal component editors, DML,
  and managed PropertyGrid projection remain. Evidence:
  `experiments/M12P19_NESTED_PROPERTY_VALUES_AND_COLLECTIONS.md`.
- **MEASURED PARTIAL M12-P20 / converter and retained-editor services:** inert
  property descriptors now carry optional bounded converter/editor service
  identities without executable callbacks. Instance-owned
  `PropertyValueConverterRegistry` and `PropertyEditorRegistry` provide
  canonical registration, kind defaults, deterministic replacement, bounded
  parsing, and typed tokenized commit. Factories donate ordinary unattached
  retained controls; PropertyList attaches before retiring the stock editor and
  preserves its scroll, focus-reveal, semantic, ownership, and disposal laws.
  PropertyGrid contains factory faults as row diagnostics, synchronizes live
  values without feedback, and defaults numeric values—including expandable
  geometry leaves—to NumericUpDown. An explicit percent converter/stepper and
  the Complete Showcase prove service selection and real-setter commit. Normal,
  renderer-free, warnings-as-errors ASan/UBSan, and Win64 focused gates pass.
  Flags/color/resource/date/path
  specializations, standard values/culture, nested member service metadata,
  modal/drop-down editor services, multiple selection, DML, and managed
  PropertyGrid projection remain. Evidence:
  `experiments/M12P20_PROPERTY_CONVERTER_AND_EDITOR_SERVICES.md`.
- **MEASURED PARTIAL M12-P21 / specialized flags and color editors:** the P20
  service seam now donates public retained `FlagsValueEditor` and
  `ColorValueEditor` controls rather than adding PropertyGrid type branches.
  Flags use a tokenized Window popup/focus scope with a CheckedListBox and typed
  immediate bit commits. Color uses an ordinary TextBox, checker/swatch,
  canonical alpha-aware hex, invalid semantics, cancellation, and an optional
  tokenized editor-failure connector routed to PropertyGrid's exact-path error
  channel. Enum schema names/choice count are explicitly bounded. The Complete
  Showcase Style target dogfoods Anchor and Label ForeColor. Normal,
  renderer-free, warnings-as-errors ASan/UBSan, Win64, and Wine focused gates
  pass; the 48-cycle slider-to-Animation regression remains green. Resource,
  nullable/date/duration/path/command and modal editors, standard values/culture,
  nested-member services, multiple selection, DML, and managed projection
  remain. Evidence:
  `experiments/M12P21_SPECIALIZED_FLAGS_AND_COLOR_EDITORS.md`.
- **MEASURED PARTIAL M12-P22 / nullable, culture, nested services, and atomic
  owners:** property payload kind is retained independently from null; bounded
  unique standard values can be exclusive; and each converter registry owns a
  validated culture/separator context without process-global locale. Nested
  object members carry their own payload/enum/standard/converter/editor schema
  through every projected edit path. PropertyGrid multiple selection projects
  compatible common properties, preflights every owner, restores prior owners
  on rejection, and publishes once after success. Getter output is
  schema-checked. Focused binding/inspection gates pass. Dynamic standard-value
  providers, mixed-value editor visuals, DML, managed projection, modal/path/
  date/duration editors, and full portability closure remain. Evidence:
  `experiments/M12P22_NULLABLE_CULTURE_NESTED_AND_ATOMIC_PROPERTIES.md`.
- **MEASURED PARTIAL M12-P23 / managed PropertyGrid ABI 0.22:** the additive
  C table creates native PropertyGrid and projects bounded same-thread control
  selection, sort, and refresh. The generator explicitly emits PropertyGrid,
  PropertySort, SelectedObject(s), and selection/sort events; generated build,
  1,104-row verifier, managed behavior smoke, C/C++ ABI tests, and the 63-test
  native suite pass. Arbitrary TypeDescriptor objects are rejected without
  replacing selection until typed converter/reset/editor callbacks can be
  carried honestly. Win64 builds and the managed PropertyGrid smoke passes
  under Wine; the callback channel remains.
  Evidence: `experiments/M12P23_MANAGED_PROPERTY_GRID_ABI_0_22.md`.
- **MEASURED PARTIAL M12-P24 / managed TypeDescriptor ABI 0.23:** a nonvisual
  foreign-object proxy deep-copies bounded descriptor/enum/standard metadata
  and projects nullable scalar/text/Color/enum values through synchronous
  getter/setter/reset/serialization/change and TypeConverter format/parse
  callbacks. The generated PropertyGrid now accepts ordinary TypeDescriptor and
  ICustomTypeDescriptor objects, preserves prior selection on adapter failure,
  and delegates converted and reset commits to the native rollback-safe
  multiple-owner transaction. C11 callback tests, the 1,104-row generated
  verifier, host .NET behavior, the 63-test native suite, renderer-free ABI
  tests, Win64, and Wine pass. UITypeEditor drop-down/modal services, dynamic
  standards, mixed-value visuals, nested arbitrary managed objects, specialized
  date/path/resource editors, DML, and public GridItem/value-change projection
  remain. Evidence:
  `experiments/M12P24_MANAGED_TYPE_DESCRIPTOR_PROXY_ABI_0_23.md`.
- **MEASURED PARTIAL M12-P25 / managed UITypeEditor ABI 0.24:** foreign
  property definitions append an optional editor callback without breaking the
  ABI 0.23 record prefix. The native PropertyEditorRegistry installs a real
  retained value/editor Button, and activation calls the managed UITypeEditor
  with a bounded ITypeDescriptorContext and IWindowsFormsEditorService.
  DropDownControl owns an anchored blocking Control host with CloseDropDown;
  ShowDialog owns a modal Form. Both return a typed value through the existing
  rollback-safe multiple-owner setter transaction. Nested-loop lifetime is
  revalidated before projection callbacks resume. Native C11, the 1,104-row
  verifier, 0-warning managed build, host .NET behavior, the 63-test suite,
  renderer-free ABI tests, Win64, and Wine pass. Dynamic standards, mixed-value
  visuals, component editors, nested arbitrary managed objects, specialized
  date/path/resource editors, DML, and public GridItem/value-change projection
  remain. Evidence:
  `experiments/M12P25_MANAGED_UI_TYPE_EDITOR_ABI_0_24.md`.
- **MEASURED PARTIAL M12-P26 / editable raster canvas and color truth:**
  GUI.Drawing `Bitmap` now supports one exclusive bounded edit lease with
  explicit commit/cancel, byte-derived damage, no-op generation stability,
  immutable prior snapshots, and a bounded multi-consumer `changes_since`
  history. Experimental Drawing ABI 0.2 appends those operations while
  preserving negotiable ABI 0.1 prefixes. `ImageRegistry` can patch one raw
  BGRA rectangle atomically, `Control::invalidate(Rect)` retains exact local
  host damage, and public `RasterCanvas` maps bitmap generations into retained
  zoom/pan, nearest/linear sampling, transparency presentation, RGBA/BGRA
  normalization, and deterministic resource retirement. GUI.Drawing also
  owns checked IEC sRGB transfer, linear sRGB↔XYZ D65, OKLab/OKLCH, unclamped
  gamut status, and deterministic chroma-only sRGB mapping. The 64-test full
  native suite, five focused renderer-free gates, C11 ABI tests, Skia and
  CoreGraphics builds, renderer audits, and strict Win64 cross-build pass.
  Renderer-side partial cache upload, pressure samples, the detailed owned
  Color dialog, ICC policy, and profile-explicit CMYK remain open. Evidence:
  `experiments/M12P26_EDITABLE_RASTER_CANVAS_AND_COLOR_TRUTH.md`.
- **MEASURED PARTIAL M12-P27 / initialization mutation ownership and Dock
  order:** native `Control` now defers and coalesces stock state publications
  during nested initialization, drains them after invalidation in stable order,
  abandons them on disposal, and suppresses routed interaction while partial.
  Focus cleanup no longer synchronously invokes application virtual/event
  callbacks from `BeginInit`. Default Dock consumes reverse public z-order in
  the native core while public index zero, paint, hit test, and bring/send retain
  one coherent topmost model. Focused lifecycle/layout gates and the full
  native and renderer-free suites, focused ASan/UBSan gates, and strict Win64
  cross-build pass. An earlier parallel AppKit wake timing miss passed on
  immediate isolated rerun and the rebuilt full follow-up. Aggregate old/new
  payload semantics and independent .NET 10 geometry traces remain open. Evidence:
  `experiments/M12P27_INITIALIZATION_OWNERSHIP_AND_DOCK_ORDER.md`.
- **MEASURED PARTIAL M11h / FM-LY05:** the public retained control library now
  owns split-pane geometry and input rather than leaving it to a consumer:
  stable panel/seam identities, distinct thin paint and enlarged hit target,
  constrained vertical/horizontal layout, live captured drag, keyboard resize,
  collapse focus transfer, and fixed-panel resize. The generated `SplitContainer`
  projection passes host, headless Wine, and physical Win32/Wine behavior.
  Responsive automatic/user collapse distinction, persistence, nested stress,
  DML/C ABI, and native semantics remain open. Evidence:
  `experiments/M11H_RETAINED_SPLIT_CONTAINER.md`.
- **MEASURED PARTIAL M11h / focus scopes:** the renderer-free retained `Window`
  now owns bounded nested focus scopes, contained focus, deterministic
  Tab/Shift+Tab traversal, LIFO and out-of-order focus restoration,
  owner-unavailable cleanup, typed scope changes, UI-thread enforcement, and
  structured scope metrics. This removes a core popup dependency but is not a
  popup implementation: click-away/Escape, capture transfer, placement,
  semantic parentage, DML/C ABI/binding projection, and native accessibility
  publication remain open. Evidence:
  `experiments/M11H_RETAINED_FOCUS_SCOPES.md`.
- **MEASURED PARTIAL M11h / complete-showcase controls:** a second independent
  native demonstration now covers fifteen retained pages and at least 180 public
  controls without changing the File Manager mockup demo. Public animation
  timelines/easing, visual variants, multiline Label layout, single-line
  TextBox, ListBox, ComboBox, NumericUpDown, H/V ScrollBar, PictureBox,
  TabControl/TabPage, CheckedListBox, Timer, ToolTip, and an
  owner-tokenized root popup controller are backed by headless conformance.
  **DESIGN CORRECTION 2026-08-06:** this evidence is valid only when the board
  consumes the public library. Eight local subclasses had hidden proportional
  layout, drawing, easing, diagnostics, timer motion, and root lifetime behavior
  inside the demonstration. They were removed. Public `ScaledPanel`,
  `ScaledGroupBox`, `DrawingSurface`, `EasingPreview`, `MetricsView`, and
  `Control::Tag` now own those reusable contracts, while timer motion composes
  public controls and design-space slots. A source-policy test rejects any
  future showcase-local class or struct inheritance.
  PictureBox covers all five canonical sizing policies over the validated,
  window-owned image registry; removal now retires cached display chunks rather
  than replaying stale generational IDs. Native dogfood proves mixed-script fallback,
  typing, drag selection, Undo, initial collection viewports, popup commit, and
  focus restoration, continuous scrollbar drag/value automation, image scaling,
  opacity, image accessibility, all four tab alignments, all three tab
  appearances, native semantic tab selection, independent list selection/check
  state, semantic check-row activation, UI timer cadence/quiescence, and
  target-anchored retained tooltip overlays. Renderer-free semantic snapshots, stock adapters, virtual
  list rows, compound boundaries, action routing, and a stable-ID-reconciled
  AppKit accessibility publisher are now measured by headless and live
  automation dogfood. A subsequent polish slice adds renderer-neutral tracked
  text, pinned HarfBuzz/FreeType and exact-hash Portsmouth/Carlito/Cousine plus
  role-independent Noto CJK/emoji fallback packaging, semantic host sound cues,
  public orthogonal enable/pause/reduced motion policy with low-cadence,
  limited-excursion reduced animation, a phase-preserving reusable timeline, atomic progress
  policy, retained AppKit deadline source and repeated hidden-activation plus
  disarm/quiescent-gap/rearm host proof. AppKit presentation no longer requests
  the next draw from inside `drawRect`; the native gate requires actual paints
  as well as scheduler callbacks,
  visibly live split-pane reallocation, and a retained DateTimePicker/calendar
  popup with semantic optional-value activation and range-aware virtual month
  buttons. The fifteenth page dogfoods public FlowLayoutPanel and
  TableLayoutPanel: four flow directions, wrap/break, margins/padding,
  absolute/auto/weighted-percent tracks, automatic placement, spans, borders,
  and retained layout slots that preserve authored preferred bounds. The same
  board now proves a renderer-free UI dispatcher: thread-safe Window/Control
  `BeginInvoke`, synchronous Window/Control `Invoke`, one FIFO snapshot per turn,
  nested next-turn deferral, explicit and owner-lifetime cancellation, fault
  isolation/propagation, queue and turn bounds, concurrent-producer ordering,
  shutdown revocation, phase ordering, and structured telemetry. AppKit executes
  async, nested, and blocking worker work on its UI thread; Win32/Wine publishes
  completed, cancelled, and marshalled-invoke evidence.
  A further
  host-services page covers common dialogs, cancellation
  preservation, clipboard, monitors, and every sound cue; Win32 now attaches the
  same protocol-v5 service seam as AppKit/headless and Wine proves native
  MessageBox/ChooseColor modal cleanup. The native suite passes 48 tests,
  including the public-consumer source policy; a fresh Skia/HarfBuzz/host-free
  configuration builds the core/control/input libraries and passes 36
  renderer-free tests. Multiline/bidi/IME, editable semantic text ranges,
  editable combo/binding/type search, synchronization-context/BackgroundWorker
  projection, generated-facade `Invoke`, UIA/AT-SPI publication, and the wider
  control catalogue, Windows HarfBuzz/FreeType raster integration, and broader
  calendar culture remain open. Evidence:
  `experiments/M11H_COMPLETE_SHOWCASE_TEXT_COLLECTIONS.md`.
  Dialog/host evidence:
  `experiments/M11H_DIALOGS_AND_HOST_SERVICES.md`.
  Dispatcher evidence: `experiments/M11H_UI_DISPATCHER.md`.

Current bounded M11 rounds:

1. **M11g:** finish exact line-height policy, the broader region/clip/raster
   .NET/Wine differential, and sustained native-surface cadence. Dynamic calls,
   zero-drawing-passthrough, direct surface transport, requested-family
   resolution, and active central pixels are already measured.
2. **M11h:** continue from the measured form/dialog-key, retained focus-scope,
   split-pane, public text/list/combo, and popup-controller slices into
   independent secondary native windows, validation, complete IME/clipboard
   editing, binding/source selection, responsive collapse/pane
   persistence, then damage isolation, sustained paint measurement, and soak.

**MEASURED PARTIAL M11h / nominal-base correction:** the pinned LibreWinForms
and .NET 10 reference surfaces now generate an 18,097-row member ledger. It
reports missing/partial/excluded status conservatively and makes the 1,104-row
specimen facade an explicit floor rather than a completion score. The first
corrective tranche stores buffering/control styles and adds revisioned,
exclusive, non-reentrant transaction paint leases with coherent callback-fault
abandon and host presentation release. It also closes a first base-Control
tranche: mutable Name, constrained finite geometry, coordinate conversion,
containment, z-order operations, and deterministic TabIndex/TabStop traversal.
Physical native-host reentry coverage, drag/semantic deferral, raster swap on
backend fault, multi-rectangle host release, ABI projection, and the majority of the
nominal API ledger remain open. Evidence:
`experiments/M11H_COMPATIBILITY_CATALOGUE_AND_PAINT_LEASES.md`.

**GIVEN / REQUIRED paint-pipeline closure:**
`planning/PAINT_PIPELINE_AVAILABILITY.md` makes the high-rate buffering premise
a baseline contract rather than an optional host feature: mutation may outrun
rendering and rendering may outrun presentation without backlog or incomplete
pixels. The existing content/rendered/presented revisions, coalesced wake,
exclusive lease, rendering-dirty follow-up, ready state, and host release are
measured. M11h-P1 replaces compatibility-surface capture-on-flush with dirty
signals and one target-scoped drain; M12-P2 requires atomic candidate-raster
swap; M12-P3 adds input/stall/producer-pressure conformance; M11h/M12-P4
projects the mandatory contract version plus only genuinely optional host
accelerations through availability reporting.

**MEASURED PARTIAL M11h-P1 / compatibility-surface drains:** generated
`Graphics.FromHwnd` now retains its private GUI.Drawing bitmap instead of
recapturing the HWND before every Flush. Flush, ReleaseHdc, EndPaint, callback
return, resize, visibility, and Invalidate mark a revisioned direct-surface
queue; one owner-thread drain imports the newest pixels, one active lease rejects
reentry, and one dirty-after bit preserves a later pass. Physical Wine proves
sixteen Flush updates plus ReleaseHdc collapse into one changed import, a raw
unobservable GDI write is found once by the adaptive 33–250 ms fallback, and
Update from OnPaint reaches depth one with one follow-up. The same generated
Control surface now reflects protected `DoubleBuffered`/style state and reuses
one size-matched managed owner-paint bitmap; background and foreground share a
transaction, resize abandons the stale epoch and posts one replacement, disable
retires persistence, and callback failure preserves the last raster without a
self-retry loop. Rectangle-aware GDI damage, presentation acknowledgement, and
the full ordering/failure corpus remain. Evidence:
`experiments/M11H_COMPATIBILITY_SURFACE_DRAINS.md`.

**MEASURED PARTIAL M11h-P1 / managed damage continuation:** all six nominal
`Control.Invalidate` overloads, `InvalidateEventArgs`, `Invalidated`,
`NotifyInvalidate`, `OnInvalidated`, and GUI.Drawing `Region.GetBounds` are now
generated behavior rather than owner-level placeholders. Persistent buffered
surfaces clip, union, and consume rectangle damage once; child invalidation is
intersected and coordinate-translated; failed paint restores its lease damage;
ephemeral paint promotes to a coherent full surface. Physical Wine proves each
center. Nonrectangular region fidelity, direct-GDI rectangle damage, and partial
native upload/presentation remain open.

**MEASURED PARTIAL M11h-P1 / managed paint-input continuation:** generated
pointer, key, and text ingress now defers behind one bounded UI-thread queue
while any managed paint lease is active and posts one ordered drain after the
outermost lease. Physical Wine proves pointer-before-key delivery outside
application paint, one ordinary localized invalidation flush, and deterministic
abandonment when a control disposes from `OnPaint`. That disposal also retires
the paint candidate without querying its dead native peer. Managed key-preview
return-value parity and physical native-host coverage remain M12-P3 work.

**MEASURED PARTIAL M12-P3 / retained input-pressure continuation:** portable
`Window` now owns a renderer-neutral deferred-input queue with a fixed 1,024
event capacity, adjacent pointer-move and same-session drag-over compaction, one
posted outermost-lease drain, per-event fault isolation, and explicit retirement
counters.
Normal and renderer-free gates inject 100 moves plus down/key/text during paint:
the moves become one latest position, critical input remains ordered, no
application callback enters paint, and one input mutation produces one ordinary
later paint with zero residual work. A 1,025-key root-retirement gate proves the
capacity, one explicit rejection, and deterministic abandonment. Thirty-two
drag overs reuse only the same session's prior valid effect and compact to one
delivery; stable-identity semantic Press invokes once after release and uses the
ordinary invalidation path. `HostDispatchResult` and `DragDispatchResult`
distinguish deferred retention from capacity rejection. Physical native-host
reentry and slow renderer/presenter/occlusion storms remain. Evidence:
`experiments/M12P3_RETAINED_INPUT_PRESSURE.md`.

**MEASURED PARTIAL M12-P3 / presentation-pressure and frame-fault
continuation:** `Window::paint` now returns an exact revision/epoch receipt
only after backend replay and a second owner/epoch check. macOS and Win32
publish and acknowledge only that receipt; duplicate, backward, and
replaced-epoch releases are rejected and counted. Deterministic gates inject
100 replay-boundary mutations, one deferred pointer, eight nested paint
attempts, replay-time resize, backend fault, retirement, and 100 occluded
mutations. Each path retains one wake/drain/follow-up and returns to zero work.
Frame/UI-timer callback faults now disconnect and count only the failing lease
while healthy peers continue; native timer callbacks contain any residual C++
exception. A supplied AppKit wake-source `std::terminate` report established
the escaped-exception class. M12-P5 then reproduced the concrete fault through
deterministic slider/navigation/tooltip churn and corrected popup unregister
ownership; host callback containment remains a last boundary, not a substitute
for fixing the model fault. Candidate-raster swap, physical native paint
reentry, and wall-clock native soak remain. Evidence:
`experiments/M12P3_PRESENTATION_PRESSURE_AND_FRAME_FAULTS.md`.

**MEASURED PARTIAL M12-P4 / guidance-provider continuation:** the generated
LibreWinForms ledger selected the previously missing ErrorProvider/HelpProvider
family rather than a showcase-local need. Public `ErrorProvider` now owns
per-control error strings, six alignments, padding, portable icon substitution,
RTL, Tag, semantic invalid/error projection, and independently retained popup
glyphs. Changed-error blink is a bounded six-transition active surface;
AlwaysBlink remains active only while schedulable, NeverBlink is idle,
occlusion suppresses deadlines, and reduced motion settles visible. Target,
ancestor, provider, and Window lifetime paths revoke or restore adornments
deterministically. Public `HelpProvider` retains string, keyword, navigator,
namespace, automatic/explicit ShowHelp, reset, and Tag; F1 walks focus ancestry
and emits a mutable Control event before provider policy. It never launches an
external help resource. The Complete Showcase dogfoods both providers. Binding
and currency integration, TopicId/raw managed enum projection, described-by
relations, DML/C ABI, localization refresh, and native accessibility publisher
verification remain open. Evidence:
`experiments/M12P4_GUIDANCE_PROVIDERS.md`.

**MEASURED PARTIAL M12-P5 / popup and callback containment:** a 48-cycle
range-to-Animation sequence deterministically reproduced the reported
intermittent abort as a duplicate tooltip popup stable ID. `ToolTip` had
released its strong overlay references before disconnecting the weak popup
attachment, so Window could no longer detach and unregister the subtree. It
now closes the token first; a separate 32-cycle keyboard tooltip test proves
stable-ID reuse. Portable host dispatch and AppKit damage/posted-work/draw
callbacks contain and count residual application exceptions so none crosses a
foreign callback boundary. Focused normal and sanitizer gates pass; another
manual trace is not required for this defect. Evidence:
`experiments/M12P5_POPUP_CALLBACK_CONTAINMENT.md`.

**MEASURED PARTIAL M12-P5 / binding and currency:** public renderer-neutral
`BindingValue`, explicit `BindableProperty`, `BindingSource`, `Binding`,
`CurrencyManager`/`BindingManagerBase`, `ControlBindingsCollection`, and
`BindingContext` now provide stable retained records, edit/list/currency
mutation, deterministic notification, suspension coalescing, immediate /
explicit-validation / never update modes, strict invariant conversion,
Format/Parse plus F0..F12, null substitution, post-commit completion and error
propagation, manager-wide push/pull, and synchronous endpoint/context cleanup.
Base state, stock text/check/range, NumericUpDown Value, and ComboBox Text /
SelectedIndex are registered properties. The Complete Showcase Values page
dogfoods three two-way fields and record currency. Normal, sanitizer, strict
renderer-free, MinGW x64, and Wine gates pass. At this stage managed-object
projection, nested DataMember, culture providers, sort/filter, focus
validation, ErrorProvider integration, BindingNavigator, DataGridView, DML,
and C ABI were the next tranche; focus validation and ErrorProvider binding are
closed immediately below by M12-P6. Evidence:
`experiments/M12P5_BINDING_CURRENCY_KERNEL.md`.

**MEASURED PARTIAL M12-P6 / validation and bound errors:** `Control` now
retains `CausesValidation`, cancellable `Validating`, and successful
`Validated`; `ContainerControl` retains exact-value inherited `AutoValidate`,
explicit validation, and constrained child validation. Window executes the
transaction before focus loss, supports prevent/allow/disable policy, walks
the previous branch only to the common ancestor, rejects nested focus
mutation, rechecks callback-mutated endpoints, and exposes deterministic
counters. `Binding` subscribes OnValidation to that transaction. BindingSource
records carry portable record-wide/field errors, and ErrorProvider now binds
to real targets, aggregates record and BindingComplete failures, follows
currency, and revokes on source retirement. The Complete Showcase dogfoods the
path, while its existing 48-cycle slider-to-Animation scenario keeps the
previous intermittent crash deterministic. Arbitrary managed IDataErrorInfo,
true nested object traversal, ABI projection, and deeper native oracle parity
remain open. Evidence:
`experiments/M12P6_VALIDATION_AND_BOUND_ERRORS.md`.

**MEASURED PARTIAL M12-P7 / dialog keys and mnemonics:** the renderer-free
control kernel now parses single-marker/escaped-ampersand mnemonic text,
projects marker-free measure/paint/semantics, and routes Alt+letter/digit in
stable effective retained order after the focused route and accelerators.
Label/GroupBox focus the next selectable control. ButtonBase owns a validated
programmatic command path shared by mnemonics and semantic press. Window/Form
retains accept/cancel targets, transfers the Button default cue, routes
Enter/Escape without focus theft, contains commands to the active focus scope,
clears disposed targets, and exposes attempt/handled counters. The Complete
Showcase dogfoods those contracts. Duplicate-mnemonic cycling, locale-sensitive
case folding/underline cues, native DialogResult/modal closure, managed
protected-call/C ABI projection, and an independent Windows oracle remain.
Evidence: `experiments/M12P7_DIALOG_KEYS_AND_MNEMONICS.md`.

**MEASURED PARTIAL M12-P8 / command arbitration and native callback
boundaries:** mnemonic dispatch now snapshots one stable eligible candidate
set before application callbacks, confines it to the active focus scope,
cycles duplicate winners, and exposes candidate/collision/cycle counters.
MenuStrip and popup rows share marker-free measure/paint/semantics and retained
Command activation. The native core publishes every exact DialogResult value,
rejects numeric gaps, defaults a previously unset cancel Button, and propagates
Button Click before Window result. Independent native modal-loop closure and
protected managed ToolStrip projection remain open. The historical
Ranges-to-Animation crash is isolated to an Objective-C exception escaping the
AppKit dispatch-source wake; that boundary now records name/reason instead of
terminating. The exact user path passes 48 deterministic retained cycles and
10 real AppKit cycles after repair. Evidence:
`experiments/M12P8_COMMAND_ARBITRATION_AND_NATIVE_CALLBACKS.md`.

**MEASURED PARTIAL M12-P9 / Control geometry and order:** the retained core and
generated facade now publish exact AutoSizeMode, BoundsSpecified, and
GetChildAtPointSkip values; masked bounds, proposed preferred size,
GrowOnly/GrowAndShrink AutoSize, point/rectangle ancestry transforms,
direct-child filtered lookup, non-wrapping nested TabIndex traversal, and
index-zero-topmost collection mutation. The audit corrected a generated-facade
ordering split where Add/BringToFront disagreed with SetChildIndex, docking,
and native paint order. Portable desktop-screen origin, multi-monitor/DPI
rounding, protected event-order parity, and hard AutoSize convergence remain
open. Evidence: `experiments/M12P9_CONTROL_GEOMETRY_AND_ORDER.md`.

**MEASURED PARTIAL M11h / progress-effects continuation:** public
`ProgressBar` now owns three additional reusable visual styles: a slow
forward-travelling luminance pulse, clipped marching stripes, and a laser-etch
style with a vertically shifting repeated phase field, leading-edge energy,
compact white-hot corona/core, short phase-shifted flame lobes, and sparse ember
flecks. Long tendril-like particles were rejected during native dogfood.
`ProgressBarAnimationAppearance` validates and
commits all effect colors and geometry atomically. Every style uses the common
retained frame lease, pause/disable/visibility quiescence, and reduced-motion
policy; none is a showcase-private painter. Deterministic painter probes and
the native Complete Showcase ranges board are recorded in
`experiments/M11H_PROGRESS_ANIMATION_STYLES.md`.

**MEASURED PARTIAL M11h / visual-material correction:** the public retained
paint vocabulary and display chunks now carry rounded clip/fill/stroke,
multi-stop linear gradients with pad/repeat/reflect spread, elliptical radial
gradients, and bounded box shadows.
Public `SurfaceMaterial`/`MaterialPanel` provides content-agnostic ordered
layers, border, shadow, normalized resizing, atomic validation, and bounded
visual outsets whose former extents participate in damage. Private Skia,
CoreGraphics, and Win32 DIB painters implement the vocabulary; the File Manager
title/ribbon consumes it through public API, including a logical-period title
texture. Exact repeated-period probes pass in Skia and CoreGraphics; Win32 DIB
uses the same renderer-neutral spread contract.

**MEASURED PARTIAL M11h / visual-composition continuation:** an immutable
complete theme matrix now covers eleven relational roles, seven surface states,
selected/unselected, and ordinary/high-contrast axes. Window replacement is
atomic; retained controls inherit or locally override a theme. Panel,
Button/command/accent Button, CheckBox, RadioButton, Card, TextBox, ListBox
selection, ComboBox, MenuStrip, and ProgressBar consume relational recipes,
while explicit local style overrides preserve compatibility paint. A stock-role
routing fixture proves application replacement reaches each family. Public Card provides atomic header/body/footer composition and full
interactive state. Public MasterDetailView provides arbitrary retained roles,
genuine splitter input, focus-safe compact navigation, and automatic wide
restoration. The File Manager dogfoods twelve public palette Cards. Immutable,
validated spacing/geometry/typography/motion tokens now drive Card and
MasterDetailView defaults while preserving explicit overrides and reset. This
still does not close M9: complete control/type adoption, density variants,
remaining typed specimen/disclosure/step patterns, rich icons,
masks/groups/effects, vector role assets, and broader image import remain open.

**MEASURED PARTIAL M11h / typed review and paint-wake continuation:** Label now
inherits explicit body/control/caption/heading/title/monospace typography roles
and state-aware foreground from the immutable structural theme, while local
font/color overrides remain authoritative and resettable. Public `ReviewCard`
atomically projects validated decision/evidence/verdict records through ordinary
Card ownership, layout, interaction, tokenized events, and complete semantics;
the File Manager DNA surface now dogfoods it. Native individual-window
inspection also reproduced a cross-window stale-frame defect: controller
mutation changed the product semantics but pixels waited for product input. A
renderer-neutral coalesced paint wake now rearms after damage consumption,
suppresses while occluded, and wakes on exposure. AppKit, Win32, and headless
hosts implement it; the repeated controller action immediately presents Program
DNA without a target-window click.

**MEASURED PARTIAL M11h / rich-image continuation:** the generated nominal
ledger's 46 ImageList/ImageCollection rows are promoted from `missing` to
conservative `partial_native` because a real renderer-neutral type now exists.
It owns ordered ASCII-case-insensitive keys, stable indices, bounded logical size,
Tag, revision/events, atomic PNG or existing-image variants, deterministic
normal/hot/pressed/selected/disabled and density fallback, and synchronous
owned-resource disposal. ButtonBase implements direct Image plus ImageList
key/index selection, alignment, five text/image relations, gap, measurement,
pressed displacement, and state painting; TreeView/ObjectView virtual rows use
the same list. The Complete Showcase visibly dogfoods an authored normal/hot/
disabled folder icon. Native handles/streams, strip slicing, color-key,
quantized import, and vector/provenance role packs remain open; reusable
nine-patch is processed by the following image-material continuation.

**MEASURED PARTIAL M11h / image-material continuation:** the retained Painter
and display-chunk vocabulary now carries source-pixel image regions. Skia,
CoreGraphics, and Win32 DIB painters realize the same crop contract. Public
SurfaceMaterial adds atomically validated stretch, exact-period tile with
cropped partial edges, and density-aware nine-patch layers; attached
MaterialPanel rejects missing or stale-size Window resources. GUI.Drawing adds
bitmap-snapshot `TextureBrush` with tile/mirror/clamp wrapping, affine
transforms, cloning, deterministic trace, and Skia shader execution. The
Complete Showcase visibly dogfoods a crisp nine-patch and diagonal texture
tile. Physical Wine automation navigates to the page, captures the 1280×820
Win32 DIB surface with both materials intact, and closes normally. All 22
TextureBrush owner rows are conservatively `partial_native`;
source-rectangle/image-attribute constructors, MatrixOrder overload parity,
managed/C ABI projection, masks/groups/effects, and vector assets remain open.

**MEASURED PARTIAL M11h / DNA and backplane correction:** the File Manager now
dogfoods `MasterDetailView` and interactive public `Card` in a four-record DNA
decision browser. Tests cover wide side-by-side projection, compact
master/detail navigation, selection transfer, controller routing, and Folder
restoration. Native AppKit inspection also exposed a stale-pixel defect after
popup detachment: damage already covered the former bounds, but a transparent
layout root did not repaint empty gaps. `Window::paint` now records the themed
window backplane in full window coordinates, clipped to damage, before
application planes. Model tests prove pointer and semantic popup-closure damage
and retained backplane replay; live Computer Use confirms the native popup
pixels clear.

M11g remains active, and the first bounded M11h Forms-semantics slice is
measured: instrumentation distinguishes missing Forms behavior from owned
drawing behavior without the temporary Microsoft drawing implementation.

**MEASURED PARTIAL M11g:** the pinned unchanged specimen now passes bitmap/icon
resource hydration, audio/device discovery, plugin loading, and main-window
construction using the owned Drawing facade. Its live tree contains 204 controls
and 204 stable IDs. ABI 0.7 now projects checkbox/radio state, DockPanelSuite's
empty full-client auto-hide strip is input-transparent, and an exact routed
`useSquelchCheckBox` click visibly toggles state and drives retired compatibility specimen's numeric-field
enablement while the host stays responsive. Remaining radio/field families,
broader differential fixtures, requested-family resolution, and visible
spectrum/waterfall closure remain M11g exit gates. Retained popup
menus now have owned vertical geometry, nested submenu lifetime, checked/hot
painting, native Portsmouth label projection, and close without terminating the
main loop. Custom-painted labels are input-transparent, and preferred-size /
AutoSize table propagation now exposes retired compatibility specimen's previously zero-height source
URI row and action buttons inside DockPanelSuite. Evidence:
`experiments/M11G_UNCHANGED_SPECIMEN_STARTUP.md`.

**MEASURED CONTINUATION:** experimental ABI 0.10 closes the previously fake-only
managed file/folder route and the nonvisual tooltip route. Windows/Wine now
shows Win7-compatible native file and folder dialogs with modal owner
suppression; AppKit's existing dialog service is connected to the same portable
request/result seam. Tooltips use delayed/cancellable retained associations and
custom non-activating host surfaces on Windows and AppKit. Wine visual gates
cover show, cancel, owner restoration, hover show, leave cancellation, and
auto-pop. This does not freeze ABI 1.0 or close M11g's direct-chunk,
differential, or active-source gates. Evidence:
`experiments/M11G_UNCHANGED_SPECIMEN_STARTUP.md`.

**MEASURED MENU CONTINUATION:** the retained menu session now owns one active
root per UI thread, deterministic keyboard traversal and nested return,
pointer-down click-away ordering, cancellable hover-open/retirement, edge-aware
submenu reversal, and idempotent chain teardown. Interactive custom raster
controls admit real tokenized key input; transparent raster overlays do not.
Wine proves physical nested keyboard routing and outside-click ordering, while
24 menu reopen cycles and twelve popup/dialog host cycles leave no retained
orphans. The unchanged retired compatibility specimen menu passes Down/Right/Left/Escape and remains
responsive with 165 retained controls. The later calendar update below closes
the call-counter and direct-surface items; broader differential and visual
streaming work remains. Evidence:
`experiments/M11G_UNCHANGED_SPECIMEN_STARTUP.md`.

**MEASURED M11g CALENDAR UPDATE:** the dynamic call ledger and direct retained
surface gates are processed. The unchanged specimen has exercised 446 of 874
instrumentable keys across 872,599 calls while the complete required surface
remains `1104/1104`. ABI 0.16 replaces production managed-paint PNG transport
with a bounded, owned premultiplied-BGRA resource consumed by all three raster
adapters. Its unchanged-specimen confirmation records 283 paints, zero PNG
bytes, and about 0.973 ms/paint versus about 2.074 ms/paint on the prior route.
The .NET 10/Wine text/raster differential slice agrees on exception behavior
and font height with reviewed width deviations of roughly 1–10%. A bounded
25-minute active-source run establishes partial-frame, damage, CPU, and memory
baselines. The central spectrum/waterfall is still visually blank; broader
differential review, portable requested-family resolution, and visual streaming
closure remain open. Evidence:
`experiments/M11G_UNCHANGED_SPECIMEN_STARTUP.md`.

**MEASURED M11g ABI 0.17 CONTINUATION:** the blank central-surface cause was a
false `Control.Handle` contract. The unchanged specimen's native bitmap performs
direct `BitBlt`, so ABI 0.17 now exposes authoritative absolute control bounds
and the Windows adapter attaches an input-disabled child HWND as a paint lease.
The specimen reports correct central geometry and, after local-source
connection, same-process samples observe non-uniform SpectrumAnalyzer and
Waterfall content. All 21 Windows test executables pass under Wine. Portable TTC
face selection plus a Windows installed-font provider resolve every family
observed in the specimen; the v2 differential matches all tested string widths
and all 19 compound-path points, bounds, translation, and hit tests. Exact
family line heights, region/clip breadth, and sustained surface cadence
remain M11g work. Evidence:
`experiments/M11G_UNCHANGED_SPECIMEN_STARTUP.md`.

Exit: two independent native consumers plus the C# gallery pass lifetime,
callback, thread, error, and packaging tests. Freeze 1.0 only after the breaking-
change audit and architect approval.

Pre-M11 documentation checkpoint:

- **OBSERVED:** `docs/LIBRARY_AND_ASSEMBLY_GUIDE.md` now separates the native
  engine, future managed facade, trusted retained-subtree extension shape,
  File Manager capability plugins, and data-only theme/language assemblies.
- **OBSERVED:** private-context resolution tolerates the authentic Forms public
  key token resolving to the unsigned laboratory assembly. **OPEN:** default
  context identity policy, target framework matrix, NuGet layout, complete
  generated surface, and unload mechanics remain M11 decisions rather than
  promises made by the guide.

### M12 — Windows/Linux hosts, packaging, dogfood, and 1.0

Deliver:

- Win32/TSF/OLE/UIA host and CPU bitmap presenter;
- Wayland/input-method/AT-SPI host and X11/XIM-or-selected-IME/AT-SPI host;
- signed/notarized macOS bundle, signed Windows installer/portable package, and
  selected Linux packages with desktop integration;
- install/upgrade/rollback/uninstall and offline/reproducible build lanes;
- long-running gallery/laboratory soak, startup/memory/input benchmarks, crash
  fault injection, dependency/license/SBOM, and release documentation.
- retired compatibility specimen dogfood lane using the captured versioned compatibility manifest and
  behavioral traces, followed by a best-effort attempt to load the unchanged
  authoritative specimen against the generated facade under Wine. Interface
  coverage comes first; failure to load an unchanged binary is recorded as
  compatibility evidence rather than retroactively expanding earlier scope.
  .NET remains outside the native runtime.

Exit: all platform conformance suites and approved budgets pass on named
hardware. Unresolved matrix rows are explicitly excluded, not silently partial.

Windows preflight after M6c:

- **MEASURED:** Wine devel 11.10, x86_64 MinGW-w64 GCC 15.2.0, an x64 Wine
  prefix, Windows Desktop Runtime 10.0.3/10.0.5, and the authoritative x64 retired compatibility specimen
  specimen are locally available.
- **OBSERVED negative:** Wine can enumerate installed runtimes, while the
  broader `dotnet --info` workload query reaches the runtime and then fails in a
  Wine process/workload-installer stub. No retired compatibility specimen executable was launched.
- **MEASURED:** the bounded W0-W4 host/toolchain gates now pass under Wine. See
  `docs/WINDOWS_WINE_HOST.md` and the preserved original preflight in
  `experiments/WINDOWS_ORACLE_PREFLIGHT.md`.
- **OPEN:** Win32 UI Automation publication, TSF/OLE integration, physical
  Windows dogfood, and the later unchanged-binary dogfood gate. The protocol-v5
  clipboard/monitor/dialog/sound services and managed facade are measured.

## 6. Work that may run in parallel

After M1 contracts freeze, these lanes can proceed concurrently:

- renderer benchmark/display chunks (M2) and host service implementation (M3);
- text engine internals (M4) and non-text layout algorithms (M5);
- pack compilers (M9) and DML parser tooling (M10), provided schemas use the
  same version registry;
- Windows and Linux host adapters after the host trace suite is normative;
- independent control families after property metadata, event order, layout,
  text, style, and semantic extension points are stable.

Do not parallelize competing definitions of ownership, handle lifetime, event
order, DML identity, text positions, or layout observability.

## 7. Verification system

### 7.1 Test layers

1. Unit: geometry, damage, ownership, handles, parser, formatter, style, text
   indices, algorithms.
2. Property/model: random tree mutation, event subscription, layout invariants,
   Unicode editing, pack/schema round trip.
3. Headless conformance: deterministic clock, input/event/focus traces,
   semantic snapshots, layout geometry, display chunks.
4. Raster golden: scale/profile-specific approved images plus semantic pixel
   probes; use perceptual tolerance only where explicitly justified.
5. Platform integration: native input/IME/accessibility/dialog/drag/drop/monitor
   behavior.
6. Compatibility oracle: selected WinForms/Modern.Forms/SDR behavior traces,
   documenting accepted corrections.
7. Stress/soak: large trees, virtual collections, event storms, theme/language
   changes, repeated plugin subtree load/unload, memory pressure, sleep/wake,
   display changes.
8. Security: fuzz every untrusted byte boundary; sanitizer and fault-injection
   runs; resource quotas and error-path cleanup.
9. Packaging: clean-machine install, launch, upgrade, downgrade rejection,
   repair, uninstall, signature, offline build, reproducibility.

### 7.2 Required benchmark records

Budgets are deliberately unfilled until the architect names reference machines.
Every record includes revision, compiler, OS, hardware, power state, scale,
font/profile, cold/warm state, fixture, correctness oracle, allocation counts,
CPU, RSS, linked/dependency size, p50/p95/p99/worst, raw output, and inference
scope.

| Workload | Required measurements |
|---|---|
| cold/warm empty app and gallery launch | process-to-window, first correct paint, RSS, bytes read, dynamic loads |
| 10k-property transaction on 1k controls | setter cost, layout passes, chunk rebuilds, damage, allocations |
| 100k retained nodes / deep and wide trees | construction, lookup, reparent/dispose, RSS, stack safety |
| 1/10/100% window damage at 1x/2x/3x | raster/present percentiles, copied pixels, CPU, allocation |
| one-million-item list/tree/grid | realized objects, scroll latency, selection/search, semantic children, RSS |
| text corpus and long document | shaping/edit latency, cache hit/miss, line reflow, IME latency, RSS |
| 30/60 Hz isolated bitmap band | input latency, frame misses, damage, CPU, surrounding repaint count |
| theme/language switch | parse/load, one transaction, layout/paint work, fallback, memory peak |
| repeated C#/plugin subtree load/unload | handles, callbacks, timers, resources, residual RSS, fault cleanup |
| idle for 10 minutes | wakeups, CPU time, paints/presents, timers, RSS drift |

No framework-wide “fast” claim follows from one benchmark lane.

## 8. Security and robustness gates

- Treat DML, packs, PNG, fonts, configuration, clipboard, drag/drop, C ABI
  inputs, and plugin messages as untrusted byte/identity boundaries.
- Apply checked arithmetic and explicit limits to counts, dimensions, strides,
  decompression, nesting, recursion, text length, path complexity, display
  chunks, timers, queued callbacks, and semantic nodes.
- Require deterministic cleanup after partial construction and callback failure.
- Prevent stale handles, use-after-dispose, event cycles, queued work after
  teardown, cross-thread mutation, and application exceptions crossing ABI.
- Keep built-in theme/language/recovery resources independent of external packs.
- Never let a theme, language pack, backplane, or plugin replace controls or
  intercept core input without a separately granted capability.
- Audit the pinned Skia patch and decoder symbol policy on every dependency bump;
  fuzz the exact shipped revision.
- Crash reports and traces must omit user text/resources unless an application
  explicitly supplies a redacted diagnostic payload.

## 9. Packaging, versioning, and compatibility policy

- Version independently: DML source schema, compiled schema, C ABI, host
  protocol, renderer vocabulary, theme pack, language pack, font/metric pack,
  configuration, and semantic snapshot. Do not use one integer to disguise
  incompatible lifecycles.
- Semantic-version public packages only after the corresponding contract has a
  compatibility suite. Experimental 0.x artifacts make no binary promise.
- Generate an ABI symbol and layout report in CI; reject unintended exports.
- Pin and hash third-party sources and tools. Record patches and negative
  results. Generate license inventory/SBOM for each package.
- Package the CPU renderer privately. Applications cannot replace it by
  dropping an arbitrary library beside the executable unless a future signed
  renderer-plugin ADR admits that power.
- Preserve migrations and downgrade behavior for DML/packs/configuration. A
  newer incompatible artifact fails with a recovery path; it is not guessed.

## 10. Principal risks and controls

| Risk | Failure | Control |
|---|---|---|
| framework swallows product schedule | years of widgets before dogfood | vertical laboratories; Matrix P0/P1 first; File Manager consumes stable slices |
| proving demo becomes architecture by inertia | demo-private switches and `shared_ptr` leak into ABI | M1 extraction; ABI 0.x gate; no public freeze before two consumers |
| Skia becomes framework model | renderer types/resources spread upward | display-chunk vocabulary; include/symbol audits; comparison gate |
| custom text fails real users | broken clusters, IME, bidi, accessibility | typed positions from day one; staged corpus; platform IME tests |
| owned fonts only appear safer | pinned fonts still exercise vulnerable parser/shaper code or omit filename scripts | minimal HarfBuzz/FreeType module inventory; bounded signed packs; exact-build fuzzing; declared coverage; isolated untrusted-font preview |
| portability is false abstraction | AppKit assumptions encoded as common truth | headless trace protocol; platform capability records; early Windows/Linux spikes |
| “compatibility” becomes unbounded | HWND/ActiveX/browser/bug emulation | declared corpus; matrix status; behavioral contracts; explicit exclusions |
| designer freezes bad ontology | DML cannot evolve or round-trip | typed IR before GUI designer; schema versions/migrations; unknown-field policy |
| optional semantics rot | inaccessible custom controls and untestable UI | stock adapters; semantic snapshots; platform publishers; matrix gates |
| resource packs become code execution | themes/languages obtain authority | data-only bounded VM/formatter; signatures/hashes; fuzzing; built-in fallback |
| CPU-only richness causes stalls | full-window effects and high-DPI redraw | independent planes, tiles/chunks, bounded effects, damage benchmarks |
| C# bridge leaks/cycles | native objects/callbacks survive plugin unload | tokenized weak events, SafeHandle, disposal traces, fault injection |

## 11. Governance and stop/report rules

Every implementation thread must:

1. read repository and GUI.Forms `AGENTS.md` plus the relevant planning files;
2. state which facts are GIVEN/OBSERVED/MEASURED/CANDIDATE;
3. inspect the dirty worktree and preserve unrelated/user changes;
4. name owned paths before editing and coordinate shared build files;
5. keep the lower build independent and green;
6. add a correctness oracle before a performance claim;
7. preserve failed experiments and exact commands/environment;
8. stop and report when work requires an unresolved high-reversal ADR, broadens
   plugin authority, changes an explicit exclusion, requires an unavailable
   proprietary asset, or cannot preserve existing behavior without destructive
   migration.

## 12. Recommended first sibling assignment

Do **M1a: reusable lifetime/event kernel and deterministic headless traces**.
Do not begin with more painted controls, a C# facade, a DML parser, or a Windows
port. Every later subsystem depends on correct lifetime, event revocation,
thread affinity, and deterministic conformance tests; these requirements are
already GIVEN or directly OBSERVED and do not require selecting a renderer,
text stack, menu policy, or binary DML format.

The exact paste-ready instruction is in `SIBLING_THREAD_HANDOFF.md`. Its exit is
a green existing gallery plus new headless tests for component ownership,
visual reparenting, tokenized subscriptions, deterministic disposal, UI-thread
violations, focus/capture revocation, handler-driven disposal, and zero work at
idle. It must not freeze the C ABI or replace the Skia/AppKit adapters.

## 13. Source locators

- `../../AGENTS.md`
- `../AGENTS.md`
- `../README.md`
- `../PROVING_SLICE.md`
- `../../planning/SURFACE_PIPELINE.md`
- `../../planning/gui_forms/GUI_FORMS_BACKEND_DECISION_MAP.md`
- `../../planning/gui_forms/GUI_FORMS_INTERVIEW_LEDGER.md`
- `../../planning/gui_forms/GUI_FORMS_RESOURCES_AND_CONFIGURATION.md`
- `../../planning/gui_forms/GUI_FORMS_ACCESSIBILITY_AND_TEXT_ROUND_001.md`
- `FILE_MANAGER_CONSUMER_CAPABILITY_PROFILE.md`
- `../../planning/gui_forms/WINFORMS_CONTROL_INVENTORY.md`
- `../../planning/gui_forms/retired compatibility specimen_COMPATIBILITY_INVENTORY.md`
- `../third_party/SKIA_BUILD_EVIDENCE.md`
