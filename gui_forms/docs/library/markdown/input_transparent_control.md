# InputTransparentControl

- Status: **generated inventory; detailed review pending**
- Kind: **class / visual retained control**
- Hierarchy: `Control → InputTransparentControl`
- Declaration: `src/abi/control_adapters/abi_control_adapters.hpp:85`
- Definition: `inline/header-only`

InputTransparentControl is a visual retained control declared in src/abi/control_adapters/abi_control_adapters.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `InputTransparentControl` (public)

```cpp
explicit InputTransparentControl(StableId stable_id) : Control(std::move(stable_id))
```

Constructs or tears down the retained InputTransparentControl object according to its ownership contract.

### `hit_test_local` (public)

```cpp
[[nodiscard]] bool hit_test_local(gui_forms::Point) const override
```

Reports the current hit test local value without mutation.
