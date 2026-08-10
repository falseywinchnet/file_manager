# GraphicsPath

- Status: **OBSERVED: bundle 009 path state-machine split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `DrawingObject → GraphicsPath`
- Declaration: `include/gui_forms/drawing/graphics_path/graphics_path.hpp:40`
- Definition: `src/core/drawing/graphics_path/graphics_path.cpp`

GraphicsPath is a bounded retained vector-geometry state machine with explicit figure boundaries, line/quadratic/cubic/polygon/rectangle/ellipse/arc/pie vocabulary, affine transformation, hit testing, cloning, bounds, and immutable snapshots.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `GraphicsPath` (public)

```cpp
explicit GraphicsPath(FillMode fill_mode = FillMode::alternate)
```

Validates and retains the initial fill rule.

### `fill_mode` (public)

```cpp
[[nodiscard]] FillMode fill_mode() const
```

Requires liveness and returns alternate or winding fill policy.

### `set_fill_mode` (public)

```cpp
void set_fill_mode(FillMode fill_mode)
```

Validates and commits fill policy.

### `reset` (public)

```cpp
void reset()
```

Clears all retained elements while preserving fill policy.

### `start_figure` (public)

```cpp
void start_figure()
```

Appends an explicit new-figure marker.

### `close_figure` (public)

```cpp
void close_figure()
```

Appends a close marker when a figure is active.

### `add_line` (public)

```cpp
void add_line(PointF from, PointF to)
```

Validates and appends an authored line segment.

### `add_quadratic` (public)

```cpp
void add_quadratic(PointF from, PointF control, PointF to)
```

Validates and appends one quadratic Bézier segment.

### `add_bezier` (public)

```cpp
void add_bezier(PointF from, PointF control1, PointF control2, PointF to)
```

Validates and appends one cubic Bézier segment.

### `add_beziers` (public)

```cpp
void add_beziers(std::span<const PointF> points)
```

Validates the 3n+1 point law and appends cubic segments transactionally.

### `add_polygon` (public)

```cpp
void add_polygon(std::span<const PointF> points)
```

Validates a bounded polygon, starts it, appends lines, and closes it.

### `add_rectangle` (public)

```cpp
void add_rectangle(RectF rectangle)
```

Validates and appends a rectangle element.

### `add_ellipse` (public)

```cpp
void add_ellipse(RectF bounds)
```

Validates and appends an ellipse element.

### `add_arc` (public)

```cpp
void add_arc(RectF bounds, double start_angle, double sweep_angle)
```

Validates bounds/angles and appends an arc element.

### `add_pie` (public)

```cpp
void add_pie(RectF bounds, double start_angle, double sweep_angle)
```

Builds a closed center-to-arc pie figure from validated geometry.

### `add_path` (public)

```cpp
void add_path(const GraphicsPath& path, bool connect)
```

Copies another live snapshot with optional connection while enforcing total element capacity.

### `transform` (public)

```cpp
void transform(const Matrix& matrix)
```

Requires a finite matrix and transforms every retained geometry field.

### `is_visible` (public)

```cpp
[[nodiscard]] bool is_visible(PointF point) const
```

Projects a snapshot through deterministic fill-rule hit testing.

### `path_points` (public)

```cpp
[[nodiscard]] std::vector<PointF> path_points() const
```

Flattens retained element control/end points into an inspectable ordered vector.

### `clone` (public)

```cpp
[[nodiscard]] std::unique_ptr<GraphicsPath> clone() const
```

Creates an independent path with identical fill policy and elements.

### `bounds` (public)

```cpp
[[nodiscard]] RectF bounds() const
```

Computes conservative bounds across every retained geometry form.

### `snapshot` (public)

```cpp
[[nodiscard]] PathSnapshot snapshot() const
```

Copies fill rule and the complete immutable element sequence.

### `append` (private)

```cpp
void append(PathElement element)
```

Enforces maximum element capacity before committing one internal element.
