# PointI

- Status: **OBSERVED: bundle 009 drawing geometry review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `PointI`
- Declaration: `include/gui_forms/drawing/geometry/drawing_geometry.hpp:8`
- Definition: `src/core/drawing/geometry/drawing_geometry.cpp`

PointI is a signed integer pixel-coordinate pair.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `offset` (public)

```cpp
void offset(std::int32_t dx, std::int32_t dy) noexcept
```

Adds signed deltas with deterministic 32-bit saturation.

### `operator==` (public)

```cpp
friend constexpr bool operator==(const PointI&, const PointI&) = default
```

Compares both coordinates.
