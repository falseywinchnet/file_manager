# ScaledPanel

- Status: **OBSERVED: bundle 003 split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → ScaledPanel`
- Declaration: `include/gui_forms/controls/panel/scaled_panel/scaled_panel.hpp:10`
- Definition: `src/controls/panel/scaled_panel/scaled_panel.cpp`

ScaledPanel maps caller-authored design-space child rectangles into current content bounds, retaining slot identity independently of child order and reconciling removed children safely.

## Visual evidence

![ScaledPanel](../captures/layout_panels.png)

## Declared methods

### `ScaledPanel` (public)

```cpp
explicit ScaledPanel(StableId stable_id, Size design_size =
```

Constructs a Panel with a validated positive design-space extent.

### `design_size` (public)

```cpp
[[nodiscard]] Size design_size() const noexcept
```

Returns the width and height that define authored design coordinates.

### `set_design_size` (public)

```cpp
void set_design_size(Size size)
```

Validates finite positive dimensions and invalidates arrangement when the design coordinate system changes.

### `add_at` (public)

```cpp
void add_at(Control::Ptr child, Rect design_bounds)
```

Validates an unparented child and finite nonnegative design rectangle, adds it, and records its stable-id slot atomically.

### `set_design_bounds` (public)

```cpp
void set_design_bounds(const Control& child, Rect design_bounds)
```

Requires an existing direct child, validates its design rectangle, and updates the stable slot.

### `design_bounds` (public)

```cpp
[[nodiscard]] std::optional<Rect> design_bounds(const Control& child) const
```

Returns the authored design-space rectangle for a current child, if assigned.

### `arrange` (public)

```cpp
void arrange(Rect final_bounds) override
```

Reconciles live slots and scales each authored rectangle independently into final content bounds.

### `reconcile_slots` (private)

```cpp
void reconcile_slots()
```

Public ScaledPanel operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
