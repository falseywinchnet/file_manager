# ComboBox

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → ComboBox`  
Declaration: `include/gui_forms/input_controls.hpp:297`  
Definition: `src/controls/input_controls.cpp`

ComboBox is a visual retained control declared in include/gui_forms/input_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ComboBox`

```cpp
explicit ComboBox(StableId stable_id)
```

Constructs or tears down the retained ComboBox object according to its ownership contract.

### `items`

```cpp
[[nodiscard]] std::span<const std::string> items() const noexcept
```

Reports the current items value without mutation.

### `set_items`

```cpp
void set_items(std::vector<std::string> items)
```

Synchronously updates the retained items property. Validation, typed invalidation, and notifications are defined by the implementation.

### `add_item`

```cpp
void add_item(std::string item)
```

Public ComboBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `selected_index`

```cpp
[[nodiscard]] std::optional<std::size_t> selected_index() const noexcept
```

Reports the current selected index value without mutation.

### `set_selected_index`

```cpp
void set_selected_index(std::optional<std::size_t> index)
```

Synchronously updates the retained selected index property. Validation, typed invalidation, and notifications are defined by the implementation.

### `selected_text`

```cpp
[[nodiscard]] std::string_view selected_text() const noexcept
```

Reports the current selected text value without mutation.

### `placeholder_text`

```cpp
[[nodiscard]] std::string_view placeholder_text() const noexcept
```

Reports the current placeholder text value without mutation.

### `set_placeholder_text`

```cpp
void set_placeholder_text(std::string text)
```

Synchronously updates the retained placeholder text property. Validation, typed invalidation, and notifications are defined by the implementation.

### `dropped_down`

```cpp
[[nodiscard]] bool dropped_down() const noexcept
```

Reports the current dropped down value without mutation.

### `set_dropped_down`

```cpp
void set_dropped_down(bool dropped_down)
```

Synchronously updates the retained dropped down property. Validation, typed invalidation, and notifications are defined by the implementation.

### `maximum_drop_down_items`

```cpp
[[nodiscard]] std::size_t maximum_drop_down_items() const noexcept
```

Reports the current maximum drop down items value without mutation.

### `set_maximum_drop_down_items`

```cpp
void set_maximum_drop_down_items(std::size_t count)
```

Synchronously updates the retained maximum drop down items property. Validation, typed invalidation, and notifications are defined by the implementation.

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

### `selected_index_changed`

```cpp
[[nodiscard]] Event<std::optional<std::size_t>>& selected_index_changed() noexcept
```

Public ComboBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `drop_down_changed`

```cpp
[[nodiscard]] Event<bool>& drop_down_changed() noexcept
```

Public ComboBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `items_changed`

```cpp
[[nodiscard]] Event<>& items_changed() noexcept
```

Public ComboBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

### `on_semantic_action`

```cpp
bool on_semantic_action(SemanticAction action, std::string_view value) override
```

Public ComboBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
