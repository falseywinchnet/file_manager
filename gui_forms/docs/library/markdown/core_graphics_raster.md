# CoreGraphicsRaster

- Status: **OBSERVED: bundle 009 CoreGraphics control raster split; focused M4 smoke tests pass**
- Kind: **class**
- Hierarchy: `Painter → CoreGraphicsRaster`
- Declaration: `src/render/coregraphics/raster/coregraphics_raster.hpp:14`
- Definition: `src/render/coregraphics/raster/coregraphics_raster.cpp`

CoreGraphicsRaster is the macOS comparison Painter terminal with the same CPU frame, image, text, clipping, material, and pixel-inspection contract as SkiaRaster, without entering the portable core.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `CoreGraphicsRaster` (public)

```cpp
CoreGraphicsRaster()
```

Allocates bounded CPU BGRA backing and CoreGraphics state; moves transfer ownership and copying is prohibited.

### `~CoreGraphicsRaster` (public)

```cpp
~CoreGraphicsRaster() override
```

Releases CoreGraphics contexts, paths, images, and typefaces.

### `CoreGraphicsRaster` (public)

```cpp
CoreGraphicsRaster(const CoreGraphicsRaster&) = delete
```

Allocates bounded CPU BGRA backing and CoreGraphics state; moves transfer ownership and copying is prohibited.

### `operator=` (public)

```cpp
CoreGraphicsRaster& operator=(const CoreGraphicsRaster&) = delete
```

Moves complete raster ownership; copying is prohibited.

### `resize` (public)

```cpp
bool resize(Size logical_size, double scale)
```

Validates and replaces backing dimensions and scale.

### `begin_frame` (public)

```cpp
void begin_frame(const DamageRegion& damage)
```

Begins one damage-clipped frame transaction.

### `end_frame` (public)

```cpp
void end_frame()
```

Requires balanced saved state and completes the transaction.

### `register_typeface` (public)

```cpp
[[nodiscard]] bool register_typeface(FontRole role, std::uint16_t weight, bool italic, std::span<const std::byte> encoded)
```

Validates and owns a bounded CoreText typeface by stable ID.

### `synchronize_images` (public)

```cpp
[[nodiscard]] bool synchronize_images(const ImageRegistry& registry)
```

Synchronizes generational PNG/BGRA registry resources and retires stale decodes.

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

Returns backing width.

### `pixel_height` (public)

```cpp
[[nodiscard]] std::uint32_t pixel_height() const noexcept
```

Returns backing height.

### `byte_size` (public)

```cpp
[[nodiscard]] std::size_t byte_size() const noexcept
```

Returns total backing byte extent.

### `save` (public)

```cpp
void save() override
```

Pushes CoreGraphics state during an active frame.

### `restore` (public)

```cpp
void restore() override
```

Pops a matched CoreGraphics state.

### `translate` (public)

```cpp
void translate(Point offset) override
```

Applies logical translation.

### `clip_rect` (public)

```cpp
void clip_rect(Rect rect) override
```

Intersects current clip with a rectangle.

### `clip_rounded_rect` (public)

```cpp
void clip_rounded_rect(Rect rect, double radius) override
```

Intersects current clip with a rounded path.

### `fill_rect` (public)

```cpp
void fill_rect(Rect rect, Color color) override
```

Fills a logical rectangle.

### `fill_rounded_rect` (public)

```cpp
void fill_rounded_rect(Rect rect, double radius, Color color) override
```

Fills rounded geometry.

### `stroke_rect` (public)

```cpp
void stroke_rect(Rect rect, Color color, double width) override
```

Strokes rectangle geometry.

### `stroke_rounded_rect` (public)

```cpp
void stroke_rounded_rect(Rect rect, double radius, Color color, double width) override
```

Strokes rounded geometry.

### `fill_linear_gradient` (public)

```cpp
void fill_linear_gradient( Rect rect, Point start, Point end, std::span<const GradientStop> stops) override
```

Realizes validated linear stops.

### `fill_linear_gradient_spread` (public)

```cpp
void fill_linear_gradient_spread( Rect rect, Point start, Point end, std::span<const GradientStop> stops, GradientSpreadMode spread) override
```

Realizes explicit linear spread policy.

### `fill_radial_gradient` (public)

```cpp
void fill_radial_gradient( Rect rect, Point center, Size radii, std::span<const GradientStop> stops) override
```

Realizes validated radial stops.

### `draw_box_shadow` (public)

```cpp
void draw_box_shadow(Rect rect, double corner_radius, Point offset, double blur_radius, double spread, Color color) override
```

Realizes bounded shadow geometry.

### `draw_line` (public)

```cpp
void draw_line(Point from, Point to, Color color, double width) override
```

Strokes a logical line.

### `draw_text_utf8` (public)

```cpp
void draw_text_utf8(Point origin, std::string_view text, FontSpec font, Color color) override
```

Draws UTF-8 through registered CoreText faces.

### `measure_text_utf8` (public)

```cpp
[[nodiscard]] Size measure_text_utf8(std::string_view text, FontSpec font) override
```

Returns logical UTF-8 extents.

### `draw_image` (public)

```cpp
void draw_image(ImageId image, Rect destination, double opacity) override
```

Draws a synchronized resource.

### `draw_image_region` (public)

```cpp
void draw_image_region(ImageId image, Rect source, Rect destination, double opacity) override
```

Draws a source region with default sampling.

### `draw_image_region_sampled` (public)

```cpp
void draw_image_region_sampled(ImageId image, Rect source, Rect destination, ImageSampling sampling, double opacity) override
```

Draws a source region with explicit sampling.

### `fill_image_pattern` (public)

```cpp
void fill_image_pattern(ImageId image, Size source_pixel_size, Rect destination, Size logical_tile_size, ImagePatternWrap wrap, double opacity) override
```

Realizes image pattern wrap policy.
