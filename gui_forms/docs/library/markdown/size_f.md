# SizeF

- Status: **OBSERVED: bundle 009 drawing geometry review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `SizeF`
- Declaration: `include/gui_forms/drawing/geometry/drawing_geometry.hpp:28`
- Definition: `inline/header-only`

SizeF carries logical width and height.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const SizeF&, const SizeF&) = default
```

Compares width and height.
