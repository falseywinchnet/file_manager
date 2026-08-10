# LiveSurfaceWakeConnection

- Status: **OBSERVED: bundle 008 wake lifetime review; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `LiveSurfaceWakeConnection`
- Declaration: `include/gui_forms/live_surface/wake_connection/live_surface_wake_connection.hpp:17`
- Definition: `src/core/live_surface/wake_connection/live_surface_wake_connection.cpp`

LiveSurfaceWakeConnection is the move-only revocable ownership edge between a producer publication and a retained consumer's best-effort scheduling signal.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `LiveSurfaceWakeConnection` (public)

```cpp
LiveSurfaceWakeConnection() = default
```

Constructs a disconnected token, moves exclusive connection ownership, or privately binds one state/wake pair for LiveSurface.

### `~LiveSurfaceWakeConnection` (public)

```cpp
~LiveSurfaceWakeConnection()
```

Disconnects idempotently so token retirement cannot retain a stale consumer callback.

### `LiveSurfaceWakeConnection` (public)

```cpp
LiveSurfaceWakeConnection(LiveSurfaceWakeConnection&&) noexcept
```

Constructs a disconnected token, moves exclusive connection ownership, or privately binds one state/wake pair for LiveSurface.

### `operator=` (public)

```cpp
LiveSurfaceWakeConnection& operator=(LiveSurfaceWakeConnection&&) noexcept
```

Disconnects any prior connection before taking move ownership; copying is prohibited.

### `LiveSurfaceWakeConnection` (public)

```cpp
LiveSurfaceWakeConnection(const LiveSurfaceWakeConnection&) = delete
```

Constructs a disconnected token, moves exclusive connection ownership, or privately binds one state/wake pair for LiveSurface.

### `operator=` (public)

```cpp
LiveSurfaceWakeConnection& operator=(const LiveSurfaceWakeConnection&) = delete
```

Disconnects any prior connection before taking move ownership; copying is prohibited.

### `connected` (public)

```cpp
[[nodiscard]] bool connected() const noexcept
```

Reads the wake's atomic revocation bit without entering the surface mutex.

### `disconnect` (public)

```cpp
void disconnect() noexcept
```

Atomically revokes delivery, erases the exact wake sequence under the state mutex when the surface remains alive, and releases both ownership edges.

### `LiveSurfaceWakeConnection` (private)

```cpp
LiveSurfaceWakeConnection( std::weak_ptr<detail::LiveSurfaceState> state, std::shared_ptr<detail::LiveSurfaceWake> wake) noexcept
```

Constructs a disconnected token, moves exclusive connection ownership, or privately binds one state/wake pair for LiveSurface.
