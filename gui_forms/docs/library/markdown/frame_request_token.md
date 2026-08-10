# FrameRequestToken

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `FrameRequestToken`  
Declaration: `include/gui_forms/scheduler.hpp:50`  
Definition: `inline/header-only`

FrameRequestToken is a class declared in include/gui_forms/scheduler.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `FrameRequestToken`

```cpp
FrameRequestToken() = default
```

Constructs or tears down the retained FrameRequestToken object according to its ownership contract.

### `~FrameRequestToken`

```cpp
~FrameRequestToken()
```

Constructs or tears down the retained FrameRequestToken object according to its ownership contract.

### `FrameRequestToken`

```cpp
FrameRequestToken(FrameRequestToken&& other) noexcept : revocable_(std::move(other.revocable_))
```

Constructs or tears down the retained FrameRequestToken object according to its ownership contract.

### `operator=`

```cpp
FrameRequestToken& operator=(FrameRequestToken&& other) noexcept
```

Public FrameRequestToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `FrameRequestToken`

```cpp
FrameRequestToken(const FrameRequestToken&) = delete
```

Constructs or tears down the retained FrameRequestToken object according to its ownership contract.

### `operator=`

```cpp
FrameRequestToken& operator=(const FrameRequestToken&) = delete
```

Public FrameRequestToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `disconnect`

```cpp
void disconnect() noexcept
```

Public FrameRequestToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `connected`

```cpp
[[nodiscard]] bool connected() const noexcept
```

Reports the current connected value without mutation.
