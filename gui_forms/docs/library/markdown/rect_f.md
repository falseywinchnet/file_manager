# RectF

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `RectF`  
Declaration: `include/gui_forms/drawing.hpp:71`  
Definition: `src/core/drawing.cpp`

RectF is a struct declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `left`

```cpp
[[nodiscard]] constexpr double left() const noexcept
```

Reports the current left value without mutation.

### `top`

```cpp
[[nodiscard]] constexpr double top() const noexcept
```

Reports the current top value without mutation.

### `right`

```cpp
[[nodiscard]] constexpr double right() const noexcept
```

Reports the current right value without mutation.

### `bottom`

```cpp
[[nodiscard]] constexpr double bottom() const noexcept
```

Reports the current bottom value without mutation.

### `empty`

```cpp
[[nodiscard]] constexpr bool empty() const noexcept
```

Reports the current empty value without mutation.

### `finite`

```cpp
[[nodiscard]] bool finite() const noexcept
```

Reports the current finite value without mutation.

### `contains`

```cpp
[[nodiscard]] bool contains(PointF point) const noexcept
```

Reports the current contains value without mutation.

### `contains`

```cpp
[[nodiscard]] bool contains(RectF rect) const noexcept
```

Reports the current contains value without mutation.

### `intersects`

```cpp
[[nodiscard]] bool intersects(RectF rect) const noexcept
```

Reports the current intersects value without mutation.

### `offset`

```cpp
void offset(double dx, double dy)
```

Public RectF operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `inflate`

```cpp
void inflate(double dx, double dy)
```

Public RectF operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `intersect`

```cpp
void intersect(RectF rect) noexcept
```

Public RectF operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `intersection`

```cpp
[[nodiscard]] static RectF intersection(RectF left, RectF right) noexcept
```

Public RectF operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `united`

```cpp
[[nodiscard]] static RectF united(RectF left, RectF right) noexcept
```

Public RectF operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `operator==`

```cpp
friend constexpr bool operator==(const RectF&, const RectF&) = default
```

Public RectF operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
