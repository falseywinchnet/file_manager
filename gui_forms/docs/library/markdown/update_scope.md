# UpdateScope

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `UpdateScope`  
Declaration: `include/gui_forms/window.hpp:750`  
Definition: `src/core/window.cpp`

UpdateScope is a class declared in include/gui_forms/window.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `UpdateScope`

```cpp
explicit UpdateScope(Window& window) noexcept : window_(&window)
```

Constructs or tears down the retained UpdateScope object according to its ownership contract.

### `~UpdateScope`

```cpp
~UpdateScope()
```

Constructs or tears down the retained UpdateScope object according to its ownership contract.

### `UpdateScope`

```cpp
UpdateScope(UpdateScope&& other) noexcept
```

Constructs or tears down the retained UpdateScope object according to its ownership contract.

### `operator=`

```cpp
UpdateScope& operator=(UpdateScope&& other) noexcept
```

Public UpdateScope operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `UpdateScope`

```cpp
UpdateScope(const UpdateScope&) = delete
```

Constructs or tears down the retained UpdateScope object according to its ownership contract.

### `operator=`

```cpp
UpdateScope& operator=(const UpdateScope&) = delete
```

Public UpdateScope operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `perform_layout`

```cpp
void perform_layout()
```

Public UpdateScope operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `close`

```cpp
void close()
```

Public UpdateScope operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
