# GraphicsRecorder

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `DrawingObject → GraphicsRecorder`  
Declaration: `include/gui_forms/drawing.hpp:804`  
Definition: `src/core/drawing.cpp`

GraphicsRecorder is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `GraphicsRecorder`

```cpp
GraphicsRecorder() = default
```

Constructs or tears down the retained GraphicsRecorder object according to its ownership contract.

### `save`

```cpp
[[nodiscard]] GraphicsStateToken save()
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `restore`

```cpp
void restore(GraphicsStateToken token)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `translate`

```cpp
void translate(double x, double y)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_transform`

```cpp
void set_transform(Matrix transform)
```

Synchronously updates the retained transform property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_clip`

```cpp
void set_clip(RectF clip)
```

Synchronously updates the retained clip property. Validation, typed invalidation, and notifications are defined by the implementation.

### `reset_clip`

```cpp
void reset_clip()
```

Returns clip to its inherited or default policy.

### `set_quality`

```cpp
void set_quality(SmoothingMode smoothing, InterpolationMode interpolation, PixelOffsetMode pixel_offset, CompositingMode compositing, CompositingQuality compositing_quality)
```

Synchronously updates the retained quality property. Validation, typed invalidation, and notifications are defined by the implementation.

### `current_state`

```cpp
[[nodiscard]] GraphicsState current_state() const
```

Reports the current current state value without mutation.

### `is_visible`

```cpp
[[nodiscard]] bool is_visible(PointF point) const
```

Reports the current is visible value without mutation.

### `measure_string`

```cpp
[[nodiscard]] SizeF measure_string(std::string_view utf8, const Font& font, const StringFormat& format, const TextMetricsProvider& provider) const
```

Reports the current measure string value without mutation.

### `clear`

```cpp
void clear(Color color)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_rectangle`

```cpp
void fill_rectangle(const Brush& brush, RectF rect)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_rectangle`

```cpp
void draw_rectangle(const Pen& pen, RectF rect)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_line`

```cpp
void draw_line(const Pen& pen, PointF from, PointF to)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_string`

```cpp
void draw_string(std::string_view utf8, const Font& font, const SolidBrush& brush, PointF origin, const StringFormat& format)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_ellipse`

```cpp
void draw_ellipse(const Pen& pen, RectF bounds)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_ellipse`

```cpp
void fill_ellipse(const Brush& brush, RectF bounds)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_polygon`

```cpp
void fill_polygon(const Brush& brush, std::span<const PointF> points, FillMode fill_mode = FillMode::alternate)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_path`

```cpp
void draw_path(const Pen& pen, const GraphicsPath& path)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_path`

```cpp
void fill_path(const Brush& brush, const GraphicsPath& path)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_image`

```cpp
void draw_image(const ImageReference& image, RectF destination, RectF source, const ImageAttributes& attributes)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_image`

```cpp
void draw_image(const Bitmap& image, RectF destination, RectF source, const ImageAttributes& attributes)
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `close`

```cpp
void close()
```

Public GraphicsRecorder operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `closed`

```cpp
[[nodiscard]] bool closed() const
```

Reports the current closed value without mutation.

### `commands`

```cpp
[[nodiscard]] std::span<const DrawingCommand> commands() const
```

Reports the current commands value without mutation.

### `deterministic_trace`

```cpp
[[nodiscard]] std::string deterministic_trace() const
```

Reports the current deterministic trace value without mutation.
