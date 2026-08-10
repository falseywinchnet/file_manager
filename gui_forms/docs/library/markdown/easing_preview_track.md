# EasingPreviewTrack

- Status: **OBSERVED: bundle 006 value-type split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **struct**
- Hierarchy: `EasingPreviewTrack`
- Declaration: `include/gui_forms/controls/easing_preview/easing_preview.hpp:11`
- Definition: `inline/header-only`

EasingPreviewTrack is one caller-authored easing comparison row: exact curve, label, and marker color.

## Visual evidence

![EasingPreviewTrack](../captures/easing_preview.png)

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const EasingPreviewTrack&, const EasingPreviewTrack&) = default
```

Compares curve, label, and color for exact retained configuration equality.
