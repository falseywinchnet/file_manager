# TextBox

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → TextBox`  
Declaration: `include/gui_forms/input_controls.hpp:37`  
Definition: `src/controls/input_controls.cpp`

TextBox is a visual retained control declared in include/gui_forms/input_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `TextBox`

```cpp
explicit TextBox(StableId stable_id, std::string text =
```

Constructs or tears down the retained TextBox object according to its ownership contract.

### `text`

```cpp
[[nodiscard]] std::string_view text() const noexcept
```

Reports the current text value without mutation.

### `set_text`

```cpp
void set_text(std::string text)
```

Synchronously updates the retained text property. Validation, typed invalidation, and notifications are defined by the implementation.

### `placeholder_text`

```cpp
[[nodiscard]] std::string_view placeholder_text() const noexcept
```

Reports the current placeholder text value without mutation.

### `set_placeholder_text`

```cpp
void set_placeholder_text(std::string text)
```

Synchronously updates the retained placeholder text property. Validation, typed invalidation, and notifications are defined by the implementation.

### `read_only`

```cpp
[[nodiscard]] bool read_only() const noexcept
```

Reports the current read only value without mutation.

### `set_read_only`

```cpp
void set_read_only(bool read_only)
```

Synchronously updates the retained read only property. Validation, typed invalidation, and notifications are defined by the implementation.

### `password_character`

```cpp
[[nodiscard]] char32_t password_character() const noexcept
```

Reports the current password character value without mutation.

### `set_password_character`

```cpp
void set_password_character(char32_t character)
```

Synchronously updates the retained password character property. Validation, typed invalidation, and notifications are defined by the implementation.

### `use_system_password_character`

```cpp
[[nodiscard]] bool use_system_password_character() const noexcept
```

Reports the current use system password character value without mutation.

### `set_use_system_password_character`

```cpp
void set_use_system_password_character(bool enabled)
```

Synchronously updates the retained use system password character property. Validation, typed invalidation, and notifications are defined by the implementation.

### `password_protected`

```cpp
[[nodiscard]] bool password_protected() const noexcept
```

Reports the current password protected value without mutation.

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

### `selection`

```cpp
[[nodiscard]] TextSelection selection() const noexcept
```

Reports the current selection value without mutation.

### `select`

```cpp
void select(Utf8Offset anchor, Utf8Offset caret)
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `select_all`

```cpp
void select_all()
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `selected_text`

```cpp
[[nodiscard]] std::string selected_text() const
```

Reports the current selected text value without mutation.

### `can_undo`

```cpp
[[nodiscard]] bool can_undo() const noexcept
```

Reports the current can undo value without mutation.

### `can_redo`

```cpp
[[nodiscard]] bool can_redo() const noexcept
```

Reports the current can redo value without mutation.

### `undo`

```cpp
bool undo()
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `redo`

```cpp
bool redo()
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `replace_selection`

```cpp
bool replace_selection(std::string_view replacement)
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `delete_selection`

```cpp
bool delete_selection()
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `copy`

```cpp
bool copy()
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `cut`

```cpp
bool cut()
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `paste`

```cpp
bool paste()
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `text_changed`

```cpp
[[nodiscard]] Event<const std::string&>& text_changed() noexcept
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `selection_changed`

```cpp
[[nodiscard]] Event<const TextSelection&>& selection_changed() noexcept
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `committed`

```cpp
[[nodiscard]] Event<const std::string&>& committed() noexcept
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `cancelled`

```cpp
[[nodiscard]] Event<>& cancelled() noexcept
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

### `on_text_input`

```cpp
void on_text_input(TextInputEvent& event) override
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_focus_changed`

```cpp
void on_focus_changed(bool focused) override
```

Updates focus-dependent retained state and invalidates affected presentation/semantics.

### `on_frame`

```cpp
void on_frame(FrameTime now) override
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.

### `on_semantic_action`

```cpp
bool on_semantic_action(SemanticAction action, std::string_view value) override
```

Public TextBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
