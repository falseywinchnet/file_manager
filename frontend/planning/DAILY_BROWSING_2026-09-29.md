# Daily browsing ergonomics — 2026-09-29

Status: **MEASURED native build and automated checks passed; native ergonomic visual verification pending**.

The owner's subsequent severe-latency report is investigated in
[`../results/2026-09-29-shadow-windows/LATENCY.md`](../results/2026-09-29-shadow-windows/LATENCY.md).
The ergonomic tests below do not establish acceptable interactive performance.

The owner asks for a practical real-world File Manager that captures the
prototype semantics and evolves onto Plan Paint's repaired toolkit. This slice
preserves one location, Home/actual drives, the authored command shelf, inline
path editing and embedded inspection. It does not select a new service or
search architecture.

## Observed problems and changes

- **MEASURED baseline:** the repaired Windows application navigates from Home
  to the repository after an accessibility snapshot; breadcrumb, expanded tree
  and 22 listed objects agree. The previous apparent navigation hang is absent.
- **OBSERVED:** tree/object type was 10 px; captions reached 8 px; dynamic menu,
  breadcrumb and property controls hid small fixed fonts behind private state.
  The frontend now requests readable body metrics through public retained
  control APIs. Authored chrome uses 11–13 px with matching target/track sizes.
- **GIVEN owner correction in the frontend chat:** chevrons fail to represent
  the intended rich textured segments. The historical atlas's divider/glyph is
  therefore not a completeness test. A raised BreadcrumbTrail presentation adds
  pearl segment depth, subtle texture, continuous pointed joints, a distinct
  current leaf, and shape-aware hit testing while preserving the inline editor.
  Plain appearance remains the toolkit default for other consumers.
- **OBSERVED:** Modified showed raw filesystem-clock seconds; selection did not
  report selected count/bytes; idle status omitted the exact current path.
  Frontend changes supply local calendar text and factual browsing status.
- **OBSERVED:** Ctrl+L and F5 lacked normal navigation bindings; Alt history/up
  already existed. New bindings preserve text editing and existing navigation.
- **MEASURED baseline:** Alt+F4 did not close the window, while native Close did.
  Windows host code swallowed all WM_SYSKEYDOWN messages. The host now lets
  Alt+F4 reach DefWindowProc and its ordinary cancellable close path.

## Evidence boundary

### Owner visual rejection: rich breadcrumb segments remain open

**GIVEN — subsequent owner correction:** the visible chevrons still do not
represent the prototype's rich textured segments; broader presentation gaps
remain. The raised appearance above is an implementation attempt, not visual
acceptance. Passing interaction, geometry, and paint-command tests does not
close this correction.

**OBSERVED — source audit:** `BreadcrumbTrail::paint_raised` in
`gui_forms/src/controls/panel/breadcrumb_trail/breadcrumb_trail.cpp` uses a
three-stop face gradient, a repeating translucent white stripe on the
rectangular body only, and nine gradient-filled rectangular strips for each
pointed tip. The grain does not cover the tip. Edge depth is represented by
highlight and border lines. These operations establish what is implemented;
they do not establish the requested material appearance at native scale.

**OBSERVED — reference limitation:** `.crumb` in
`frontend/planning/visual/frontend-concept-atlas.html` still specifies a
rectangular divider and a `::after` right-angle quotation glyph. That older
specimen is insufficient to override the owner's richer segment direction.

**OPEN — acceptance:** compare the intended rich segment reference with the
exact staged native build at matching scale, including continuous body/tip
material, joint depth, current-leaf distinction, hover, pressed, focus, and
inline editing. Retain screenshot evidence and rerun the existing paint
latency workload after repair. The command shelf's material/group geography
and object-view focus/selection/full-name presentation also remain open under
FM-R009 and FM-R013; this source audit is not a complete visual inventory.

The root chat owns authored HTML/CSS, breadcrumb rendering, SDK build and native
visual checks. The existing frontend chat owns application logic and tests; the
existing Orchestrator chat owns the bounded MenuStrip/PropertyList public type
metrics additions. These are visible sibling chats, with no worker agents.

Current prototype constraints remain explicit: ordinary launch is read-only;
Windows installed service transport and indexed search are unavailable. This
slice improves browsing and inspection but does not claim a complete Explorer
replacement, native cross-platform parity or installer readiness.

## Verification update

**MEASURED:** GUI.Forms rebuilt and all 64 native Windows CTest suites passed
in 9.78 seconds, including raised breadcrumb geometry/font tests, MenuStrip and
PropertyList metrics/state tests, MSAA termination, and cancellable Alt+F4.
The installed development SDK and its manifest DLL digest were refreshed.
Orchestrator's two fixture tests and fixture consistency check passed after the
presentation registry note updated its documentation digest. Web.Forms stage-one
compilation and the 1340×850 browser fidelity capture passed; neither establishes
native pixel parity.

**MEASURED:** `./tools/Build-Windows.ps1 -Component Frontend -Jobs 2 -Test`
completed against the updated installed SDK. All **11/11 frontend CTest suites
passed in 2.67 seconds**, including application interaction tests in 1.77 seconds.
The regressions cover public 13 px fonts and 26/28/30 px minimum rows, raised
breadcrumb opt-in, compact retained empty-inspector state, Ctrl+L exact-path
focus and draft preservation, F5 draft preservation and fresh enumeration,
selection count/observed file bytes/exact-path status, and local-calendar
Modified timestamps. Existing history/up, rename, search, criteria and settings
interaction checks also pass. Windows symlink fixture privilege skips remain;
the native junction tests execute. No service was installed or personal file
mutated.

The staged executable is `frontend/.build/shadow-windows/File Manager.exe`,
SHA-256 `b538061dff9ec2a204dc1b248a2de6f8a787f7a844524a04c1f4f11c88e99d83`.
Its GUI.Forms DLL uses manifest digest
`3ad427accd2a9830764b359c15f0457deee19be5063b55ff4f47056dce5b5a48`.
The retained CTest log is
`frontend/results/2026-09-29-shadow-windows/daily-browsing-ctest.txt`.

**OBSERVED:** zero selection now hides the retained preview surface and property
control and keeps a compact selection prompt; selecting an object restores those
same controls. The root selector uses concise Home/drive/folder labels with the
exact tree root in its accessible description. Verification input remains
truthfully disabled where unavailable; group visibility was not added.

The subsequent Computer Use call reported that the user stopped it with physical
Escape. No further desktop automation was attempted. The rebuilt ergonomic
layout, preview restoration, and actual desktop Alt+F4 remain visually unverified.
