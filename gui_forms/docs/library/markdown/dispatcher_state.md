# DispatcherState

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `DispatcherState`  
Declaration: `src/core/dispatcher_state.hpp:31`  
Definition: `inline/header-only`

DispatcherState is a struct declared in src/core/dispatcher_state.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `DispatcherState`

```cpp
explicit DispatcherState(std::thread::id thread) : ui_thread(thread)
```

Constructs or tears down the retained DispatcherState object according to its ownership contract.

### `void`

```cpp
std::function<void()> wake
```

Public DispatcherState operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
