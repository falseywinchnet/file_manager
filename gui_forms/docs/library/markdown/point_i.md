# PointI

- Status: **OBSERVED: bundle 012 isolated integer-point declaration/definition; native and MinGW M4 builds pass**
- Kind: **struct**
- Hierarchy: `PointI`
- Declaration: `include/gui_forms/drawing/geometry/point_i/point_i.hpp:7`
- Definition: `src/core/drawing/geometry/point_i/point_i.cpp`

PointI is a signed integer pixel-coordinate pair.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

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
