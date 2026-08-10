# PropertyRowSpec

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `PropertyRowSpec`  
Declaration: `include/gui_forms/inspection_controls.hpp:240`  
Definition: `inline/header-only`

PropertyRowSpec is a struct declared in include/gui_forms/inspection_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `PropertyRowSpec`

```cpp
PropertyRowSpec() = default
```

Constructs or tears down the retained PropertyRowSpec object according to its ownership contract.

### `PropertyRowSpec`

```cpp
PropertyRowSpec(std::string authored_stable_id, std::string authored_name, std::string authored_value, std::string authored_description =
```

Constructs or tears down the retained PropertyRowSpec object according to its ownership contract.

### `stable_id`

```cpp
std::uint8_t authored_depth = 0, bool authored_expandable = false, bool authored_expanded = false, bool authored_resettable = false, bool authored_reset_enabled = false) : stable_id(std::move(authored_stable_id)), name(std::move(authored_name)), value(std::move(authored_value)), description(std::move(authored_description)), editor(authored_editor), choices(std::move(authored_choices)), validation_message(std::move(authored_validation)), parent_id(std::move(authored_parent_id)), depth(authored_depth), enabled(authored_enabled), required(authored_required), expandable(authored_expandable), expanded(authored_expanded), resettable(authored_resettable), reset_enabled(authored_reset_enabled)
```

Public PropertyRowSpec operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
