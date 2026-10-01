# TextBox caret damage candidate

Status: independent source review and focused Windows tests passed; native
Mac CPU comparison pending. Separate from the Mac initial-visibility correction.

## Change and ownership

**OBSERVED:** `TextBox::on_frame` invalidated the complete control every 530 ms.
The existing localized `Control::invalidate(Rect)` and scheduler coalescing path
permit a smaller region without changing Window or scheduler semantics.

The candidate records the caret geometry after TextBox paint commands complete,
including the invisible blink phase. Blink invalidation uses that rectangle only
when the retained geometry is valid and no measure, arrange, or paint change is
pending. Bounds, effective font, text revision, selection, scroll offset, device
scale, and metrics-provider identity must match. Caret reset, focus transitions,
detach, and the start of a new paint recording revoke old geometry. A missing,
empty, offscreen, or stale rectangle retains the previous full-paint fallback.

The rectangle includes half the one-DIP stroke width plus one device pixel of
antialiasing padding around the line and endpoints. It is intersected with the
same text clip as the drawing and the control client rectangle. The host remains
responsible for outward device-pixel alignment. Geometry is ordinary owned
scalar state; the provider pointer is compared for identity only and never
dereferenced through this record. No queued geometry borrow is introduced.

Localized damage still invalidates and rebuilds the control's complete display
chunk. This change does not claim to eliminate text shaping or command recording.
The measured effect established here is damage localization, not CPU savings or
native raster pixel equivalence. Mac before/after measurements remain required.

## Exact reviewed scope

- `include/gui_forms/controls/panel/text_box/text_box.hpp`: private caret record
  and helpers, and the private multiline paint return value.
- `src/controls/panel/text_box/text_box.cpp`: caret geometry collection in the
  two paint paths, validation, blink invalidation, reset/focus/detach revocation.
- `tests/multiline_text_box_tests.cpp`: new retained-paint/scheduler helpers and
  caret regressions, plus their direct standard-library includes and invocations.

Reviewed these authored hunks against `planning/PROGRAMMING_HOUSE_STYLE.md`:
explicit types, named callbacks and behavior, initialized state, ownership and
provider lifetime, effect-before-assert order, conversions and rounding, failure
fallback, and reuse of fixed scalar storage. No implementation allocations or
new locks are introduced by the retained geometry. Existing unrelated TextBox,
test, renderer, and scheduler code is not claimed style-compliant by this review.
No known house-style violation remains in the authored scope.

## Verification

Shadow Windows, borrowed read-only Plan Paint MinGW toolchain, C++20 Release,
`gui_forms/.build/house-style-text`, at most two compile jobs:

```text
cmake --build gui_forms/.build/house-style-text --target gui_forms_multiline_text_box_tests gui_forms_input_controls_tests gui_forms_frame_scheduler_tests --parallel 2
ctest --test-dir gui_forms/.build/house-style-text -R gui_forms_(multiline_text_box|input_controls|frame_scheduler)_tests --output-on-failure
```

Final result: input controls, multiline TextBox, and frame scheduler **3/3 passed
in 0.42 seconds**. The first fixture compile incorrectly called protected
`Panel::local_bounds`; corrected to the public client rectangle before this run.

New regressions invoke the actual scheduled callback and check coalescing rather
than directly assuming scheduler behavior. They cover empty single/multiline
editors, visible/invisible toggles, old line endpoints inside erase damage,
multiline row placement, scales 0.5/1.5/2, unchanged layout-pass counts, retained
display-chunk rebuilding, occlusion suspension, and full fallback for absent
geometry, edit, resize, scroll, scale, font, provider, selection, and refocus.
They are headless command/damage tests, not native visual or CPU evidence.

`git diff --check` passed. No Window, scheduler, Mac host, Windows A2 integration,
SDK packaging, or release source was changed for this candidate.

## Independent coordinator review

The coordinator verified all three submitted source hashes before reviewing every
authored hunk, the actual single/multiline drawing clips, `FontSpec` scalar storage,
localized invalidation, and abandoned-paint dirty restoration. The retained metrics
provider is identity-only; it is never dereferenced through the caret record.
Failed recording restores paint dirtiness, preventing reuse of candidate geometry.
No functional defect was found within this scope. Read-only value parameters in
the new geometry helper and test painter were made explicitly `const` to match
the house style; the accompanying hash manifest records the final sources.

Independent rebuild and execution of the three named targets passed **3/3 in
0.38 seconds** on the same Windows toolchain. This is command/damage evidence,
not native pixel equivalence or CPU improvement. Existing unrelated source is
outside this style review; there are no known violations in the reviewed hunks.
