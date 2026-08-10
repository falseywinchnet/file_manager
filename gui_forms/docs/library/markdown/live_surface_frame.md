# LiveSurfaceFrame

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `LiveSurfaceFrame`  
Declaration: `include/gui_forms/live_surface.hpp:73`  
Definition: `src/core/live_surface.cpp`

LiveSurfaceFrame is a class declared in include/gui_forms/live_surface.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `LiveSurfaceFrame`

```cpp
LiveSurfaceFrame() = default
```

Constructs or tears down the retained LiveSurfaceFrame object according to its ownership contract.

### `~LiveSurfaceFrame`

```cpp
~LiveSurfaceFrame() = default
```

Constructs or tears down the retained LiveSurfaceFrame object according to its ownership contract.

### `LiveSurfaceFrame`

```cpp
LiveSurfaceFrame(LiveSurfaceFrame&&) noexcept = default
```

Constructs or tears down the retained LiveSurfaceFrame object according to its ownership contract.

### `operator=`

```cpp
LiveSurfaceFrame& operator=(LiveSurfaceFrame&&) noexcept = default
```

Public LiveSurfaceFrame operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `LiveSurfaceFrame`

```cpp
LiveSurfaceFrame(const LiveSurfaceFrame&) = delete
```

Constructs or tears down the retained LiveSurfaceFrame object according to its ownership contract.

### `operator=`

```cpp
LiveSurfaceFrame& operator=(const LiveSurfaceFrame&) = delete
```

Public LiveSurfaceFrame operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

### `pixel_format`

```cpp
[[nodiscard]] LiveSurfacePixelFormat pixel_format() const noexcept
```

Reports the current pixel format value without mutation.

### `epoch`

```cpp
[[nodiscard]] std::uint64_t epoch() const noexcept
```

Reports the current epoch value without mutation.

### `generation`

```cpp
[[nodiscard]] std::uint64_t generation() const noexcept
```

Reports the current generation value without mutation.

### `damage`

```cpp
[[nodiscard]] Rect damage() const noexcept
```

Reports the current damage value without mutation.

### `pixels`

```cpp
[[nodiscard]] std::span<const std::byte> pixels() const noexcept
```

Reports the current pixels value without mutation.
