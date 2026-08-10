# FlagsValueEditor

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → FlagsValueEditor`  
Declaration: `include/gui_forms/inspection_controls.hpp:105`  
Definition: `src/controls/inspection_controls.cpp`

FlagsValueEditor is a visual retained control declared in include/gui_forms/inspection_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `FlagsValueEditor`

```cpp
FlagsValueEditor(StableId stable_id, PropertyEnumDescriptor descriptor, PropertyEnumValue value)
```

Constructs or tears down the retained FlagsValueEditor object according to its ownership contract.

### `descriptor`

```cpp
[[nodiscard]] const PropertyEnumDescriptor& descriptor() const noexcept
```

Reports the current descriptor value without mutation.

### `set_descriptor`

```cpp
void set_descriptor(PropertyEnumDescriptor descriptor)
```

Synchronously updates the retained descriptor property. Validation, typed invalidation, and notifications are defined by the implementation.

### `value`

```cpp
[[nodiscard]] const PropertyEnumValue& value() const noexcept
```

Reports the current value value without mutation.

### `set_value`

```cpp
void set_value(PropertyEnumValue value)
```

Synchronously updates the retained value property. Validation, typed invalidation, and notifications are defined by the implementation.

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

### `value_changed`

```cpp
[[nodiscard]] Event<const PropertyEnumValue&>& value_changed() noexcept
```

Public FlagsValueEditor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `drop_down_changed`

```cpp
[[nodiscard]] Event<bool>& drop_down_changed() noexcept
```

Public FlagsValueEditor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

Public FlagsValueEditor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
