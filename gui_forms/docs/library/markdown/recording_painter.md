# RecordingPainter

- Status: **OBSERVED: bundle 008 atomic recording review; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `Painter → RecordingPainter`
- Declaration: `src/core/display/recording_painter/recording_painter.hpp:10`
- Definition: `src/core/display/recording_painter/recording_painter.cpp`

RecordingPainter implements Painter by validating and owning renderer-neutral commands until finish seals one immutable DisplayChunk; no partial command reaches the terminal renderer.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `save` (public)

```cpp
void save() override
```

Records a state save and increments the balance depth.

### `restore` (public)

```cpp
void restore() override
```

Rejects an unmatched restore, records a valid restore, and decrements balance depth.

### `translate` (public)

```cpp
void translate(Point offset) override
```

Records a logical coordinate translation.

### `clip_rect` (public)

```cpp
void clip_rect(Rect rect) override
```

Records an axis-aligned clip rectangle.

### `clip_rounded_rect` (public)

```cpp
void clip_rounded_rect(Rect rect, double radius) override
```

Records a rounded clip with logical radius.

### `fill_rect` (public)

```cpp
void fill_rect(Rect rect, Color color) override
```

Records a solid rectangle fill.

### `fill_rounded_rect` (public)

```cpp
void fill_rounded_rect(Rect rect, double radius, Color color) override
```

Records a solid rounded-rectangle fill.

### `stroke_rect` (public)

```cpp
void stroke_rect(Rect rect, Color color, double width) override
```

Records a rectangle stroke and width.

### `stroke_rounded_rect` (public)

```cpp
void stroke_rounded_rect(Rect rect, double radius, Color color, double width) override
```

Records a rounded rectangle stroke, radius, color, and width.

### `fill_linear_gradient` (public)

```cpp
void fill_linear_gradient(Rect rect, Point start, Point end, std::span<const GradientStop> stops) override
```

Validates and owns gradient stops before recording linear geometry.

### `fill_linear_gradient_spread` (public)

```cpp
void fill_linear_gradient_spread( Rect rect, Point start, Point end, std::span<const GradientStop> stops, GradientSpreadMode spread) override
```

Validates stops and the closed spread vocabulary before recording.

### `fill_radial_gradient` (public)

```cpp
void fill_radial_gradient(Rect rect, Point center, Size radii, std::span<const GradientStop> stops) override
```

Validates and owns stops before recording center and radii.

### `draw_box_shadow` (public)

```cpp
void draw_box_shadow(Rect rect, double corner_radius, Point offset, double blur_radius, double spread, Color color) override
```

Records box bounds, corner radius, offset, blur, spread, and color.

### `draw_line` (public)

```cpp
void draw_line(Point from, Point to, Color color, double width) override
```

Records endpoints, color, and stroke width.

### `draw_text_utf8` (public)

```cpp
void draw_text_utf8(Point origin, std::string_view text, FontSpec font, Color color) override
```

Owns UTF-8 text with origin, font, and color.

### `measure_text_utf8` (public)

```cpp
[[nodiscard]] Size measure_text_utf8(std::string_view text, FontSpec font) override
```

Returns a deterministic scalar-count fallback because display recording deliberately does not own renderer text shaping.

### `draw_image` (public)

```cpp
void draw_image(ImageId image, Rect destination, double opacity) override
```

Records generational image identity, destination, and opacity.

### `draw_live_surface` (public)

```cpp
void draw_live_surface(std::shared_ptr<LiveSurface> surface, Rect destination, double opacity) override
```

Retains a live surface with destination and opacity for terminal sampling.

### `draw_image_region` (public)

```cpp
void draw_image_region(ImageId image, Rect source, Rect destination, double opacity) override
```

Records source/destination rectangles and opacity.

### `draw_image_region_sampled` (public)

```cpp
void draw_image_region_sampled(ImageId image, Rect source, Rect destination, ImageSampling sampling, double opacity) override
```

Records region geometry plus explicit sampling policy.

### `fill_image_pattern` (public)

```cpp
void fill_image_pattern(ImageId image, Size source_pixel_size, Rect destination, Size logical_tile_size, ImagePatternWrap wrap, double opacity) override
```

Records source size, tile size, wrap policy, destination, and opacity.

### `finish` (public)

```cpp
[[nodiscard]] std::shared_ptr<const DisplayChunk> finish( std::uint64_t generation, PaintPlane plane, Rect logical_bounds)
```

Rejects unbalanced save state and moves the complete command collection into an immutable DisplayChunk.
