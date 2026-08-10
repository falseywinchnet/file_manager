# ContainerControl

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `ScrollableControl → ContainerControl`  
Declaration: `include/gui_forms/container_controls.hpp:20`  
Definition: `src/controls/container_controls.cpp`

ContainerControl is a visual retained control declared in include/gui_forms/container_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ContainerControl`

```cpp
explicit ContainerControl(StableId stable_id)
```

Constructs or tears down the retained ContainerControl object according to its ownership contract.

### `contains_descendant`

```cpp
[[nodiscard]] bool contains_descendant(const Control::Ptr& control) const noexcept
```

Reports the current contains descendant value without mutation.

### `active_control`

```cpp
[[nodiscard]] Control::Ptr active_control() const noexcept
```

Reports the current active control value without mutation.

### `request_active_control`

```cpp
bool request_active_control(const Control::Ptr& control)
```

Public ContainerControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear_active_control`

```cpp
bool clear_active_control()
```

Removes the explicit active control value and restores fallback behavior.

### `auto_validate`

```cpp
[[nodiscard]] AutoValidate auto_validate() const noexcept
```

Reports the current auto validate value without mutation.

### `effective_auto_validate`

```cpp
[[nodiscard]] AutoValidate effective_auto_validate() const noexcept
```

Reports the current effective auto validate value without mutation.

### `set_auto_validate`

```cpp
void set_auto_validate(AutoValidate value)
```

Synchronously updates the retained auto validate property. Validation, typed invalidation, and notifications are defined by the implementation.

### `auto_validate_changed`

```cpp
[[nodiscard]] Event<AutoValidate>& auto_validate_changed() noexcept
```

Public ContainerControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `validate`

```cpp
bool validate(bool check_auto_validate = false)
```

Public ContainerControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `validate_children`

```cpp
bool validate_children( ValidationConstraints constraints = ValidationConstraints::selectable)
```

Public ContainerControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.
