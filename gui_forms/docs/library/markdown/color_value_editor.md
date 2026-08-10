# ColorValueEditor

- Status: **OBSERVED: bundle 006 split and swatch-width enhancement; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → ColorValueEditor`
- Declaration: `include/gui_forms/controls/panel/color_value_editor/color_value_editor.hpp:12`
- Definition: `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp, src/controls/panel/color_value_editor/color_value_editor.cpp`

ColorValueEditor is a retained compound editor that delegates selection, keyboard, clipboard, commit, and cancellation to a real TextBox while adding a checker-backed color swatch, canonical #RRGGBBAA conversion, bounded swatch width, typed change events, and exact invalid-edit feedback.

## Visual evidence

![ColorValueEditor](../captures/property_grid.png)

## Declared methods

### `ColorValueEditor` (public)

```cpp
explicit ColorValueEditor(StableId stable_id, Color value =
```

Constructs the color model and defers child creation until shared ownership exists.

### `initialize_control_tree` (public)

```cpp
void initialize_control_tree()
```

Idempotently creates, owns, and subscribes the ordinary TextBox child.

### `value` (public)

```cpp
[[nodiscard]] Color value() const noexcept
```

Returns the authoritative retained Color.

### `set_value` (public)

```cpp
void set_value(Color value)
```

Commits a typed color, synchronizes canonical text without feedback, clears invalid state, and publishes a real transition.

### `editor` (public)

```cpp
[[nodiscard]] std::shared_ptr<TextBox> editor() const noexcept
```

Returns the real retained TextBox child.

### `swatch_width` (public)

```cpp
[[nodiscard]] double swatch_width() const noexcept
```

Returns the logical checker/color preview column width.

### `set_swatch_width` (public)

```cpp
void set_swatch_width(double width)
```

Validates [12, 96] and refreshes layout, paint, and hit testing.

### `value_changed` (public)

```cpp
[[nodiscard]] Event<Color>& value_changed() noexcept
```

Returns the event published after a valid typed color commit.

### `edit_failed` (public)

```cpp
[[nodiscard]] Event<const PropertyEditorInputError&>& edit_failed() noexcept
```

Returns the event carrying attempted text and parsing guidance after rejection.

### `format_value` (public)

```cpp
[[nodiscard]] static std::string format_value(Color value)
```

Formats every RGBA channel into canonical uppercase #RRGGBBAA text.

### `parse_value` (public)

```cpp
[[nodiscard]] static std::optional<Color> parse_value(std::string_view text)
```

Parses exact #RRGGBB or #RRGGBBAA text into a Color without locale dependence.

### `measure` (public)

```cpp
[[nodiscard]] Size measure(Size available) override
```

Requests available width and one scaled field row height.

### `arrange` (public)

```cpp
void arrange(Rect final_bounds) override
```

Commits bounds and fills the area after configurable swatch width with the TextBox.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records four-cell transparency checker, current alpha color, and bounded swatch border.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects group role, canonical value, description, descendant inclusion, and invalid state.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Public ColorValueEditor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `commit` (private)

```cpp
void commit(std::string_view text)
```

Public ColorValueEditor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `cancel` (private)

```cpp
void cancel()
```

Public ColorValueEditor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
