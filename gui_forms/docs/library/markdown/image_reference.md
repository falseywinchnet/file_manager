# ImageReference

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `DrawingObject → ImageReference`  
Declaration: `include/gui_forms/drawing.hpp:585`  
Definition: `src/core/drawing.cpp`

ImageReference is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ImageReference`

```cpp
ImageReference(std::uint64_t stable_id, std::uint32_t width, std::uint32_t height, PixelFormat pixel_format, std::uint64_t generation = 1)
```

Constructs or tears down the retained ImageReference object according to its ownership contract.

### `snapshot`

```cpp
[[nodiscard]] ImageSnapshot snapshot() const
```

Reports the current snapshot value without mutation.
