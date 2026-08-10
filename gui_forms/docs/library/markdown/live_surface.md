# LiveSurface

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `enable_shared_from_this → LiveSurface`  
Declaration: `include/gui_forms/live_surface.hpp:147`  
Definition: `src/core/live_surface.cpp`

LiveSurface is a class declared in include/gui_forms/live_surface.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `create`

```cpp
static std::shared_ptr<LiveSurface> create( LiveSurfaceDescription description)
```

Public LiveSurface operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `~LiveSurface`

```cpp
~LiveSurface()
```

Constructs or tears down the retained LiveSurface object according to its ownership contract.

### `LiveSurface`

```cpp
LiveSurface(const LiveSurface&) = delete
```

Constructs or tears down the retained LiveSurface object according to its ownership contract.

### `operator=`

```cpp
LiveSurface& operator=(const LiveSurface&) = delete
```

Public LiveSurface operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `reconfigure`

```cpp
[[nodiscard]] bool reconfigure(LiveSurfaceDescription description)
```

Public LiveSurface operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `try_acquire_write`

```cpp
[[nodiscard]] LiveSurfaceWriteLease try_acquire_write( bool preserve_published_contents = false) noexcept
```

Public LiveSurface operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `acquire_latest`

```cpp
[[nodiscard]] LiveSurfaceFrame acquire_latest() const noexcept
```

Reports the current acquire latest value without mutation.

### `snapshot`

```cpp
[[nodiscard]] LiveSurfaceSnapshot snapshot() const noexcept
```

Reports the current snapshot value without mutation.

### `connect_presentation_wake`

```cpp
[[nodiscard]] LiveSurfaceWakeConnection connect_presentation_wake( std::function<void()> wake)
```

Public LiveSurface operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
