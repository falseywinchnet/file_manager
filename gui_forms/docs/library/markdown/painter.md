# Painter

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Painter`  
Declaration: `include/gui_forms/types.hpp:161`  
Definition: `src/core/types.cpp`

Painter is a class declared in include/gui_forms/types.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `~Painter`

```cpp
virtual ~Painter() = default
```

Constructs or tears down the retained Painter object according to its ownership contract.

### `save`

```cpp
virtual void save() = 0
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `restore`

```cpp
virtual void restore() = 0
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `translate`

```cpp
virtual void translate(Point offset) = 0
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clip_rect`

```cpp
virtual void clip_rect(Rect rect) = 0
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clip_rounded_rect`

```cpp
virtual void clip_rounded_rect(Rect rect, double radius)
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_rect`

```cpp
virtual void fill_rect(Rect rect, Color color) = 0
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_rounded_rect`

```cpp
virtual void fill_rounded_rect(Rect rect, double radius, Color color)
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `stroke_rect`

```cpp
virtual void stroke_rect(Rect rect, Color color, double width) = 0
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `stroke_rounded_rect`

```cpp
virtual void stroke_rounded_rect(Rect rect, double radius, Color color, double width)
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_linear_gradient`

```cpp
virtual void fill_linear_gradient( Rect rect, Point start, Point end, std::span<const GradientStop> stops)
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_linear_gradient_spread`

```cpp
virtual void fill_linear_gradient_spread( Rect rect, Point start, Point end, std::span<const GradientStop> stops, GradientSpreadMode spread)
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_radial_gradient`

```cpp
virtual void fill_radial_gradient( Rect rect, Point center, Size radii, std::span<const GradientStop> stops)
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_box_shadow`

```cpp
virtual void draw_box_shadow(Rect rect, double corner_radius, Point offset, double blur_radius, double spread, Color color)
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_line`

```cpp
virtual void draw_line(Point from, Point to, Color color, double width) = 0
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_text_utf8`

```cpp
virtual void draw_text_utf8(Point origin, std::string_view text, FontSpec font, Color color) = 0
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `measure_text_utf8`

```cpp
[[nodiscard]] virtual Size measure_text_utf8(std::string_view text, FontSpec font)
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_image`

```cpp
virtual void draw_image(ImageId image, Rect destination, double opacity = 1.0) = 0
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_live_surface`

```cpp
virtual void draw_live_surface(std::shared_ptr<LiveSurface> surface, Rect destination, double opacity = 1.0)
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_image_region`

```cpp
virtual void draw_image_region(ImageId image, Rect source, Rect destination, double opacity = 1.0)
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `draw_image_region_sampled`

```cpp
virtual void draw_image_region_sampled( ImageId image, Rect source, Rect destination, ImageSampling sampling, double opacity = 1.0)
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `fill_image_pattern`

```cpp
virtual void fill_image_pattern(ImageId image, Size source_pixel_size, Rect destination, Size logical_tile_size, ImagePatternWrap wrap = ImagePatternWrap::tile, double opacity = 1.0)
```

Public Painter operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
