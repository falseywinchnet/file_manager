# PresentationSettings

- Status: **OBSERVED: bundle 008 presentation policy review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `PresentationSettings`
- Declaration: `include/gui_forms/window/presentation/presentation_types.hpp:15`
- Definition: `inline/header-only`

PresentationSettings keeps logical text scaling, contrast, motion, and sound preferences independent from device scale and native host mechanics.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const PresentationSettings&, const PresentationSettings&) = default
```

Compares all logical presentation axes for exact retained-policy equality.
