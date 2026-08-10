# ScheduledFrameRequest

- Status: **OBSERVED: bundle 008 private scheduler request review; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `Revocable → ScheduledFrameRequest`
- Declaration: `src/core/scheduler/request/scheduled_frame_request.hpp:16`
- Definition: `src/core/scheduler/request/scheduled_frame_request.cpp`

ScheduledFrameRequest is the revocable state record for one deadline, periodic active surface, or UI timer, separating weak control targets from callback-owned timer delivery.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `ScheduledFrameRequest` (public)

```cpp
ScheduledFrameRequest(FrameRequestKind request_kind, Control::WeakPtr request_target, FrameTime request_deadline, FrameInterval request_interval =
```

Constructs either a weak-target paint request or a callback timer with explicit kind, deadline, and interval.

### `ScheduledFrameRequest` (public)

```cpp
ScheduledFrameRequest( FrameTime request_deadline, FrameInterval request_interval, std::function<void(FrameTime)> request_callback) noexcept
```

Constructs either a weak-target paint request or a callback timer with explicit kind, deadline, and interval.

### `disconnect` (public)

```cpp
void disconnect() noexcept override
```

Revokes future delivery and clears callback ownership.

### `connected` (public)

```cpp
[[nodiscard]] bool connected() const noexcept override
```

Returns the authoritative request liveness bit.
