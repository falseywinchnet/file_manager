# Timer

- Status: **OBSERVED: bundle 010 UI timer split; focused M4 timer tests pass**
- Kind: **class**
- Hierarchy: `Component → Timer`
- Declaration: `include/gui_forms/timer/timer/timer.hpp:17`
- Definition: `src/core/timer/timer/timer.cpp`

Timer schedules coalescing callbacks through the owning Window scheduler and serializes ticks with input, layout, and paint on the UI thread.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `Timer` (public)

```cpp
explicit Timer(Window& window, std::chrono::milliseconds interval = std::chrono::milliseconds(100))
```

Validates owner affinity and a minimum one-millisecond interval, then creates callback indirection.

### `~Timer` (public)

```cpp
~Timer() override
```

Disconnects the frame request and clears callback ownership.

### `interval` (public)

```cpp
[[nodiscard]] std::chrono::milliseconds interval() const noexcept
```

Returns the retained interval.

### `set_interval` (public)

```cpp
void set_interval(std::chrono::milliseconds interval)
```

Validates affinity and minimum duration, then reschedules an enabled timer from now.

### `enabled` (public)

```cpp
[[nodiscard]] bool enabled() const noexcept
```

Requires both logical enablement and a connected scheduler request.

### `start` (public)

```cpp
void start()
```

Schedules the first deadline at now plus interval unless already active.

### `start_at` (public)

```cpp
void start_at(FrameTime first_deadline)
```

Replaces any request with an explicit deterministic first deadline.

### `stop` (public)

```cpp
void stop()
```

Clears logical enablement and disconnects the request.

### `tick` (public)

```cpp
[[nodiscard]] Event<>& tick() noexcept
```

Returns the serialized tick event.

### `verify_dispose_thread` (protected)

```cpp
void verify_dispose_thread() override
```

Enforces Window affinity while its lifetime remains available.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Revokes scheduling, callback ownership, events, and Window lifetime.

### `bound_window` (private)

```cpp
[[nodiscard]] Window* bound_window() const noexcept
```

Resolves the weak Window lifetime without prolonging ownership.

### `require_mutable_timer` (private)

```cpp
void require_mutable_timer(std::string_view operation) const
```

Rejects disposed, retired-window, or wrong-thread mutation.

### `schedule` (private)

```cpp
void schedule(FrameTime first_deadline)
```

Creates the repeating coalescing UI-timer request through Window.
