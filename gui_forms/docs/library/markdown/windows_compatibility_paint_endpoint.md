# WindowsCompatibilityPaintEndpoint

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `WindowsCompatibilityPaintEndpoint`
- Declaration: `include/gui_forms/platform/windows_host.hpp:56`
- Definition: `src/host/windows/application/windows_host.cpp`

WindowsCompatibilityPaintEndpoint is a class declared in include/gui_forms/platform/windows_host.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `acquire` (public)

```cpp
static std::shared_ptr<WindowsCompatibilityPaintEndpoint> acquire( std::uint32_t width, std::uint32_t height)
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `~WindowsCompatibilityPaintEndpoint` (public)

```cpp
~WindowsCompatibilityPaintEndpoint()
```

Constructs or tears down the retained WindowsCompatibilityPaintEndpoint object according to its ownership contract.

### `WindowsCompatibilityPaintEndpoint` (public)

```cpp
WindowsCompatibilityPaintEndpoint( const WindowsCompatibilityPaintEndpoint&) = delete
```

Constructs or tears down the retained WindowsCompatibilityPaintEndpoint object according to its ownership contract.

### `operator=` (public)

```cpp
WindowsCompatibilityPaintEndpoint& operator=( const WindowsCompatibilityPaintEndpoint&) = delete
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `compatibility_handle` (public)

```cpp
[[nodiscard]] std::uintptr_t compatibility_handle() const noexcept
```

Reports the current compatibility handle value without mutation.

### `device_context` (public)

```cpp
[[nodiscard]] std::uintptr_t device_context() const noexcept
```

Reports the current device context value without mutation.

### `live_surface` (public)

```cpp
[[nodiscard]] std::shared_ptr<LiveSurface> live_surface() const noexcept
```

Reports the current live surface value without mutation.

### `publish_device_context` (public)

```cpp
[[nodiscard]] bool publish_device_context( std::uintptr_t device_context) noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `begin_device_context_write` (public)

```cpp
[[nodiscard]] bool begin_device_context_write( std::uintptr_t device_context) noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `end_device_context_write` (public)

```cpp
[[nodiscard]] bool end_device_context_write( std::uintptr_t device_context, bool publish) noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `configure` (public)

```cpp
[[nodiscard]] bool configure(std::uint32_t width, std::uint32_t height) noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `submit_bgra32_premultiplied` (public)

```cpp
[[nodiscard]] bool submit_bgra32_premultiplied( std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels) noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `touch` (public)

```cpp
void touch(bool explicit_boundary = false) noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `drain_now` (public)

```cpp
[[nodiscard]] bool drain_now() noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `snapshot` (public)

```cpp
[[nodiscard]] std::string snapshot() const
```

Reports the current snapshot value without mutation.

### `release` (public)

```cpp
void release() noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `WindowsCompatibilityPaintEndpoint` (private)

```cpp
explicit WindowsCompatibilityPaintEndpoint( std::unique_ptr<Implementation> implementation) noexcept
```

Constructs or tears down the retained WindowsCompatibilityPaintEndpoint object according to its ownership contract.
