# SkiaRaster

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Painter → SkiaRaster`  
Declaration: `src/render/skia/skia_raster.hpp:12`  
Definition: `src/render/skia/skia_raster.cpp`

SkiaRaster is a class declared in src/render/skia/skia_raster.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `SkiaRaster`

```cpp
SkiaRaster()
```

Constructs or tears down the retained SkiaRaster object according to its ownership contract.

### `~SkiaRaster`

```cpp
~SkiaRaster() override
```

Constructs or tears down the retained SkiaRaster object according to its ownership contract.

### `SkiaRaster`

```cpp
SkiaRaster(const SkiaRaster&) = delete
```

Constructs or tears down the retained SkiaRaster object according to its ownership contract.

### `operator=`

```cpp
SkiaRaster& operator=(const SkiaRaster&) = delete
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `resize`

```cpp
bool resize(Size logical_size, double scale)
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `begin_frame`

```cpp
void begin_frame(const DamageRegion& damage)
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `end_frame`

```cpp
void end_frame()
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `register_typeface`

```cpp
[[nodiscard]] bool register_typeface(FontRole role, std::uint16_t weight, bool italic, std::span<const std::byte> encoded)
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `register_fallback_typeface`

```cpp
[[nodiscard]] bool register_fallback_typeface( std::uint16_t weight, bool italic, std::span<const std::byte> encoded)
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `synchronize_images`

```cpp
[[nodiscard]] bool synchronize_images(const ImageRegistry& registry)
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `pixels`

```cpp
[[nodiscard]] const void* pixels() const noexcept
```

Reports the current pixels value without mutation.

### `row_bytes`

```cpp
[[nodiscard]] std::size_t row_bytes() const noexcept
```

Reports the current row bytes value without mutation.

### `pixel_width`

```cpp
[[nodiscard]] std::uint32_t pixel_width() const noexcept
```

Reports the current pixel width value without mutation.

### `pixel_height`

```cpp
[[nodiscard]] std::uint32_t pixel_height() const noexcept
```

Reports the current pixel height value without mutation.

### `byte_size`

```cpp
[[nodiscard]] std::size_t byte_size() const noexcept
```

Reports the current byte size value without mutation.

### `save`

```cpp
void save() override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `restore`

```cpp
void restore() override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `translate`

```cpp
void translate(Point offset) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clip_rect`

```cpp
void clip_rect(Rect rect) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clip_rounded_rect`

```cpp
void clip_rounded_rect(Rect rect, double radius) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_rect`

```cpp
void fill_rect(Rect rect, Color color) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_rounded_rect`

```cpp
void fill_rounded_rect(Rect rect, double radius, Color color) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `stroke_rect`

```cpp
void stroke_rect(Rect rect, Color color, double width) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `stroke_rounded_rect`

```cpp
void stroke_rounded_rect(Rect rect, double radius, Color color, double width) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_linear_gradient`

```cpp
void fill_linear_gradient( Rect rect, Point start, Point end, std::span<const GradientStop> stops) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_linear_gradient_spread`

```cpp
void fill_linear_gradient_spread( Rect rect, Point start, Point end, std::span<const GradientStop> stops, GradientSpreadMode spread) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_radial_gradient`

```cpp
void fill_radial_gradient( Rect rect, Point center, Size radii, std::span<const GradientStop> stops) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_box_shadow`

```cpp
void draw_box_shadow(Rect rect, double corner_radius, Point offset, double blur_radius, double spread, Color color) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_line`

```cpp
void draw_line(Point from, Point to, Color color, double width) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_text_utf8`

```cpp
void draw_text_utf8(Point origin, std::string_view text, FontSpec font, Color color) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `measure_text_utf8`

```cpp
[[nodiscard]] Size measure_text_utf8(std::string_view text, FontSpec font) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_image`

```cpp
void draw_image(ImageId image, Rect destination, double opacity) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_live_surface`

```cpp
void draw_live_surface(std::shared_ptr<LiveSurface> surface, Rect destination, double opacity) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_image_region`

```cpp
void draw_image_region(ImageId image, Rect source, Rect destination, double opacity) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_image_region_sampled`

```cpp
void draw_image_region_sampled(ImageId image, Rect source, Rect destination, ImageSampling sampling, double opacity) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_image_pattern`

```cpp
void fill_image_pattern(ImageId image, Size source_pixel_size, Rect destination, Size logical_tile_size, ImagePatternWrap wrap, double opacity) override
```

Public SkiaRaster operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
