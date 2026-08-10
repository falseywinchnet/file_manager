# MaterialFillLayer

- Status: **OBSERVED: bundle 012 isolated material-fill declaration/definition; native and MinGW M4 builds pass**
- Kind: **struct**
- Hierarchy: `MaterialFillLayer`
- Declaration: `include/gui_forms/surface_material/material_fill_layer/material_fill_layer.hpp:22`
- Definition: `src/core/surface_material/material_fill_layer/material_fill_layer.cpp`

MaterialFillLayer is one renderer-neutral solid, gradient, or image recipe with explicit coordinate space, spread/wrap, source geometry, scale, and opacity.

## Visual evidence

![MaterialFillLayer](../captures/drawing_raster_material.png)

## Declared methods

### `solid` (public)

```cpp
[[nodiscard]] static MaterialFillLayer solid(Color color)
```

Creates one solid-color layer.

### `linear` (public)

```cpp
[[nodiscard]] static MaterialFillLayer linear( Point start, Point end, std::vector<GradientStop> stops, MaterialCoordinateSpace space = MaterialCoordinateSpace::normalized, GradientSpreadMode spread = GradientSpreadMode::pad)
```

Creates a linear gradient with owned stops, coordinate space, and spread.

### `repeating_linear` (public)

```cpp
[[nodiscard]] static MaterialFillLayer repeating_linear( Point start, Point end, std::vector<GradientStop> stops, MaterialCoordinateSpace space = MaterialCoordinateSpace::logical)
```

Creates a logical-coordinate repeating linear gradient.

### `radial` (public)

```cpp
[[nodiscard]] static MaterialFillLayer radial( Point center, Size radii, std::vector<GradientStop> stops, MaterialCoordinateSpace space = MaterialCoordinateSpace::normalized)
```

Creates a radial gradient with owned stops and explicit center/radii space.

### `stretched_image` (public)

```cpp
[[nodiscard]] static MaterialFillLayer stretched_image( ImageId image, Size pixel_size, double opacity = 1.0)
```

Creates a stretched image layer with source pixel size and bounded opacity.

### `tiled_image` (public)

```cpp
[[nodiscard]] static MaterialFillLayer tiled_image( ImageId image, Size pixel_size, double source_pixels_per_logical_pixel = 1.0, double opacity = 1.0)
```

Creates a tiled image layer with explicit source-pixel scale.

### `nine_patch` (public)

```cpp
[[nodiscard]] static MaterialFillLayer nine_patch( ImageId image, Size pixel_size, Insets source_slice, double source_pixels_per_logical_pixel = 1.0, double opacity = 1.0)
```

Creates a nine-slice image layer with source insets and scale.

### `operator==` (public)

```cpp
friend bool operator==(const MaterialFillLayer&, const MaterialFillLayer&) = default
```

Compares the complete authored fill recipe.
