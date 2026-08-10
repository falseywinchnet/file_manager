# TreeView

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → TreeView`  
Declaration: `include/gui_forms/collection_controls.hpp:40`  
Definition: `src/controls/collection_controls.cpp`

TreeView is a visual retained control declared in include/gui_forms/collection_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `TreeView`

```cpp
explicit TreeView(StableId stable_id)
```

Constructs or tears down the retained TreeView object according to its ownership contract.

### `items`

```cpp
[[nodiscard]] std::span<const TreeViewItem> items() const noexcept
```

Reports the current items value without mutation.

### `set_items`

```cpp
void set_items(std::vector<TreeViewItem> items)
```

Synchronously updates the retained items property. Validation, typed invalidation, and notifications are defined by the implementation.

### `selected_id`

```cpp
[[nodiscard]] std::string_view selected_id() const noexcept
```

Reports the current selected id value without mutation.

### `set_selected_id`

```cpp
void set_selected_id(std::string_view stable_id)
```

Synchronously updates the retained selected id property. Validation, typed invalidation, and notifications are defined by the implementation.

### `active_id`

```cpp
[[nodiscard]] std::string_view active_id() const noexcept
```

Reports the current active id value without mutation.

### `expanded`

```cpp
[[nodiscard]] bool expanded(std::string_view stable_id) const
```

Reports the current expanded value without mutation.

### `set_expanded`

```cpp
void set_expanded(std::string_view stable_id, bool expanded)
```

Synchronously updates the retained expanded property. Validation, typed invalidation, and notifications are defined by the implementation.

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

### `indentation`

```cpp
[[nodiscard]] double indentation() const noexcept
```

Reports the current indentation value without mutation.

### `set_indentation`

```cpp
void set_indentation(double indentation)
```

Synchronously updates the retained indentation property. Validation, typed invalidation, and notifications are defined by the implementation.

### `top_row`

```cpp
[[nodiscard]] std::size_t top_row() const noexcept
```

Reports the current top row value without mutation.

### `set_top_row`

```cpp
void set_top_row(std::size_t row)
```

Synchronously updates the retained top row property. Validation, typed invalidation, and notifications are defined by the implementation.

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

### `image_list`

```cpp
[[nodiscard]] std::shared_ptr<ImageList> image_list() const noexcept
```

Reports the current image list value without mutation.

### `set_image_list`

```cpp
void set_image_list(std::shared_ptr<ImageList> image_list)
```

Synchronously updates the retained image list property. Validation, typed invalidation, and notifications are defined by the implementation.

### `selection_changed`

```cpp
[[nodiscard]] Event<const TreeSelectionChange&>& selection_changed() noexcept
```

Public TreeView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `expansion_changed`

```cpp
[[nodiscard]] Event<const TreeExpansionChange&>& expansion_changed() noexcept
```

Public TreeView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `item_activated`

```cpp
[[nodiscard]] Event<const std::string&>& item_activated() noexcept
```

Public TreeView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `arrange`

```cpp
void arrange(Rect final_bounds) override
```

Commits final geometry and arranges retained child roles within it.

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

### `on_text_input`

```cpp
void on_text_input(TextInputEvent& event) override
```

Public TreeView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

Public TreeView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
