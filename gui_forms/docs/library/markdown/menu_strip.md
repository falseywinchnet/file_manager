# MenuStrip

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Control → MenuStrip`  
Declaration: `include/gui_forms/menu_controls.hpp:120`  
Definition: `src/controls/menu_controls.cpp`

MenuStrip is a visual retained control declared in include/gui_forms/menu_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `MenuStrip`

```cpp
explicit MenuStrip(StableId stable_id)
```

Constructs or tears down the retained MenuStrip object according to its ownership contract.

### `~MenuStrip`

```cpp
~MenuStrip() override
```

Constructs or tears down the retained MenuStrip object according to its ownership contract.

### `items`

```cpp
[[nodiscard]] const std::vector<MenuStripItemSpec>& items() const noexcept
```

Reports the current items value without mutation.

### `set_items`

```cpp
void set_items(std::vector<MenuStripItemSpec> items)
```

Synchronously updates the retained items property. Validation, typed invalidation, and notifications are defined by the implementation.

### `use_mnemonic`

```cpp
[[nodiscard]] bool use_mnemonic() const noexcept
```

Reports the current use mnemonic value without mutation.

### `set_use_mnemonic`

```cpp
void set_use_mnemonic(bool value)
```

Synchronously updates the retained use mnemonic property. Validation, typed invalidation, and notifications are defined by the implementation.

### `active_index`

```cpp
[[nodiscard]] std::optional<std::size_t> active_index() const noexcept
```

Reports the current active index value without mutation.

### `open`

```cpp
bool open(std::size_t index)
```

Public MenuStrip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `close`

```cpp
void close() noexcept
```

Public MenuStrip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `is_open`

```cpp
[[nodiscard]] bool is_open() const noexcept
```

Reports the current is open value without mutation.

### `item_invoked`

```cpp
[[nodiscard]] Event<const MenuStripInvocation&>& item_invoked() noexcept
```

Public MenuStrip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `open_changed`

```cpp
[[nodiscard]] Event<std::optional<std::size_t>>& open_changed() noexcept
```

Public MenuStrip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `measure`

```cpp
[[nodiscard]] Size measure(Size available) override
```

Computes desired size from the available constraint without arranging children.

### `on_paint`

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records renderer-neutral paint operations for the damaged local region.

### `on_pointer`

```cpp
void on_pointer(PointerEvent& event) override
```

Consumes normalized routed pointer input and updates retained interaction state.

### `on_key`

```cpp
void on_key(KeyEvent& event) override
```

Consumes normalized keyboard input for this control's interaction contract.

### `on_focus_changed`

```cpp
void on_focus_changed(bool focused) override
```

Updates focus-dependent retained state and invalidates affected presentation/semantics.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.

### `semantic_virtual_children`

```cpp
[[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override
```

Reports the current semantic virtual children value without mutation.

### `on_semantic_child_action`

```cpp
bool on_semantic_child_action(std::string_view stable_id, SemanticAction action, std::string_view value) override
```

Public MenuStrip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
