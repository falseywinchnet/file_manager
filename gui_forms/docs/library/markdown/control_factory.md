# ControlFactory

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `ControlFactory`
- Declaration: `include/gui_forms/static_tree.hpp:21`
- Definition: `src/core/static_tree.cpp`

ControlFactory is a class declared in include/gui_forms/static_tree.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `register_type` (public)

```cpp
void register_type(std::string type, Creator creator)
```

Public ControlFactory operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `create` (public)

```cpp
[[nodiscard]] Control::Ptr create(std::string_view type, StableId stable_id) const
```

Reports the current create value without mutation.
