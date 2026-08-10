# MaterialPanel

- Status: **OBSERVED: bundle 006 hierarchical move and material contract review; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → MaterialPanel`
- Declaration: `include/gui_forms/controls/panel/material_panel/material_panel.hpp:9`
- Definition: `src/controls/panel/material_panel/material_panel.cpp`

MaterialPanel is a content-agnostic Panel specialization that paints a validated SurfaceMaterial behind ordinary retained children. It verifies attached Window image identities/pixel sizes, publishes atomic material changes, and reports exact shadow/border outsets while material recipes retain all fill, pattern, border, radius, and shadow customization.

## Visual evidence

![MaterialPanel](../captures/drawing_raster_material.png)

## Declared methods

### `MaterialPanel` (public)

```cpp
explicit MaterialPanel(StableId stable_id)
```

Constructs a transparent Panel whose visual authority is its SurfaceMaterial recipe.

### `material` (public)

```cpp
[[nodiscard]] const SurfaceMaterial& material() const noexcept
```

Returns the retained validated material recipe.

### `set_material` (public)

```cpp
void set_material(SurfaceMaterial material)
```

Validates bounded material structure and attached image resources before atomic commit and change publication.

### `material_changed` (public)

```cpp
[[nodiscard]] Event<const SurfaceMaterial&>& material_changed() noexcept
```

Returns the event published after a distinct valid recipe commits.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Resolves and records all material fills, clips, borders, images, and shadows against local bounds.

### `visual_outsets` (public)

```cpp
[[nodiscard]] Insets visual_outsets() const noexcept override
```

Returns exact material shadow/border overflow for damage and composition.

### `on_attached_to_window` (protected)

```cpp
void on_attached_to_window() override
```

Executes MaterialPanel's on attached to window operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `validate_window_images` (private)

```cpp
void validate_window_images(const SurfaceMaterial& material) const
```

Reports the current validate window images value without mutation.
