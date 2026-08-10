# PointF

- Status: **OBSERVED: bundle 009 drawing geometry review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `PointF`
- Declaration: `include/gui_forms/drawing/geometry/drawing_geometry.hpp:15`
- Definition: `src/core/drawing/geometry/drawing_geometry.cpp`

PointF is a finite logical-coordinate pair.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `offset` (public)

```cpp
void offset(double dx, double dy)
```

Validates finite inputs/result before committing translation.

### `operator==` (public)

```cpp
friend constexpr bool operator==(const PointF&, const PointF&) = default
```

Compares both coordinates.
