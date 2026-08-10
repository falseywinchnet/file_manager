# MenuItemSpec

- Status: **OBSERVED: bundle 011 context-menu item review**
- Kind: **struct**
- Hierarchy: `MenuItemSpec`
- Declaration: `include/gui_forms/components/context_menu/context_menu.hpp:31`
- Definition: `inline/header-only`

MenuItemSpec owns stable identity, mnemonic text, shortcut display, enabled/checked/separator state, optional command, and nested items for one context-menu entry.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `MenuItemSpec` (public)

```cpp
MenuItemSpec() = default
```

Constructs or tears down the retained MenuItemSpec object according to its ownership contract.

### `MenuItemSpec` (public)

```cpp
MenuItemSpec(std::string stable_identity, MenuItemKind item_kind, std::shared_ptr<Command> item_command =
```

Constructs or tears down the retained MenuItemSpec object according to its ownership contract.

### `stable_id` (public)

```cpp
: stable_id(std::move(stable_identity)), kind(item_kind), command(std::move(item_command)), text(std::move(item_text)), children(std::move(item_children))
```

Executes MenuItemSpec's stable id operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
