# FrameRequestToken

- Status: **OBSERVED: bundle 008 scheduler token review; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `FrameRequestToken`
- Declaration: `include/gui_forms/scheduler/frame_request_token/frame_request_token.hpp:11`
- Definition: `src/core/scheduler/frame_request_token/frame_request_token.cpp`

FrameRequestToken is move-only revocable ownership of one Window frame request, with destruction providing deterministic cancellation.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `FrameRequestToken` (public)

```cpp
FrameRequestToken() = default
```

Constructs a disconnected token, moves request ownership, rejects copying, or privately binds one Window-created Revocable.

### `~FrameRequestToken` (public)

```cpp
~FrameRequestToken()
```

Disconnects the request before releasing its shared ownership edge.

### `FrameRequestToken` (public)

```cpp
FrameRequestToken(FrameRequestToken&& other) noexcept
```

Constructs a disconnected token, moves request ownership, rejects copying, or privately binds one Window-created Revocable.

### `operator=` (public)

```cpp
FrameRequestToken& operator=(FrameRequestToken&& other) noexcept
```

Disconnects any prior request before taking move ownership; copying is prohibited.

### `FrameRequestToken` (public)

```cpp
FrameRequestToken(const FrameRequestToken&) = delete
```

Constructs a disconnected token, moves request ownership, rejects copying, or privately binds one Window-created Revocable.

### `operator=` (public)

```cpp
FrameRequestToken& operator=(const FrameRequestToken&) = delete
```

Disconnects any prior request before taking move ownership; copying is prohibited.

### `disconnect` (public)

```cpp
void disconnect() noexcept
```

Idempotently revokes and releases the owned request.

### `connected` (public)

```cpp
[[nodiscard]] bool connected() const noexcept
```

Reports whether a revocable is present and still admits delivery.

### `FrameRequestToken` (private)

```cpp
explicit FrameRequestToken(std::shared_ptr<detail::Revocable> revocable)
```

Constructs a disconnected token, moves request ownership, rejects copying, or privately binds one Window-created Revocable.
