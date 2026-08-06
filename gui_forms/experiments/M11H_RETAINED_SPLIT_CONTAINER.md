# M11h retained split-container slice

Date: 2026-08-05

Status: **MEASURED PARTIAL**

## Question

Can GUI.Forms provide the reusable FM-LY05 physical split-pane behavior needed
by the future File Manager demoboard and ordinary WinForms consumers without a
demo-local pane implementation or release-only pointer update?

## Scope

- Public C++ `SplitContainer`, two stable `SplitterPanel` children, and one
  stable splitter-control identity.
- Vertical/horizontal allocation, minimum extents, fixed-second-panel resize,
  fixed splitter, collapse/restore, and remembered distance.
- Three-logical-pixel default painted seam with a separate nine-logical-pixel
  retained hit target.
- Live retained pointer capture and update on every move, plus focusable
  keyboard resizing and focus transfer when a focused pane collapses.
- Deterministic `SplitChangeEvent` causes for programmatic, pointer, keyboard,
  collapse, and container-resize changes.
- Compatible managed `SplitContainer` projection for the captured .NET 10
  surface, using the owned GUI.Drawing paint path.

This slice does not implement the demoboard.

## Evidence

**MEASURED:** `gui_forms_split_container_tests` covers stable child IDs,
vertical/horizontal geometry, separate hit geometry, live pointer tracking,
capture release, keyboard increments, impossible-minimum compromise,
collapse/focus transfer, fixed-second resize, fixed input rejection, and UI
thread enforcement.

**MEASURED:** the normal Skia/AppKit-enabled build passes 33/33 tests. The
renderer-free, Skia-disabled, AppKit-disabled build passes 25/25, including the
same split fixture.

**MEASURED:** the generated facade resolves and verifies 1,104/1,104 required
rows with zero build warnings. The `split-container` behavior fixture passes on
host .NET, headless Wine, and a physical Win32/Wine window:

```text
split-container=geometry:constrained|drag:live|collapse:focus-transferred|orientation:horizontal|fixed:enforced
```

The complete repeatable facade gate also passes its existing form, secondary
form, cursor, dock-padding, and radio-console fixtures on host and Wine.

## Negative result retained

**OBSERVED:** the first compound-control attempt added panels from the C++
constructor and raised `bad_weak_ptr`; parent/cycle validation correctly
requires established shared ownership. `make_control` now supplies a bounded
post-construction hook for compound retained controls rather than weakening
ownership validation.

**OBSERVED:** the first managed projection constrained `SplitterDistance`
against the default/pre-parent size. Physical Wine therefore turned a requested
120-pixel distance into the 80-pixel minimum after attachment. Requested and
effective distance are now distinct until real layout, and orientation changes
still enforce panel minima.

## Remaining boundary

- No experimental C ABI or DML spelling for the split primitive yet.
- Automatic responsive collapse versus explicit user collapse, persisted pane
  extent storage, collapse-tab presentation/proximity, nested split stress,
  and semantic/native accessibility publication remain open.
- This is FM-LY05 partial evidence. It does not close FM-LY03 responsive
  priority collapse, FM-LY04 text-scale reflow, or the demoboard acceptance
  gates.
