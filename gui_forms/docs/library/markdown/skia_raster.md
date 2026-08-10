# SkiaRaster

- Status: **OBSERVED: bundle 009 retained Skia raster split; focused M4 smoke and core tests pass**
- Kind: **class**
- Hierarchy: `Painter → SkiaRaster`
- Declaration: `src/render/skia/raster/skia_raster.hpp:12`
- Definition: `src/render/skia/raster/skia_raster.cpp`

SkiaRaster is GUI.Forms' private CPU-only Painter terminal with explicit frame bounds, state stack, rounded clips/materials, PNG registry synchronization, live-surface sampling, HarfBuzz fallback text, and caller-readable BGRA output.

## Visual evidence

![SkiaRaster](../captures/drawing_raster_material.png)

## Declared methods

### `SkiaRaster` (public)

```cpp
SkiaRaster()
```

Allocates a bounded CPU raster surface; moves transfer ownership and copying is prohibited.

### `~SkiaRaster` (public)

```cpp
~SkiaRaster() override
```

Releases private CPU surface, font, image, and shaping state.

### `SkiaRaster` (public)

```cpp
SkiaRaster(const SkiaRaster&) = delete
```

Allocates a bounded CPU raster surface; moves transfer ownership and copying is prohibited.

### `operator=` (public)

```cpp
SkiaRaster& operator=(const SkiaRaster&) = delete
```

Moves complete raster ownership; copying is prohibited.

### `resize` (public)

```cpp
bool resize(Size logical_size, double scale)
```

Validates and replaces CPU backing dimensions/scale while resetting frame state.

### `begin_frame` (public)

```cpp
void begin_frame(const DamageRegion& damage)
```

Begins one nonnested damage-clipped paint transaction.

### `end_frame` (public)

```cpp
void end_frame()
```

Requires balanced Painter save state and completes the active transaction.

### `register_typeface` (public)

```cpp
[[nodiscard]] bool register_typeface(FontRole role, std::uint16_t weight, bool italic, std::span<const std::byte> encoded)
```

Validates and registers primary font bytes for Skia/HarfBuzz use.

### `register_fallback_typeface` (public)

```cpp
[[nodiscard]] bool register_fallback_typeface( std::uint16_t weight, bool italic, std::span<const std::byte> encoded)
```

Registers a deterministic fallback face without exposing native objects.

### `synchronize_images` (public)

```cpp
[[nodiscard]] bool synchronize_images(const ImageRegistry& registry)
```

Synchronizes generational registry resources and retires stale decoded images.

### `pixels` (public)

```cpp
[[nodiscard]] const void* pixels() const noexcept
```

Returns const CPU backing bytes.

### `row_bytes` (public)

```cpp
[[nodiscard]] std::size_t row_bytes() const noexcept
```

Returns backing stride.

### `pixel_width` (public)

```cpp
[[nodiscard]] std::uint32_t pixel_width() const noexcept
```

Returns backing pixel width.

### `pixel_height` (public)

```cpp
[[nodiscard]] std::uint32_t pixel_height() const noexcept
```

Returns backing pixel height.

### `byte_size` (public)

```cpp
[[nodiscard]] std::size_t byte_size() const noexcept
```

Returns total backing byte extent.

### `save` (public)

```cpp
void save() override
```

Pushes Painter state during an active frame.

### `restore` (public)

```cpp
void restore() override
```

Pops a matched Painter state.

### `translate` (public)

```cpp
void translate(Point offset) override
```

Applies logical translation to current canvas state.

### `clip_rect` (public)

```cpp
void clip_rect(Rect rect) override
```

Intersects current clip with an axis-aligned rectangle.

### `clip_rounded_rect` (public)

```cpp
void clip_rounded_rect(Rect rect, double radius) override
```

Intersects current clip with a rounded rectangle.

### `fill_rect` (public)

```cpp
void fill_rect(Rect rect, Color color) override
```

Fills a logical rectangle with straight-alpha Color.

### `fill_rounded_rect` (public)

```cpp
void fill_rounded_rect(Rect rect, double radius, Color color) override
```

Fills validated rounded geometry.

### `stroke_rect` (public)

```cpp
void stroke_rect(Rect rect, Color color, double width) override
```

Strokes validated rectangle geometry and width.

### `stroke_rounded_rect` (public)

```cpp
void stroke_rounded_rect(Rect rect, double radius, Color color, double width) override
```

Strokes rounded geometry.

### `fill_linear_gradient` (public)

```cpp
void fill_linear_gradient( Rect rect, Point start, Point end, std::span<const GradientStop> stops) override
```

Realizes validated linear gradient stops.

### `fill_linear_gradient_spread` (public)

```cpp
void fill_linear_gradient_spread( Rect rect, Point start, Point end, std::span<const GradientStop> stops, GradientSpreadMode spread) override
```

Realizes pad, repeat, or reflect linear spread.

### `fill_radial_gradient` (public)

```cpp
void fill_radial_gradient( Rect rect, Point center, Size radii, std::span<const GradientStop> stops) override
```

Realizes validated radial gradient geometry/stops.

### `draw_box_shadow` (public)

```cpp
void draw_box_shadow(Rect rect, double corner_radius, Point offset, double blur_radius, double spread, Color color) override
```

Realizes bounded CPU blur/spread shadow geometry.

### `draw_line` (public)

```cpp
void draw_line(Point from, Point to, Color color, double width) override
```

Strokes a validated logical line.

### `draw_text_utf8` (public)

```cpp
void draw_text_utf8(Point origin, std::string_view text, FontSpec font, Color color) override
```

Shapes and draws UTF-8 through registered primary/fallback faces.

### `measure_text_utf8` (public)

```cpp
[[nodiscard]] Size measure_text_utf8(std::string_view text, FontSpec font) override
```

Shapes and returns logical text extents without drawing.

### `draw_image` (public)

```cpp
void draw_image(ImageId image, Rect destination, double opacity) override
```

Draws a synchronized generational image resource.

### `draw_live_surface` (public)

```cpp
void draw_live_surface(std::shared_ptr<LiveSurface> surface, Rect destination, double opacity) override
```

Samples the newest immutable LiveSurface frame without blocking its producer.

### `draw_image_region` (public)

```cpp
void draw_image_region(ImageId image, Rect source, Rect destination, double opacity) override
```

Draws a source region with default linear sampling.

### `draw_image_region_sampled` (public)

```cpp
void draw_image_region_sampled(ImageId image, Rect source, Rect destination, ImageSampling sampling, double opacity) override
```

Draws a source region with explicit nearest/linear policy.

### `fill_image_pattern` (public)

```cpp
void fill_image_pattern(ImageId image, Size source_pixel_size, Rect destination, Size logical_tile_size, ImagePatternWrap wrap, double opacity) override
```

Realizes tiled/flipped/clamped image patterns.
