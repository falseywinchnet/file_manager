# ProgressBarAnimationAppearance

- Status: **OBSERVED: bundle 004 animation-value review; M4 build, focused tests, and Screen Sharing pass**
- Kind: **struct**
- Hierarchy: `ProgressBarAnimationAppearance`
- Declaration: `include/gui_forms/controls/range_control/progress_bar/progress_bar.hpp:25`
- Definition: `inline/header-only`

ProgressBarAnimationAppearance is the validated value object for stripe width, gap, angle, and opacity used by painter-neutral progress animation.

## Visual evidence

![ProgressBarAnimationAppearance](../captures/range_controls.png)

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const ProgressBarAnimationAppearance&, const ProgressBarAnimationAppearance&) = default
```

Compares every animation appearance field exactly.
