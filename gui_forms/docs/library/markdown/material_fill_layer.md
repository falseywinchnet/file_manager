# MaterialFillLayer

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `MaterialFillLayer`  
Declaration: `include/gui_forms/surface_material.hpp:29`  
Definition: `src/core/theme.cpp`

MaterialFillLayer is a struct declared in include/gui_forms/surface_material.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `solid`

```cpp
[[nodiscard]] static MaterialFillLayer solid(Color color)
```

Public MaterialFillLayer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `linear`

```cpp
[[nodiscard]] static MaterialFillLayer linear( Point start, Point end, std::vector<GradientStop> stops, MaterialCoordinateSpace space = MaterialCoordinateSpace::normalized, GradientSpreadMode spread = GradientSpreadMode::pad)
```

Public MaterialFillLayer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `repeating_linear`

```cpp
[[nodiscard]] static MaterialFillLayer repeating_linear( Point start, Point end, std::vector<GradientStop> stops, MaterialCoordinateSpace space = MaterialCoordinateSpace::logical)
```

Public MaterialFillLayer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `radial`

```cpp
[[nodiscard]] static MaterialFillLayer radial( Point center, Size radii, std::vector<GradientStop> stops, MaterialCoordinateSpace space = MaterialCoordinateSpace::normalized)
```

Public MaterialFillLayer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `stretched_image`

```cpp
[[nodiscard]] static MaterialFillLayer stretched_image( ImageId image, Size pixel_size, double opacity = 1.0)
```

Public MaterialFillLayer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `tiled_image`

```cpp
[[nodiscard]] static MaterialFillLayer tiled_image( ImageId image, Size pixel_size, double source_pixels_per_logical_pixel = 1.0, double opacity = 1.0)
```

Public MaterialFillLayer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `nine_patch`

```cpp
[[nodiscard]] static MaterialFillLayer nine_patch( ImageId image, Size pixel_size, Insets source_slice, double source_pixels_per_logical_pixel = 1.0, double opacity = 1.0)
```

Public MaterialFillLayer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `operator==`

```cpp
friend bool operator==(const MaterialFillLayer&, const MaterialFillLayer&) = default
```

Public MaterialFillLayer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
