# MenuItemSpec

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `MenuItemSpec`  
Declaration: `include/gui_forms/menu_controls.hpp:31`  
Definition: `inline/header-only`

MenuItemSpec is a struct declared in include/gui_forms/menu_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `MenuItemSpec`

```cpp
MenuItemSpec() = default
```

Constructs or tears down the retained MenuItemSpec object according to its ownership contract.

### `MenuItemSpec`

```cpp
MenuItemSpec(std::string stable_identity, MenuItemKind item_kind, std::shared_ptr<Command> item_command =
```

Constructs or tears down the retained MenuItemSpec object according to its ownership contract.

### `stable_id`

```cpp
: stable_id(std::move(stable_identity)), kind(item_kind), command(std::move(item_command)), text(std::move(item_text)), children(std::move(item_children))
```

Public MenuItemSpec operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
