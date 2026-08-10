# ImageSnapshot

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `ImageSnapshot`  
Declaration: `include/gui_forms/drawing.hpp:315`  
Definition: `src/core/drawing.cpp`

ImageSnapshot is a struct declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `has_pixels`

```cpp
[[nodiscard]] bool has_pixels() const noexcept
```

Reports the current has pixels value without mutation.

### `row_bytes`

```cpp
[[nodiscard]] std::size_t row_bytes() const noexcept
```

Reports the current row bytes value without mutation.

### `pixels`

```cpp
[[nodiscard]] std::span<const std::byte> pixels() const noexcept
```

Reports the current pixels value without mutation.
