# PropertyEditorDropDownLayer

- Status: **OBSERVED: bundle 006 source-private transient split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → PropertyEditorDropDownLayer`
- Declaration: `src/controls/panel/flags_value_editor/property_editor_drop_down_layer/property_editor_drop_down_layer.hpp:7`
- Definition: `src/controls/panel/flags_value_editor/property_editor_drop_down_layer/property_editor_drop_down_layer.cpp`

PropertyEditorDropDownLayer is FlagsValueEditor's source-private transient boundary. It owns no value state; it converts outside primary presses and Escape preview into a single dismissal event while the editor owns popup and focus leases.

## Visual evidence

![PropertyEditorDropDownLayer](../captures/property_grid.png)

## Declared methods

### `PropertyEditorDropDownLayer` (public)

```cpp
explicit PropertyEditorDropDownLayer(StableId stable_id)
```

Constructs a transparent overlay-plane panel for the complete client boundary.

### `dismissed` (public)

```cpp
[[nodiscard]] Event<>& dismissed() noexcept
```

Returns the unified outside-click/Escape dismissal event.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Consumes a primary press reaching the outside layer and requests dismissal.

### `on_key_preview` (public)

```cpp
void on_key_preview(KeyEvent& event) override
```

Intercepts Escape before descendants and requests dismissal.
