# ImageAttributes

- Status: **OBSERVED: bundle 009 image-attribute split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `DrawingObject → ImageAttributes`
- Declaration: `include/gui_forms/drawing/image_attributes/image_attributes.hpp:25`
- Definition: `src/core/drawing/image_attributes/image_attributes.cpp`

ImageAttributes retains an optional finite 5x5 color matrix and bounded unique remap table under DrawingObject lifetime rules.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `set_color_matrix` (public)

```cpp
void set_color_matrix(std::span<const double, 25> matrix)
```

Validates every coefficient is finite and commits the complete 5x5 matrix.

### `reset_color_matrix` (public)

```cpp
void reset_color_matrix()
```

Disables and clears the color-matrix transform.

### `set_remap_table` (public)

```cpp
void set_remap_table(std::span<const ImageAttributesSnapshot::ColorRemap> table)
```

Validates bounded unique old colors and owns the complete remap collection.

### `reset_remap_table` (public)

```cpp
void reset_remap_table()
```

Clears retained remapping.

### `clone` (public)

```cpp
[[nodiscard]] std::unique_ptr<ImageAttributes> clone() const
```

Creates an independent live ImageAttributes with identical state.

### `snapshot` (public)

```cpp
[[nodiscard]] ImageAttributesSnapshot snapshot() const
```

Requires liveness and copies matrix and remap state.
