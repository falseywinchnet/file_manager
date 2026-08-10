# Matrix

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Matrix`  
Declaration: `include/gui_forms/drawing.hpp:232`  
Definition: `src/core/drawing.cpp`

Matrix is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Matrix`

```cpp
constexpr Matrix() noexcept = default
```

Constructs or tears down the retained Matrix object according to its ownership contract.

### `Matrix`

```cpp
constexpr Matrix(double m11, double m12, double m21, double m22, double dx, double dy) noexcept : m11_(m11), m12_(m12), m21_(m21), m22_(m22), dx_(dx), dy_(dy)
```

Constructs or tears down the retained Matrix object according to its ownership contract.

### `translation`

```cpp
[[nodiscard]] static Matrix translation(double x, double y)
```

Public Matrix operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `rotation_at`

```cpp
[[nodiscard]] static Matrix rotation_at(double degrees, PointF center)
```

Public Matrix operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `finite`

```cpp
[[nodiscard]] bool finite() const noexcept
```

Reports the current finite value without mutation.

### `transform`

```cpp
[[nodiscard]] PointF transform(PointF point) const
```

Reports the current transform value without mutation.

### `transform_bounds`

```cpp
[[nodiscard]] RectF transform_bounds(RectF rect) const
```

Reports the current transform bounds value without mutation.

### `followed_by`

```cpp
[[nodiscard]] Matrix followed_by(const Matrix& next) const
```

Reports the current followed by value without mutation.

### `m11`

```cpp
[[nodiscard]] constexpr double m11() const noexcept
```

Reports the current m11 value without mutation.

### `m12`

```cpp
[[nodiscard]] constexpr double m12() const noexcept
```

Reports the current m12 value without mutation.

### `m21`

```cpp
[[nodiscard]] constexpr double m21() const noexcept
```

Reports the current m21 value without mutation.

### `m22`

```cpp
[[nodiscard]] constexpr double m22() const noexcept
```

Reports the current m22 value without mutation.

### `dx`

```cpp
[[nodiscard]] constexpr double dx() const noexcept
```

Reports the current dx value without mutation.

### `dy`

```cpp
[[nodiscard]] constexpr double dy() const noexcept
```

Reports the current dy value without mutation.

### `operator==`

```cpp
friend constexpr bool operator==(const Matrix&, const Matrix&) = default
```

Public Matrix operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
