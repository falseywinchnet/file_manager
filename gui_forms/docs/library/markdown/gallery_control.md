# GalleryControl

- Status: **OBSERVED: bundle 012 hierarchy-isolated demo declaration; retained in one source-private composition unit**
- Kind: **class / visual retained control**
- Hierarchy: `Control → GalleryControl`
- Declaration: `src/controls/gallery/control/gallery_control.hpp:12`
- Definition: `src/controls/gallery_controls.cpp`

GalleryControl is a source-private demoboard adapter that renders and exercises distinct control kinds without enlarging the public control surface.

## Visual evidence

![GalleryControl](../captures/basic_controls_overview.png)

## Declared methods

### `GalleryControl` (public)

```cpp
GalleryControl(StableId stable_id, const dml::NodeSpec& specification, std::shared_ptr<GalleryContext> context)
```

Constructs or tears down the retained GalleryControl object according to its ownership contract.

### `measure` (public)

```cpp
[[nodiscard]] Size measure(Size available) override
```

Computes desired size from the available constraint without arranging children.

### `arrange` (public)

```cpp
void arrange(Rect final_bounds) override
```

Commits final geometry and arranges retained child roles within it.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records renderer-neutral paint operations for the damaged local region.

### `hit_test_local` (public)

```cpp
[[nodiscard]] bool hit_test_local(Point local_point) const override
```

Reports the current hit test local value without mutation.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Consumes normalized routed pointer input and updates retained interaction state.

### `on_text_input` (public)

```cpp
void on_text_input(TextInputEvent& event) override
```

Executes GalleryControl's on text input operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `on_drag` (public)

```cpp
void on_drag(DragEvent& event) override
```

Executes GalleryControl's on drag operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `on_focus_changed` (public)

```cpp
void on_focus_changed(bool focused) override
```

Updates focus-dependent retained state and invalidates affected presentation/semantics.

### `on_activate` (public)

```cpp
void on_activate() override
```

Runs the control's single authoritative activation path.

### `kind` (public)

```cpp
[[nodiscard]] dml::NodeKind kind() const noexcept
```

Reports the current kind value without mutation.

### `selected` (public)

```cpp
[[nodiscard]] bool selected() const
```

Reports the current selected value without mutation.

### `value` (public)

```cpp
[[nodiscard]] double value() const
```

Reports the current value value without mutation.

### `display_text` (public)

```cpp
[[nodiscard]] std::string display_text() const
```

Reports the current display text value without mutation.

### `arrange_children` (private)

```cpp
void arrange_children(Size size)
```

Executes GalleryControl's arrange children operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
