# PropertyList

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → PropertyList`  
Declaration: `include/gui_forms/inspection_controls.hpp:318`  
Definition: `src/controls/inspection_controls.cpp`

PropertyList is a visual retained control declared in include/gui_forms/inspection_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `PropertyList`

```cpp
explicit PropertyList(StableId stable_id)
```

Constructs or tears down the retained PropertyList object according to its ownership contract.

### `~PropertyList`

```cpp
~PropertyList() override
```

Constructs or tears down the retained PropertyList object according to its ownership contract.

### `groups`

```cpp
[[nodiscard]] const std::vector<PropertyGroupSpec>& groups() const noexcept
```

Reports the current groups value without mutation.

### `set_groups`

```cpp
void set_groups(std::vector<PropertyGroupSpec> groups)
```

Synchronously updates the retained groups property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_value`

```cpp
bool set_value(std::string_view row_id, std::string value)
```

Synchronously updates the retained value property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_description`

```cpp
bool set_description(std::string_view row_id, std::string description)
```

Synchronously updates the retained description property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_validation`

```cpp
bool set_validation(std::string_view row_id, std::string message)
```

Synchronously updates the retained validation property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_reset_enabled`

```cpp
bool set_reset_enabled(std::string_view row_id, bool enabled)
```

Synchronously updates the retained reset enabled property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_group_expanded`

```cpp
bool set_group_expanded(std::string_view group_id, bool expanded)
```

Synchronously updates the retained group expanded property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_row_expanded`

```cpp
bool set_row_expanded(std::string_view row_id, bool expanded)
```

Synchronously updates the retained row expanded property. Validation, typed invalidation, and notifications are defined by the implementation.

### `row_expanded`

```cpp
[[nodiscard]] std::optional<bool> row_expanded( std::string_view row_id) const
```

Reports the current row expanded value without mutation.

### `value`

```cpp
[[nodiscard]] std::optional<std::string> value(std::string_view row_id) const
```

Reports the current value value without mutation.

### `editor`

```cpp
[[nodiscard]] Control::Ptr editor(std::string_view row_id) const
```

Reports the current editor value without mutation.

### `replace_editor`

```cpp
bool replace_editor(std::string_view row_id, Control::Ptr editor)
```

Public PropertyList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `reset_button`

```cpp
[[nodiscard]] std::shared_ptr<Button> reset_button( std::string_view row_id) const
```

Returns button to its inherited or default policy.

### `set_header_content`

```cpp
void set_header_content(Control::Ptr content, double height)
```

Synchronously updates the retained header content property. Validation, typed invalidation, and notifications are defined by the implementation.

### `header_content`

```cpp
[[nodiscard]] Control::Ptr header_content() const noexcept
```

Reports the current header content value without mutation.

### `header_height`

```cpp
[[nodiscard]] double header_height() const noexcept
```

Reports the current header height value without mutation.

### `set_header_height`

```cpp
void set_header_height(double height)
```

Synchronously updates the retained header height property. Validation, typed invalidation, and notifications are defined by the implementation.

### `label_width`

```cpp
[[nodiscard]] double label_width() const noexcept
```

Reports the current label width value without mutation.

### `set_label_width`

```cpp
void set_label_width(double width)
```

Synchronously updates the retained label width property. Validation, typed invalidation, and notifications are defined by the implementation.

### `scroll_offset`

```cpp
[[nodiscard]] double scroll_offset() const noexcept
```

Reports the current scroll offset value without mutation.

### `set_scroll_offset`

```cpp
void set_scroll_offset(double offset)
```

Synchronously updates the retained scroll offset property. Validation, typed invalidation, and notifications are defined by the implementation.

### `content_height`

```cpp
[[nodiscard]] double content_height() const noexcept
```

Reports the current content height value without mutation.

### `value_changed`

```cpp
[[nodiscard]] Event<const PropertyValueChange&>& value_changed() noexcept
```

Public PropertyList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `value_committed`

```cpp
[[nodiscard]] Event<const PropertyValueChange&>& value_committed() noexcept
```

Public PropertyList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `group_changed`

```cpp
[[nodiscard]] Event<const PropertyGroupChange&>& group_changed() noexcept
```

Public PropertyList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `row_expansion_changed`

```cpp
[[nodiscard]] Event<const PropertyRowExpansionChange&>& row_expansion_changed() noexcept
```

Public PropertyList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `reset_requested`

```cpp
[[nodiscard]] Event<const PropertyResetRequest&>& reset_requested() noexcept
```

Returns requested to its inherited or default policy.

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

### `on_key`

```cpp
void on_key(KeyEvent& event) override
```

Consumes normalized keyboard input for this control's interaction contract.

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

Public PropertyList operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
