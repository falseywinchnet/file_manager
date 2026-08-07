# M12-P9 Control geometry, ordering, and preferred size

Status: **MEASURED PARTIAL M12-P9** on 2026-08-06.

## Question

Does GUI.Forms expose one coherent retained base-Control geometry model through
native C++ and the generated WinForms facade, or are compatibility members only
surface-shaped helpers with conflicting layout, hit-test, and z-order meaning?

## Observed gap

The native core already retained requested/arranged/client bounds, finite
minimum/maximum constraints, ancestry conversion, containment, and front/back
mutation. The generated facade exposed only part of that family. Its managed
child list also mixed insertion order with topmost order: Add and
BringToFront disagreed with SetChildIndex and the native painter. Exact
`AutoSizeMode`, `BoundsSpecified`, `GetChildAtPointSkip`, child lookup,
non-wrapping nested traversal, and preferred-size calls remained catalogue-open.

## Implemented

- exact retained `AutoSizeMode`, `BoundsSpecified`, and
  `GetChildAtPointSkip` values with undefined-bit rejection;
- masked bounds mutation preserving unspecified coordinates and constrained
  dimensions;
- topmost-first direct-child indices, filtered client-point lookup, coherent
  BringToFront/SendToBack, and an explicit paintable hit-transparent state
  that immediately releases stale pointer capture and pressed/hover state;
- non-wrapping stable nested TabIndex traversal independent of focusability;
- point and rectangle conversion to and from Window client coordinates;
- proposed-size preferred measurement plus base GrowOnly and GrowAndShrink
  AutoSize, including child bounds, trailing margins, padding, and min/max;
- FlowLayoutPanel and TableLayoutPanel adoption of the base AutoSize contract;
- nominal generated facade emission and behavior for the complete bounded
  Control/ControlCollection geometry family; and
- generated index-zero-topmost ordering across Add, front/back mutation,
  SetChildIndex, docking, and point lookup.

The generated facade still projects PointToScreen through its owned top-level
presentation coordinates. The portable native API deliberately names its
operation `point_to_window`: a truthful desktop-screen origin requires a host
monitor/window-origin contract and is not inferred from AppKit or Win32.

## Measured gates

- `gui_forms_core_tests` proves constraints, masked mutation, enum rejection,
  ancestry point/rectangle round trips, direct-child filters, transparent
  pointer pass-through, topmost-first indices, stable nested traversal, and
  both AutoSize modes.
- `control-geometry` in `facade_behavior_smoke` proves the same generated
  family through public WinForms-shaped calls.
- Existing `form-semantics`, `dock-padding`, `split-container`, and
  `secondary-form` facade gates remain green after the ordering correction.
- `gui_forms_layout_panel_tests` and the 48-cycle
  `gui_forms_showcase_interaction_tests` remain green.
- A rebuilt AppKit showcase was exercised through its accessibility surface:
  the Filled rail moved from 52 to 84, its bound Continuous progress followed
  to 84, Animation opened, and Pause/Resume completed while the page remained
  alive and responsive.
- The strict x64 Win32 showcase and the macOS showcase both rebuild after the
  final pointer-routing change.
- The generated facade resolves 1,104/1,104 required captured identities and
  compiles with zero warnings.

## Honest boundary

This is a measured behavioral center, not complete WinForms layout parity.
Host-relative screen origin, multi-monitor transforms, DPI rounding, RTL
mirroring, protected event-order parity, baseline layout, scaling overloads,
and adversarial AutoSize convergence remain open. Invisible direct-child
geometry retains the last committed slot until a later layout; this is
deterministic but still requires comparison with an independent WinForms
oracle before exact compatibility can be claimed.
