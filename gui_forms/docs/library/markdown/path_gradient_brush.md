# PathGradientBrush

- Status: **OBSERVED: bundle 009 path-gradient brush split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `Brush → PathGradientBrush`
- Declaration: `include/gui_forms/drawing/brush/path_gradient_brush.hpp:7`
- Definition: `src/core/drawing/brush/path_gradient_brush.cpp`

PathGradientBrush retains a bounded point boundary, center, center/surround colors, wrap policy, and optional interpolation stops.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `PathGradientBrush` (public)

```cpp
explicit PathGradientBrush(std::span<const PointF> points, WrapMode wrap_mode = WrapMode::clamp)
```

Requires a bounded finite polygon and valid wrap policy, then derives its initial center.

### `set_center_color` (public)

```cpp
void set_center_color(Color color)
```

Commits the center color under liveness rules.

### `set_center_point` (public)

```cpp
void set_center_point(PointF point)
```

Validates and commits a finite center.

### `set_surround_colors` (public)

```cpp
void set_surround_colors(std::span<const Color> colors)
```

Requires a nonempty bounded color collection compatible with the boundary.

### `set_interpolation_colors` (public)

```cpp
void set_interpolation_colors(const ColorBlend& blend)
```

Validates and owns explicit interpolation colors/positions.

### `snapshot` (public)

```cpp
[[nodiscard]] BrushSnapshot snapshot() const override
```

Requires liveness and copies the complete path-gradient recipe.
