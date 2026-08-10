# RectI

- Status: **OBSERVED: bundle 009 integer rectangle review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `RectI`
- Declaration: `include/gui_forms/drawing/geometry/drawing_geometry.hpp:34`
- Definition: `src/core/drawing/geometry/drawing_geometry.cpp`

RectI provides overflow-aware integer pixel containment, intersection, union, translation, and inflation.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `left` (public)

```cpp
[[nodiscard]] constexpr std::int64_t left() const noexcept
```

Returns signed left edge.

### `top` (public)

```cpp
[[nodiscard]] constexpr std::int64_t top() const noexcept
```

Returns signed top edge.

### `right` (public)

```cpp
[[nodiscard]] constexpr std::int64_t right() const noexcept
```

Returns 64-bit x-plus-width edge.

### `bottom` (public)

```cpp
[[nodiscard]] constexpr std::int64_t bottom() const noexcept
```

Returns 64-bit y-plus-height edge.

### `empty` (public)

```cpp
[[nodiscard]] constexpr bool empty() const noexcept
```

Reports nonpositive width or height.

### `contains` (public)

```cpp
[[nodiscard]] bool contains(PointI point) const noexcept
```

Tests point or rectangle containment with wide edge arithmetic.

### `contains` (public)

```cpp
[[nodiscard]] bool contains(RectI rect) const noexcept
```

Tests point or rectangle containment with wide edge arithmetic.

### `intersects` (public)

```cpp
[[nodiscard]] bool intersects(RectI rect) const noexcept
```

Tests positive-area overlap.

### `offset` (public)

```cpp
void offset(std::int32_t dx, std::int32_t dy) noexcept
```

Translates with 32-bit saturation.

### `inflate` (public)

```cpp
void inflate(std::int32_t dx, std::int32_t dy) noexcept
```

Expands symmetrically with saturation.

### `intersect` (public)

```cpp
void intersect(RectI rect) noexcept
```

Replaces this rectangle with exact intersection.

### `intersection` (public)

```cpp
[[nodiscard]] static RectI intersection(RectI left, RectI right) noexcept
```

Returns exact overlap or empty.

### `united` (public)

```cpp
[[nodiscard]] static RectI united(RectI left, RectI right) noexcept
```

Returns conservative union with saturation.

### `operator==` (public)

```cpp
friend constexpr bool operator==(const RectI&, const RectI&) = default
```

Compares all four fields.
