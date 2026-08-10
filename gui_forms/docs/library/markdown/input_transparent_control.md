# InputTransparentControl

- Status: **OBSERVED: bundle 010 private ABI control-adapter split; native and MinGW ABI builds pass**
- Kind: **class / visual retained control**
- Hierarchy: `Control → InputTransparentControl`
- Declaration: `src/abi/control_adapters/abi_control_adapters.hpp:85`
- Definition: `inline/header-only`

InputTransparentControl participates in retained layout/paint ordering while deliberately declining hit testing for compatibility placeholder surfaces.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `InputTransparentControl` (public)

```cpp
explicit InputTransparentControl(StableId stable_id) : Control(std::move(stable_id))
```

Forwards stable identity to the retained Control base.

### `hit_test_local` (public)

```cpp
[[nodiscard]] bool hit_test_local(gui_forms::Point) const override
```

Always returns false so the placeholder cannot intercept input.
