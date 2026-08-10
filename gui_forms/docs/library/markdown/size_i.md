# SizeI

- Status: **OBSERVED: bundle 009 drawing geometry review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `SizeI`
- Declaration: `include/gui_forms/drawing/geometry/drawing_geometry.hpp:22`
- Definition: `inline/header-only`

SizeI carries signed integer width and height.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const SizeI&, const SizeI&) = default
```

Compares width and height.
