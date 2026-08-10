# Matrix

- Status: **OBSERVED: bundle 009 affine matrix split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `Matrix`
- Declaration: `include/gui_forms/drawing/matrix/matrix.hpp:7`
- Definition: `src/core/drawing/matrix/matrix.cpp`

Matrix is an immutable-value 2D affine transform with finite validation, point/bounds transformation, explicit composition order, and inspectable coefficients.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `Matrix` (public)

```cpp
constexpr Matrix() noexcept = default
```

Constructs identity or explicit six-coefficient affine state.

### `Matrix` (public)

```cpp
constexpr Matrix(double m11, double m12, double m21, double m22, double dx, double dy) noexcept : m11_(m11), m12_(m12), m21_(m21), m22_(m22), dx_(dx), dy_(dy)
```

Constructs identity or explicit six-coefficient affine state.

### `translation` (public)

```cpp
[[nodiscard]] static Matrix translation(double x, double y)
```

Validates and creates a translation.

### `rotation_at` (public)

```cpp
[[nodiscard]] static Matrix rotation_at(double degrees, PointF center)
```

Validates angle/center and composes a rotation about that point.

### `finite` (public)

```cpp
[[nodiscard]] bool finite() const noexcept
```

Reports whether every coefficient is finite.

### `transform` (public)

```cpp
[[nodiscard]] PointF transform(PointF point) const
```

Applies the affine map to one validated point.

### `transform_bounds` (public)

```cpp
[[nodiscard]] RectF transform_bounds(RectF rect) const
```

Transforms four rectangle corners and returns their axis-aligned bounds.

### `followed_by` (public)

```cpp
[[nodiscard]] Matrix followed_by(const Matrix& next) const
```

Composes this transform followed by the supplied transform in authored order.

### `m11` (public)

```cpp
[[nodiscard]] constexpr double m11() const noexcept
```

Returns the first scale/rotation coefficient.

### `m12` (public)

```cpp
[[nodiscard]] constexpr double m12() const noexcept
```

Returns the second rotation/shear coefficient.

### `m21` (public)

```cpp
[[nodiscard]] constexpr double m21() const noexcept
```

Returns the third rotation/shear coefficient.

### `m22` (public)

```cpp
[[nodiscard]] constexpr double m22() const noexcept
```

Returns the second scale/rotation coefficient.

### `dx` (public)

```cpp
[[nodiscard]] constexpr double dx() const noexcept
```

Returns x translation.

### `dy` (public)

```cpp
[[nodiscard]] constexpr double dy() const noexcept
```

Returns y translation.

### `operator==` (public)

```cpp
friend constexpr bool operator==(const Matrix&, const Matrix&) = default
```

Compares all six coefficients exactly.
