# GraphicsPath

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `DrawingObject → GraphicsPath`  
Declaration: `include/gui_forms/drawing.hpp:525`  
Definition: `src/core/drawing.cpp`

GraphicsPath is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `GraphicsPath`

```cpp
explicit GraphicsPath(FillMode fill_mode = FillMode::alternate)
```

Constructs or tears down the retained GraphicsPath object according to its ownership contract.

### `fill_mode`

```cpp
[[nodiscard]] FillMode fill_mode() const
```

Reports the current fill mode value without mutation.

### `set_fill_mode`

```cpp
void set_fill_mode(FillMode fill_mode)
```

Synchronously updates the retained fill mode property. Validation, typed invalidation, and notifications are defined by the implementation.

### `reset`

```cpp
void reset()
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `start_figure`

```cpp
void start_figure()
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `close_figure`

```cpp
void close_figure()
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add_line`

```cpp
void add_line(PointF from, PointF to)
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add_quadratic`

```cpp
void add_quadratic(PointF from, PointF control, PointF to)
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add_bezier`

```cpp
void add_bezier(PointF from, PointF control1, PointF control2, PointF to)
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add_beziers`

```cpp
void add_beziers(std::span<const PointF> points)
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add_polygon`

```cpp
void add_polygon(std::span<const PointF> points)
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add_rectangle`

```cpp
void add_rectangle(RectF rectangle)
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add_ellipse`

```cpp
void add_ellipse(RectF bounds)
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add_arc`

```cpp
void add_arc(RectF bounds, double start_angle, double sweep_angle)
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add_pie`

```cpp
void add_pie(RectF bounds, double start_angle, double sweep_angle)
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add_path`

```cpp
void add_path(const GraphicsPath& path, bool connect)
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `transform`

```cpp
void transform(const Matrix& matrix)
```

Public GraphicsPath operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `is_visible`

```cpp
[[nodiscard]] bool is_visible(PointF point) const
```

Reports the current is visible value without mutation.

### `path_points`

```cpp
[[nodiscard]] std::vector<PointF> path_points() const
```

Reports the current path points value without mutation.

### `clone`

```cpp
[[nodiscard]] std::unique_ptr<GraphicsPath> clone() const
```

Reports the current clone value without mutation.

### `bounds`

```cpp
[[nodiscard]] RectF bounds() const
```

Reports the current bounds value without mutation.

### `snapshot`

```cpp
[[nodiscard]] PathSnapshot snapshot() const
```

Reports the current snapshot value without mutation.
