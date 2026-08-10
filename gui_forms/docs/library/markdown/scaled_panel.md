# ScaledPanel

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → ScaledPanel`  
Declaration: `include/gui_forms/container_controls.hpp:77`  
Definition: `src/controls/container_controls.cpp`

ScaledPanel is a visual retained control declared in include/gui_forms/container_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ScaledPanel`

```cpp
explicit ScaledPanel(StableId stable_id, Size design_size =
```

Constructs or tears down the retained ScaledPanel object according to its ownership contract.

### `design_size`

```cpp
[[nodiscard]] Size design_size() const noexcept
```

Reports the current design size value without mutation.

### `set_design_size`

```cpp
void set_design_size(Size size)
```

Synchronously updates the retained design size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `add_at`

```cpp
void add_at(Control::Ptr child, Rect design_bounds)
```

Public ScaledPanel operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_design_bounds`

```cpp
void set_design_bounds(const Control& child, Rect design_bounds)
```

Synchronously updates the retained design bounds property. Validation, typed invalidation, and notifications are defined by the implementation.

### `design_bounds`

```cpp
[[nodiscard]] std::optional<Rect> design_bounds(const Control& child) const
```

Reports the current design bounds value without mutation.

### `arrange`

```cpp
void arrange(Rect final_bounds) override
```

Commits final geometry and arranges retained child roles within it.
