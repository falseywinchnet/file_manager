# WindowsCompatibilityPaintEndpoint

- Status: **OBSERVED: bundle 012 isolated Win32 compatibility-paint state machine; complete MinGW/Skia M4 build passes**
- Kind: **class**
- Hierarchy: `WindowsCompatibilityPaintEndpoint`
- Declaration: `include/gui_forms/platform/windows_compatibility_paint_endpoint/windows_compatibility_paint_endpoint.hpp:17`
- Definition: `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp`

WindowsCompatibilityPaintEndpoint is a shared move-only façade over a RasterControl live surface, staging DC, generational writes, invalidation, submission, drain, and retirement.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `acquire` (public)

```cpp
static std::shared_ptr<WindowsCompatibilityPaintEndpoint> acquire( std::uint32_t width, std::uint32_t height)
```

Executes WindowsCompatibilityPaintEndpoint's acquire operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

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

Executes WindowsCompatibilityPaintEndpoint's operator= operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

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

Executes WindowsCompatibilityPaintEndpoint's publish device context operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `begin_device_context_write` (public)

```cpp
[[nodiscard]] bool begin_device_context_write( std::uintptr_t device_context) noexcept
```

Executes WindowsCompatibilityPaintEndpoint's begin device context write operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `end_device_context_write` (public)

```cpp
[[nodiscard]] bool end_device_context_write( std::uintptr_t device_context, bool publish) noexcept
```

Executes WindowsCompatibilityPaintEndpoint's end device context write operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `configure` (public)

```cpp
[[nodiscard]] bool configure(std::uint32_t width, std::uint32_t height) noexcept
```

Executes WindowsCompatibilityPaintEndpoint's configure operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `submit_bgra32_premultiplied` (public)

```cpp
[[nodiscard]] bool submit_bgra32_premultiplied( std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels) noexcept
```

Executes WindowsCompatibilityPaintEndpoint's submit bgra32 premultiplied operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `touch` (public)

```cpp
void touch(bool explicit_boundary = false) noexcept
```

Executes WindowsCompatibilityPaintEndpoint's touch operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `drain_now` (public)

```cpp
[[nodiscard]] bool drain_now() noexcept
```

Executes WindowsCompatibilityPaintEndpoint's drain now operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `snapshot` (public)

```cpp
[[nodiscard]] std::string snapshot() const
```

Reports the current snapshot value without mutation.

### `release` (public)

```cpp
void release() noexcept
```

Executes WindowsCompatibilityPaintEndpoint's release operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `WindowsCompatibilityPaintEndpoint` (private)

```cpp
explicit WindowsCompatibilityPaintEndpoint( std::unique_ptr<Implementation> implementation) noexcept
```

Constructs or tears down the retained WindowsCompatibilityPaintEndpoint object according to its ownership contract.
