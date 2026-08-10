# Bitmap

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `DrawingObject → Bitmap`  
Declaration: `include/gui_forms/drawing.hpp:658`  
Definition: `src/core/drawing.cpp`

Bitmap is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Bitmap`

```cpp
Bitmap(std::uint32_t width, std::uint32_t height, PixelFormat pixel_format = PixelFormat::bgra32_premultiplied)
```

Constructs or tears down the retained Bitmap object according to its ownership contract.

### `width`

```cpp
[[nodiscard]] std::uint32_t width() const
```

Reports the current width value without mutation.

### `height`

```cpp
[[nodiscard]] std::uint32_t height() const
```

Reports the current height value without mutation.

### `pixel_format`

```cpp
[[nodiscard]] PixelFormat pixel_format() const
```

Reports the current pixel format value without mutation.

### `generation`

```cpp
[[nodiscard]] std::uint64_t generation() const
```

Reports the current generation value without mutation.

### `get_pixel`

```cpp
[[nodiscard]] Color get_pixel(std::uint32_t x, std::uint32_t y) const
```

Reports the current get pixel value without mutation.

### `set_pixel`

```cpp
void set_pixel(std::uint32_t x, std::uint32_t y, Color color)
```

Synchronously updates the retained pixel property. Validation, typed invalidation, and notifications are defined by the implementation.

### `make_transparent`

```cpp
void make_transparent(Color key)
```

Public Bitmap operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clone`

```cpp
[[nodiscard]] std::unique_ptr<Bitmap> clone(RectI source) const
```

Reports the current clone value without mutation.

### `thumbnail`

```cpp
[[nodiscard]] std::unique_ptr<Bitmap> thumbnail(std::uint32_t width, std::uint32_t height) const
```

Reports the current thumbnail value without mutation.

### `adjusted`

```cpp
[[nodiscard]] std::unique_ptr<Bitmap> adjusted( const ImageAttributes& attributes) const
```

Reports the current adjusted value without mutation.

### `lock`

```cpp
[[nodiscard]] BitmapLockView lock(BitmapLockMode mode)
```

Public Bitmap operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `unlock`

```cpp
void unlock(std::uint64_t token)
```

Public Bitmap operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `locked`

```cpp
[[nodiscard]] bool locked() const
```

Reports the current locked value without mutation.

### `begin_edit`

```cpp
[[nodiscard]] BitmapEditView begin_edit(RectI bounds)
```

Public Bitmap operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `commit_edit`

```cpp
[[nodiscard]] std::uint64_t commit_edit(std::uint64_t token)
```

Public Bitmap operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `cancel_edit`

```cpp
void cancel_edit(std::uint64_t token)
```

Public Bitmap operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `changes_since`

```cpp
[[nodiscard]] BitmapDamageSnapshot changes_since( std::uint64_t generation) const
```

Reports the current changes since value without mutation.

### `snapshot`

```cpp
[[nodiscard]] ImageSnapshot snapshot() const
```

Reports the current snapshot value without mutation.
