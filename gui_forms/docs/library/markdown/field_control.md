# FieldControl

- Status: **generated inventory; detailed review pending**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → FieldControl`
- Declaration: `src/abi/control_adapters/abi_control_adapters.hpp:129`
- Definition: `inline/header-only`

FieldControl is a visual retained control declared in src/abi/control_adapters/abi_control_adapters.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `FieldControl` (public)

```cpp
FieldControl(StableId stable_id, FieldControlKind kind) : Panel(std::move(stable_id)), kind_(kind)
```

Constructs or tears down the retained FieldControl object according to its ownership contract.

### `set_colors` (public)

```cpp
void set_colors(gui_forms::Color foreground, gui_forms::Color background)
```

Synchronously updates the retained colors property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_text` (public)

```cpp
void set_text(std::string text)
```

Synchronously updates the retained text property. Validation, typed invalidation, and notifications are defined by the implementation.

### `replace` (public)

```cpp
[[nodiscard]] bool replace(std::uint64_t start, std::uint64_t length, std::string_view replacement, gf_field_edit_result& result)
```

Public FieldControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `history` (public)

```cpp
[[nodiscard]] bool history(std::int32_t direction, gf_field_edit_result& result)
```

Public FieldControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear_history` (public)

```cpp
void clear_history() noexcept
```

Removes the explicit history value and restores fallback behavior.

### `text` (public)

```cpp
[[nodiscard]] std::string_view text() const noexcept
```

Reports the current text value without mutation.

### `set_selection` (public)

```cpp
bool set_selection(std::uint64_t start, std::uint64_t length, bool caret_visible)
```

Synchronously updates the retained selection property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_edit_state` (public)

```cpp
bool set_edit_state(std::uint64_t anchor, std::uint64_t caret, bool caret_visible)
```

Synchronously updates the retained edit state property. Validation, typed invalidation, and notifications are defined by the implementation.

### `position_at` (public)

```cpp
[[nodiscard]] std::uint64_t position_at(double local_x) const noexcept
```

Reports the current position at value without mutation.

### `navigate` (public)

```cpp
[[nodiscard]] bool navigate(std::uint64_t position, std::int32_t direction, std::uint64_t& result) const noexcept
```

Reports the current navigate value without mutation.

### `on_paint` (public)

```cpp
void on_paint(gui_forms::Painter& painter, Rect damage) override
```

Records renderer-neutral paint operations for the damaged local region.

### `on_focus_changed` (public)

```cpp
void on_focus_changed(bool focused) override
```

Updates focus-dependent retained state and invalidates affected presentation/semantics.

### `pointer_input` (public)

```cpp
[[nodiscard]] gui_forms::Event<const RasterPointerSample&>& pointer_input() noexcept
```

Public FieldControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `key_input` (public)

```cpp
[[nodiscard]] gui_forms::Event<const RasterKeySample&>& key_input() noexcept
```

Public FieldControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `text_input` (public)

```cpp
[[nodiscard]] gui_forms::Event<const RasterTextSample&>& text_input() noexcept
```

Public FieldControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_pointer` (public)

```cpp
void on_pointer(gui_forms::PointerEvent& event) override
```

Consumes normalized routed pointer input and updates retained interaction state.

### `on_key` (public)

```cpp
void on_key(gui_forms::KeyEvent& event) override
```

Consumes normalized keyboard input for this control's interaction contract.

### `on_text_input` (public)

```cpp
void on_text_input(gui_forms::TextInputEvent& event) override
```

Public FieldControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `snapshot` (private)

```cpp
[[nodiscard]] FieldSnapshot snapshot() const
```

Reports the current snapshot value without mutation.

### `apply_snapshot` (private)

```cpp
void apply_snapshot(FieldSnapshot snapshot)
```

Public FieldControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `push_history` (private)

```cpp
void push_history(std::deque<FieldSnapshot>& history, FieldSnapshot snapshot)
```

Public FieldControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `push_undo` (private)

```cpp
void push_undo(FieldSnapshot snapshot)
```

Public FieldControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear_redo` (private)

```cpp
void clear_redo() noexcept
```

Removes the explicit redo value and restores fallback behavior.

### `edit_result` (private)

```cpp
[[nodiscard]] gf_field_edit_result edit_result(bool changed) const noexcept
```

Reports the current edit result value without mutation.
