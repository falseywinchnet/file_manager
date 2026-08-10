# Rect

- Status: **generated inventory; detailed review pending**
- Kind: **struct**
- Hierarchy: `Rect`
- Declaration: `include/gui_forms/types.hpp:29`
- Definition: `src/core/types.cpp, src/core/window/presentation/window_presentation.cpp`

Rect is a struct declared in include/gui_forms/types.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const Rect&, const Rect&) = default
```

Public Rect operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `empty` (public)

```cpp
[[nodiscard]] constexpr bool empty() const noexcept
```

Reports the current empty value without mutation.

### `finite` (public)

```cpp
[[nodiscard]] bool finite() const noexcept
```

Reports the current finite value without mutation.

### `left` (public)

```cpp
[[nodiscard]] constexpr double left() const noexcept
```

Reports the current left value without mutation.

### `top` (public)

```cpp
[[nodiscard]] constexpr double top() const noexcept
```

Reports the current top value without mutation.

### `right` (public)

```cpp
[[nodiscard]] constexpr double right() const noexcept
```

Reports the current right value without mutation.

### `bottom` (public)

```cpp
[[nodiscard]] constexpr double bottom() const noexcept
```

Reports the current bottom value without mutation.

### `area` (public)

```cpp
[[nodiscard]] constexpr double area() const noexcept
```

Reports the current area value without mutation.

### `contains` (public)

```cpp
[[nodiscard]] constexpr bool contains(Point point) const noexcept
```

Reports the current contains value without mutation.

### `contains` (public)

```cpp
[[nodiscard]] constexpr bool contains(Rect rect) const noexcept
```

Reports the current contains value without mutation.

### `intersection` (public)

```cpp
[[nodiscard]] static Rect intersection(Rect left, Rect right) noexcept
```

Public Rect operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `united` (public)

```cpp
[[nodiscard]] static Rect united(Rect left, Rect right) noexcept
```

Public Rect operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
