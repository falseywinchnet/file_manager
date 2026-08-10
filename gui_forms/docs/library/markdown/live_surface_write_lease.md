# LiveSurfaceWriteLease

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `LiveSurfaceWriteLease`  
Declaration: `include/gui_forms/live_surface.hpp:111`  
Definition: `src/core/live_surface.cpp`

LiveSurfaceWriteLease is a class declared in include/gui_forms/live_surface.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `LiveSurfaceWriteLease`

```cpp
LiveSurfaceWriteLease() = default
```

Constructs or tears down the retained LiveSurfaceWriteLease object according to its ownership contract.

### `~LiveSurfaceWriteLease`

```cpp
~LiveSurfaceWriteLease()
```

Constructs or tears down the retained LiveSurfaceWriteLease object according to its ownership contract.

### `LiveSurfaceWriteLease`

```cpp
LiveSurfaceWriteLease(LiveSurfaceWriteLease&& other) noexcept
```

Constructs or tears down the retained LiveSurfaceWriteLease object according to its ownership contract.

### `operator=`

```cpp
LiveSurfaceWriteLease& operator=(LiveSurfaceWriteLease&& other) noexcept
```

Public LiveSurfaceWriteLease operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `LiveSurfaceWriteLease`

```cpp
LiveSurfaceWriteLease(const LiveSurfaceWriteLease&) = delete
```

Constructs or tears down the retained LiveSurfaceWriteLease object according to its ownership contract.

### `operator=`

```cpp
LiveSurfaceWriteLease& operator=(const LiveSurfaceWriteLease&) = delete
```

Public LiveSurfaceWriteLease operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `operatorbool`

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports the current operatorbool value without mutation.

### `width`

```cpp
[[nodiscard]] std::uint32_t width() const noexcept
```

Reports the current width value without mutation.

### `height`

```cpp
[[nodiscard]] std::uint32_t height() const noexcept
```

Reports the current height value without mutation.

### `row_bytes`

```cpp
[[nodiscard]] std::uint64_t row_bytes() const noexcept
```

Reports the current row bytes value without mutation.

### `pixels`

```cpp
[[nodiscard]] std::span<std::byte> pixels() noexcept
```

Public LiveSurfaceWriteLease operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `publish`

```cpp
[[nodiscard]] std::uint64_t publish(Rect damage =
```

Public LiveSurfaceWriteLease operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `abandon`

```cpp
void abandon() noexcept
```

Public LiveSurfaceWriteLease operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
