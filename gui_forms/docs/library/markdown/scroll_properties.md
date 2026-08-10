# ScrollProperties

- Status: **OBSERVED: bundle 003 split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class**
- Hierarchy: `ScrollProperties`
- Declaration: `include/gui_forms/controls/scrollable_control/scroll_properties/scroll_properties.hpp:58`
- Definition: `src/controls/scrollable_control/scroll_properties/scroll_properties.cpp`

ScrollProperties is the owner-bound, per-axis state model for enabled/visible state, authored or automatic range increments, clamped position, orientation, and inspectable snapshot projection.

## Visual evidence

![ScrollProperties](../captures/scrollable_control.png)

## Declared methods

### `enabled` (public)

```cpp
[[nodiscard]] bool enabled() const noexcept
```

Reports whether user-driven movement is admitted on this axis.

### `set_enabled` (public)

```cpp
void set_enabled(bool enabled)
```

Commits axis availability and routes the change through the owning ScrollableControl's retained invalidation path.

### `visible` (public)

```cpp
[[nodiscard]] bool visible() const noexcept
```

Reports the resolved or explicitly requested scrollbar visibility.

### `set_visible` (public)

```cpp
void set_visible(bool visible)
```

Sets explicit axis visibility and asks the owner to recompute viewport, geometry, paint, and semantics.

### `minimum` (public)

```cpp
[[nodiscard]] double minimum() const noexcept
```

Returns the inclusive authored minimum scroll value.

### `set_minimum` (public)

```cpp
void set_minimum(double minimum)
```

Validates a finite bound, preserves an ordered range, clamps the current position, and notifies the owner atomically.

### `maximum` (public)

```cpp
[[nodiscard]] double maximum() const noexcept
```

Returns the inclusive content-range maximum before large-change viewport reduction.

### `set_maximum` (public)

```cpp
void set_maximum(double maximum)
```

Validates a finite bound, preserves an ordered range, clamps the current position, and notifies the owner atomically.

### `large_change` (public)

```cpp
[[nodiscard]] double large_change() const noexcept
```

Returns the authored page increment or the automatic viewport-sized increment.

### `set_large_change` (public)

```cpp
void set_large_change(double value)
```

Accepts a finite nonnegative increment, marks it caller-authored, and recomputes axis geometry.

### `small_change` (public)

```cpp
[[nodiscard]] double small_change() const noexcept
```

Returns the authored line/wheel increment or the automatic default.

### `set_small_change` (public)

```cpp
void set_small_change(double value)
```

Accepts a finite nonnegative increment, marks it caller-authored, and recomputes axis behavior.

### `value` (public)

```cpp
[[nodiscard]] double value() const noexcept
```

Returns the committed axis position within the effective maximum-position interval.

### `set_value` (public)

```cpp
void set_value(double value)
```

Validates and clamps a requested position, then commits it through the owner's display-rectangle state without synthesizing user input.

### `orientation` (public)

```cpp
[[nodiscard]] ScrollOrientation orientation() const noexcept
```

Returns the immutable horizontal or vertical identity assigned by the owner.

### `snapshot` (public)

```cpp
[[nodiscard]] ScrollAxisSnapshot snapshot() const noexcept
```

Returns a value-only copy of the complete resolved axis state for inspection and tests.

### `ScrollProperties` (private)

```cpp
ScrollProperties(ScrollableControl& owner, ScrollOrientation orientation) : owner_(&owner), orientation_(orientation)
```

Constructs or tears down the retained ScrollProperties object according to its ownership contract.

### `set_automatic` (private)

```cpp
void set_automatic(double viewport_extent, double content_extent, double value) noexcept
```

Synchronously updates the retained automatic property. Validation, typed invalidation, and notifications are defined by the implementation.

### `maximum_position` (private)

```cpp
[[nodiscard]] double maximum_position() const noexcept
```

Reports the current maximum position value without mutation.
