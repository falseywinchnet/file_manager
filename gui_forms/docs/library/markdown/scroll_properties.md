# ScrollProperties

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `ScrollProperties`  
Declaration: `include/gui_forms/scrolling.hpp:61`  
Definition: `src/controls/scrolling.cpp`

ScrollProperties is a class declared in include/gui_forms/scrolling.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `enabled`

```cpp
[[nodiscard]] bool enabled() const noexcept
```

Reports the current enabled value without mutation.

### `set_enabled`

```cpp
void set_enabled(bool enabled)
```

Synchronously updates the retained enabled property. Validation, typed invalidation, and notifications are defined by the implementation.

### `visible`

```cpp
[[nodiscard]] bool visible() const noexcept
```

Reports the current visible value without mutation.

### `set_visible`

```cpp
void set_visible(bool visible)
```

Synchronously updates the retained visible property. Validation, typed invalidation, and notifications are defined by the implementation.

### `minimum`

```cpp
[[nodiscard]] double minimum() const noexcept
```

Reports the current minimum value without mutation.

### `set_minimum`

```cpp
void set_minimum(double minimum)
```

Synchronously updates the retained minimum property. Validation, typed invalidation, and notifications are defined by the implementation.

### `maximum`

```cpp
[[nodiscard]] double maximum() const noexcept
```

Reports the current maximum value without mutation.

### `set_maximum`

```cpp
void set_maximum(double maximum)
```

Synchronously updates the retained maximum property. Validation, typed invalidation, and notifications are defined by the implementation.

### `large_change`

```cpp
[[nodiscard]] double large_change() const noexcept
```

Reports the current large change value without mutation.

### `set_large_change`

```cpp
void set_large_change(double value)
```

Synchronously updates the retained large change property. Validation, typed invalidation, and notifications are defined by the implementation.

### `small_change`

```cpp
[[nodiscard]] double small_change() const noexcept
```

Reports the current small change value without mutation.

### `set_small_change`

```cpp
void set_small_change(double value)
```

Synchronously updates the retained small change property. Validation, typed invalidation, and notifications are defined by the implementation.

### `value`

```cpp
[[nodiscard]] double value() const noexcept
```

Reports the current value value without mutation.

### `set_value`

```cpp
void set_value(double value)
```

Synchronously updates the retained value property. Validation, typed invalidation, and notifications are defined by the implementation.

### `orientation`

```cpp
[[nodiscard]] ScrollOrientation orientation() const noexcept
```

Reports the current orientation value without mutation.

### `snapshot`

```cpp
[[nodiscard]] ScrollAxisSnapshot snapshot() const noexcept
```

Reports the current snapshot value without mutation.
