# PopupToken

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `PopupToken`  
Declaration: `include/gui_forms/window.hpp:43`  
Definition: `src/core/window.cpp`

PopupToken is a class declared in include/gui_forms/window.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `PopupToken`

```cpp
PopupToken() = default
```

Constructs or tears down the retained PopupToken object according to its ownership contract.

### `~PopupToken`

```cpp
~PopupToken()
```

Constructs or tears down the retained PopupToken object according to its ownership contract.

### `PopupToken`

```cpp
PopupToken(PopupToken&& other) noexcept : attachment_(std::move(other.attachment_))
```

Constructs or tears down the retained PopupToken object according to its ownership contract.

### `operator=`

```cpp
PopupToken& operator=(PopupToken&& other) noexcept
```

Public PopupToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `PopupToken`

```cpp
PopupToken(const PopupToken&) = delete
```

Constructs or tears down the retained PopupToken object according to its ownership contract.

### `operator=`

```cpp
PopupToken& operator=(const PopupToken&) = delete
```

Public PopupToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `disconnect`

```cpp
void disconnect() noexcept
```

Public PopupToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `connected`

```cpp
[[nodiscard]] bool connected() const noexcept
```

Reports the current connected value without mutation.

### `closed_event`

```cpp
[[nodiscard]] Event<>* closed_event() noexcept
```

Public PopupToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
