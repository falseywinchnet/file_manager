# Rect

- Status: **OBSERVED: bundle 011 geometry review**
- Kind: **struct**
- Hierarchy: `Rect`
- Declaration: `include/gui_forms/types/geometry/geometry.hpp:19`
- Definition: `src/core/types/rect/rect.cpp, src/core/window/presentation/window_presentation.cpp`

Rect is a half-open logical rectangle with finite/empty checks, containment, exact intersection, and bounding union operations.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const Rect&, const Rect&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.

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

Executes Rect's intersection operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `united` (public)

```cpp
[[nodiscard]] static Rect united(Rect left, Rect right) noexcept
```

Executes Rect's united operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
