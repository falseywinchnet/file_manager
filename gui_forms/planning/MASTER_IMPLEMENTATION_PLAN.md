# GUI.Forms master implementation program

Status: **execution-ready candidate program; not an accepted architecture
decision**. Date: 2026-08-03.

This document decomposes the route from the current proving slice to a usable,
portable GUI.Forms 1.0. It does not override the repository's pre-architecture
rules. Items marked **GIVEN** may constrain implementation. Items marked
**CANDIDATE** require the named gate and, where reversal cost is high, a numbered
ADR with grand-architect approval before they become **DECIDED**.

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
  C++, C#, DML, control, plugin, or test seam.
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
- DML is authoritative and round-trippable; generated code is disposable.
  Production retains compiled schema, stable IDs, and inspection metadata.
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
  incompatibilities; alternate historical orders are not supported. DML
  preserves authored initialization assignment order (ADR-003).
- Accessibility, tooltips, guidance, automation, and AI-readable inspection use
  an optional semantic-hook graph. Controls can exist without hook metadata.
- Ordinary controls use GUI.Forms-authored material recipes and named
  foreground/background roles. A style-only backplane may be composited below
  controls but cannot intercept input or impersonate controls.
- Ordinary text uses platform `system-ui`. Control labels prefer the supplied
  Latin-only Portmouth Rapids face and fall back per uncovered cluster.
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
- There is no stable C ABI, C# binding, component model, reusable event-token
  API, resource pack, language catalogue, theme compiler, configuration
  registry, accessibility publisher, Windows host, Linux host, packaging, or
  compatibility runner.

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
2. The retained kernel, DML schema, C ABI, C++ wrapper, renderer vocabulary,
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
5. DML parses, validates, compiles reproducibly, round-trips losslessly for its
   authored subset, emits native bindings, and can be inspected without running
   application code.
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
authoritative DML source ----> parser / validator / typed IR
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

### 4.2 Host boundary

**CANDIDATE H6, recommended:** native OS event loops implement a narrow
GUI.Forms-owned service protocol. The protocol covers lifecycle, scheduled
wakeup/dispatch, monitors/scaling/coordinates/occlusion, normalized input,
cursor/capture, IME sessions and candidate geometry, clipboard, typed drag/drop,
dialogs/menus, font discovery/fallback, optional accessibility publication,
session/power/display changes, and native surface handles required for isolated
composition. The deterministic headless implementation is the behavioral
reference.

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

**CANDIDATE T-HB, recommended for test:** HarfBuzz shapes script/language/
direction runs; platform adapters discover fonts and fallback faces; the private
renderer rasterizes positioned glyph runs. Alternatives include platform
shapers behind a common run contract or another portable shaping stack. Gain of
HarfBuzz: cross-platform shaping behavior and cacheable shape plans. Loss:
dependency/Unicode-data policy and possible metric differences from platform
editing conventions.

Portmouth Rapids selection is by typography role and cluster coverage; content
uses `system-ui`. Missing face/assets always resolve to a labelled system
fallback. No missing cluster changes the whole control's family.

Staged delivery is allowed: Latin/dead-key editing first, then grapheme-safe
selection/clipboard/undo, bidi, and representative multi-stage CJK IMEs. The
data model and ABI may not encode Latin-only assumptions at any stage.

**Gate T1:** approve shaping stack after a corpus of Latin, combining marks,
emoji sequences, Arabic/Hebrew bidi, Indic scripts, fallback runs, malformed
UTF-8 rejection, and scale/font changes. **Gate T2:** platform editor tests cover
composition updates, replacement ranges, candidate positioning, commit/cancel,
selection, navigation, clipboard, undo/redo, password policy, multiline, and
accessible text ranges.

### 4.7 Optional semantic hooks, accessibility, help, and inspection

A semantic node attaches by stable control identity and may state role, name,
description, value/state, actions, relationships, logical order, bounds policy,
text ranges, tooltip, and help-overlay anchors. Absence is valid. Platform
publishers, test automation, tooltips, and the greaseboard help overlay consume
the graph independently; disabling one consumer does not erase the graph or
disable another.

The help overlay is a separate per-panel display plane. It may dim, highlight,
draw arrows, and label anchors but does not rebuild the control plane or receive
ordinary application input.

**Gate S1:** headless semantic snapshots plus VoiceOver, UI Automation, and
AT-SPI tests for roles, focus, actions, selection, values, virtual children,
geometry, and text ranges. A release policy may choose accessibility as a
secondary system, but the host and control architecture must not prevent it.

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
decision gates. Portmouth fallback occurs per cluster.

Mutable configuration is separate from immutable packs and high-churn session
state. **CANDIDATE:** a schema-validated flat namespaced textual map with atomic
temp/write/sync/replace and one known-good predecessor; a transactional store
holds session/hive data. Secrets and grants use OS-protected storage.

**Gate RSC1:** fuzz pack manifests, indexes, decompression, PNG dimensions/
stride/color metadata, message programs, and fallback chains. **Gate RSC2:**
prove byte-reproducible compilation and recovery from truncated, corrupt,
incompatible, oversized, and partially translated packs. Decide signatures,
parent-locale chaining, and live replacement in ADRs.

### 4.9 DML compiler, generated code, and designer

The DML toolchain is a compiler project, not a runtime text parser embedded in
every application:

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
10. visual designer using the same compiler APIs and producing DML, never
    treating generated C++/C# as authoritative.

The compiler rejects unknown required controls/properties, duplicate or unstable
IDs, invalid ownership, undeclared invalidation, illegal backplane input,
resource/type mismatch, event signature mismatch, and unsafe limits. Production
does not execute arbitrary expressions from DML; imperative post-construction
logic is a generated-code hook, not a serialized programming language.

**Gate D1:** compile the existing gallery DML and delete the hand-maintained
header only after deterministic output, diagnostics, source maps, and malformed
input tests pass. **Gate D2:** parse-format-parse and designer round trips preserve
semantic IR and authored trivia. **Gate D3:** schema migration supports the two
previous minor versions or gives a deterministic upgrade error and tool.

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
  font discovery, composition geometry, optional VoiceOver publisher, and clean
  shutdown;
- host conformance/event replay suite and platform capability reporting.

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
- **OPEN:** outbound drag-source initiation, owned-form/sheet integration and native nested-modal
  focus restoration, menu policy, font discovery, complete IME geometry,
  VoiceOver publication, Windows/Linux adapters, and session/power events. Gate
  H1 remains open.

### M4 — Unicode text, typography, and editing

Deliver in sub-slices:

1. typed Unicode positions, rope/gap/piece candidate experiment, line index,
   text/style spans, shaping/fallback service, glyph-run cache;
2. Latin/dead-key single-line editor with caret, selection, clipboard, undo;
3. grapheme navigation, multiline layout, password/read-only modes;
4. bidi, representative complex scripts, multi-stage IME, accessible ranges;
5. Portmouth Rapids pack integration and per-cluster system fallback.

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

Exit: gallery is built solely from reusable controls; basic control behavior and
event traces pass through headless, AppKit, C++, and early C ABI lanes.

### M7 — collection, command, modal, data, and advanced controls

Deliver in independent packages/slices:

- virtual TreeView and ListView with item models, columns/groups/checks/editing;
- split and tab containers;
- ToolStrip/MenuStrip/ContextMenuStrip/StatusStrip and hosted item model;
- native common-dialog request/result adapters and message/task dialogs;
- data binding/currency/format-parse pipeline;
- DataGridView model, virtualization, selection, styles, editing hosts, custom
  cells/columns;
- PropertyGrid after descriptor/editor contracts are proven;
- SDR custom-control porting laboratory. GUI.Forms-native docking or map
  equivalents remain separately admitted parent-use candidates, not M7 or retired compatibility specimen
  bridge exit requirements.

Exit: each matrix row has behavior tests; calendar grid edit lifecycle and
SDR-style theme/invalidation/plugin-subtree fixtures pass. Legacy/browser/
printing/MDI families remain explicit exclusions or separately approved work.

### M8 — semantic hooks, accessibility, help, and automation

Deliver:

- optional semantic graph and stable snapshot/inspection protocol;
- tooltip and keyboard-help consumers;
- per-panel greaseboard overlay plane;
- VoiceOver, UI Automation, and AT-SPI publishers;
- semantic virtual children and editable text ranges;
- keyboard-only and assistive-technology conformance suites.

Exit: every supported stock control has a tested default semantic adapter, while
controls still instantiate without metadata. Disabling accessibility does not
disable tooltips/help/inspection.

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

Exit: corrupt/partial packs cannot make the interface unusable; reproducible
pack output and fallback/fuzz suites pass; theme/language switch is one bounded
transaction.

### M10 — DML compiler, code generation, hot reload, and designer foundation

Deliver the ten-stage toolchain in section 4.9. Start by replacing
`demo/gallery_dml.hpp`; then build typed property/event/layout/resource/semantic
coverage and C++ generation against the experimental ABI. C# generation may be
exercised here as a toolchain preview, but it is not distributed or promised
until M11. The designer begins only after round-trip source and schema migrations
are proven.

Exit: all shipped examples originate from DML, generated files are reproducible
and marked, production needs no source parser, and round-trip tests preserve
authored intent.

### M11 — ABI freeze and supported language bridges

Deliver:

- breaking-change audit and migration of the M1 experimental C ABI to the
  proposed 1.0 function tables and symbol allowlist;
- supported thin C++ public wrapper with no exposed `shared_ptr` ownership;
- generated C# facade and control gallery;
- compatibility analyzer, retired compatibility specimen reflection/usage capture, and documented
  deviation manifest;
- package metadata for native and managed consumers.

Exit: two independent native consumers plus the C# gallery pass lifetime,
callback, thread, error, and packaging tests. Freeze 1.0 only after the breaking-
change audit and architect approval.

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
  protocol, renderer vocabulary, theme pack, language pack, configuration, and
  semantic snapshot. Do not use one integer to disguise incompatible lifecycles.
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
- `../../planning/gui_forms/WINFORMS_CONTROL_INVENTORY.md`
- `../../planning/gui_forms/retired compatibility specimen_COMPATIBILITY_INVENTORY.md`
- `../third_party/SKIA_BUILD_EVIDENCE.md`
