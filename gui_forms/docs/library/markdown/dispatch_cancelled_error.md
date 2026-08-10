# DispatchCancelledError

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `runtime_error → DispatchCancelledError`  
Declaration: `include/gui_forms/dispatcher.hpp:29`  
Definition: `inline/header-only`

DispatchCancelledError is a class declared in include/gui_forms/dispatcher.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `DispatchCancelledError`

```cpp
DispatchCancelledError() : std::runtime_error( "GUI.Forms synchronous Invoke was cancelled before execution")
```

Constructs or tears down the retained DispatchCancelledError object according to its ownership contract.
