# RangeControl

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Control → RangeControl`  
Declaration: `include/gui_forms/range_controls.hpp:77`  
Definition: `src/controls/range_controls.cpp`

RangeControl is a visual retained control declared in include/gui_forms/range_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `RangeControl`

```cpp
explicit RangeControl(StableId stable_id)
```

Constructs or tears down the retained RangeControl object according to its ownership contract.

### `minimum`

```cpp
[[nodiscard]] double minimum() const noexcept
```

Reports the current minimum value without mutation.

### `maximum`

```cpp
[[nodiscard]] double maximum() const noexcept
```

Reports the current maximum value without mutation.

### `value`

```cpp
[[nodiscard]] double value() const noexcept
```

Reports the current value value without mutation.

### `small_change`

```cpp
[[nodiscard]] double small_change() const noexcept
```

Reports the current small change value without mutation.

### `large_change`

```cpp
[[nodiscard]] double large_change() const noexcept
```

Reports the current large change value without mutation.

### `orientation`

```cpp
[[nodiscard]] Orientation orientation() const noexcept
```

Reports the current orientation value without mutation.

### `normalized_value`

```cpp
[[nodiscard]] double normalized_value() const noexcept
```

Reports the current normalized value value without mutation.

### `style`

```cpp
[[nodiscard]] const BasicControlStyle& style() const noexcept
```

Reports the current style value without mutation.

### `set_range`

```cpp
void set_range(double minimum, double maximum)
```

Synchronously updates the retained range property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_minimum`

```cpp
void set_minimum(double minimum)
```

Synchronously updates the retained minimum property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_maximum`

```cpp
void set_maximum(double maximum)
```

Synchronously updates the retained maximum property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_value`

```cpp
virtual void set_value(double value)
```

Synchronously updates the retained value property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_small_change`

```cpp
void set_small_change(double change)
```

Synchronously updates the retained small change property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_large_change`

```cpp
void set_large_change(double change)
```

Synchronously updates the retained large change property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_orientation`

```cpp
void set_orientation(Orientation orientation)
```

Synchronously updates the retained orientation property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_style`

```cpp
void set_style(BasicControlStyle style)
```

Synchronously updates the retained style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `increment`

```cpp
void increment(double delta)
```

Public RangeControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `range_changed`

```cpp
[[nodiscard]] Event<double, double>& range_changed() noexcept
```

Public RangeControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `value_changed`

```cpp
[[nodiscard]] Event<double>& value_changed() noexcept
```

Public RangeControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `scroll`

```cpp
[[nodiscard]] Event<const RangeScrollEvent&>& scroll() noexcept
```

Public RangeControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
