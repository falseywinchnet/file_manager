# LiveSurfaceFrame

- Status: **OBSERVED: bundle 008 immutable read-lease review; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `LiveSurfaceFrame`
- Declaration: `include/gui_forms/live_surface/frame/live_surface_frame.hpp:20`
- Definition: `src/core/live_surface/frame/live_surface_frame.cpp`

LiveSurfaceFrame is a move-only immutable lease over one completely published buffer, retaining pixels while the producer searches other unleased slots.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `LiveSurfaceFrame` (public)

```cpp
LiveSurfaceFrame() = default
```

Constructs an empty lease, moves a lease, rejects copying, or privately binds one immutable buffer to exact epoch, generation, and damage metadata.

### `~LiveSurfaceFrame` (public)

```cpp
~LiveSurfaceFrame() = default
```

Releases the shared buffer lease; no producer coordination or callback occurs.

### `LiveSurfaceFrame` (public)

```cpp
LiveSurfaceFrame(LiveSurfaceFrame&&) noexcept = default
```

Constructs an empty lease, moves a lease, rejects copying, or privately binds one immutable buffer to exact epoch, generation, and damage metadata.

### `operator=` (public)

```cpp
LiveSurfaceFrame& operator=(LiveSurfaceFrame&&) noexcept = default
```

Moves or rejects copying according to exclusive lease-handle ownership.

### `LiveSurfaceFrame` (public)

```cpp
LiveSurfaceFrame(const LiveSurfaceFrame&) = delete
```

Constructs an empty lease, moves a lease, rejects copying, or privately binds one immutable buffer to exact epoch, generation, and damage metadata.

### `operator=` (public)

```cpp
LiveSurfaceFrame& operator=(const LiveSurfaceFrame&) = delete
```

Moves or rejects copying according to exclusive lease-handle ownership.

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports whether an immutable buffer is currently retained.

### `width` (public)

```cpp
[[nodiscard]] std::uint32_t width() const noexcept
```

Returns the retained buffer width or zero for an empty lease.

### `height` (public)

```cpp
[[nodiscard]] std::uint32_t height() const noexcept
```

Returns the retained buffer height or zero for an empty lease.

### `row_bytes` (public)

```cpp
[[nodiscard]] std::uint64_t row_bytes() const noexcept
```

Returns the exact byte stride or zero for an empty lease.

### `pixel_format` (public)

```cpp
[[nodiscard]] LiveSurfacePixelFormat pixel_format() const noexcept
```

Returns the admitted BGRA premultiplied-sRGB format for retained and empty leases.

### `epoch` (public)

```cpp
[[nodiscard]] std::uint64_t epoch() const noexcept
```

Returns the configuration epoch sampled with the publication.

### `generation` (public)

```cpp
[[nodiscard]] std::uint64_t generation() const noexcept
```

Returns the monotonically published generation within the sampled epoch.

### `damage` (public)

```cpp
[[nodiscard]] Rect damage() const noexcept
```

Returns producer-authored damage clipped to surface bounds.

### `pixels` (public)

```cpp
[[nodiscard]] std::span<const std::byte> pixels() const noexcept
```

Returns a const byte span whose lifetime is protected by the retained shared buffer.

### `LiveSurfaceFrame` (private)

```cpp
LiveSurfaceFrame(std::shared_ptr<const detail::LiveSurfaceBuffer> buffer, std::uint64_t epoch, std::uint64_t generation, Rect damage) noexcept
```

Constructs an empty lease, moves a lease, rejects copying, or privately binds one immutable buffer to exact epoch, generation, and damage metadata.
