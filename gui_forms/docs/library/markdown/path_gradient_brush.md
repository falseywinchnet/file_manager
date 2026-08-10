# PathGradientBrush

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Brush → PathGradientBrush`  
Declaration: `include/gui_forms/drawing.hpp:408`  
Definition: `src/core/drawing.cpp`

PathGradientBrush is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `PathGradientBrush`

```cpp
explicit PathGradientBrush(std::span<const PointF> points, WrapMode wrap_mode = WrapMode::clamp)
```

Constructs or tears down the retained PathGradientBrush object according to its ownership contract.

### `set_center_color`

```cpp
void set_center_color(Color color)
```

Synchronously updates the retained center color property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_center_point`

```cpp
void set_center_point(PointF point)
```

Synchronously updates the retained center point property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_surround_colors`

```cpp
void set_surround_colors(std::span<const Color> colors)
```

Synchronously updates the retained surround colors property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_interpolation_colors`

```cpp
void set_interpolation_colors(const ColorBlend& blend)
```

Synchronously updates the retained interpolation colors property. Validation, typed invalidation, and notifications are defined by the implementation.

### `snapshot`

```cpp
[[nodiscard]] BrushSnapshot snapshot() const override
```

Reports the current snapshot value without mutation.
