# ImageRegistry

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `ImageRegistry`  
Declaration: `include/gui_forms/resources.hpp:123`  
Definition: `src/core/resources.cpp`

ImageRegistry is a class declared in include/gui_forms/resources.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ImageRegistry`

```cpp
explicit ImageRegistry(ImageRegistryLimits limits =
```

Constructs or tears down the retained ImageRegistry object according to its ownership contract.

### `~ImageRegistry`

```cpp
~ImageRegistry()
```

Constructs or tears down the retained ImageRegistry object according to its ownership contract.

### `ImageRegistry`

```cpp
ImageRegistry(ImageRegistry&&) noexcept
```

Constructs or tears down the retained ImageRegistry object according to its ownership contract.

### `operator=`

```cpp
ImageRegistry& operator=(ImageRegistry&&) noexcept
```

Public ImageRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `ImageRegistry`

```cpp
ImageRegistry(const ImageRegistry&) = delete
```

Constructs or tears down the retained ImageRegistry object according to its ownership contract.

### `operator=`

```cpp
ImageRegistry& operator=(const ImageRegistry&) = delete
```

Public ImageRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `load_png`

```cpp
[[nodiscard]] ImageLoadResult load_png(std::span<const std::byte> encoded)
```

Public ImageRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `load_bgra32_premultiplied`

```cpp
[[nodiscard]] ImageLoadResult load_bgra32_premultiplied( std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels)
```

Public ImageRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `replace_png`

```cpp
[[nodiscard]] ImageLoadResult replace_png(ImageId image, std::span<const std::byte> encoded)
```

Public ImageRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `replace_bgra32_premultiplied`

```cpp
[[nodiscard]] ImageLoadResult replace_bgra32_premultiplied( ImageId image, std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels)
```

Public ImageRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `update_bgra32_premultiplied`

```cpp
[[nodiscard]] ImageLoadResult update_bgra32_premultiplied( ImageId image, std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels)
```

Public ImageRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `patch_bgra32_premultiplied`

```cpp
[[nodiscard]] ImageLoadResult patch_bgra32_premultiplied( ImageId image, std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height, std::uint64_t source_row_bytes, std::span<const std::byte> pixels)
```

Public ImageRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove`

```cpp
[[nodiscard]] bool remove(ImageId image) noexcept
```

Public ImageRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear`

```cpp
void clear() noexcept
```

Public ImageRegistry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `find`

```cpp
[[nodiscard]] std::optional<ImageResourceView> find(ImageId image) const noexcept
```

Reports the current find value without mutation.

### `image_ids`

```cpp
[[nodiscard]] std::vector<ImageId> image_ids() const
```

Reports the current image ids value without mutation.

### `snapshot`

```cpp
[[nodiscard]] ImageRegistrySnapshot snapshot() const noexcept
```

Reports the current snapshot value without mutation.

### `limits`

```cpp
[[nodiscard]] const ImageRegistryLimits& limits() const noexcept
```

Reports the current limits value without mutation.
