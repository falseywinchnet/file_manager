# GroupBox

- Status: **OBSERVED: bundle 002 split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → GroupBox`
- Declaration: `include/gui_forms/controls/panel/group_box/group_box.hpp:9`
- Definition: `src/controls/panel/group_box/group_box.cpp`

GroupBox is a titled Panel specialization with validated typography, mnemonic focus routing, caption-aware border painting, and group semantics.

## Visual evidence

![GroupBox](../captures/basic_controls_overview.png)

## Declared methods

### `GroupBox` (public)

```cpp
explicit GroupBox(StableId stable_id, std::string text =
```

Constructs a Panel with authored caption text and the control-caption font policy.

### `text` (public)

```cpp
[[nodiscard]] const std::string& text() const noexcept
```

Returns the authored caption including any mnemonic marker.

### `set_text` (public)

```cpp
void set_text(std::string text)
```

Commits caption text and invalidates measurement, painting, and semantics only for a real change.

### `font` (public)

```cpp
[[nodiscard]] FontSpec font() const noexcept
```

Returns the explicit caption FontSpec.

### `set_font` (public)

```cpp
void set_font(FontSpec font)
```

Validates and commits the caption font, then invalidates size, paint, and semantics.

### `use_mnemonic` (public)

```cpp
[[nodiscard]] bool use_mnemonic() const noexcept
```

Reports whether ampersand mnemonic parsing and focus routing are enabled.

### `set_use_mnemonic` (public)

```cpp
void set_use_mnemonic(bool value)
```

Toggles caption mnemonic interpretation and invalidates affected retained phases.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Paints the Panel material and a caption-aware border gap before recording the caption text.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the group role and uses the displayed caption as the accessible name when no explicit name exists.

### `mnemonic_matches` (private)

```cpp
[[nodiscard]] bool mnemonic_matches( char32_t character) const noexcept override
```

Reports the current mnemonic matches value without mutation.

### `process_mnemonic_self` (private)

```cpp
bool process_mnemonic_self(char32_t character) override
```

Public GroupBox operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
