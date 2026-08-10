# SplitterPanel

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `ContainerControl → SplitterPanel`  
Declaration: `include/gui_forms/container_controls.hpp:400`  
Definition: `src/controls/container_controls.cpp`

SplitterPanel is a visual retained control declared in include/gui_forms/container_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `SplitterPanel`

```cpp
explicit SplitterPanel(StableId stable_id)
```

Constructs or tears down the retained SplitterPanel object according to its ownership contract.

### `background`

```cpp
[[nodiscard]] Color background() const noexcept
```

Reports the current background value without mutation.

### `set_background`

```cpp
void set_background(Color color)
```

Synchronously updates the retained background property. Validation, typed invalidation, and notifications are defined by the implementation.

### `on_paint`

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records renderer-neutral paint operations for the damaged local region.
