# Orchestrator ↔ GUI.Forms interface negotiation

## Details semantic item focus correction — 2026-10-03

The existing Details input contract now explicitly requires an item-addressed
semantic action to leave internal header focus, including when the selected
identity is unchanged. The correction changes no public type or signature.
The retained Mac ordinary-action failure and focused regression are recorded in
`../../frontend/results/2026-10-03-ordinary-local-actions/README.md`; native
acceptance of the correction remains pending. This is a correction to the
existing in-process control behavior, not a new capability or stable ABI.

## Details and title/menu source reconciliation — 2026-10-02

The bounded source candidates are registered in the canonical
[Details contract](../../orchestrator/spec/contracts/GUI_OBJECT_DETAILS_DEVELOPMENT.md)
and [title/menu contract](../../orchestrator/spec/contracts/GUI_TITLE_MENU_DEVELOPMENT.md).
Provider evidence is in [Details development](OBJECT_VIEW_DETAILS_DEVELOPMENT_001.md)
and [title/menu development](TITLE_MENU_DEVELOPMENT_2026-10-02.md).
Named source review and local tests passed; native CI and matching installed SDK
consumption remain pending. C++ layout/signature changes require coherent
consumer rebuilds. No stable ABI or runtime availability promotion is claimed.

## SwiftEdit staged document-view proposal — 2026-10-01

The concrete additive provider proposal is
[Document-view provider proposal](DOCUMENT_VIEW_PROVIDER_PROPOSAL_2026-10-01.md).
**OBSERVED:** D1 semantics are reconciled for bounded development implementation
in the canonical
[Orchestrator D1 contract](../../orchestrator/spec/contracts/GUI_DOCUMENT_VIEW_DEVELOPMENT.md).
D1 covers identity/revision, page publication, exact source/display mapping and
viewport requests; selection/editing is deferred. Later retained control/cluster layout and independent print
stages remain separately gated. Existing TextBox limits and frozen SDKs are
unchanged; the proposal itself advertises no implemented capability.

Status: **round 001 GUI.Forms reply published; Orchestrator reconciliation open**.

Participants: Orchestrator integration authority, GUI.Forms provider, and File
Manager consumer. Canonical families: `ORC-COM-001`, `ORC-GUI-001`,
`ORC-FE-001`.

ADR-006 makes GUI.Forms one of two independent technical predecessors for
Frontend 001. Orchestrator advances headlessly to Core 1.0 while GUI.Forms
advances to FM0. The grand architect starts the frontend after both are ready;
this note is the provider-owned place for the GUI.Forms reply.

Orchestrator has no GUI and does not call controls remotely. This negotiation
registers the ABI File Manager consumes and the capability facts Orchestrator
may report.

## Orchestrator proposal 001

GUI.Forms should publish a machine-readable experimental consumption manifest
containing:

```text
abi family and version; target triple; exported symbol/table identity;
thread/lifecycle rules; host protocol version; implemented capabilities;
explicitly unsupported capabilities; fixture/trace digests;
build/source manifest; compatibility horizon
```

The manifest must distinguish construction/API presence from behavioral
support. File Manager negotiates a minimum set; Orchestrator records compatible,
degraded, or unavailable state. Orchestrator never receives GUI handles,
callbacks, native windows, or renderer objects.

### First requested capability groups

1. runtime/table negotiation and deterministic disposal;
2. UI-thread identity and queued dispatch;
3. retained control tree, initialization, and lifecycle;
4. invalidation/layout/paint transaction behavior;
5. pointer, keyboard, focus, committed text, and cancellation-safe close;
6. host capability discovery rather than simulated support;
7. headless trace projection for frontend fakes;
8. explicit control-family support matrix.

### Required fixtures

- compatible and incompatible ABI-table negotiation;
- stale/wrong-kind/disposed/wrong-thread handles;
- nested initialization and failed attachment rollback;
- dispatch during shutdown;
- unsupported host capability;
- byte-stable headless lifecycle/input trace.

## Frontend 001 go-ahead request

GUI.Forms should reply with one named consumption snapshot and state either
`ready` or `not ready` for each item in
[`../../frontend/planning/FRONTEND_001.md`](../../frontend/planning/FRONTEND_001.md):

- public build/install target and exact C ABI/C++ wrapper identity;
- application/window/tree/UI-thread/ownership/shutdown lifecycle;
- layout, resize, DPI, damage, input, focus, and keyboard behavior;
- compositional primitives for the shelf, path/search row, collapsible panes,
  object field, status, and overlay;
- admitted 001 text display/editing behavior;
- PNG/resources, semantic naming, theme/state, and headless traces;
- unsupported 001 behavior that the frontend must omit or visibly mark.

The reply need not claim framework completeness. It must identify the stable
public seam Frontend 001 may consume without reaching into private GUI.Forms
sources.

## GUI.Forms reply 001 request

Status: **answered by GUI.Forms reply 001 below**.

Please provide:

- the proposed manifest location and schema counterproposal;
- which ABI 0.6 behaviors are ready to name in a consumption snapshot;
- authoritative test/trace locators for each requested group;
- unsupported groups that must block the first File Manager slice;
- versioning and table-extension constraints;
- any request that would leak platform/backend state and must be rejected.

## GUI.Forms reply 001 — named FM0 consumption snapshot

Date: 2026-08-10.

Status: **ready for Frontend 001 on macOS arm64 within the named snapshot**.

The provider manifest is
`../manifests/gui-forms-fm0-macos-arm64-2026-08-10.json`. The stable public
consumer seam is the installed CMake package `GUIForms 0.1`, especially
`GUIForms::Application` for the native AppKit/Skia runtime and
`GUIForms::Controls` for independently testable model consumers. Public host
entry points are `gui_forms::host::run_macos` and
`gui_forms::host::run_macos_application`. The retained C ABI remains
`gui_forms_abi0` with additive experimental tables through 0.26; Frontend 001
does not need to substitute the C table for its public C++ application target.

**MEASURED:** on the named M4, a clean external project used only
`find_package(GUIForms 0.1 CONFIG)`, linked `GUIForms::Application`, found the
installed font payload through `GUIForms_FONT_DIR`, and successfully executed
its model-only construction probe. The installed dylib is arm64 Mach-O and
exports the public Window and macOS application entry points. The manifest
records its SHA-256 digest and the exact commands.

The snapshot admits the retained tree, layout/paint, application/window
lifecycle, public File Manager control primitives, input/focus/commands,
popups, headless conformance, deterministic fonts, CPU Skia host, and
fail-closed Web.Forms generated public C++ tree. Full VoiceOver campaign and
browser/native raster equivalence remain `degraded`, not silently complete.
Signed packaging, other native host platforms, outbound drag source, and any
runtime web stack remain unavailable.

Thread rule: the AppKit host owns the main UI thread. Provider/service calls do
not enter GUI.Forms. Frontend workers post immutable application snapshots to
their own UI dispatch boundary. Native handles and renderer objects never
cross Orchestrator.

## Orchestrator reconciliation 001

Status: **provider reply complete; Orchestrator may project
`gui_forms.consumption_manifest=available` with the exact manifest id**.

The reply preserves `ORC-GUI-001` authority: Orchestrator mirrors availability
and manifest identity but receives no GUI handle, callback, native window, or
renderer state. Any target mismatch or absent required capability fails the
frontend build/opening check. Later FM1/FM2/FMX rows and proposal 002 remain
independent negotiations.

## Orchestrator proposal 002 — first-party application surfaces

Date: 2026-08-06.

Status: **proposal; no ABI or implementation priority frozen**.

The grand architect has introduced File Manager's reusable Document Picker,
future Paint and Text Editor consumers, a shared Orchestrator administration
surface, application-local help, and File Manager → Paint compositional drag.
Canonical proposal:
[`../../orchestrator/proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md`](../../orchestrator/proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md).

GUI.Forms is asked to report capability and counterpropose on these generic
mechanics only; it does not gain file-browser, settings, help-content, Paint, or
Orchestrator policy.

### Requested GUI.Forms capability groups

1. **Application-owned modal surface:** an arbitrary retained view may run as an
   owned modal window or platform-appropriate sheet with owner suppression,
   exactly one terminal result, Escape/cancel, shutdown handling, and focus
   restoration. A queued/asynchronous modal session must exist even if a
   Forms-shaped synchronous `ShowDialog` convenience wraps it.
2. **Portable presentation route:** a shared File Manager picker composition can
   use only public C++/C ABI controls and host services. It must not require a
   native file-dialog widget or platform handle in portable state.
3. **Native fallback identity:** existing `OpenFileDialog`, `SaveFileDialog`,
   and folder adapters report which route completed, cancellation preservation,
   owner identity, and supported filter/access-grant semantics.
4. **Outbound transfer:** complement the measured inbound drag destination with
   drag-source initiation, typed file references, bounded media bytes,
   lazy/promised data, cancellation/expiry, clipboard formats, and
   cross-participating-window behavior.
5. **Help mechanics:** **MEASURED PARTIAL M12-P4:** Forms-shaped
   `HelpProvider` attachment, help strings/keywords/navigators, automatic and
   explicit ShowHelp, focus-ancestor F1 routing, control-first/provider-second
   handled events, and semantic descriptions are public. The provider emits
   intent and never opens an external resource. Stable TopicId/context-ID
   registry, Orchestrator capability routing, greaseboard anchors, described-by
   relations, and native publisher verification remain to negotiate without
   coupling help availability to accessibility publication.
6. **Inspection:** consumption manifests and headless traces name modal, native
   dialog, help, inbound/outbound transfer, lazy payload, and clipboard
   capability independently.

### Required fixtures

- nested owned popup → custom modal → focus restore;
- accept, cancel, duplicate-terminal rejection, owner close, and host shutdown;
- headless custom picker result distinct from native common-dialog fallback;
- application-modal versus window-modal participation on each host;
- outbound file-reference drag to synthetic Paint target;
- lazy alpha-clipart success, cancellation, expiry, timeout, and oversized
  rejection;
- F1/context help, missing topic, disposed owner, overlay toggle, and proof that
  disabling help leaves native accessibility intact;
- capability manifest distinguishing unsupported Linux/platform adapters from
  empty success.

### GUI.Forms reply 002

Status: **awaiting project reply**.

Please identify which requirements are already covered by M11h evidence, which
need additive public projection only, which require new implementation, and any
request that would improperly place application policy in GUI.Forms.

## GUI.Forms implementation fact M12-P5 — local binding projection

Status: **MEASURED; not yet an ABI proposal**.

GUI.Forms now has a renderer-neutral retained binding/currency kernel for local
application models. It deliberately accepts explicit typed property
descriptors and stable `BindingRecord` projections, not arbitrary managed
objects or Orchestrator-owned records. Orchestrator remains the authority for
cross-project contract meaning; a future frontend adapter may project a
negotiated snapshot into this local model without giving GUI.Forms service,
persistence, or capability-policy ownership.

Before this becomes a consumption-manifest capability, GUI.Forms still owes a
versioned C ABI table for value/record/binding handles, bounded string/value
transport, callback cleanup, source replacement, and headless trace identity.
Nested object reflection is explicitly not required of the native ABI. Current
C++ evidence is recorded in
`../experiments/M12P5_BINDING_CURRENCY_KERNEL.md`.

## GUI.Forms implementation fact M12-P6 — validation and local errors

Status: **MEASURED; not yet an ABI proposal**.

GUI.Forms now owns deterministic retained focus validation and a local
BindingSource/ErrorProvider error projection. This is presentation-local
behavior: it does not make GUI.Forms authoritative for Orchestrator service
validity, capability policy, or persisted diagnostic records. A future ABI may
carry bounded error snapshots into `BindingRecord`, but the producer remains
the authority and arbitrary managed-object reflection is not part of the
native contract. Evidence:
`../experiments/M12P6_VALIDATION_AND_BOUND_ERRORS.md`.

## GUI.Forms implementation fact M12-P7 — local dialog commands

Status: **MEASURED; not yet an ABI proposal**.

GUI.Forms now owns renderer-neutral mnemonic parsing/routing and retained
accept/cancel command roles. These are local presentation commands, not
Orchestrator capabilities: they do not authorize work, close a service session,
or select a cross-project action by themselves. A future ABI projection may
carry stable command identity and handled/result state, but Orchestrator remains
the authority for the meaning and availability of the invoked operation.
Evidence: `../experiments/M12P7_DIALOG_KEYS_AND_MNEMONICS.md`.

## GUI.Forms implementation fact M12-P8 — deterministic command arbitration

Status: **MEASURED; not yet an ABI proposal**.

GUI.Forms now precomputes mnemonic candidates before callbacks, cycles
duplicate winners, retains exact DialogResult state, and routes MenuStrip and
popup mnemonics through local shared Command authority. These remain
presentation facts. A non-None local result does not authorize an Orchestrator
operation, and native modal closure is still owned by the presenting host.
Evidence:
`../experiments/M12P8_COMMAND_ARBITRATION_AND_NATIVE_CALLBACKS.md`.

## Shadow Windows development consumption — 2026-09-29

Status: **MEASURED Windows development build/install and 64/64 native tests passed**.

The current owner directed shared-toolchain Windows compilation and reuse of
Plan Paint's mature GUI.Forms behavior while preserving File Manager styling.
The hash-verified source and six-patch provenance is in
`../manifests/plan-paint-backport-2026-09-29.json`; implementation scope is in
`PLAN_PAINT_BACKPORT_2026-09-29.md`.

The development installed package exposes the existing portable targets plus
GUIForms::Application on Windows MinGW, with host protocol 7. This does not
replace the frozen historical macOS FM0 artifact or promote Windows release
readiness. Orchestrator reconciliation records a separate development entry.

File Manager additionally requires portable wake publication and UI-thread
pending-result dispatch. ApplicationWindowOptions now names those callbacks,
contains callback exceptions, and specifies worker shutdown before window
closure. Native tests cover worker wake and dispatch exception containment.
These are local presentation mechanics, not permission to call services or
change cross-process scheduling/authority. macOS custom titlebar controls remain
adapter-specific; no unsupported portable chrome capability is advertised.

## Daily browsing typography and breadcrumb presentation — 2026-09-29

Status: **GIVEN consumer need; development C++ projection, not a frozen ABI**.

The owner requests a practical File Manager interface closer to its prototype,
including materially rendered chevron segments. File Manager requests public
font metrics for MenuStrip, BreadcrumbTrail and PropertyList, plus PropertyList
row height. These are retained presentation values, not domain/service policy.
Changes must invalidate layout and semantic hit geometry, preserve active editor
values, and use the same text metrics for rendering and hit testing.

BreadcrumbTrail additionally offers an opt-in raised appearance; plain remains
the default. Rich faces retain stable segment identities, same-row editing,
overflow, semantic activation and keyboard navigation. Pointer ownership follows
the visible chevron nose instead of an overlapping rectangular approximation.
This API is consumed through the development Windows SDK. Cross-platform source
availability does not replace native platform evidence.

## Plain-text multiline TextBox development extension — 2026-09-29

Status: **GIVEN** current owner direction to build a traditional Notepad using
GUI.Forms. **OBSERVED** additive development C++ implementation; not a stable
C ABI addition or final large-document architecture decision.

The provider adds `TextBox::set_multiline(bool)`, `set_word_wrap(bool)`,
`set_newline_sequence(std::string)` and `set_accepts_tab(bool)`, with corresponding
getters. Existing controls retain single-line defaults. Multiline/word-wrap/Tab
insertion default false; the insertion newline defaults LF and accepts exactly
LF, CR, or CRLF. Stored UTF-8 and existing line endings are preserved exactly.
The existing font, selection, clipboard, text/selection events, read-only,
maximum-length, and undo/redo APIs remain the common edit path.

Multiline behavior includes visual-row Up/Down/Home/End/PageUp/PageDown,
Ctrl+Home/End document navigation, Shift selection, two-dimensional hit testing,
retained caret reveal, three-row wheel scrolling, and wrapping at spaces/tabs
with grapheme fallback. Four-space tab stops are presentation only. Soft-wrap
caret affinity retains the end of the preceding visual row when appropriate.
Font metrics are resolved through the attached public TextMetricsProvider;
visual-row geometry is cached by document revision, effective font, width,
device scale and provider. The follow-up device-scale key corrects in-place
provider DPI changes; its private C++ layout change requires a coordinated
consumer rebuild before the next SDK publication. Paint emits only visible
rows and retains complete shaping runs
between tabs instead of cutting arbitrary chunks through joining text.

**CANDIDATE development limits, not accepted final Notepad semantics:**
`maximum_multiline_bytes` is 1 MiB; `maximum_multiline_line_bytes` is 4096 UTF-8
bytes excluding the logical line terminator. Static `validate_multiline_text`
returns `MultilineValidation::{valid, invalid_utf8, document_too_large,
line_too_long}` before a consumer replaces its current document. Oversized or
invalid `set_text` throws without changing text/selection/history; edits/paste
return false without truncation. Consumers must surface a truthful refusal or
separate read-only route and preserve their current document. Password masking
and multiline mode are mutually exclusive. Existing undo history remains
bounded to 128 snapshots and 8 MiB in total.

The current metric seam provides whole-text width and font metrics, not shaping
cluster caret positions. Cold prefix measurement is quadratic within the
4096-byte logical-line bound; repeated paints reuse geometry. Removing that
provisional bound requires a negotiated cluster-position/line-layout metric
extension and separate storage/history workload evidence. Logical Unicode
grapheme editing is covered; visual bidi caret parity, full IME composition and
accessible multiline text-range parity are not claimed. No native OS edit
control, browser engine, or Notepad-specific state is introduced.

`visual_line_count()` and `scroll_offset()` expose retained presentation facts
for inspection. Focused evidence is in
`../experiments/MULTILINE_TEXT_BOX_2026-09-29.md`. Canonical development registry
and installed SDK publication remain parent-owned coordination steps.

## SwiftEdit successful-save history boundary — 2026-09-30

**GIVEN consumer requirement:** SwiftEdit needs to discard pre-save undo/redo
only after a save succeeds, without replacing text or disturbing document
presentation. Save success and the application's modified state remain consumer
policy. GUI.Forms does not save files or infer whether a document was saved.

The additive development C++ method `TextBox::clear_undo_history()` clears both
history directions and their byte accounting. It follows the existing mutable
UI-thread/lifetime guard, works for single-line and multiline controls including
read-only controls, and is idempotent. It preserves text, selection, viewport,
layout/paint dirtiness and caret state and emits no text/selection events.
Subsequent edits may undo back to this boundary; earlier states are unavailable.
Consumers update their own dirty indicators and command availability following
successful save and the explicit reset.

No private fields, virtual entries or C ABI methods are added. Existing C++
object layout remains unchanged. A consumer calling the new symbol requires a
matching newly built provider library; the installed and staged SDKs are not
updated as part of this source change. Publication and consumer rebuild remain
parent/build-coordinated.

**OBSERVED current capability report, not a broader implementation approval:**

| SwiftEdit requirement | Current public support |
|---|---|
| Clear undo on successful save | New explicit `clear_undo_history()` development source API; consumer owns save policy |
| Grapheme counts | Public `TextStore::grapheme_count()`; no direct cached TextBox count getter. Constructing TextStore from TextBox text copies/analyzes it |
| Intelligent wrapping | Space/tab wrap with grapheme fallback; no general Unicode line-break or language-aware wrapping contract |
| Visible draggable scrollbars | Generic ScrollBar control exists; TextBox itself exposes only wheel/caret scrolling and offset inspection, without scrollbar integration or a public scroll setter |
| Visible Unicode controls | No show-invisibles or control-character visualization API |
| Discontiguous selection | Not supported; one UTF-8 anchor/caret range |
| Editable documents below 16 MiB; paged read-only at or above 16 MiB | Not supported by TextBox. The 1 MiB/4096-byte-line multiline limits apply in read-only mode too; contiguous storage and whole-text history are not a paged document provider |

The size, virtual document, scrollbar, visualization and selection requirements
need separately scoped provider contracts and evidence. They are not satisfied
by silently lifting the current limits or moving a private editor into SwiftEdit.

**MEASURED:** native Windows Release focused build with two jobs succeeded;
`gui_forms_input_controls_tests` and `gui_forms_multiline_text_box_tests` passed
2/2 in 0.38 seconds. The new history-boundary fixture starts with both history
directions populated, verifies read-only/idempotent clearing with no text,
selection, viewport, dirtiness, metric or event changes, then proves new edits
undo exactly to the boundary and redo normally. A single-line case preserves
its value too. No SDK install or staged/application DLL replacement occurred.

### Coordinated Windows development SDK publication

**MEASURED, 2026-09-30:** the parent subsequently built the complete Windows
Release provider from source `dbe3766b3c8c6ded939fdbb791694d30262cd20f` and ran
all 65 configured toolkit CTests successfully. It installed a separate SDK at
`.build/sdk-checkpoints/dbe3766/windows-x64/gui-forms-sdk` under the workspace
root. The File Manager picker was rebuilt into the sibling `picker-sdk` prefix;
its controller/view tests passed 2/2. Independent installed-only consumers
passed for the picker (1/1) and for calling `clear_undo_history()` through
`GUIForms::Application` (1/1). This establishes the new linked symbol as well
as source-level behavior; SwiftEdit application acceptance remains consumer work.

Checkpoint hashes:

- Application DLL: `45cb7bb9267686f5e93333952f691b55f2c6db0aa80d256ef9df3df2ab8c12ea`.
- TextBox header: `4d6f98b2af0b16c40f8386dd06b98c99e50ee625beeb1e5c98f54f7ea2facb80`.
- Combined GUI/picker fingerprint using SwiftEdit's `Build-Windows.ps1`
  algorithm at the supplied absolute prefixes:
  `cd8f39da910556e56b401f783727bb4370bb363ebae2f8e60b7c850908ceb654`.

The local `sdk-receipt.json`, preserved toolkit test log and independent
consumer build/test directories live beside those two prefixes. Both prefixes
are frozen for the consumer handoff. The previous `shadow-sdk` DLL remains
`3c66e1c188a183eff2c93833c7c9a3781de57936a14721f00fa9ddbc865c664f`;
existing application stages and the published File Manager release were not
replaced. This is a Windows development SDK publication, not a stable ABI or
new three-platform application release.

### Opaque live-surface development promise (2026-10-06)

**GIVEN:** the owner explicitly requests `LiveSurfaceDescription::opaque`,
default false, as the producer's alpha-255 promise for copy composition and
occluded-background omission. This extends the existing ORC-GUI-001 development
C++ live-surface consumption projection; it does not change a stable C ABI.
`LiveSurfaceFrame::opaque()` retains the promise with the frame's immutable
buffer so reconfiguration cannot change an in-flight renderer's interpretation.
Consumers must rebuild against the matching library and explicitly opt in.
The field and host guards are documented in `CURRENT_API_REFERENCE.md`.

Source review against `planning/PROGRAMMING_HOUSE_STYLE.md` covers the new
description field and frame accessor; Skia's two live draw methods and internal
lease overload declaration; the new `live_surface_damage.hpp`; Mac live-frame
storage, recovery invalidation, draw exception cleanup and changed
`drawRetainedRect:` blocks; both `opaque_live_surface*_tests.cpp` files; and
their CMake registration. Types, initialization, conversion bounds, named
behavior, immutable lease ownership, synchronous raster borrows, failure
cleanup and reusable per-view storage were reviewed. Coverage uses a rectangle
sweep, not a per-pixel scan or per-frame scratch allocation. No violations were
identified in that changed scope. The rest of the legacy Mac/Skia sources and
vendored dependencies are not claimed compliant. The six complete small
source/header/test files also passed the spelling scanner; that is not a
substitute for this review.

The independent review caught an image-allocation failure that could have
published an incomplete prepared candidate after retained paint was skipped.
The internal frame-draw seam now reports success: failed live compositions are
not presented or committed, and pending presentations retry on later display
ticks even without a new generation. Reconfigured surfaces with no current
frame grant no coverage and leave the ordinary retained background visible.
The raster test includes a private image-factory refusal and, when the prepared
profile is enabled, verifies that abort retains the complete previous front
and receipt after a partial candidate draw. Review corrections also made value
parameters const, initialized SkPaint explicitly and marked test factories and
pixel readers nodiscard. This is a scoped correction, not a legacy style claim.

The review also required failed-batch retries to refresh current placements.
`Window::take_live_surface_presentations(bool include_unchanged = false)`
preserves the usual changed-generation drain, while true recomputes all eligible
placements with current visibility/overlay rules. This avoids dropping failed
surface A when only surface B advances, and avoids replaying cached clips over
new overlays or hidden controls. The existing declaration and drain-condition
change are additional reviewed scope; the core fixture exercises two surfaces,
an unchanged failed generation, hiding and full overlay coverage.
The independent follow-up review closed those failure-path and style findings.
The updated core live-surface and opaque-surface fixtures pass 2/2 on Shadow
Windows; native raster/host verification remains a separate build gate.

### Native-order live-surface development formats (2026-10-06)

**GIVEN:** the owner requests RGBA32 alongside the existing default BGRA32,
an attachment-independent native-format query, and pixel-exact opaque 1:1
presentation. This extends the same ORC-GUI-001 development C++ projection.
The existing immutable buffer description already owns each frame's format;
format changes replace the pool, refuse active writers, and cannot relabel an
outstanding read lease. Consumers rebuild matching headers and libraries.

**OBSERVED:** `native_live_surface_pixel_format()` reports RGBA for macOS/Linux
Skia hosts and BGRA for Windows DIB. Skia resolves device scale and translation,
uses clipped `writePixels` for opaque/full-opacity 1:1 integer geometry with a
full-coverage rectangular clip, and uses nearest/source-copy for that geometry
under complex clips. Scaled/fractionally translated draws remain linear;
translucent/reduced-opacity draws retain source-over. The pinned raster Skia
implementation's `isClipRect` excludes partial-coverage AA edges and clip
shaders. Windows converts RGBA into reusable host-owned BGRA scratch before DC
mutation; BGRA still reads directly from the lease through the existing
`memcpy` / `StretchDIBits` realization. Retry, coverage, and commit authorities
are unchanged. See `CURRENT_API_REFERENCE.md` for the producer contract.

Source review against `planning/PROGRAMMING_HOUSE_STYLE.md` covers the enum and
query declaration/definition, format validation condition, Skia geometry helper
and live-frame draw changes, Windows conversion and scratch owner, added cases
in both opaque fixtures and the Windows DIB lifecycle fixture, and the new
benchmark/CMake target. Review checked explicit types, named execution,
initialization, integer bounds, frame borrows, clipping before raw writes,
format selection outside pixel loops, reusable scratch, allocation before DC
mutation, and failure propagation. No violations were identified in this changed
scope; legacy/vendored source outside it is not claimed compliant. The complete
small header, opaque fixtures, and benchmark pass the spelling scanner.

**MEASURED locally:** macOS 26.5 (25F71), Apple M4 arm64, Apple Clang 21.0.0,
Release build, pinned Skia `2a9b593bab4b2fd019fa494c8d401ff1fab0b883`.
The default native GUI.Forms suite passes 85/85. The prepared-text raster,
Skia, display, and opaque rollback fixtures also pass 4/4 with both development
profiles enabled. Windows/Linux validation is delegated to the native CI matrix.
The benchmark target
`gui_forms_live_surface_benchmark` measures an opaque 1060x618 immutable lease
at scale 1, full rectangular clip, full opacity, without retained painting or
CoreGraphics presentation. Each result is the median of nine batches of 300
synchronous draws after warmup; new formats alternate batch order.

| Raster implementation / format | Median ms/draw | Batch minimum–maximum |
|---|---:|---:|
| Main `63e7128` raster, BGRA, before new run | 0.160917 | 0.159368–0.166966 |
| Updated raster, BGRA | 0.040145 | 0.039531–0.040414 |
| Updated raster, native RGBA | 0.028603 | 0.028160–0.028836 |
| Main `63e7128` raster, BGRA, after new run | 0.159955 | 0.158378–0.163542 |

The baseline substitutes only `63e7128`'s original `skia_raster.cpp` object in a
copy of the same renderer archive, with identical compiler options and pinned
dependencies; the benchmark runs `--bgra-only`. Core lease code and the harness
are shared. Builds/tests were idle during these recorded runs. Reproduce the
updated comparison with `cmake --build <build> --target
gui_forms_live_surface_benchmark` then `<build>/gui_forms_live_surface_benchmark`.
These are draw-only measurements, not PlaySuite/Stillwater frame timings or a
claim about the complete native presentation pipeline.
