# ScheduledFrameRequest

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Revocable → ScheduledFrameRequest`  
Declaration: `src/core/frame_scheduler.hpp:16`  
Definition: `inline/header-only`

ScheduledFrameRequest is a class declared in src/core/frame_scheduler.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ScheduledFrameRequest`

```cpp
ScheduledFrameRequest(FrameRequestKind request_kind, Control::WeakPtr request_target, FrameTime request_deadline, FrameInterval request_interval =
```

Constructs or tears down the retained ScheduledFrameRequest object according to its ownership contract.

### `kind`

```cpp
: kind(request_kind), target(std::move(request_target)), deadline(request_deadline), interval(request_interval)
```

Public ScheduledFrameRequest operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `ScheduledFrameRequest`

```cpp
ScheduledFrameRequest(FrameTime request_deadline, FrameInterval request_interval, std::function<void(FrameTime)> request_callback) noexcept : kind(FrameRequestKind::ui_timer), deadline(request_deadline), interval(request_interval), callback(std::move(request_callback))
```

Constructs or tears down the retained ScheduledFrameRequest object according to its ownership contract.

### `disconnect`

```cpp
void disconnect() noexcept override
```

Public ScheduledFrameRequest operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `connected`

```cpp
[[nodiscard]] bool connected() const noexcept override
```

Reports the current connected value without mutation.

### `void`

```cpp
std::function<void(FrameTime)> callback
```

Public ScheduledFrameRequest operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
