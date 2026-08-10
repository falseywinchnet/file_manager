# LinearGradientBrush

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Brush → LinearGradientBrush`  
Declaration: `include/gui_forms/drawing.hpp:387`  
Definition: `src/core/drawing.cpp`

LinearGradientBrush is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `LinearGradientBrush`

```cpp
LinearGradientBrush(RectF bounds, Color first, Color second, double angle = 0.0, WrapMode wrap_mode = WrapMode::tile)
```

Constructs or tears down the retained LinearGradientBrush object according to its ownership contract.

### `set_blend`

```cpp
void set_blend(std::span<const double> factors, std::span<const double> positions)
```

Synchronously updates the retained blend property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_interpolation_colors`

```cpp
void set_interpolation_colors(const ColorBlend& blend)
```

Synchronously updates the retained interpolation colors property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_wrap_mode`

```cpp
void set_wrap_mode(WrapMode mode)
```

Synchronously updates the retained wrap mode property. Validation, typed invalidation, and notifications are defined by the implementation.

### `snapshot`

```cpp
[[nodiscard]] BrushSnapshot snapshot() const override
```

Reports the current snapshot value without mutation.
