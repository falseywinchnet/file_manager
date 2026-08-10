# TreeView

- Status: **OBSERVED: bundle 005 split and show-expanders enhancement; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → TreeView`
- Declaration: `include/gui_forms/controls/panel/tree_view/tree_view.hpp:40`
- Definition: `src/controls/panel/tree_view/tree_view.cpp`

TreeView is a virtualized retained hierarchy over caller-owned stable item identities. It derives the visible preorder from expansion state, keeps selection and active focus distinct, supports configurable row geometry and expander presentation, and projects virtual tree-item semantics without materializing child controls.

## Visual evidence

![TreeView](../captures/tree_view_object_view.png)

## Declared methods

### `TreeView` (public)

```cpp
explicit TreeView(StableId stable_id)
```

Constructs a focusable hierarchical selector with stable selection, expansion, type-selection, and virtual accessibility state.

### `items` (public)

```cpp
[[nodiscard]] std::span<const TreeViewItem> items() const noexcept
```

Returns the authored flat hierarchy records in caller order.

### `set_items` (public)

```cpp
void set_items(std::vector<TreeViewItem> items)
```

Validates stable identities and parent relationships, rebuilds the visible preorder, and reconciles selection, focus, and scrolling.

### `selected_id` (public)

```cpp
[[nodiscard]] std::string_view selected_id() const noexcept
```

Returns the selected stable item identity, or no value when selection is empty.

### `set_selected_id` (public)

```cpp
void set_selected_id(std::string_view stable_id)
```

Selects an admitted identity, makes it visible, and publishes only a real selection transition.

### `active_id` (public)

```cpp
[[nodiscard]] std::string_view active_id() const noexcept
```

Returns the keyboard-active visible item independently of selection.

### `expanded` (public)

```cpp
[[nodiscard]] bool expanded(std::string_view stable_id) const
```

Reports whether the identified branch is retained as expanded.

### `set_expanded` (public)

```cpp
void set_expanded(std::string_view stable_id, bool expanded)
```

Commits branch expansion, rebuilds visible rows, preserves valid focus/selection, and publishes the transition.

### `item_height` (public)

```cpp
[[nodiscard]] double item_height() const noexcept
```

Returns the logical height of each realized row.

### `set_item_height` (public)

```cpp
void set_item_height(double height)
```

Validates positive bounded row geometry and invalidates layout, paint, and semantics.

### `indentation` (public)

```cpp
[[nodiscard]] double indentation() const noexcept
```

Returns the logical horizontal offset applied per hierarchy depth.

### `set_indentation` (public)

```cpp
void set_indentation(double indentation)
```

Validates bounded depth spacing and refreshes hierarchy geometry.

### `show_expanders` (public)

```cpp
[[nodiscard]] bool show_expanders() const noexcept
```

Reports whether expandable rows paint disclosure affordances.

### `set_show_expanders` (public)

```cpp
void set_show_expanders(bool show)
```

Toggles disclosure painting without changing retained expansion authority.

### `top_row` (public)

```cpp
[[nodiscard]] std::size_t top_row() const noexcept
```

Returns the first visible preorder row projected into the viewport.

### `set_top_row` (public)

```cpp
void set_top_row(std::size_t row)
```

Clamps and commits the visible-row origin, then refreshes paint and virtual semantics.

### `font` (public)

```cpp
[[nodiscard]] FontSpec font() const noexcept
```

Returns the retained typography used for tree rows.

### `set_font` (public)

```cpp
void set_font(FontSpec font)
```

Validates typography and invalidates row measurement, paint, and semantics.

### `image_list` (public)

```cpp
[[nodiscard]] std::shared_ptr<ImageList> image_list() const noexcept
```

Returns the optional shared image source used by item image keys.

### `set_image_list` (public)

```cpp
void set_image_list(std::shared_ptr<ImageList> image_list)
```

Rebinds item imagery and refreshes visual and semantic projections.

### `selection_changed` (public)

```cpp
[[nodiscard]] Event<const TreeSelectionChange&>& selection_changed() noexcept
```

Returns the event published after authoritative selection changes.

### `expansion_changed` (public)

```cpp
[[nodiscard]] Event<const TreeExpansionChange&>& expansion_changed() noexcept
```

Returns the event carrying branch identity and committed expanded state.

### `item_activated` (public)

```cpp
[[nodiscard]] Event<const std::string&>& item_activated() noexcept
```

Returns the event raised after qualified pointer, keyboard, or semantic activation.

### `arrange` (public)

```cpp
void arrange(Rect final_bounds) override
```

Commits viewport geometry, clamps the row origin, and keeps the active row visible.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records visible hierarchy rows, depth guides, optional expanders, imagery, selection, hover, and focus.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Handles row hit testing, disclosure toggling, selection, capture qualification, and activation.

### `on_key` (public)

```cpp
void on_key(KeyEvent& event) override
```

Implements tree navigation, parent/child disclosure, paging, activation, and selection.

### `on_text_input` (public)

```cpp
void on_text_input(TextInputEvent& event) override
```

Performs time-bounded prefix selection across the current visible preorder.

### `on_focus_changed` (public)

```cpp
void on_focus_changed(bool focused) override
```

Commits focus presentation and ensures the active row remains usable.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the control as a tree with selection and item-count state.

### `semantic_virtual_children` (public)

```cpp
[[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override
```

Exposes visible items as stable virtual tree nodes with depth, expansion, selection, and action metadata.

### `on_semantic_child_action` (public)

```cpp
bool on_semantic_child_action(std::string_view stable_id, SemanticAction action, std::string_view value) override
```

Maps virtual selection, expansion, collapse, and press back through ordinary retained transitions.

### `on_attached_to_window` (protected)

```cpp
void on_attached_to_window() override
```

Public TreeView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `rebuild_visible` (private)

```cpp
void rebuild_visible()
```

Public TreeView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `item_index` (private)

```cpp
[[nodiscard]] std::optional<std::size_t> item_index( std::string_view stable_id) const noexcept
```

Reports the current item index value without mutation.

### `visible_row_at` (private)

```cpp
[[nodiscard]] std::optional<std::size_t> visible_row_at( Point absolute) const noexcept
```

Reports the current visible row at value without mutation.

### `visible_row_count` (private)

```cpp
[[nodiscard]] std::size_t visible_row_count() const noexcept
```

Reports the current visible row count value without mutation.

### `ensure_visible` (private)

```cpp
void ensure_visible(std::size_t visible_row)
```

Public TreeView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `select_visible_row` (private)

```cpp
void select_visible_row(std::size_t visible_row, bool activate)
```

Public TreeView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `type_select` (private)

```cpp
void type_select(std::string_view text)
```

Public TreeView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
