# LiveSurface

- Status: **OBSERVED: bundle 008 high-throughput surface review; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `enable_shared_from_this → LiveSurface`
- Declaration: `include/gui_forms/live_surface/surface/live_surface.hpp:20`
- Definition: `src/core/live_surface/surface/live_surface.cpp`

LiveSurface is a renderer-neutral newest-frame exchange with configurable retained buffering, nonblocking producer acquisition, immutable reader leases, bounded configuration, telemetry, and revocable wake signaling.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `create` (public)

```cpp
static std::shared_ptr<LiveSurface> create( LiveSurfaceDescription description)
```

Validates dimensions, pixel format, and two-to-eight buffer depth, allocates the full pool before publication, and returns null on invalid input or allocation failure.

### `~LiveSurface` (public)

```cpp
~LiveSurface()
```

Releases the protocol state after outstanding frame, write, and wake handles release their own shared edges.

### `LiveSurface` (public)

```cpp
LiveSurface(const LiveSurface&) = delete
```

Privately binds a validated shared protocol state; copying is prohibited so construction remains factory-controlled.

### `operator=` (public)

```cpp
LiveSurface& operator=(const LiveSurface&) = delete
```

Prohibits copying of the factory-owned surface identity.

### `reconfigure` (public)

```cpp
[[nodiscard]] bool reconfigure(LiveSurfaceDescription description)
```

Allocates a complete replacement pool before locking, refuses an active writer, resets generation/frame state, advances epoch, and signals consumers after commit.

### `try_acquire_write` (public)

```cpp
[[nodiscard]] LiveSurfaceWriteLease try_acquire_write( bool preserve_published_contents = false) noexcept
```

Never blocks: selects a nonpublished buffer held only by state, optionally copies newest pixels, or counts and returns a dropped acquisition when no slot is safe.

### `acquire_latest` (public)

```cpp
[[nodiscard]] LiveSurfaceFrame acquire_latest() const noexcept
```

Returns the newest immutable published buffer and records sampling telemetry, or an empty frame before the first publish.

### `snapshot` (public)

```cpp
[[nodiscard]] LiveSurfaceSnapshot snapshot() const noexcept
```

Copies the complete configuration and protocol counters under the state mutex.

### `connect_presentation_wake` (public)

```cpp
[[nodiscard]] LiveSurfaceWakeConnection connect_presentation_wake( std::function<void()> wake)
```

Allocates a sequenced revocable callback record and appends it under the state mutex; callbacks are scheduling signals, not paint authority.

### `LiveSurface` (private)

```cpp
explicit LiveSurface(std::shared_ptr<detail::LiveSurfaceState> state) noexcept
```

Privately binds a validated shared protocol state; copying is prohibited so construction remains factory-controlled.
