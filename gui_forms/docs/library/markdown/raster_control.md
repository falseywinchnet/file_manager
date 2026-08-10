# RasterControl

- Status: **OBSERVED: bundle 010 private ABI raster-control split; native and MinGW ABI builds pass**
- Kind: **class / visual retained control**
- Hierarchy: `ScrollableControl → RasterControl`
- Declaration: `src/abi/control_adapters/abi_control_adapters.hpp:576`
- Definition: `inline/header-only`

RasterControl is the compatibility owner-painted and scroll-capable retained surface for bounded PNG/BGRA frames or a generational LiveSurface, with input projection and revocable wake scheduling.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `RasterControl` (public)

```cpp
explicit RasterControl(StableId stable_id, bool input_transparent = false) : ScrollableControl(std::move(stable_id)), input_transparent_(input_transparent)
```

Constructs a scrollable compatibility surface.

### `hit_test_local` (public)

```cpp
[[nodiscard]] bool hit_test_local(gui_forms::Point point) const override
```

Uses retained geometry for ordinary input admission.

### `pointer_input` (public)

```cpp
[[nodiscard]] gui_forms::Event<const RasterPointerSample&>& pointer_input() noexcept
```

Returns projected pointer observation.

### `key_input` (public)

```cpp
[[nodiscard]] gui_forms::Event<const RasterKeySample&>& key_input() noexcept
```

Returns projected key observation.

### `set_png` (public)

```cpp
bool set_png(std::span<const std::byte> encoded)
```

Validates/stores bounded PNG through the Window image registry and switches from pixel/live sources.

### `set_bgra32_premultiplied` (public)

```cpp
bool set_bgra32_premultiplied(std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels)
```

Validates dimensions/stride/extent, copies immutable pixels, and switches from PNG/live sources.

### `set_live_surface` (public)

```cpp
void set_live_surface(std::shared_ptr<gui_forms::LiveSurface> surface)
```

Binds a generational frame source and refreshes wake integration.

### `clear_live_surface` (public)

```cpp
void clear_live_surface(const std::shared_ptr<gui_forms::LiveSurface>& surface)
```

Clears only the matching source and revokes its wake path.

### `on_paint` (public)

```cpp
void on_paint(gui_forms::Painter& painter, Rect) override
```

Selects the newest live frame, retained image, or copied pixels and paints within clipped bounds.

### `on_pointer` (public)

```cpp
void on_pointer(gui_forms::PointerEvent& event) override
```

Projects a core pointer event to compatibility callbacks.

### `on_key` (public)

```cpp
void on_key(gui_forms::KeyEvent& event) override
```

Projects a core key event and copies handled state back.

### `on_attached_to_window` (protected)

```cpp
void on_attached_to_window() override
```

Connects the revocable live wake once Window ownership is available.

### `on_detached_from_window` (protected)

```cpp
void on_detached_from_window() noexcept override
```

Revokes wake integration before base detachment.

### `queue_live_surface_paint` (private)

```cpp
static void queue_live_surface_paint( const std::weak_ptr<RasterControl>& weak_target, const std::weak_ptr<LiveWakeState>& weak_state) noexcept
```

Marshals a producer wake to owner-thread invalidation through Window dispatch.

### `connect_live_surface_wake` (private)

```cpp
void connect_live_surface_wake()
```

Installs one weak source/owner wake callback.

### `disconnect_live_surface_wake` (private)

```cpp
void disconnect_live_surface_wake() noexcept
```

Revokes and clears wake state.
