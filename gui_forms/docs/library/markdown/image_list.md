# ImageList

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `Component → ImageList`
- Declaration: `include/gui_forms/image_list.hpp:65`
- Definition: `src/controls/image_list.cpp`

ImageList is a class declared in include/gui_forms/image_list.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `ImageList` (public)

```cpp
explicit ImageList(Window& window, Size image_size =
```

Constructs or tears down the retained ImageList object according to its ownership contract.

### `~ImageList` (public)

```cpp
~ImageList() override
```

Constructs or tears down the retained ImageList object according to its ownership contract.

### `bound_window` (public)

```cpp
[[nodiscard]] Window* bound_window() const noexcept
```

Reports the current bound window value without mutation.

### `belongs_to` (public)

```cpp
[[nodiscard]] bool belongs_to(const Window& window) const noexcept
```

Reports the current belongs to value without mutation.

### `image_size` (public)

```cpp
[[nodiscard]] Size image_size() const noexcept
```

Reports the current image size value without mutation.

### `set_image_size` (public)

```cpp
void set_image_size(Size image_size)
```

Synchronously updates the retained image size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `count` (public)

```cpp
[[nodiscard]] std::size_t count() const noexcept
```

Reports the current count value without mutation.

### `empty` (public)

```cpp
[[nodiscard]] bool empty() const noexcept
```

Reports the current empty value without mutation.

### `revision` (public)

```cpp
[[nodiscard]] std::uint64_t revision() const noexcept
```

Reports the current revision value without mutation.

### `add_png` (public)

```cpp
[[nodiscard]] ImageLoadResult add_png( std::string key, std::span<const std::byte> encoded, double density_scale = 1.0)
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_variant_png` (public)

```cpp
[[nodiscard]] ImageLoadResult set_variant_png( std::string_view key, ImageVisualState state, double density_scale, std::span<const std::byte> encoded)
```

Synchronously updates the retained variant png property. Validation, typed invalidation, and notifications are defined by the implementation.

### `add_image` (public)

```cpp
std::size_t add_image(std::string key, ImageId image, double density_scale = 1.0)
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_variant_image` (public)

```cpp
void set_variant_image(std::string_view key, ImageVisualState state, double density_scale, ImageId image)
```

Synchronously updates the retained variant image property. Validation, typed invalidation, and notifications are defined by the implementation.

### `key_at` (public)

```cpp
[[nodiscard]] std::string_view key_at(std::size_t index) const
```

Reports the current key at value without mutation.

### `index_of_key` (public)

```cpp
[[nodiscard]] std::optional<std::size_t> index_of_key( std::string_view key) const noexcept
```

Reports the current index of key value without mutation.

### `contains_key` (public)

```cpp
[[nodiscard]] bool contains_key(std::string_view key) const noexcept
```

Reports the current contains key value without mutation.

### `set_key_name` (public)

```cpp
void set_key_name(std::size_t index, std::string key)
```

Synchronously updates the retained key name property. Validation, typed invalidation, and notifications are defined by the implementation.

### `remove_at` (public)

```cpp
bool remove_at(std::size_t index)
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_by_key` (public)

```cpp
bool remove_by_key(std::string_view key)
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear` (public)

```cpp
void clear()
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `resolve` (public)

```cpp
[[nodiscard]] ImageListResolution resolve( std::size_t index, ImageVisualState state = ImageVisualState::normal, double density_scale = 1.0) const noexcept
```

Reports the current resolve value without mutation.

### `resolve` (public)

```cpp
[[nodiscard]] ImageListResolution resolve( std::string_view key, ImageVisualState state = ImageVisualState::normal, double density_scale = 1.0) const noexcept
```

Reports the current resolve value without mutation.

### `tag` (public)

```cpp
[[nodiscard]] const std::any& tag() const noexcept
```

Reports the current tag value without mutation.

### `set_tag` (public)

```cpp
void set_tag(std::any tag)
```

Synchronously updates the retained tag property. Validation, typed invalidation, and notifications are defined by the implementation.

### `clear_tag` (public)

```cpp
void clear_tag()
```

Removes the explicit tag value and restores fallback behavior.

### `changed` (public)

```cpp
[[nodiscard]] Event<const ImageListChange&>& changed() noexcept
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `verify_dispose_thread` (protected)

```cpp
void verify_dispose_thread() override
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `require_access` (private)

```cpp
void require_access(std::string_view operation) const
```

Reports the current require access value without mutation.

### `valid_state` (private)

```cpp
[[nodiscard]] static bool valid_state(ImageVisualState state) noexcept
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `validate_key` (private)

```cpp
static void validate_key(std::string_view key)
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `validate_density` (private)

```cpp
static void validate_density(double density_scale)
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `validate_image_size` (private)

```cpp
static void validate_image_size(Size image_size)
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `require_index` (private)

```cpp
[[nodiscard]] std::size_t require_index(std::string_view key) const
```

Reports the current require index value without mutation.

### `variant_index` (private)

```cpp
[[nodiscard]] std::optional<std::size_t> variant_index( const Entry& entry, ImageVisualState state, double density_scale) const noexcept
```

Reports the current variant index value without mutation.

### `source_size` (private)

```cpp
[[nodiscard]] Size source_size(ImageId image) const
```

Reports the current source size value without mutation.

### `release_entry` (private)

```cpp
void release_entry(Entry& entry) noexcept
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `release_all` (private)

```cpp
void release_all() noexcept
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `emit_change` (private)

```cpp
void emit_change(ImageListChangeKind kind, std::size_t index, std::string_view key)
```

Public ImageList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
