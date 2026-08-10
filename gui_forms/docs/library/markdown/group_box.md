# GroupBox

Status: **OBSERVED: bundle 002 split; M4 build, focused tests, and Screen Sharing pass**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → GroupBox`  
Declaration: `include/gui_forms/controls/panel/group_box/group_box.hpp:9`  
Definition: `src/controls/panel/group_box/group_box.cpp`

GroupBox is a titled Panel specialization with validated typography, mnemonic focus routing, caption-aware border painting, and group semantics.

## Visual evidence

![GroupBox](../captures/basic_controls_overview.png)

## Public methods

### `GroupBox`

```cpp
explicit GroupBox(StableId stable_id, std::string text =
```

Constructs a Panel with authored caption text and the control-caption font policy.

### `text`

```cpp
[[nodiscard]] const std::string& text() const noexcept
```

Returns the authored caption including any mnemonic marker.

### `set_text`

```cpp
void set_text(std::string text)
```

Commits caption text and invalidates measurement, painting, and semantics only for a real change.

### `font`

```cpp
[[nodiscard]] FontSpec font() const noexcept
```

Returns the explicit caption FontSpec.

### `set_font`

```cpp
void set_font(FontSpec font)
```

Validates and commits the caption font, then invalidates size, paint, and semantics.

### `use_mnemonic`

```cpp
[[nodiscard]] bool use_mnemonic() const noexcept
```

Reports whether ampersand mnemonic parsing and focus routing are enabled.

### `set_use_mnemonic`

```cpp
void set_use_mnemonic(bool value)
```

Toggles caption mnemonic interpretation and invalidates affected retained phases.

### `on_paint`

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Paints the Panel material and a caption-aware border gap before recording the caption text.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the group role and uses the displayed caption as the accessible name when no explicit name exists.
