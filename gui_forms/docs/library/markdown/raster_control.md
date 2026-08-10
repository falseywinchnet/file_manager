# RasterControl

- Status: **generated inventory; detailed review pending**
- Kind: **class / visual retained control**
- Hierarchy: `ScrollableControl → RasterControl`
- Declaration: `src/abi/control_adapters/abi_control_adapters.hpp:576`
- Definition: `inline/header-only`

RasterControl is a visual retained control declared in src/abi/control_adapters/abi_control_adapters.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `RasterControl` (public)

```cpp
explicit RasterControl(StableId stable_id, bool input_transparent = false) : ScrollableControl(std::move(stable_id)), input_transparent_(input_transparent)
```

Constructs or tears down the retained RasterControl object according to its ownership contract.

### `hit_test_local` (public)

```cpp
[[nodiscard]] bool hit_test_local(gui_forms::Point point) const override
```

Reports the current hit test local value without mutation.

### `pointer_input` (public)

```cpp
[[nodiscard]] gui_forms::Event<const RasterPointerSample&>& pointer_input() noexcept
```

Public RasterControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `key_input` (public)

```cpp
[[nodiscard]] gui_forms::Event<const RasterKeySample&>& key_input() noexcept
```

Public RasterControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_png` (public)

```cpp
bool set_png(std::span<const std::byte> encoded)
```

Synchronously updates the retained png property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_bgra32_premultiplied` (public)

```cpp
bool set_bgra32_premultiplied(std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels)
```

Synchronously updates the retained bgra32 premultiplied property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_live_surface` (public)

```cpp
void set_live_surface(std::shared_ptr<gui_forms::LiveSurface> surface)
```

Synchronously updates the retained live surface property. Validation, typed invalidation, and notifications are defined by the implementation.

### `clear_live_surface` (public)

```cpp
void clear_live_surface(const std::shared_ptr<gui_forms::LiveSurface>& surface)
```

Removes the explicit live surface value and restores fallback behavior.

### `on_paint` (public)

```cpp
void on_paint(gui_forms::Painter& painter, Rect) override
```

Records renderer-neutral paint operations for the damaged local region.

### `on_pointer` (public)

```cpp
void on_pointer(gui_forms::PointerEvent& event) override
```

Consumes normalized routed pointer input and updates retained interaction state.

### `on_key` (public)

```cpp
void on_key(gui_forms::KeyEvent& event) override
```

Consumes normalized keyboard input for this control's interaction contract.

### `on_attached_to_window` (protected)

```cpp
void on_attached_to_window() override
```

Public RasterControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_detached_from_window` (protected)

```cpp
void on_detached_from_window() noexcept override
```

Public RasterControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `queue_live_surface_paint` (private)

```cpp
static void queue_live_surface_paint( const std::weak_ptr<RasterControl>& weak_target, const std::weak_ptr<LiveWakeState>& weak_state) noexcept
```

Public RasterControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `connect_live_surface_wake` (private)

```cpp
void connect_live_surface_wake()
```

Public RasterControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `disconnect_live_surface_wake` (private)

```cpp
void disconnect_live_surface_wake() noexcept
```

Public RasterControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
