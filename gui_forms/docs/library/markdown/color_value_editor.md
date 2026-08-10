# ColorValueEditor

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → ColorValueEditor`  
Declaration: `include/gui_forms/inspection_controls.hpp:166`  
Definition: `src/controls/inspection_controls.cpp`

ColorValueEditor is a visual retained control declared in include/gui_forms/inspection_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ColorValueEditor`

```cpp
explicit ColorValueEditor(StableId stable_id, Color value =
```

Constructs or tears down the retained ColorValueEditor object according to its ownership contract.

### `initialize_control_tree`

```cpp
void initialize_control_tree()
```

Idempotently attaches lazily constructed internal controls before layout or use.

### `value`

```cpp
[[nodiscard]] Color value() const noexcept
```

Reports the current value value without mutation.

### `set_value`

```cpp
void set_value(Color value)
```

Synchronously updates the retained value property. Validation, typed invalidation, and notifications are defined by the implementation.

### `editor`

```cpp
[[nodiscard]] std::shared_ptr<TextBox> editor() const noexcept
```

Reports the current editor value without mutation.

### `value_changed`

```cpp
[[nodiscard]] Event<Color>& value_changed() noexcept
```

Public ColorValueEditor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `edit_failed`

```cpp
[[nodiscard]] Event<const PropertyEditorInputError&>& edit_failed() noexcept
```

Public ColorValueEditor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `format_value`

```cpp
[[nodiscard]] static std::string format_value(Color value)
```

Public ColorValueEditor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `parse_value`

```cpp
[[nodiscard]] static std::optional<Color> parse_value( std::string_view text)
```

Public ColorValueEditor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.
