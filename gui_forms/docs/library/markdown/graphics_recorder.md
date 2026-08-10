# GraphicsRecorder

- Status: **OBSERVED: bundle 009 command-recorder state-machine split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `DrawingObject → GraphicsRecorder`
- Declaration: `include/gui_forms/drawing/graphics_recorder/graphics_recorder.hpp:84`
- Definition: `src/core/drawing/graphics_recorder/graphics_recorder.cpp`

GraphicsRecorder is a bounded thread-affine command transaction with explicit save-token stack discipline, retained transform/clip/quality state, validated resource snapshots, deterministic trace output, and a terminal close phase.

## Visual evidence

![GraphicsRecorder](../captures/drawing_raster_material.png)

## Declared methods

### `GraphicsRecorder` (public)

```cpp
GraphicsRecorder() = default
```

Constructs an open recorder with identity transform and default quality state.

### `save` (public)

```cpp
[[nodiscard]] GraphicsStateToken save()
```

Enforces stack capacity, allocates a nonzero token, retains current state, and records the save command.

### `restore` (public)

```cpp
void restore(GraphicsStateToken token)
```

Requires LIFO exact-token ownership, restores state, and records the transition.

### `translate` (public)

```cpp
void translate(double x, double y)
```

Validates and composes translation before recording it.

### `set_transform` (public)

```cpp
void set_transform(Matrix transform)
```

Requires a finite matrix, commits it, and records the state change.

### `set_clip` (public)

```cpp
void set_clip(RectF clip)
```

Validates and commits a finite clip rectangle.

### `reset_clip` (public)

```cpp
void reset_clip()
```

Clears retained clipping and records the reset.

### `set_quality` (public)

```cpp
void set_quality(SmoothingMode smoothing, InterpolationMode interpolation, PixelOffsetMode pixel_offset, CompositingMode compositing, CompositingQuality compositing_quality)
```

Validates every closed quality/compositing vocabulary and records their atomic state update.

### `current_state` (public)

```cpp
[[nodiscard]] GraphicsState current_state() const
```

Requires an open live recorder and returns transform, clip, and quality state.

### `is_visible` (public)

```cpp
[[nodiscard]] bool is_visible(PointF point) const
```

Tests a point against current optional clip.

### `measure_string` (public)

```cpp
[[nodiscard]] SizeF measure_string(std::string_view utf8, const Font& font, const StringFormat& format, const TextMetricsProvider& provider) const
```

Validates UTF-8/resource liveness and delegates immutable snapshots to the supplied metrics provider.

### `clear` (public)

```cpp
void clear(Color color)
```

Records a surface clear color.

### `fill_rectangle` (public)

```cpp
void fill_rectangle(const Brush& brush, RectF rect)
```

Validates bounds and snapshots a live brush into a fill command.

### `draw_rectangle` (public)

```cpp
void draw_rectangle(const Pen& pen, RectF rect)
```

Validates bounds and snapshots a live pen into a stroke command.

### `draw_line` (public)

```cpp
void draw_line(const Pen& pen, PointF from, PointF to)
```

Validates endpoints and snapshots a live pen.

### `draw_string` (public)

```cpp
void draw_string(std::string_view utf8, const Font& font, const SolidBrush& brush, PointF origin, const StringFormat& format)
```

Validates bounded UTF-8 and origin, then owns text plus font/brush/format snapshots.

### `draw_ellipse` (public)

```cpp
void draw_ellipse(const Pen& pen, RectF bounds)
```

Validates bounds and records a pen snapshot.

### `fill_ellipse` (public)

```cpp
void fill_ellipse(const Brush& brush, RectF bounds)
```

Validates bounds and records a brush snapshot.

### `fill_polygon` (public)

```cpp
void fill_polygon(const Brush& brush, std::span<const PointF> points, FillMode fill_mode = FillMode::alternate)
```

Validates bounded finite points/fill rule and owns points plus brush snapshot.

### `draw_path` (public)

```cpp
void draw_path(const Pen& pen, const GraphicsPath& path)
```

Owns live pen and path snapshots.

### `fill_path` (public)

```cpp
void fill_path(const Brush& brush, const GraphicsPath& path)
```

Owns live brush and path snapshots.

### `draw_image` (public)

```cpp
void draw_image(const ImageReference& image, RectF destination, RectF source, const ImageAttributes& attributes)
```

Owns ImageReference or Bitmap snapshot, validated source/destination geometry, and ImageAttributes snapshot.

### `draw_image` (public)

```cpp
void draw_image(const Bitmap& image, RectF destination, RectF source, const ImageAttributes& attributes)
```

Owns ImageReference or Bitmap snapshot, validated source/destination geometry, and ImageAttributes snapshot.

### `close` (public)

```cpp
void close()
```

Requires balanced saved state and enters terminal closed state.

### `closed` (public)

```cpp
[[nodiscard]] bool closed() const
```

Reports terminal recorder state.

### `commands` (public)

```cpp
[[nodiscard]] std::span<const DrawingCommand> commands() const
```

Returns the immutable recorded command span.

### `deterministic_trace` (public)

```cpp
[[nodiscard]] std::string deterministic_trace() const
```

Serializes state and commands with stable numeric/resource vocabulary for equivalence tests.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Clears commands/state stack and terminally closes without throwing.

### `require_recordable` (private)

```cpp
void require_recordable() const
```

Requires live, owner-thread, nonclosed state.

### `append` (private)

```cpp
void append(DrawingCommand command)
```

Enforces command capacity before committing one complete command.
