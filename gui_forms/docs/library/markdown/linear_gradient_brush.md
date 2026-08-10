# LinearGradientBrush

- Status: **OBSERVED: bundle 009 linear-gradient brush split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `Brush → LinearGradientBrush`
- Declaration: `include/gui_forms/drawing/brush/linear_gradient_brush.hpp:7`
- Definition: `src/core/drawing/brush/linear_gradient_brush.cpp`

LinearGradientBrush retains bounded geometry, angle, wrap, and either factor-derived or explicit interpolation stops.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `LinearGradientBrush` (public)

```cpp
LinearGradientBrush(RectF bounds, Color first, Color second, double angle = 0.0, WrapMode wrap_mode = WrapMode::tile)
```

Validates bounds, angle, and wrap before retaining endpoint colors.

### `set_blend` (public)

```cpp
void set_blend(std::span<const double> factors, std::span<const double> positions)
```

Validates equal bounded monotonic factor/position spans and derives interpolation colors.

### `set_interpolation_colors` (public)

```cpp
void set_interpolation_colors(const ColorBlend& blend)
```

Validates and owns explicit colors and positions.

### `set_wrap_mode` (public)

```cpp
void set_wrap_mode(WrapMode mode)
```

Validates and commits the closed wrap vocabulary.

### `snapshot` (public)

```cpp
[[nodiscard]] BrushSnapshot snapshot() const override
```

Requires liveness and copies the complete linear-gradient recipe.
