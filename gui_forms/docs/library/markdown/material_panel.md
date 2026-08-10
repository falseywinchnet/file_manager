# MaterialPanel

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → MaterialPanel`  
Declaration: `include/gui_forms/material.hpp:11`  
Definition: `src/controls/material.cpp`

MaterialPanel is a visual retained control declared in include/gui_forms/material.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `MaterialPanel`

```cpp
explicit MaterialPanel(StableId stable_id)
```

Constructs or tears down the retained MaterialPanel object according to its ownership contract.

### `material`

```cpp
[[nodiscard]] const SurfaceMaterial& material() const noexcept
```

Reports the current material value without mutation.

### `set_material`

```cpp
void set_material(SurfaceMaterial material)
```

Synchronously updates the retained material property. Validation, typed invalidation, and notifications are defined by the implementation.

### `material_changed`

```cpp
[[nodiscard]] Event<const SurfaceMaterial&>& material_changed() noexcept
```

Public MaterialPanel operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_paint`

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records renderer-neutral paint operations for the damaged local region.

### `visual_outsets`

```cpp
[[nodiscard]] Insets visual_outsets() const noexcept override
```

Reports the current visual outsets value without mutation.
