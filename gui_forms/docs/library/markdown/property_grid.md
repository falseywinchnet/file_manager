# PropertyGrid

- Status: **OBSERVED: bundle 006 control/Impl source split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → PropertyGrid`
- Declaration: `include/gui_forms/controls/panel/property_grid/property_grid.hpp:14`
- Definition: `src/controls/panel/property_grid/property_grid.cpp`

PropertyGrid is a metadata-driven retained inspector/editor over one or multiple selected Controls. It projects inert descriptors into PropertyList, instance-owned converters and editor factories, expands bounded compound and immutable collection values through stable paths, commits multi-owner edits atomically with rollback, preserves value origins/reset policy, and exposes the same real editors to pointer, accessibility, binding, and automation.

## Visual evidence

![PropertyGrid](../captures/property_grid.png)

## Declared methods

### `PropertyGrid` (public)

```cpp
explicit PropertyGrid(StableId stable_id)
```

Constructs source-private transactional state with default converter/editor registries.

### `~PropertyGrid` (public)

```cpp
~PropertyGrid() override
```

Disconnects selected-object/list/editor state after ordinary disposal.

### `initialize_control_tree` (public)

```cpp
void initialize_control_tree()
```

Idempotently creates and owns the internal PropertyList after shared ownership exists.

### `selected_object` (public)

```cpp
[[nodiscard]] Control::Ptr selected_object() const noexcept
```

Returns the sole selected object, or no object for empty/multiple selection.

### `set_selected_object` (public)

```cpp
void set_selected_object(Control::Ptr object)
```

Replaces selection with zero or one Control through the common multiple-selection path.

### `selected_objects` (public)

```cpp
[[nodiscard]] std::vector<Control::Ptr> selected_objects() const
```

Returns all selected Controls in transaction order.

### `set_selected_objects` (public)

```cpp
void set_selected_objects(std::vector<Control::Ptr> objects)
```

Validates live Controls, derives their common editable schema, reconnects lifetime observation, and rebuilds rows/editors.

### `property_sort` (public)

```cpp
[[nodiscard]] PropertySort property_sort() const noexcept
```

Returns categorized or alphabetical projection policy.

### `set_property_sort` (public)

```cpp
void set_property_sort(PropertySort sort)
```

Validates sort policy and rebuilds the visible descriptor projection without changing selected values.

### `refresh_properties` (public)

```cpp
void refresh_properties()
```

Re-reads descriptors, values, origins, and reset state from selected Controls and synchronizes existing editor projection.

### `property_list` (public)

```cpp
[[nodiscard]] std::shared_ptr<PropertyList> property_list() const noexcept
```

Returns the owned retained PropertyList surface.

### `converter_registry` (public)

```cpp
[[nodiscard]] std::shared_ptr<PropertyValueConverterRegistry> converter_registry() const noexcept
```

Returns the active instance-owned value converter registry.

### `set_converter_registry` (public)

```cpp
void set_converter_registry( std::shared_ptr<PropertyValueConverterRegistry> registry)
```

Requires a registry, replaces formatting/parsing policy, and rebuilds rows/editors.

### `editor_registry` (public)

```cpp
[[nodiscard]] std::shared_ptr<PropertyEditorRegistry> editor_registry() const noexcept
```

Returns the active instance-owned retained-editor registry.

### `set_editor_registry` (public)

```cpp
void set_editor_registry(std::shared_ptr<PropertyEditorRegistry> registry)
```

Requires a registry, replaces factory policy, and rebuilds real editors.

### `editor` (public)

```cpp
[[nodiscard]] Control::Ptr editor(std::string_view property_name) const
```

Returns the real retained editor at a stable property/member/index path.

### `reset_button` (public)

```cpp
[[nodiscard]] std::shared_ptr<Button> reset_button( std::string_view property_name) const
```

Returns the real reset button for a property path.

### `selected_descriptor` (public)

```cpp
[[nodiscard]] std::optional<PropertyDescriptor> selected_descriptor( std::string_view property_name) const
```

Returns the common inert descriptor for a selected property path.

### `selected_origin` (public)

```cpp
[[nodiscard]] std::optional<PropertyValueOrigin> selected_origin( std::string_view property_name) const
```

Returns a common value origin when all selected owners agree.

### `set_property_expanded` (public)

```cpp
bool set_property_expanded(std::string_view property_name, bool expanded)
```

Commits stable compound/collection path disclosure through PropertyList.

### `property_expanded` (public)

```cpp
[[nodiscard]] std::optional<bool> property_expanded( std::string_view property_name) const
```

Returns retained disclosure state for a projected property path.

### `try_set_property_value` (public)

```cpp
bool try_set_property_value(std::string_view property_name, BindingValue value)
```

Validates and commits a typed value to every selected owner as one rollback-safe transaction.

### `try_set_property_text` (public)

```cpp
bool try_set_property_text(std::string_view property_name, std::string_view text)
```

Parses context-formatted text with the active converter and enters the same typed transaction.

### `activate_property_editor` (public)

```cpp
bool activate_property_editor(std::string_view property_name)
```

Invokes the installed real editor through its semantic Press action rather than bypassing control behavior.

### `reset_property` (public)

```cpp
bool reset_property(std::string_view property_name)
```

Requests registered reset on every owner as one rollback-safe transaction.

### `insert_collection_item` (public)

```cpp
bool insert_collection_item(std::string_view property_name, std::size_t index, BindingValue value)
```

Rebuilds an immutable collection with an inserted item and commits through the owning registered setter.

### `remove_collection_item` (public)

```cpp
bool remove_collection_item(std::string_view property_name, std::size_t index)
```

Rebuilds an immutable collection without the indexed item and commits transactionally.

### `move_collection_item` (public)

```cpp
bool move_collection_item(std::string_view property_name, std::size_t from, std::size_t to)
```

Reorders an immutable collection by stable index path and commits transactionally.

### `last_error` (public)

```cpp
[[nodiscard]] std::optional<PropertyGridEditError> last_error() const
```

Returns the last exact edit/parse/transaction failure.

### `selected_object_changed` (public)

```cpp
[[nodiscard]] Event<Control::Ptr>& selected_object_changed() noexcept
```

Returns the event published after selected-object projection commits.

### `property_value_changed` (public)

```cpp
[[nodiscard]] Event<const PropertyGridValueChange&>& property_value_changed() noexcept
```

Returns the typed post-transaction event for edits and resets.

### `edit_failed` (public)

```cpp
[[nodiscard]] Event<const PropertyGridEditError&>& edit_failed() noexcept
```

Returns the event published after parse rejection, setter failure, or completed rollback.

### `measure` (public)

```cpp
[[nodiscard]] Size measure(Size available) override
```

Delegates desired extent to the owned PropertyList.

### `arrange` (public)

```cpp
void arrange(Rect final_bounds) override
```

Commits own bounds and fills them with the PropertyList scroll surface.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects a named property grid with selection count and included descendant editors.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Executes PropertyGrid's on dispose operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
