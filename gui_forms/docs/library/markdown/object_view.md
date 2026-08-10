# ObjectView

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → ObjectView`  
Declaration: `include/gui_forms/collection_controls.hpp:172`  
Definition: `src/controls/collection_controls.cpp`

ObjectView is a visual retained control declared in include/gui_forms/collection_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ObjectView`

```cpp
explicit ObjectView(StableId stable_id)
```

Constructs or tears down the retained ObjectView object according to its ownership contract.

### `items`

```cpp
[[nodiscard]] std::span<const ObjectViewItem> items() const noexcept
```

Reports the current items value without mutation.

### `set_items`

```cpp
void set_items(std::vector<ObjectViewItem> items)
```

Synchronously updates the retained items property. Validation, typed invalidation, and notifications are defined by the implementation.

### `view_mode`

```cpp
[[nodiscard]] ObjectViewMode view_mode() const noexcept
```

Reports the current view mode value without mutation.

### `set_view_mode`

```cpp
void set_view_mode(ObjectViewMode mode)
```

Synchronously updates the retained view mode property. Validation, typed invalidation, and notifications are defined by the implementation.

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

### `selected_ids`

```cpp
[[nodiscard]] std::span<const std::string> selected_ids() const noexcept
```

Reports the current selected ids value without mutation.

### `set_selected_ids`

```cpp
void set_selected_ids(std::vector<std::string> stable_ids, std::string_view primary_id =
```

Synchronously updates the retained selected ids property. Validation, typed invalidation, and notifications are defined by the implementation.

### `clear_selection`

```cpp
void clear_selection()
```

Removes the explicit selection value and restores fallback behavior.

### `select_all`

```cpp
void select_all()
```

Public ObjectView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `selection_mode`

```cpp
[[nodiscard]] ObjectSelectionMode selection_mode() const noexcept
```

Reports the current selection mode value without mutation.

### `set_selection_mode`

```cpp
void set_selection_mode(ObjectSelectionMode mode)
```

Synchronously updates the retained selection mode property. Validation, typed invalidation, and notifications are defined by the implementation.

### `selection_anchor_id`

```cpp
[[nodiscard]] std::string_view selection_anchor_id() const noexcept
```

Reports the current selection anchor id value without mutation.

### `focused_id`

```cpp
[[nodiscard]] std::string_view focused_id() const noexcept
```

Reports the current focused id value without mutation.

### `icon_cell_size`

```cpp
[[nodiscard]] Size icon_cell_size() const noexcept
```

Reports the current icon cell size value without mutation.

### `set_icon_cell_size`

```cpp
void set_icon_cell_size(Size size)
```

Synchronously updates the retained icon cell size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `details_row_height`

```cpp
[[nodiscard]] double details_row_height() const noexcept
```

Reports the current details row height value without mutation.

### `set_details_row_height`

```cpp
void set_details_row_height(double height)
```

Synchronously updates the retained details row height property. Validation, typed invalidation, and notifications are defined by the implementation.

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
[[nodiscard]] Event<const ObjectSelectionChange&>& selection_changed() noexcept
```

Public ObjectView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `item_activated`

```cpp
[[nodiscard]] Event<const std::string&>& item_activated() noexcept
```

Public ObjectView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `context_requested`

```cpp
[[nodiscard]] Event<const ObjectContextRequest&>& context_requested() noexcept
```

Public ObjectView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

Public ObjectView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

Public ObjectView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
