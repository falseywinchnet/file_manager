# ErrorProvider

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Component → ErrorProvider`  
Declaration: `include/gui_forms/guidance.hpp:72`  
Definition: `src/controls/guidance.cpp`

ErrorProvider is a class declared in include/gui_forms/guidance.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ErrorProvider`

```cpp
explicit ErrorProvider(Window& window)
```

Constructs or tears down the retained ErrorProvider object according to its ownership contract.

### `~ErrorProvider`

```cpp
~ErrorProvider() override
```

Constructs or tears down the retained ErrorProvider object according to its ownership contract.

### `can_extend`

```cpp
[[nodiscard]] bool can_extend(const std::shared_ptr<Control>& target) const
```

Reports the current can extend value without mutation.

### `set_error`

```cpp
void set_error(const std::shared_ptr<Control>& target, std::string error)
```

Synchronously updates the retained error property. Validation, typed invalidation, and notifications are defined by the implementation.

### `error`

```cpp
[[nodiscard]] std::string error(const Control& target) const
```

Reports the current error value without mutation.

### `clear`

```cpp
void clear()
```

Public ErrorProvider operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `has_errors`

```cpp
[[nodiscard]] bool has_errors() const noexcept
```

Reports the current has errors value without mutation.

### `set_icon_alignment`

```cpp
void set_icon_alignment(const std::shared_ptr<Control>& target, ErrorIconAlignment alignment)
```

Synchronously updates the retained icon alignment property. Validation, typed invalidation, and notifications are defined by the implementation.

### `icon_alignment`

```cpp
[[nodiscard]] ErrorIconAlignment icon_alignment(const Control& target) const
```

Reports the current icon alignment value without mutation.

### `set_icon_padding`

```cpp
void set_icon_padding(const std::shared_ptr<Control>& target, double padding)
```

Synchronously updates the retained icon padding property. Validation, typed invalidation, and notifications are defined by the implementation.

### `icon_padding`

```cpp
[[nodiscard]] double icon_padding(const Control& target) const
```

Reports the current icon padding value without mutation.

### `blink_rate`

```cpp
[[nodiscard]] std::chrono::milliseconds blink_rate() const noexcept
```

Reports the current blink rate value without mutation.

### `set_blink_rate`

```cpp
void set_blink_rate(std::chrono::milliseconds rate)
```

Synchronously updates the retained blink rate property. Validation, typed invalidation, and notifications are defined by the implementation.

### `blink_style`

```cpp
[[nodiscard]] ErrorBlinkStyle blink_style() const noexcept
```

Reports the current blink style value without mutation.

### `set_blink_style`

```cpp
void set_blink_style(ErrorBlinkStyle style)
```

Synchronously updates the retained blink style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `right_to_left`

```cpp
[[nodiscard]] bool right_to_left() const noexcept
```

Reports the current right to left value without mutation.

### `set_right_to_left`

```cpp
void set_right_to_left(bool value)
```

Synchronously updates the retained right to left property. Validation, typed invalidation, and notifications are defined by the implementation.

### `right_to_left_changed`

```cpp
[[nodiscard]] Event<bool>& right_to_left_changed() noexcept
```

Public ErrorProvider operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `icon`

```cpp
[[nodiscard]] std::optional<ImageId> icon() const noexcept
```

Reports the current icon value without mutation.

### `set_icon`

```cpp
void set_icon(std::optional<ImageId> icon)
```

Synchronously updates the retained icon property. Validation, typed invalidation, and notifications are defined by the implementation.

### `container_control`

```cpp
[[nodiscard]] std::shared_ptr<Control> container_control() const noexcept
```

Reports the current container control value without mutation.

### `data_source`

```cpp
[[nodiscard]] std::shared_ptr<BindingSource> data_source() const noexcept
```

Reports the current data source value without mutation.

### `set_data_source`

```cpp
void set_data_source(std::shared_ptr<BindingSource> source)
```

Synchronously updates the retained data source property. Validation, typed invalidation, and notifications are defined by the implementation.

### `data_member`

```cpp
[[nodiscard]] const std::string& data_member() const noexcept
```

Reports the current data member value without mutation.

### `set_data_member`

```cpp
void set_data_member(std::string member)
```

Synchronously updates the retained data member property. Validation, typed invalidation, and notifications are defined by the implementation.

### `bind_to_data_and_errors`

```cpp
void bind_to_data_and_errors(std::shared_ptr<BindingSource> source, std::string member =
```

Public ErrorProvider operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `update_binding`

```cpp
void update_binding()
```

Public ErrorProvider operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `data_source_changed`

```cpp
[[nodiscard]] Event<>& data_source_changed() noexcept
```

Public ErrorProvider operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `data_member_changed`

```cpp
[[nodiscard]] Event<const std::string&>& data_member_changed() noexcept
```

Public ErrorProvider operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `tag`

```cpp
[[nodiscard]] const std::any& tag() const noexcept
```

Reports the current tag value without mutation.

### `set_tag`

```cpp
void set_tag(std::any tag)
```

Synchronously updates the retained tag property. Validation, typed invalidation, and notifications are defined by the implementation.

### `error_changed`

```cpp
[[nodiscard]] Event<const ErrorProviderChange&>& error_changed() noexcept
```

Public ErrorProvider operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `snapshot`

```cpp
[[nodiscard]] ErrorProviderSnapshot snapshot() const
```

Reports the current snapshot value without mutation.
