# ImageAttributes

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `DrawingObject → ImageAttributes`  
Declaration: `include/gui_forms/drawing.hpp:608`  
Definition: `src/core/drawing.cpp`

ImageAttributes is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `set_color_matrix`

```cpp
void set_color_matrix(std::span<const double, 25> matrix)
```

Synchronously updates the retained color matrix property. Validation, typed invalidation, and notifications are defined by the implementation.

### `reset_color_matrix`

```cpp
void reset_color_matrix()
```

Returns color matrix to its inherited or default policy.

### `set_remap_table`

```cpp
void set_remap_table(std::span<const ImageAttributesSnapshot::ColorRemap> table)
```

Synchronously updates the retained remap table property. Validation, typed invalidation, and notifications are defined by the implementation.

### `reset_remap_table`

```cpp
void reset_remap_table()
```

Returns remap table to its inherited or default policy.

### `clone`

```cpp
[[nodiscard]] std::unique_ptr<ImageAttributes> clone() const
```

Reports the current clone value without mutation.

### `snapshot`

```cpp
[[nodiscard]] ImageAttributesSnapshot snapshot() const
```

Reports the current snapshot value without mutation.
