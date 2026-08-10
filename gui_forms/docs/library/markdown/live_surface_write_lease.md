# LiveSurfaceWriteLease

- Status: **OBSERVED: bundle 008 producer lease review; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `LiveSurfaceWriteLease`
- Declaration: `include/gui_forms/live_surface/write_lease/live_surface_write_lease.hpp:22`
- Definition: `src/core/live_surface/write_lease/live_surface_write_lease.cpp`

LiveSurfaceWriteLease is a move-only exclusive producer candidate; publish commits complete pixels atomically, while destruction or abandon returns the slot without publishing.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `LiveSurfaceWriteLease` (public)

```cpp
LiveSurfaceWriteLease() = default
```

Constructs an empty token, moves exclusive ownership, rejects copying, or privately binds the state, buffer slot, and configuration epoch selected by LiveSurface.

### `~LiveSurfaceWriteLease` (public)

```cpp
~LiveSurfaceWriteLease()
```

Abandons an unpublished candidate so producer failure cannot strand the writing slot.

### `LiveSurfaceWriteLease` (public)

```cpp
LiveSurfaceWriteLease(LiveSurfaceWriteLease&& other) noexcept
```

Constructs an empty token, moves exclusive ownership, rejects copying, or privately binds the state, buffer slot, and configuration epoch selected by LiveSurface.

### `operator=` (public)

```cpp
LiveSurfaceWriteLease& operator=(LiveSurfaceWriteLease&& other) noexcept
```

Abandons any current candidate before taking move ownership; copying is prohibited.

### `LiveSurfaceWriteLease` (public)

```cpp
LiveSurfaceWriteLease(const LiveSurfaceWriteLease&) = delete
```

Constructs an empty token, moves exclusive ownership, rejects copying, or privately binds the state, buffer slot, and configuration epoch selected by LiveSurface.

### `operator=` (public)

```cpp
LiveSurfaceWriteLease& operator=(const LiveSurfaceWriteLease&) = delete
```

Abandons any current candidate before taking move ownership; copying is prohibited.

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports whether state and candidate buffer ownership are both current.

### `width` (public)

```cpp
[[nodiscard]] std::uint32_t width() const noexcept
```

Returns candidate width or zero for an empty lease.

### `height` (public)

```cpp
[[nodiscard]] std::uint32_t height() const noexcept
```

Returns candidate height or zero for an empty lease.

### `row_bytes` (public)

```cpp
[[nodiscard]] std::uint64_t row_bytes() const noexcept
```

Returns candidate stride or zero for an empty lease.

### `pixels` (public)

```cpp
[[nodiscard]] std::span<std::byte> pixels() noexcept
```

Returns the mutable candidate byte span; these bytes remain invisible until publish succeeds.

### `publish` (public)

```cpp
[[nodiscard]] std::uint64_t publish(Rect damage =
```

Validates epoch, slot, and buffer identity under the mutex, clips/defaults damage, advances generation, releases producer ownership, then invokes copied best-effort wakes outside the mutex.

### `abandon` (public)

```cpp
void abandon() noexcept
```

Clears the writing slot only when this lease still owns the current epoch and releases candidate state without changing publication identity.

### `LiveSurfaceWriteLease` (private)

```cpp
LiveSurfaceWriteLease(std::shared_ptr<detail::LiveSurfaceState> state, std::shared_ptr<detail::LiveSurfaceBuffer> buffer, std::size_t slot, std::uint64_t epoch) noexcept
```

Constructs an empty token, moves exclusive ownership, rejects copying, or privately binds the state, buffer slot, and configuration epoch selected by LiveSurface.
