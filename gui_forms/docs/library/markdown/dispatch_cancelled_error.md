# DispatchCancelledError

- Status: **OBSERVED: bundle 007 dispatch-operation split; M4 build and focused tests pass**
- Kind: **class**
- Hierarchy: `runtime_error → DispatchCancelledError`
- Declaration: `include/gui_forms/dispatcher/operation/dispatch_operation.hpp:18`
- Definition: `inline/header-only`

DispatchCancelledError is the stable synchronous-Invoke failure raised when posted work is revoked before execution.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `DispatchCancelledError` (public)

```cpp
DispatchCancelledError() : std::runtime_error( "GUI.Forms synchronous Invoke was cancelled before execution")
```

Constructs the stable cancellation message used across hosts.
