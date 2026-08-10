# ListBox

- Status: **OBSERVED: bundle 004 split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → ListBox`
- Declaration: `include/gui_forms/controls/panel/list_box/list_box.hpp:26`
- Definition: `src/controls/panel/list_box/list_box.cpp`

ListBox is a retained virtual-row selector with single, multi-simple, and multi-extended policies; stable item identities; scroll-window projection; keyboard/pointer range selection; activation; and virtual child semantics.

## Visual evidence

![ListBox](../captures/list_box_combo_box.png)

## Declared methods

### `ListBox` (public)

```cpp
explicit ListBox(StableId stable_id)
```

Constructs a focusable clipped row surface with vertical scrolling and collection semantics.

### `items` (public)

```cpp
[[nodiscard]] std::span<const std::string> items() const noexcept
```

Returns the authoritative ordered display strings.

### `set_items` (public)

```cpp
virtual void set_items(std::vector<std::string> items)
```

Replaces the collection, repairs identity/selection/scroll state, and publishes only real selection changes.

### `add_item` (public)

```cpp
virtual void add_item(std::string item)
```

Appends one display item with a deterministic identity and invalidates layout, paint, and semantics.

### `remove_item` (public)

```cpp
virtual void remove_item(std::size_t index)
```

Removes a valid row, shifts selection and identity state, and reports whether removal occurred.

### `clear_items` (public)

```cpp
virtual void clear_items()
```

Clears rows, identities, selection, active anchor, and scroll position atomically.

### `set_item_stable_ids` (public)

```cpp
void set_item_stable_ids(std::vector<std::string> stable_ids)
```

Installs caller-provided unique nonzero row identities after exact cardinality validation.

### `item_stable_id` (public)

```cpp
[[nodiscard]] std::string item_stable_id(std::size_t index) const
```

Returns the durable semantic identity for a valid row.

### `selection_mode` (public)

```cpp
[[nodiscard]] ListSelectionMode selection_mode() const noexcept
```

Returns the active none, single, multi-simple, or multi-extended policy.

### `set_selection_mode` (public)

```cpp
void set_selection_mode(ListSelectionMode mode)
```

Validates policy and reconciles existing selection to its admitted cardinality.

### `selected_indices` (public)

```cpp
[[nodiscard]] std::span<const std::size_t> selected_indices() const noexcept
```

Returns selected row indexes in ascending order.

### `selected_index` (public)

```cpp
[[nodiscard]] std::optional<std::size_t> selected_index() const noexcept
```

Returns the active selected row or no value when selection is empty.

### `select_index` (public)

```cpp
void select_index(std::size_t index, bool extend = false, bool toggle = false)
```

Applies programmatic selection using the current selection-mode law.

### `clear_selection` (public)

```cpp
void clear_selection()
```

Clears selected rows and publishes a typed old/new selection transition.

### `top_index` (public)

```cpp
[[nodiscard]] std::size_t top_index() const noexcept
```

Returns the first row projected into the viewport.

### `set_top_index` (public)

```cpp
void set_top_index(std::size_t index)
```

Constrains a requested first row to the available collection and viewport.

### `item_height` (public)

```cpp
[[nodiscard]] double item_height() const noexcept
```

Returns the logical fixed row height.

### `set_item_height` (public)

```cpp
void set_item_height(double height)
```

Accepts a finite bounded positive height and invalidates scrolling, layout, and paint.

### `font` (public)

```cpp
[[nodiscard]] FontSpec font() const noexcept
```

Returns the retained row FontSpec.

### `set_font` (public)

```cpp
void set_font(FontSpec font)
```

Validates row typography and invalidates measurement, paint, and semantics.

### `selection_changed` (public)

```cpp
[[nodiscard]] Event<const ListSelectionChange&>& selection_changed() noexcept
```

Returns typed old/new index collections after selection commits.

### `item_activated` (public)

```cpp
[[nodiscard]] Event<std::size_t>& item_activated() noexcept
```

Returns the event published for qualified double-click or keyboard activation.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records only visible rows with alternating, hover, selection, focus, text, and extension adornment states.

### `arrange` (public)

```cpp
void arrange(Rect final_bounds) override
```

Updates viewport geometry and vertical scroll extent from row count and fixed height.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Maps pointer gestures to rows, focus, selection-mode modifiers, scrolling, and activation.

### `on_key` (public)

```cpp
void on_key(KeyEvent& event) override
```

Implements navigation, home/end/page movement, range extension, toggling, activation, and type-independent selection.

### `on_focus_changed` (public)

```cpp
void on_focus_changed(bool focused) override
```

Commits the focus cue and invalidates active-row painting.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects a list role with selection cardinality and active value.

### `semantic_virtual_children` (public)

```cpp
[[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override
```

Exposes every logical row as a stable selectable virtual option independent of viewport realization.

### `on_semantic_child_action` (public)

```cpp
bool on_semantic_child_action(std::string_view stable_id, SemanticAction action, std::string_view value) override
```

Maps row select/press actions to the same selection and activation state machine.

### `row_text_left` (protected)

```cpp
[[nodiscard]] virtual double row_text_left() const noexcept
```

Reports the current row text left value without mutation.

### `paint_row_adornment` (protected)

```cpp
virtual void paint_row_adornment(Painter& painter, std::size_t index, Rect row_bounds, bool selected, bool focused) const
```

Reports the current paint row adornment value without mutation.

### `active_index_for_extension` (protected)

```cpp
[[nodiscard]] std::optional<std::size_t> active_index_for_extension() const noexcept
```

Reports the current active index for extension value without mutation.

### `focused_for_extension` (protected)

```cpp
[[nodiscard]] bool focused_for_extension() const noexcept
```

Reports the current focused for extension value without mutation.

### `index_at` (protected)

```cpp
[[nodiscard]] std::optional<std::size_t> index_at( Point absolute) const noexcept
```

Reports the current index at value without mutation.

### `apply_selection` (private)

```cpp
void apply_selection(std::vector<std::size_t> selection, std::optional<std::size_t> active)
```

Executes ListBox's apply selection operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `ensure_visible` (private)

```cpp
void ensure_visible(std::size_t index)
```

Executes ListBox's ensure visible operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `visible_row_count` (private)

```cpp
[[nodiscard]] std::size_t visible_row_count() const noexcept
```

Reports the current visible row count value without mutation.
