# M12-P10 retained scrolling substrate

Status: **MEASURED PARTIAL M12-P10** on 2026-08-06.

## Question

Can GUI.Forms provide one renderer- and host-neutral `ScrollableControl` model
that preserves authored geometry, drives real input and accessibility, and is
consumed by the generated WinForms facade without a second managed layout
engine?

## Observed gap

Panel and ContainerControl previously had no general retained viewport. The
facade carried a destructive Y-only wheel approximation that moved every child
and did not model content extent, two interdependent bars, clipping, manual
`ScrollProperties`, thumb capture, reveal offsets, or exact scroll events. That
was incompatible behavior, not an acceptable no-op boundary.

## Implemented

- public renderer-neutral `ScrollableControl` with Panel and ContainerControl
  inheritance;
- positive internal two-axis position and WinForms-shaped negative
  `AutoScrollPosition`/`DisplayRectangle` projection;
- non-destructive authored child bounds, retained viewport clipping for paint
  and hit testing, overlay chrome, and deterministic resize clamping;
- automatic extent from child bounds plus margin/minimum size, including
  horizontal/vertical bar interdependence, and separately authored manual
  `ScrollProperties` axes;
- arrow, page, full-drag thumb capture, wheel fallback through nested
  containers, and stable virtual semantic children with increment/decrement and
  set-value actions;
- per-control `AutoScrollOffset`, attached and pre-show authored-tree
  `ScrollControlIntoView`, and focus/capture-safe disposal through the existing
  Control/Window substrate;
- exact `ScrollEventType`, orientation, old/new value, last-event retention,
  and monotonic event revision;
- ABI 0.20 point/size/axis/snapshot operations and typed `GF_EVENT_SCROLL`;
  callback-time state reads observe the completed native input turn without
  recursively forcing layout; and
- nominal generated `Control.AutoScrollOffset`, `ScrollableControl`,
  `ScrollProperties`, H/V properties, state constants, `ScrollEventArgs`, and
  `Scroll` event delivery. The facade uses the ABI snapshot and contains no
  child-position scrolling loop.

## Measured gates

- `gui_forms_scrollable_control_tests`: automatic two-axis 84x64 viewport,
  exact clamping, repeated non-destructive movement, child clipping and hit
  exclusion, resize reset, wheel semantics/event ordering/state, semantic
  action, and control reveal.
- `gui_forms_c_api_c11_tests`: ABI 0.20 negotiation, layout read barrier,
  automatic/manual state, display origin, reveal, scroll subscription kind
  validation, and invalid argument rejection.
- generated surface: 1,104/1,104 required identities, 253 types, zero warnings;
  verifier reports `checked=1104 missing=0`.
- host and Wine `scroll-panel` behavior gates report retained geometry, exact
  48-pixel wheel step, forward/reverse/end reach, control reveal, manual axes,
  exact event args, and one standard event delivery.
- a physical Wine `click-at` on the vertical increment button crosses the
  Win32 pointer host, retained scrollbar, ABI typed callback, and generated
  facade exactly once: `SmallIncrement`, vertical, position 12.
- renderer-free focused suite passes with Skia, HarfBuzz, AppKit, and Win32 host
  disabled: core, basic controls, range controls, scrolling, and C11 ABI.
- warnings-as-errors ASan+UBSan gates pass for scrolling, ABI, and the Complete
  Showcase interaction suite.
- the warnings-as-errors renderer-free x64 MinGW Win32 ABI/automation probe and
  the Skia Win32 build both rebuild; the updated Skia-host DLL passes the .NET
  10 facade behavior gate under Wine.

The existing deterministic showcase gate also performs 48 captured slider
drags followed immediately by Animation navigation, retained paint, and timer
deadline checks. It passes normally and under ASan+UBSan. This is the accepted
regression gate for the reported intermittent transition crash; another manual
reproduction is not required to keep the defect covered.

## Honest boundary

This is the reusable behavioral center, not a claim of exhaustive WinForms
parity. RTL scrollbar mirroring, logical/DPI scaling oracles, high-resolution
horizontal wheel policy, press-and-hold repeat timing, drag/drop edge
autoscroll, virtualization anchoring under insert/remove/resize, native
accessibility publisher verification, and an independent exhaustive WinForms
event-order oracle remain open.
