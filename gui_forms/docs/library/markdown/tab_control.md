# TabControl

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `ContainerControl → TabControl`  
Declaration: `include/gui_forms/container_controls.hpp:306`  
Definition: `src/controls/container_controls.cpp`

TabControl is a visual retained control declared in include/gui_forms/container_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `TabControl`

```cpp
explicit TabControl(StableId stable_id)
```

Constructs or tears down the retained TabControl object according to its ownership contract.

### `add_page`

```cpp
void add_page(std::shared_ptr<TabPage> page)
```

Public TabControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_page`

```cpp
[[nodiscard]] std::shared_ptr<TabPage> remove_page(const TabPage& page)
```

Public TabControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `pages`

```cpp
[[nodiscard]] std::vector<std::shared_ptr<TabPage>> pages() const
```

Reports the current pages value without mutation.

### `page_count`

```cpp
[[nodiscard]] std::size_t page_count() const
```

Reports the current page count value without mutation.

### `page_at`

```cpp
[[nodiscard]] std::shared_ptr<TabPage> page_at(std::size_t index) const
```

Reports the current page at value without mutation.

### `selected_index`

```cpp
[[nodiscard]] std::optional<std::size_t> selected_index() const
```

Reports the current selected index value without mutation.

### `selected_tab`

```cpp
[[nodiscard]] std::shared_ptr<TabPage> selected_tab() const noexcept
```

Reports the current selected tab value without mutation.

### `set_selected_index`

```cpp
void set_selected_index(std::size_t index)
```

Synchronously updates the retained selected index property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_selected_tab`

```cpp
void set_selected_tab(const std::shared_ptr<TabPage>& page)
```

Synchronously updates the retained selected tab property. Validation, typed invalidation, and notifications are defined by the implementation.

### `alignment`

```cpp
[[nodiscard]] TabAlignment alignment() const noexcept
```

Reports the current alignment value without mutation.

### `set_alignment`

```cpp
void set_alignment(TabAlignment alignment)
```

Synchronously updates the retained alignment property. Validation, typed invalidation, and notifications are defined by the implementation.

### `appearance`

```cpp
[[nodiscard]] TabAppearance appearance() const noexcept
```

Reports the current appearance value without mutation.

### `set_appearance`

```cpp
void set_appearance(TabAppearance appearance)
```

Synchronously updates the retained appearance property. Validation, typed invalidation, and notifications are defined by the implementation.

### `item_size`

```cpp
[[nodiscard]] Size item_size() const noexcept
```

Reports the current item size value without mutation.

### `set_item_size`

```cpp
void set_item_size(Size size)
```

Synchronously updates the retained item size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `style`

```cpp
[[nodiscard]] const BasicControlStyle& style() const noexcept
```

Reports the current style value without mutation.

### `set_style`

```cpp
void set_style(BasicControlStyle style)
```

Synchronously updates the retained style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `tab_bounds`

```cpp
[[nodiscard]] Rect tab_bounds(std::size_t index) const
```

Reports the current tab bounds value without mutation.

### `display_bounds`

```cpp
[[nodiscard]] Rect display_bounds() const noexcept
```

Reports the current display bounds value without mutation.

### `selected_index_changed`

```cpp
[[nodiscard]] Event<const TabSelectionChange&>& selected_index_changed() noexcept
```

Public TabControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `measure`

```cpp
[[nodiscard]] Size measure(Size available) override
```

Computes desired size from the available constraint without arranging children.

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

### `on_key_preview`

```cpp
void on_key_preview(KeyEvent& event) override
```

Public TabControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

Public TabControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
