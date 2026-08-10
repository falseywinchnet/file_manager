# ListBox

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → ListBox`  
Declaration: `include/gui_forms/input_controls.hpp:162`  
Definition: `src/controls/input_controls.cpp`

ListBox is a visual retained control declared in include/gui_forms/input_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ListBox`

```cpp
explicit ListBox(StableId stable_id)
```

Constructs or tears down the retained ListBox object according to its ownership contract.

### `items`

```cpp
[[nodiscard]] std::span<const std::string> items() const noexcept
```

Reports the current items value without mutation.

### `set_items`

```cpp
virtual void set_items(std::vector<std::string> items)
```

Synchronously updates the retained items property. Validation, typed invalidation, and notifications are defined by the implementation.

### `add_item`

```cpp
virtual void add_item(std::string item)
```

Public ListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_item`

```cpp
virtual void remove_item(std::size_t index)
```

Public ListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear_items`

```cpp
virtual void clear_items()
```

Removes the explicit items value and restores fallback behavior.

### `set_item_stable_ids`

```cpp
void set_item_stable_ids(std::vector<std::string> stable_ids)
```

Synchronously updates the retained item stable ids property. Validation, typed invalidation, and notifications are defined by the implementation.

### `item_stable_id`

```cpp
[[nodiscard]] std::string item_stable_id(std::size_t index) const
```

Reports the current item stable id value without mutation.

### `selection_mode`

```cpp
[[nodiscard]] ListSelectionMode selection_mode() const noexcept
```

Reports the current selection mode value without mutation.

### `set_selection_mode`

```cpp
void set_selection_mode(ListSelectionMode mode)
```

Synchronously updates the retained selection mode property. Validation, typed invalidation, and notifications are defined by the implementation.

### `selected_indices`

```cpp
[[nodiscard]] std::span<const std::size_t> selected_indices() const noexcept
```

Reports the current selected indices value without mutation.

### `selected_index`

```cpp
[[nodiscard]] std::optional<std::size_t> selected_index() const noexcept
```

Reports the current selected index value without mutation.

### `select_index`

```cpp
void select_index(std::size_t index, bool extend = false, bool toggle = false)
```

Public ListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear_selection`

```cpp
void clear_selection()
```

Removes the explicit selection value and restores fallback behavior.

### `top_index`

```cpp
[[nodiscard]] std::size_t top_index() const noexcept
```

Reports the current top index value without mutation.

### `set_top_index`

```cpp
void set_top_index(std::size_t index)
```

Synchronously updates the retained top index property. Validation, typed invalidation, and notifications are defined by the implementation.

### `item_height`

```cpp
[[nodiscard]] double item_height() const noexcept
```

Reports the current item height value without mutation.

### `set_item_height`

```cpp
void set_item_height(double height)
```

Synchronously updates the retained item height property. Validation, typed invalidation, and notifications are defined by the implementation.

### `font`

```cpp
[[nodiscard]] FontSpec font() const noexcept
```

Reports the current font value without mutation.

### `set_font`

```cpp
void set_font(FontSpec font)
```

Synchronously updates the retained font property. Validation, typed invalidation, and notifications are defined by the implementation.

### `selection_changed`

```cpp
[[nodiscard]] Event<const ListSelectionChange&>& selection_changed() noexcept
```

Public ListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `item_activated`

```cpp
[[nodiscard]] Event<std::size_t>& item_activated() noexcept
```

Public ListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_paint`

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records renderer-neutral paint operations for the damaged local region.

### `arrange`

```cpp
void arrange(Rect final_bounds) override
```

Commits final geometry and arranges retained child roles within it.

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

Public ListBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
