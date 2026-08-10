# ControlFactory

- Status: **OBSERVED: bundle 011 static-tree construction review**
- Kind: **class**
- Hierarchy: `ControlFactory`
- Declaration: `include/gui_forms/control/static_tree/control_factory/control_factory.hpp:21`
- Definition: `src/core/control/static_tree/control_factory/control_factory.cpp`

ControlFactory maps explicit type names to construction callbacks, rejects duplicates/unknown types, and recursively creates retained trees without a bundled scripting runtime.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `register_type` (public)

```cpp
void register_type(std::string type, Creator creator)
```

Executes ControlFactory's register type operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `create` (public)

```cpp
[[nodiscard]] Control::Ptr create(std::string_view type, StableId stable_id) const
```

Reports the current create value without mutation.
