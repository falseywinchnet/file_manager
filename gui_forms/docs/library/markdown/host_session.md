# HostSession

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `HostSession`  
Declaration: `include/gui_forms/host.hpp:469`  
Definition: `src/core/host.cpp`

HostSession is a class declared in include/gui_forms/host.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `HostSession`

```cpp
HostSession(Window& window, HostCapabilities capabilities, HostServices* services = nullptr)
```

Constructs or tears down the retained HostSession object according to its ownership contract.

### `~HostSession`

```cpp
~HostSession()
```

Constructs or tears down the retained HostSession object according to its ownership contract.

### `HostSession`

```cpp
HostSession(const HostSession&) = delete
```

Constructs or tears down the retained HostSession object according to its ownership contract.

### `operator=`

```cpp
HostSession& operator=(const HostSession&) = delete
```

Public HostSession operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `dispatch`

```cpp
[[nodiscard]] HostDispatchResult dispatch(HostEvent event)
```

Public HostSession operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `shutdown`

```cpp
void shutdown() noexcept
```

Public HostSession operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `closing`

```cpp
[[nodiscard]] Event<HostCloseRequest&>& closing() noexcept
```

Public HostSession operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `observed`

```cpp
[[nodiscard]] Event<const HostEvent&, const HostDispatchResult&>& observed() noexcept
```

Public HostSession operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `snapshot`

```cpp
[[nodiscard]] HostSessionSnapshot snapshot() const
```

Reports the current snapshot value without mutation.
