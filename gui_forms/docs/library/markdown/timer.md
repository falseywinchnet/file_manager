# Timer

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Component → Timer`  
Declaration: `include/gui_forms/timer.hpp:17`  
Definition: `src/core/timer.cpp`

Timer is a class declared in include/gui_forms/timer.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Timer`

```cpp
explicit Timer(Window& window, std::chrono::milliseconds interval = std::chrono::milliseconds(100))
```

Constructs or tears down the retained Timer object according to its ownership contract.

### `~Timer`

```cpp
~Timer() override
```

Constructs or tears down the retained Timer object according to its ownership contract.

### `interval`

```cpp
[[nodiscard]] std::chrono::milliseconds interval() const noexcept
```

Reports the current interval value without mutation.

### `set_interval`

```cpp
void set_interval(std::chrono::milliseconds interval)
```

Synchronously updates the retained interval property. Validation, typed invalidation, and notifications are defined by the implementation.

### `enabled`

```cpp
[[nodiscard]] bool enabled() const noexcept
```

Reports the current enabled value without mutation.

### `start`

```cpp
void start()
```

Public Timer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `start_at`

```cpp
void start_at(FrameTime first_deadline)
```

Public Timer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `stop`

```cpp
void stop()
```

Public Timer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `tick`

```cpp
[[nodiscard]] Event<>& tick() noexcept
```

Public Timer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
