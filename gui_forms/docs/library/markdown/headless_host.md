# HeadlessHost

- Status: **OBSERVED: bundle 007 deterministic host/session split; M4 build and focused tests pass**
- Kind: **class**
- Hierarchy: `HeadlessHost`
- Declaration: `src/host/headless/session/headless_host.hpp:14`
- Definition: `src/host/headless/session/headless_host.cpp`

HeadlessHost owns the reference HostServices and HostSession, installs atomic dispatcher/paint wake flags, supplies caller-controlled timestamps, and records a machine-stable lifecycle trace.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `HeadlessHost` (public)

```cpp
explicit HeadlessHost(Window& window)
```

Binds one Window, installs reference services/session and coalesced wake handlers, and subscribes exact event/result trace publication.

### `~HeadlessHost` (public)

```cpp
~HeadlessHost()
```

Removes paint and dispatcher wake seams before member shutdown.

### `HeadlessHost` (public)

```cpp
HeadlessHost(const HeadlessHost&) = delete
```

Binds one Window, installs reference services/session and coalesced wake handlers, and subscribes exact event/result trace publication.

### `operator=` (public)

```cpp
HeadlessHost& operator=(const HeadlessHost&) = delete
```

Is deleted because reference service/session attachment is singular.

### `dispatch` (public)

```cpp
[[nodiscard]] HostDispatchResult dispatch( HostEventPayload payload, std::uint64_t timestamp_nanoseconds)
```

Assigns the next monotonic sequence to a caller-timestamped payload and enters HostSession.

### `pump_dispatcher` (public)

```cpp
[[nodiscard]] DispatchDrainResult pump_dispatcher( std::size_t maximum_callbacks = maximum_callbacks_per_dispatch_turn)
```

Consumes the atomic wake flag and drains one bounded Window dispatcher turn.

### `dispatcher_wake_pending` (public)

```cpp
[[nodiscard]] bool dispatcher_wake_pending() const noexcept
```

Atomically reports a coalesced dispatcher wake.

### `paint_wake_pending` (public)

```cpp
[[nodiscard]] bool paint_wake_pending() const noexcept
```

Atomically reports a coalesced retained-paint wake.

### `consume_paint_wake` (public)

```cpp
[[nodiscard]] bool consume_paint_wake() noexcept
```

Atomically acknowledges and returns retained-paint wake state.

### `session` (public)

```cpp
[[nodiscard]] HostSession& session() noexcept
```

Returns mutable or const access to the owned reference HostSession.

### `session` (public)

```cpp
[[nodiscard]] const HostSession& session() const noexcept
```

Returns mutable or const access to the owned reference HostSession.

### `services` (public)

```cpp
[[nodiscard]] HostServices& services() noexcept
```

Returns mutable or const access to the owned reference HostServices.

### `services` (public)

```cpp
[[nodiscard]] const HostServices& services() const noexcept
```

Returns mutable or const access to the owned reference HostServices.

### `trace` (public)

```cpp
[[nodiscard]] const std::string& trace() const noexcept
```

Returns the stable event/result/lifecycle/service trace.
