# RectI

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `RectI`  
Declaration: `include/gui_forms/drawing.hpp:43`  
Definition: `src/core/drawing.cpp`

RectI is a struct declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `left`

```cpp
[[nodiscard]] constexpr std::int64_t left() const noexcept
```

Reports the current left value without mutation.

### `top`

```cpp
[[nodiscard]] constexpr std::int64_t top() const noexcept
```

Reports the current top value without mutation.

### `right`

```cpp
[[nodiscard]] constexpr std::int64_t right() const noexcept
```

Reports the current right value without mutation.

### `bottom`

```cpp
[[nodiscard]] constexpr std::int64_t bottom() const noexcept
```

Reports the current bottom value without mutation.

### `empty`

```cpp
[[nodiscard]] constexpr bool empty() const noexcept
```

Reports the current empty value without mutation.

### `contains`

```cpp
[[nodiscard]] bool contains(PointI point) const noexcept
```

Reports the current contains value without mutation.

### `contains`

```cpp
[[nodiscard]] bool contains(RectI rect) const noexcept
```

Reports the current contains value without mutation.

### `intersects`

```cpp
[[nodiscard]] bool intersects(RectI rect) const noexcept
```

Reports the current intersects value without mutation.

### `offset`

```cpp
void offset(std::int32_t dx, std::int32_t dy) noexcept
```

Public RectI operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `inflate`

```cpp
void inflate(std::int32_t dx, std::int32_t dy) noexcept
```

Public RectI operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `intersect`

```cpp
void intersect(RectI rect) noexcept
```

Public RectI operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `intersection`

```cpp
[[nodiscard]] static RectI intersection(RectI left, RectI right) noexcept
```

Public RectI operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `united`

```cpp
[[nodiscard]] static RectI united(RectI left, RectI right) noexcept
```

Public RectI operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `operator==`

```cpp
friend constexpr bool operator==(const RectI&, const RectI&) = default
```

Public RectI operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
