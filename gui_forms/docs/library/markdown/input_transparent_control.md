# InputTransparentControl

- Status: **OBSERVED: bundle 013 per-adapter source isolation; native and MinGW ABI builds pass**
- Kind: **class / visual retained control**
- Hierarchy: `Control → InputTransparentControl`
- Declaration: `src/abi/control_adapters/input_transparent_control/input_transparent_control.hpp:11`
- Definition: `src/abi/control_adapters/input_transparent_control/input_transparent_control.cpp`

InputTransparentControl participates in retained layout/paint ordering while deliberately declining hit testing for compatibility placeholder surfaces.

## Visual evidence

![InputTransparentControl](../captures/container_bundle_003_screen.png)

## Declared methods

### `~InputTransparentControl` (public)

```cpp
~InputTransparentControl() override
```

Releases the compatibility placeholder through its isolated translation-unit boundary.

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
