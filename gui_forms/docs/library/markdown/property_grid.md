# PropertyGrid

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → PropertyGrid`  
Declaration: `include/gui_forms/inspection_controls.hpp:421`  
Definition: `src/controls/inspection_controls.cpp`

PropertyGrid is a visual retained control declared in include/gui_forms/inspection_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `PropertyGrid`

```cpp
explicit PropertyGrid(StableId stable_id)
```

Constructs or tears down the retained PropertyGrid object according to its ownership contract.

### `~PropertyGrid`

```cpp
~PropertyGrid() override
```

Constructs or tears down the retained PropertyGrid object according to its ownership contract.

### `initialize_control_tree`

```cpp
void initialize_control_tree()
```

Idempotently attaches lazily constructed internal controls before layout or use.

### `selected_object`

```cpp
[[nodiscard]] Control::Ptr selected_object() const noexcept
```

Reports the current selected object value without mutation.

### `set_selected_object`

```cpp
void set_selected_object(Control::Ptr object)
```

Synchronously updates the retained selected object property. Validation, typed invalidation, and notifications are defined by the implementation.

### `selected_objects`

```cpp
[[nodiscard]] std::vector<Control::Ptr> selected_objects() const
```

Reports the current selected objects value without mutation.

### `set_selected_objects`

```cpp
void set_selected_objects(std::vector<Control::Ptr> objects)
```

Synchronously updates the retained selected objects property. Validation, typed invalidation, and notifications are defined by the implementation.

### `property_sort`

```cpp
[[nodiscard]] PropertySort property_sort() const noexcept
```

Reports the current property sort value without mutation.

### `set_property_sort`

```cpp
void set_property_sort(PropertySort sort)
```

Synchronously updates the retained property sort property. Validation, typed invalidation, and notifications are defined by the implementation.

### `refresh_properties`

```cpp
void refresh_properties()
```

Public PropertyGrid operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `property_list`

```cpp
[[nodiscard]] std::shared_ptr<PropertyList> property_list() const noexcept
```

Reports the current property list value without mutation.

### `converter_registry`

```cpp
[[nodiscard]] std::shared_ptr<PropertyValueConverterRegistry> converter_registry() const noexcept
```

Reports the current converter registry value without mutation.

### `set_converter_registry`

```cpp
void set_converter_registry( std::shared_ptr<PropertyValueConverterRegistry> registry)
```

Synchronously updates the retained converter registry property. Validation, typed invalidation, and notifications are defined by the implementation.

### `editor_registry`

```cpp
[[nodiscard]] std::shared_ptr<PropertyEditorRegistry> editor_registry() const noexcept
```

Reports the current editor registry value without mutation.

### `set_editor_registry`

```cpp
void set_editor_registry(std::shared_ptr<PropertyEditorRegistry> registry)
```

Synchronously updates the retained editor registry property. Validation, typed invalidation, and notifications are defined by the implementation.

### `editor`

```cpp
[[nodiscard]] Control::Ptr editor(std::string_view property_name) const
```

Reports the current editor value without mutation.

### `reset_button`

```cpp
[[nodiscard]] std::shared_ptr<Button> reset_button( std::string_view property_name) const
```

Returns button to its inherited or default policy.

### `selected_descriptor`

```cpp
[[nodiscard]] std::optional<PropertyDescriptor> selected_descriptor( std::string_view property_name) const
```

Reports the current selected descriptor value without mutation.

### `selected_origin`

```cpp
[[nodiscard]] std::optional<PropertyValueOrigin> selected_origin( std::string_view property_name) const
```

Reports the current selected origin value without mutation.

### `set_property_expanded`

```cpp
bool set_property_expanded(std::string_view property_name, bool expanded)
```

Synchronously updates the retained property expanded property. Validation, typed invalidation, and notifications are defined by the implementation.

### `property_expanded`

```cpp
[[nodiscard]] std::optional<bool> property_expanded( std::string_view property_name) const
```

Reports the current property expanded value without mutation.

### `try_set_property_value`

```cpp
bool try_set_property_value(std::string_view property_name, BindingValue value)
```

Public PropertyGrid operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `try_set_property_text`

```cpp
bool try_set_property_text(std::string_view property_name, std::string_view text)
```

Public PropertyGrid operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `activate_property_editor`

```cpp
bool activate_property_editor(std::string_view property_name)
```

Public PropertyGrid operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `reset_property`

```cpp
bool reset_property(std::string_view property_name)
```

Returns property to its inherited or default policy.

### `insert_collection_item`

```cpp
bool insert_collection_item(std::string_view property_name, std::size_t index, BindingValue value)
```

Public PropertyGrid operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_collection_item`

```cpp
bool remove_collection_item(std::string_view property_name, std::size_t index)
```

Public PropertyGrid operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `move_collection_item`

```cpp
bool move_collection_item(std::string_view property_name, std::size_t from, std::size_t to)
```

Public PropertyGrid operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `last_error`

```cpp
[[nodiscard]] std::optional<PropertyGridEditError> last_error() const
```

Reports the current last error value without mutation.

### `selected_object_changed`

```cpp
[[nodiscard]] Event<Control::Ptr>& selected_object_changed() noexcept
```

Public PropertyGrid operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `property_value_changed`

```cpp
[[nodiscard]] Event<const PropertyGridValueChange&>& property_value_changed() noexcept
```

Public PropertyGrid operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `edit_failed`

```cpp
[[nodiscard]] Event<const PropertyGridEditError&>& edit_failed() noexcept
```

Public PropertyGrid operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.
