# Painter

- Status: **OBSERVED: bundle 011 renderer-neutral painting review**
- Kind: **class**
- Hierarchy: `Painter`
- Declaration: `include/gui_forms/types/painter/painter.hpp:14`
- Definition: `src/core/types/painter/painter.cpp`

Painter is the renderer-neutral command vocabulary for transforms, clips, solids, rounded geometry, gradients, shadows, text, registered images, live surfaces, sampled regions, and bounded patterns; rich operations provide coherent minimal-host fallbacks.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `~Painter` (public)

```cpp
virtual ~Painter() = default
```

Constructs or tears down the retained Painter object according to its ownership contract.

### `save` (public)

```cpp
virtual void save() = 0
```

Executes Painter's save operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `restore` (public)

```cpp
virtual void restore() = 0
```

Executes Painter's restore operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `translate` (public)

```cpp
virtual void translate(Point offset) = 0
```

Executes Painter's translate operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `clip_rect` (public)

```cpp
virtual void clip_rect(Rect rect) = 0
```

Executes Painter's clip rect operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `clip_rounded_rect` (public)

```cpp
virtual void clip_rounded_rect(Rect rect, double radius)
```

Executes Painter's clip rounded rect operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `fill_rect` (public)

```cpp
virtual void fill_rect(Rect rect, Color color) = 0
```

Executes Painter's fill rect operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `fill_rounded_rect` (public)

```cpp
virtual void fill_rounded_rect(Rect rect, double radius, Color color)
```

Executes Painter's fill rounded rect operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `stroke_rect` (public)

```cpp
virtual void stroke_rect(Rect rect, Color color, double width) = 0
```

Executes Painter's stroke rect operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `stroke_rounded_rect` (public)

```cpp
virtual void stroke_rounded_rect(Rect rect, double radius, Color color, double width)
```

Executes Painter's stroke rounded rect operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `fill_linear_gradient` (public)

```cpp
virtual void fill_linear_gradient( Rect rect, Point start, Point end, std::span<const GradientStop> stops)
```

Executes Painter's fill linear gradient operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `fill_linear_gradient_spread` (public)

```cpp
virtual void fill_linear_gradient_spread( Rect rect, Point start, Point end, std::span<const GradientStop> stops, GradientSpreadMode spread)
```

Executes Painter's fill linear gradient spread operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `fill_radial_gradient` (public)

```cpp
virtual void fill_radial_gradient( Rect rect, Point center, Size radii, std::span<const GradientStop> stops)
```

Executes Painter's fill radial gradient operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `draw_box_shadow` (public)

```cpp
virtual void draw_box_shadow(Rect rect, double corner_radius, Point offset, double blur_radius, double spread, Color color)
```

Executes Painter's draw box shadow operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `draw_line` (public)

```cpp
virtual void draw_line(Point from, Point to, Color color, double width) = 0
```

Executes Painter's draw line operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `draw_text_utf8` (public)

```cpp
virtual void draw_text_utf8(Point origin, std::string_view text, FontSpec font, Color color) = 0
```

Executes Painter's draw text utf8 operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `measure_text_utf8` (public)

```cpp
[[nodiscard]] virtual Size measure_text_utf8(std::string_view text, FontSpec font)
```

Executes Painter's measure text utf8 operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `draw_image` (public)

```cpp
virtual void draw_image(ImageId image, Rect destination, double opacity = 1.0) = 0
```

Executes Painter's draw image operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `draw_live_surface` (public)

```cpp
virtual void draw_live_surface(std::shared_ptr<LiveSurface> surface, Rect destination, double opacity = 1.0)
```

Executes Painter's draw live surface operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `draw_image_region` (public)

```cpp
virtual void draw_image_region(ImageId image, Rect source, Rect destination, double opacity = 1.0)
```

Executes Painter's draw image region operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `draw_image_region_sampled` (public)

```cpp
virtual void draw_image_region_sampled( ImageId image, Rect source, Rect destination, ImageSampling sampling, double opacity = 1.0)
```

Executes Painter's draw image region sampled operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `fill_image_pattern` (public)

```cpp
virtual void fill_image_pattern(ImageId image, Size source_pixel_size, Rect destination, Size logical_tile_size, ImagePatternWrap wrap = ImagePatternWrap::tile, double opacity = 1.0)
```

Executes Painter's fill image pattern operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
