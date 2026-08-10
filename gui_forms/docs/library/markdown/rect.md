# Rect

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `Rect`  
Declaration: `include/gui_forms/types.hpp:29`  
Definition: `src/core/types.cpp, src/core/window.cpp`

Rect is a struct declared in include/gui_forms/types.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `operator==`

```cpp
friend constexpr bool operator==(const Rect&, const Rect&) = default
```

Public Rect operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

### `area`

```cpp
[[nodiscard]] constexpr double area() const noexcept
```

Reports the current area value without mutation.

### `contains`

```cpp
[[nodiscard]] constexpr bool contains(Point point) const noexcept
```

Reports the current contains value without mutation.

### `contains`

```cpp
[[nodiscard]] constexpr bool contains(Rect rect) const noexcept
```

Reports the current contains value without mutation.

### `intersection`

```cpp
[[nodiscard]] static Rect intersection(Rect left, Rect right) noexcept
```

Public Rect operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `united`

```cpp
[[nodiscard]] static Rect united(Rect left, Rect right) noexcept
```

Public Rect operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
