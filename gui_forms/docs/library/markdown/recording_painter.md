# RecordingPainter

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Painter → RecordingPainter`  
Declaration: `src/core/display_chunk.hpp:77`  
Definition: `src/core/display_chunk.cpp`

RecordingPainter is a class declared in src/core/display_chunk.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `save`

```cpp
void save() override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `restore`

```cpp
void restore() override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `translate`

```cpp
void translate(Point offset) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clip_rect`

```cpp
void clip_rect(Rect rect) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clip_rounded_rect`

```cpp
void clip_rounded_rect(Rect rect, double radius) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_rect`

```cpp
void fill_rect(Rect rect, Color color) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_rounded_rect`

```cpp
void fill_rounded_rect(Rect rect, double radius, Color color) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `stroke_rect`

```cpp
void stroke_rect(Rect rect, Color color, double width) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `stroke_rounded_rect`

```cpp
void stroke_rounded_rect(Rect rect, double radius, Color color, double width) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_linear_gradient`

```cpp
void fill_linear_gradient( Rect rect, Point start, Point end, std::span<const GradientStop> stops) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_linear_gradient_spread`

```cpp
void fill_linear_gradient_spread( Rect rect, Point start, Point end, std::span<const GradientStop> stops, GradientSpreadMode spread) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_radial_gradient`

```cpp
void fill_radial_gradient( Rect rect, Point center, Size radii, std::span<const GradientStop> stops) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_box_shadow`

```cpp
void draw_box_shadow(Rect rect, double corner_radius, Point offset, double blur_radius, double spread, Color color) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_line`

```cpp
void draw_line(Point from, Point to, Color color, double width) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_text_utf8`

```cpp
void draw_text_utf8(Point origin, std::string_view text, FontSpec font, Color color) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `measure_text_utf8`

```cpp
[[nodiscard]] Size measure_text_utf8(std::string_view text, FontSpec font) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_image`

```cpp
void draw_image(ImageId image, Rect destination, double opacity) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_live_surface`

```cpp
void draw_live_surface(std::shared_ptr<LiveSurface> surface, Rect destination, double opacity) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_image_region`

```cpp
void draw_image_region(ImageId image, Rect source, Rect destination, double opacity) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_image_region_sampled`

```cpp
void draw_image_region_sampled(ImageId image, Rect source, Rect destination, ImageSampling sampling, double opacity) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_image_pattern`

```cpp
void fill_image_pattern(ImageId image, Size source_pixel_size, Rect destination, Size logical_tile_size, ImagePatternWrap wrap, double opacity) override
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `finish`

```cpp
[[nodiscard]] std::shared_ptr<const DisplayChunk> finish( std::uint64_t generation, PaintPlane plane, Rect logical_bounds)
```

Public RecordingPainter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
