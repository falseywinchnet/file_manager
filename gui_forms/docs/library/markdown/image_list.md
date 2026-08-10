# ImageList

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Component → ImageList`  
Declaration: `include/gui_forms/image_list.hpp:65`  
Definition: `src/controls/image_list.cpp`

ImageList is a class declared in include/gui_forms/image_list.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ImageList`

```cpp
explicit ImageList(Window& window, Size image_size =
```

Constructs or tears down the retained ImageList object according to its ownership contract.

### `~ImageList`

```cpp
~ImageList() override
```

Constructs or tears down the retained ImageList object according to its ownership contract.

### `bound_window`

```cpp
[[nodiscard]] Window* bound_window() const noexcept
```

Reports the current bound window value without mutation.

### `belongs_to`

```cpp
[[nodiscard]] bool belongs_to(const Window& window) const noexcept
```

Reports the current belongs to value without mutation.

### `image_size`

```cpp
[[nodiscard]] Size image_size() const noexcept
```

Reports the current image size value without mutation.

### `set_image_size`

```cpp
void set_image_size(Size image_size)
```

Synchronously updates the retained image size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `count`

```cpp
[[nodiscard]] std::size_t count() const noexcept
```

Reports the current count value without mutation.

### `empty`

```cpp
[[nodiscard]] bool empty() const noexcept
```

Reports the current empty value without mutation.

### `revision`

```cpp
[[nodiscard]] std::uint64_t revision() const noexcept
```

Reports the current revision value without mutation.

### `add_png`

```cpp
[[nodiscard]] ImageLoadResult add_png( std::string key, std::span<const std::byte> encoded, double density_scale = 1.0)
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_variant_png`

```cpp
[[nodiscard]] ImageLoadResult set_variant_png( std::string_view key, ImageVisualState state, double density_scale, std::span<const std::byte> encoded)
```

Synchronously updates the retained variant png property. Validation, typed invalidation, and notifications are defined by the implementation.

### `add_image`

```cpp
std::size_t add_image(std::string key, ImageId image, double density_scale = 1.0)
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_variant_image`

```cpp
void set_variant_image(std::string_view key, ImageVisualState state, double density_scale, ImageId image)
```

Synchronously updates the retained variant image property. Validation, typed invalidation, and notifications are defined by the implementation.

### `key_at`

```cpp
[[nodiscard]] std::string_view key_at(std::size_t index) const
```

Reports the current key at value without mutation.

### `index_of_key`

```cpp
[[nodiscard]] std::optional<std::size_t> index_of_key( std::string_view key) const noexcept
```

Reports the current index of key value without mutation.

### `contains_key`

```cpp
[[nodiscard]] bool contains_key(std::string_view key) const noexcept
```

Reports the current contains key value without mutation.

### `set_key_name`

```cpp
void set_key_name(std::size_t index, std::string key)
```

Synchronously updates the retained key name property. Validation, typed invalidation, and notifications are defined by the implementation.

### `remove_at`

```cpp
bool remove_at(std::size_t index)
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_by_key`

```cpp
bool remove_by_key(std::string_view key)
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear`

```cpp
void clear()
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `resolve`

```cpp
[[nodiscard]] ImageListResolution resolve( std::size_t index, ImageVisualState state = ImageVisualState::normal, double density_scale = 1.0) const noexcept
```

Reports the current resolve value without mutation.

### `resolve`

```cpp
[[nodiscard]] ImageListResolution resolve( std::string_view key, ImageVisualState state = ImageVisualState::normal, double density_scale = 1.0) const noexcept
```

Reports the current resolve value without mutation.

### `tag`

```cpp
[[nodiscard]] const std::any& tag() const noexcept
```

Reports the current tag value without mutation.

### `set_tag`

```cpp
void set_tag(std::any tag)
```

Synchronously updates the retained tag property. Validation, typed invalidation, and notifications are defined by the implementation.

### `clear_tag`

```cpp
void clear_tag()
```

Removes the explicit tag value and restores fallback behavior.

### `changed`

```cpp
[[nodiscard]] Event<const ImageListChange&>& changed() noexcept
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
