# PointF

- Status: **OBSERVED: bundle 012 isolated logical-point declaration/definition; native and MinGW M4 builds pass**
- Kind: **struct**
- Hierarchy: `PointF`
- Declaration: `include/gui_forms/drawing/geometry/point_f/point_f.hpp:5`
- Definition: `src/core/drawing/geometry/point_f/point_f.cpp`

PointF is a finite logical-coordinate pair.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

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
