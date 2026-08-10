# LiveSurfaceWakeConnection

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `LiveSurfaceWakeConnection`  
Declaration: `include/gui_forms/live_surface.hpp:49`  
Definition: `src/core/live_surface.cpp`

LiveSurfaceWakeConnection is a class declared in include/gui_forms/live_surface.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `LiveSurfaceWakeConnection`

```cpp
LiveSurfaceWakeConnection() = default
```

Constructs or tears down the retained LiveSurfaceWakeConnection object according to its ownership contract.

### `~LiveSurfaceWakeConnection`

```cpp
~LiveSurfaceWakeConnection()
```

Constructs or tears down the retained LiveSurfaceWakeConnection object according to its ownership contract.

### `LiveSurfaceWakeConnection`

```cpp
LiveSurfaceWakeConnection(LiveSurfaceWakeConnection&&) noexcept
```

Constructs or tears down the retained LiveSurfaceWakeConnection object according to its ownership contract.

### `operator=`

```cpp
LiveSurfaceWakeConnection& operator=(LiveSurfaceWakeConnection&&) noexcept
```

Public LiveSurfaceWakeConnection operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `LiveSurfaceWakeConnection`

```cpp
LiveSurfaceWakeConnection(const LiveSurfaceWakeConnection&) = delete
```

Constructs or tears down the retained LiveSurfaceWakeConnection object according to its ownership contract.

### `operator=`

```cpp
LiveSurfaceWakeConnection& operator=(const LiveSurfaceWakeConnection&) = delete
```

Public LiveSurfaceWakeConnection operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `connected`

```cpp
[[nodiscard]] bool connected() const noexcept
```

Reports the current connected value without mutation.

### `disconnect`

```cpp
void disconnect() noexcept
```

Public LiveSurfaceWakeConnection operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
