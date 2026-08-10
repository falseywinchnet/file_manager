# FlowLayoutPanel

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `ContainerControl → FlowLayoutPanel`  
Declaration: `include/gui_forms/container_controls.hpp:122`  
Definition: `src/controls/container_controls.cpp`

FlowLayoutPanel is a visual retained control declared in include/gui_forms/container_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `FlowLayoutPanel`

```cpp
explicit FlowLayoutPanel(StableId stable_id)
```

Constructs or tears down the retained FlowLayoutPanel object according to its ownership contract.

### `flow_direction`

```cpp
[[nodiscard]] FlowDirection flow_direction() const noexcept
```

Reports the current flow direction value without mutation.

### `set_flow_direction`

```cpp
void set_flow_direction(FlowDirection direction)
```

Synchronously updates the retained flow direction property. Validation, typed invalidation, and notifications are defined by the implementation.

### `wrap_contents`

```cpp
[[nodiscard]] bool wrap_contents() const noexcept
```

Reports the current wrap contents value without mutation.

### `set_wrap_contents`

```cpp
void set_wrap_contents(bool wrap)
```

Synchronously updates the retained wrap contents property. Validation, typed invalidation, and notifications are defined by the implementation.

### `auto_size`

```cpp
[[nodiscard]] bool auto_size() const noexcept override
```

Reports the current auto size value without mutation.

### `set_auto_size`

```cpp
void set_auto_size(bool auto_size) override
```

Synchronously updates the retained auto size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_flow_break`

```cpp
void set_flow_break(const Control& child, bool flow_break)
```

Synchronously updates the retained flow break property. Validation, typed invalidation, and notifications are defined by the implementation.

### `flow_break`

```cpp
[[nodiscard]] bool flow_break(const Control& child) const
```

Reports the current flow break value without mutation.

### `measure`

```cpp
[[nodiscard]] Size measure(Size available) override
```

Computes desired size from the available constraint without arranging children.

### `arrange`

```cpp
void arrange(Rect final_bounds) override
```

Commits final geometry and arranges retained child roles within it.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.
