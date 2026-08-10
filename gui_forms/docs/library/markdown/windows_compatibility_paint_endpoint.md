# WindowsCompatibilityPaintEndpoint

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `WindowsCompatibilityPaintEndpoint`  
Declaration: `include/gui_forms/platform/windows_host.hpp:56`  
Definition: `src/host/windows/windows_host.cpp`

WindowsCompatibilityPaintEndpoint is a class declared in include/gui_forms/platform/windows_host.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `acquire`

```cpp
static std::shared_ptr<WindowsCompatibilityPaintEndpoint> acquire( std::uint32_t width, std::uint32_t height)
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `~WindowsCompatibilityPaintEndpoint`

```cpp
~WindowsCompatibilityPaintEndpoint()
```

Constructs or tears down the retained WindowsCompatibilityPaintEndpoint object according to its ownership contract.

### `WindowsCompatibilityPaintEndpoint`

```cpp
WindowsCompatibilityPaintEndpoint( const WindowsCompatibilityPaintEndpoint&) = delete
```

Constructs or tears down the retained WindowsCompatibilityPaintEndpoint object according to its ownership contract.

### `operator=`

```cpp
WindowsCompatibilityPaintEndpoint& operator=( const WindowsCompatibilityPaintEndpoint&) = delete
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `compatibility_handle`

```cpp
[[nodiscard]] std::uintptr_t compatibility_handle() const noexcept
```

Reports the current compatibility handle value without mutation.

### `device_context`

```cpp
[[nodiscard]] std::uintptr_t device_context() const noexcept
```

Reports the current device context value without mutation.

### `live_surface`

```cpp
[[nodiscard]] std::shared_ptr<LiveSurface> live_surface() const noexcept
```

Reports the current live surface value without mutation.

### `publish_device_context`

```cpp
[[nodiscard]] bool publish_device_context( std::uintptr_t device_context) noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `begin_device_context_write`

```cpp
[[nodiscard]] bool begin_device_context_write( std::uintptr_t device_context) noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `end_device_context_write`

```cpp
[[nodiscard]] bool end_device_context_write( std::uintptr_t device_context, bool publish) noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `configure`

```cpp
[[nodiscard]] bool configure(std::uint32_t width, std::uint32_t height) noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `submit_bgra32_premultiplied`

```cpp
[[nodiscard]] bool submit_bgra32_premultiplied( std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels) noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `touch`

```cpp
void touch(bool explicit_boundary = false) noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `drain_now`

```cpp
[[nodiscard]] bool drain_now() noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `snapshot`

```cpp
[[nodiscard]] std::string snapshot() const
```

Reports the current snapshot value without mutation.

### `release`

```cpp
void release() noexcept
```

Public WindowsCompatibilityPaintEndpoint operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
