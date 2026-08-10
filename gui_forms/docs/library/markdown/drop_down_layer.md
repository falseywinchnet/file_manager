# DropDownLayer

- Status: **OBSERVED: bundle 004 source-private state-machine split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → DropDownLayer`
- Declaration: `src/controls/panel/combo_box/drop_down_layer.hpp:7`
- Definition: `src/controls/panel/combo_box/drop_down_layer.cpp`

DropDownLayer is ComboBox's source-private transient modal boundary. It owns outside-click and Escape dismissal while leaving item selection and value commit with ComboBox.

## Visual evidence

![DropDownLayer](../captures/list_box_combo_box.png)

## Declared methods

### `DropDownLayer` (public)

```cpp
explicit DropDownLayer(StableId stable_id)
```

Constructs a full-window transient panel around the owned popup list.

### `dismissed` (public)

```cpp
[[nodiscard]] Event<>& dismissed() noexcept
```

Returns the event raised when outside pointer or Escape requests closure.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Dismisses a primary press outside the popup bounds and consumes the transient gesture.

### `on_key_preview` (public)

```cpp
void on_key_preview(KeyEvent& event) override
```

Dismisses Escape before descendant routing.
