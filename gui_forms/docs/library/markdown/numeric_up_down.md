# NumericUpDown

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → NumericUpDown`  
Declaration: `include/gui_forms/input_controls.hpp:370`  
Definition: `src/controls/input_controls.cpp`

NumericUpDown is a visual retained control declared in include/gui_forms/input_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `NumericUpDown`

```cpp
explicit NumericUpDown(StableId stable_id)
```

Constructs or tears down the retained NumericUpDown object according to its ownership contract.

### `initialize_control_tree`

```cpp
void initialize_control_tree()
```

Idempotently attaches lazily constructed internal controls before layout or use.

### `minimum`

```cpp
[[nodiscard]] double minimum() const noexcept
```

Reports the current minimum value without mutation.

### `maximum`

```cpp
[[nodiscard]] double maximum() const noexcept
```

Reports the current maximum value without mutation.

### `set_range`

```cpp
void set_range(double minimum, double maximum)
```

Synchronously updates the retained range property. Validation, typed invalidation, and notifications are defined by the implementation.

### `value`

```cpp
[[nodiscard]] double value() const noexcept
```

Reports the current value value without mutation.

### `set_value`

```cpp
void set_value(double value)
```

Synchronously updates the retained value property. Validation, typed invalidation, and notifications are defined by the implementation.

### `increment`

```cpp
[[nodiscard]] double increment() const noexcept
```

Reports the current increment value without mutation.

### `set_increment`

```cpp
void set_increment(double increment)
```

Synchronously updates the retained increment property. Validation, typed invalidation, and notifications are defined by the implementation.

### `decimal_places`

```cpp
[[nodiscard]] std::uint8_t decimal_places() const noexcept
```

Reports the current decimal places value without mutation.

### `set_decimal_places`

```cpp
void set_decimal_places(std::uint8_t places)
```

Synchronously updates the retained decimal places property. Validation, typed invalidation, and notifications are defined by the implementation.

### `hexadecimal`

```cpp
[[nodiscard]] bool hexadecimal() const noexcept
```

Reports the current hexadecimal value without mutation.

### `set_hexadecimal`

```cpp
void set_hexadecimal(bool hexadecimal)
```

Synchronously updates the retained hexadecimal property. Validation, typed invalidation, and notifications are defined by the implementation.

### `editor`

```cpp
[[nodiscard]] std::shared_ptr<TextBox> editor() const noexcept
```

Reports the current editor value without mutation.

### `value_changed`

```cpp
[[nodiscard]] Event<double>& value_changed() noexcept
```

Public NumericUpDown operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `arrange`

```cpp
void arrange(Rect final_bounds) override
```

Commits final geometry and arranges retained child roles within it.

### `on_key_preview`

```cpp
void on_key_preview(KeyEvent& event) override
```

Public NumericUpDown operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_pointer_preview`

```cpp
void on_pointer_preview(PointerEvent& event) override
```

Public NumericUpDown operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.

### `on_semantic_action`

```cpp
bool on_semantic_action(SemanticAction action, std::string_view value) override
```

Public NumericUpDown operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
