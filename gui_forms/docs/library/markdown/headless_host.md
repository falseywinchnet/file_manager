# HeadlessHost

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `HeadlessHost`  
Declaration: `src/host/headless/headless_host.hpp:58`  
Definition: `src/host/headless/headless_host.cpp`

HeadlessHost is a class declared in src/host/headless/headless_host.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `HeadlessHost`

```cpp
explicit HeadlessHost(Window& window)
```

Constructs or tears down the retained HeadlessHost object according to its ownership contract.

### `~HeadlessHost`

```cpp
~HeadlessHost()
```

Constructs or tears down the retained HeadlessHost object according to its ownership contract.

### `HeadlessHost`

```cpp
HeadlessHost(const HeadlessHost&) = delete
```

Constructs or tears down the retained HeadlessHost object according to its ownership contract.

### `operator=`

```cpp
HeadlessHost& operator=(const HeadlessHost&) = delete
```

Public HeadlessHost operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `dispatch`

```cpp
[[nodiscard]] HostDispatchResult dispatch(HostEventPayload payload, std::uint64_t timestamp_nanoseconds)
```

Public HeadlessHost operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `pump_dispatcher`

```cpp
[[nodiscard]] DispatchDrainResult pump_dispatcher( std::size_t maximum_callbacks = maximum_callbacks_per_dispatch_turn)
```

Public HeadlessHost operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `dispatcher_wake_pending`

```cpp
[[nodiscard]] bool dispatcher_wake_pending() const noexcept
```

Reports the current dispatcher wake pending value without mutation.

### `paint_wake_pending`

```cpp
[[nodiscard]] bool paint_wake_pending() const noexcept
```

Reports the current paint wake pending value without mutation.

### `consume_paint_wake`

```cpp
[[nodiscard]] bool consume_paint_wake() noexcept
```

Public HeadlessHost operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `session`

```cpp
[[nodiscard]] HostSession& session() noexcept
```

Public HeadlessHost operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `session`

```cpp
[[nodiscard]] const HostSession& session() const noexcept
```

Reports the current session value without mutation.

### `services`

```cpp
[[nodiscard]] HostServices& services() noexcept
```

Public HeadlessHost operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `services`

```cpp
[[nodiscard]] const HostServices& services() const noexcept
```

Reports the current services value without mutation.

### `trace`

```cpp
[[nodiscard]] const std::string& trace() const noexcept
```

Reports the current trace value without mutation.
