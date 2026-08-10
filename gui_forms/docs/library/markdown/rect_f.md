# RectF

- Status: **OBSERVED: bundle 009 logical rectangle review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `RectF`
- Declaration: `include/gui_forms/drawing/geometry/drawing_geometry.hpp:62`
- Definition: `src/core/drawing/geometry/drawing_geometry.cpp`

RectF provides validated finite logical containment, intersection, union, translation, and inflation.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `left` (public)

```cpp
[[nodiscard]] constexpr double left() const noexcept
```

Returns left edge.

### `top` (public)

```cpp
[[nodiscard]] constexpr double top() const noexcept
```

Returns top edge.

### `right` (public)

```cpp
[[nodiscard]] constexpr double right() const noexcept
```

Returns x-plus-width.

### `bottom` (public)

```cpp
[[nodiscard]] constexpr double bottom() const noexcept
```

Returns y-plus-height.

### `empty` (public)

```cpp
[[nodiscard]] constexpr bool empty() const noexcept
```

Reports nonpositive width or height.

### `finite` (public)

```cpp
[[nodiscard]] bool finite() const noexcept
```

Checks all four fields.

### `contains` (public)

```cpp
[[nodiscard]] bool contains(PointF point) const noexcept
```

Tests point or rectangle containment.

### `contains` (public)

```cpp
[[nodiscard]] bool contains(RectF rect) const noexcept
```

Tests point or rectangle containment.

### `intersects` (public)

```cpp
[[nodiscard]] bool intersects(RectF rect) const noexcept
```

Tests positive-area overlap.

### `offset` (public)

```cpp
void offset(double dx, double dy)
```

Validates and commits finite translation.

### `inflate` (public)

```cpp
void inflate(double dx, double dy)
```

Validates and commits symmetric expansion.

### `intersect` (public)

```cpp
void intersect(RectF rect) noexcept
```

Replaces this rectangle with exact intersection.

### `intersection` (public)

```cpp
[[nodiscard]] static RectF intersection(RectF left, RectF right) noexcept
```

Returns finite overlap or empty.

### `united` (public)

```cpp
[[nodiscard]] static RectF united(RectF left, RectF right) noexcept
```

Returns finite conservative union.

### `operator==` (public)

```cpp
friend constexpr bool operator==(const RectF&, const RectF&) = default
```

Compares all four fields.
