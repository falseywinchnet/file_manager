# AcceleratorToken

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `AcceleratorToken`  
Declaration: `include/gui_forms/window.hpp:177`  
Definition: `src/core/window.cpp`

AcceleratorToken is a class declared in include/gui_forms/window.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `AcceleratorToken`

```cpp
AcceleratorToken() = default
```

Constructs or tears down the retained AcceleratorToken object according to its ownership contract.

### `~AcceleratorToken`

```cpp
~AcceleratorToken()
```

Constructs or tears down the retained AcceleratorToken object according to its ownership contract.

### `AcceleratorToken`

```cpp
AcceleratorToken(AcceleratorToken&& other) noexcept : attachment_(std::move(other.attachment_))
```

Constructs or tears down the retained AcceleratorToken object according to its ownership contract.

### `operator=`

```cpp
AcceleratorToken& operator=(AcceleratorToken&& other) noexcept
```

Public AcceleratorToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `AcceleratorToken`

```cpp
AcceleratorToken(const AcceleratorToken&) = delete
```

Constructs or tears down the retained AcceleratorToken object according to its ownership contract.

### `operator=`

```cpp
AcceleratorToken& operator=(const AcceleratorToken&) = delete
```

Public AcceleratorToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `disconnect`

```cpp
void disconnect() noexcept
```

Public AcceleratorToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `connected`

```cpp
[[nodiscard]] bool connected() const noexcept
```

Reports the current connected value without mutation.
