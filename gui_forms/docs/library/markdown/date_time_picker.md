# DateTimePicker

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → DateTimePicker`  
Declaration: `include/gui_forms/date_time_picker.hpp:57`  
Definition: `src/controls/date_time_picker.cpp`

DateTimePicker is a visual retained control declared in include/gui_forms/date_time_picker.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `DateTimePicker`

```cpp
explicit DateTimePicker(StableId stable_id)
```

Constructs or tears down the retained DateTimePicker object according to its ownership contract.

### `value`

```cpp
[[nodiscard]] DateTimeValue value() const noexcept
```

Reports the current value value without mutation.

### `set_value`

```cpp
void set_value(DateTimeValue value)
```

Synchronously updates the retained value property. Validation, typed invalidation, and notifications are defined by the implementation.

### `minimum`

```cpp
[[nodiscard]] DateTimeValue minimum() const noexcept
```

Reports the current minimum value without mutation.

### `maximum`

```cpp
[[nodiscard]] DateTimeValue maximum() const noexcept
```

Reports the current maximum value without mutation.

### `set_range`

```cpp
void set_range(DateTimeValue minimum, DateTimeValue maximum)
```

Synchronously updates the retained range property. Validation, typed invalidation, and notifications are defined by the implementation.

### `format`

```cpp
[[nodiscard]] DateTimePickerFormat format() const noexcept
```

Reports the current format value without mutation.

### `set_format`

```cpp
void set_format(DateTimePickerFormat format)
```

Synchronously updates the retained format property. Validation, typed invalidation, and notifications are defined by the implementation.

### `custom_format`

```cpp
[[nodiscard]] const std::string& custom_format() const noexcept
```

Reports the current custom format value without mutation.

### `set_custom_format`

```cpp
void set_custom_format(std::string format)
```

Synchronously updates the retained custom format property. Validation, typed invalidation, and notifications are defined by the implementation.

### `format_provider`

```cpp
[[nodiscard]] const DateTimeFormatProvider& format_provider() const noexcept
```

Reports the current format provider value without mutation.

### `set_format_provider`

```cpp
void set_format_provider(DateTimeFormatProvider provider)
```

Synchronously updates the retained format provider property. Validation, typed invalidation, and notifications are defined by the implementation.

### `formatted_value`

```cpp
[[nodiscard]] std::string formatted_value() const
```

Reports the current formatted value value without mutation.

### `show_check_box`

```cpp
[[nodiscard]] bool show_check_box() const noexcept
```

Reports the current show check box value without mutation.

### `set_show_check_box`

```cpp
void set_show_check_box(bool show)
```

Synchronously updates the retained show check box property. Validation, typed invalidation, and notifications are defined by the implementation.

### `checked`

```cpp
[[nodiscard]] bool checked() const noexcept
```

Reports the current checked value without mutation.

### `set_checked`

```cpp
void set_checked(bool checked)
```

Synchronously updates the retained checked property. Validation, typed invalidation, and notifications are defined by the implementation.

### `show_up_down`

```cpp
[[nodiscard]] bool show_up_down() const noexcept
```

Reports the current show up down value without mutation.

### `set_show_up_down`

```cpp
void set_show_up_down(bool show)
```

Synchronously updates the retained show up down property. Validation, typed invalidation, and notifications are defined by the implementation.

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

### `value_changed`

```cpp
[[nodiscard]] Event<DateTimeValue>& value_changed() noexcept
```

Public DateTimePicker operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `checked_changed`

```cpp
[[nodiscard]] Event<bool>& checked_changed() noexcept
```

Public DateTimePicker operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `drop_down_changed`

```cpp
[[nodiscard]] Event<bool>& drop_down_changed() noexcept
```

Public DateTimePicker operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

### `on_activate`

```cpp
void on_activate() override
```

Runs the control's single authoritative activation path.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.

### `on_semantic_action`

```cpp
bool on_semantic_action(SemanticAction action, std::string_view value) override
```

Public DateTimePicker operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
