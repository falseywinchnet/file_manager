# Pen

- Status: **OBSERVED: bundle 009 pen split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `DrawingObject → Pen`
- Declaration: `include/gui_forms/drawing/pen/pen.hpp:14`
- Definition: `src/core/drawing/pen/pen.cpp`

Pen is a disposable stroke recipe with bounded positive width and closed or validated custom dash policy.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `Pen` (public)

```cpp
explicit Pen(Color color, double width = 1.0)
```

Constructs from a color or a brush's primary projected color and validates width.

### `Pen` (public)

```cpp
explicit Pen(const Brush& brush, double width = 1.0)
```

Constructs from a color or a brush's primary projected color and validates width.

### `set_width` (public)

```cpp
void set_width(double width)
```

Requires liveness and commits a finite positive bounded width.

### `set_dash_style` (public)

```cpp
void set_dash_style(DashStyle style)
```

Validates the closed dash vocabulary and clears incompatible custom data.

### `set_dash_pattern` (public)

```cpp
void set_dash_pattern(std::span<const double> pattern)
```

Validates a bounded even/positive custom sequence and selects custom dash mode.

### `snapshot` (public)

```cpp
[[nodiscard]] PenSnapshot snapshot() const
```

Requires liveness and copies color, width, dash style, and owned custom pattern.
