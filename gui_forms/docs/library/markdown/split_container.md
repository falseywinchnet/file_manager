# SplitContainer

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `ContainerControl → SplitContainer`  
Declaration: `include/gui_forms/container_controls.hpp:418`  
Definition: `src/controls/container_controls.cpp`

SplitContainer is a visual retained control declared in include/gui_forms/container_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `SplitContainer`

```cpp
explicit SplitContainer(StableId stable_id)
```

Constructs or tears down the retained SplitContainer object according to its ownership contract.

### `initialize_control_tree`

```cpp
void initialize_control_tree()
```

Idempotently attaches lazily constructed internal controls before layout or use.

### `first_panel`

```cpp
[[nodiscard]] std::shared_ptr<SplitterPanel> first_panel() const noexcept
```

Reports the current first panel value without mutation.

### `second_panel`

```cpp
[[nodiscard]] std::shared_ptr<SplitterPanel> second_panel() const noexcept
```

Reports the current second panel value without mutation.

### `splitter_control`

```cpp
[[nodiscard]] Control::Ptr splitter_control() const noexcept
```

Reports the current splitter control value without mutation.

### `orientation`

```cpp
[[nodiscard]] Orientation orientation() const noexcept
```

Reports the current orientation value without mutation.

### `set_orientation`

```cpp
void set_orientation(Orientation orientation)
```

Synchronously updates the retained orientation property. Validation, typed invalidation, and notifications are defined by the implementation.

### `splitter_distance`

```cpp
[[nodiscard]] double splitter_distance() const noexcept
```

Reports the current splitter distance value without mutation.

### `set_splitter_distance`

```cpp
void set_splitter_distance(double distance)
```

Synchronously updates the retained splitter distance property. Validation, typed invalidation, and notifications are defined by the implementation.

### `splitter_width`

```cpp
[[nodiscard]] double splitter_width() const noexcept
```

Reports the current splitter width value without mutation.

### `set_splitter_width`

```cpp
void set_splitter_width(double width)
```

Synchronously updates the retained splitter width property. Validation, typed invalidation, and notifications are defined by the implementation.

### `splitter_hit_width`

```cpp
[[nodiscard]] double splitter_hit_width() const noexcept
```

Reports the current splitter hit width value without mutation.

### `set_splitter_hit_width`

```cpp
void set_splitter_hit_width(double width)
```

Synchronously updates the retained splitter hit width property. Validation, typed invalidation, and notifications are defined by the implementation.

### `first_minimum`

```cpp
[[nodiscard]] double first_minimum() const noexcept
```

Reports the current first minimum value without mutation.

### `set_first_minimum`

```cpp
void set_first_minimum(double extent)
```

Synchronously updates the retained first minimum property. Validation, typed invalidation, and notifications are defined by the implementation.

### `second_minimum`

```cpp
[[nodiscard]] double second_minimum() const noexcept
```

Reports the current second minimum value without mutation.

### `set_second_minimum`

```cpp
void set_second_minimum(double extent)
```

Synchronously updates the retained second minimum property. Validation, typed invalidation, and notifications are defined by the implementation.

### `first_maximum`

```cpp
[[nodiscard]] std::optional<double> first_maximum() const noexcept
```

Reports the current first maximum value without mutation.

### `set_first_maximum`

```cpp
void set_first_maximum(std::optional<double> extent)
```

Synchronously updates the retained first maximum property. Validation, typed invalidation, and notifications are defined by the implementation.

### `second_maximum`

```cpp
[[nodiscard]] std::optional<double> second_maximum() const noexcept
```

Reports the current second maximum value without mutation.

### `set_second_maximum`

```cpp
void set_second_maximum(std::optional<double> extent)
```

Synchronously updates the retained second maximum property. Validation, typed invalidation, and notifications are defined by the implementation.

### `first_collapsed`

```cpp
[[nodiscard]] bool first_collapsed() const noexcept
```

Reports the current first collapsed value without mutation.

### `first_collapse_origin`

```cpp
[[nodiscard]] SplitCollapseOrigin first_collapse_origin() const noexcept
```

Reports the current first collapse origin value without mutation.

### `set_first_collapsed`

```cpp
void set_first_collapsed( bool collapsed, SplitCollapseOrigin origin = SplitCollapseOrigin::programmatic)
```

Synchronously updates the retained first collapsed property. Validation, typed invalidation, and notifications are defined by the implementation.

### `second_collapsed`

```cpp
[[nodiscard]] bool second_collapsed() const noexcept
```

Reports the current second collapsed value without mutation.

### `second_collapse_origin`

```cpp
[[nodiscard]] SplitCollapseOrigin second_collapse_origin() const noexcept
```

Reports the current second collapse origin value without mutation.

### `set_second_collapsed`

```cpp
void set_second_collapsed( bool collapsed, SplitCollapseOrigin origin = SplitCollapseOrigin::programmatic)
```

Synchronously updates the retained second collapsed property. Validation, typed invalidation, and notifications are defined by the implementation.

### `splitter_fixed`

```cpp
[[nodiscard]] bool splitter_fixed() const noexcept
```

Reports the current splitter fixed value without mutation.

### `set_splitter_fixed`

```cpp
void set_splitter_fixed(bool fixed)
```

Synchronously updates the retained splitter fixed property. Validation, typed invalidation, and notifications are defined by the implementation.

### `fixed_panel`

```cpp
[[nodiscard]] SplitFixedPanel fixed_panel() const noexcept
```

Reports the current fixed panel value without mutation.

### `set_fixed_panel`

```cpp
void set_fixed_panel(SplitFixedPanel panel)
```

Synchronously updates the retained fixed panel property. Validation, typed invalidation, and notifications are defined by the implementation.

### `collapse_panel`

```cpp
[[nodiscard]] SplitFixedPanel collapse_panel() const noexcept
```

Reports the current collapse panel value without mutation.

### `set_collapse_panel`

```cpp
void set_collapse_panel(SplitFixedPanel panel)
```

Synchronously updates the retained collapse panel property. Validation, typed invalidation, and notifications are defined by the implementation.

### `automatic_collapse_threshold`

```cpp
[[nodiscard]] double automatic_collapse_threshold() const noexcept
```

Reports the current automatic collapse threshold value without mutation.

### `set_automatic_collapse_threshold`

```cpp
void set_automatic_collapse_threshold(double extent)
```

Synchronously updates the retained automatic collapse threshold property. Validation, typed invalidation, and notifications are defined by the implementation.

### `keyboard_increment`

```cpp
[[nodiscard]] double keyboard_increment() const noexcept
```

Reports the current keyboard increment value without mutation.

### `set_keyboard_increment`

```cpp
void set_keyboard_increment(double increment)
```

Synchronously updates the retained keyboard increment property. Validation, typed invalidation, and notifications are defined by the implementation.

### `splitter_changed`

```cpp
[[nodiscard]] Event<const SplitChangeEvent&>& splitter_changed() noexcept
```

Public SplitContainer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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

### `on_pointer_preview`

```cpp
void on_pointer_preview(PointerEvent& event) override
```

Public SplitContainer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_key_preview`

```cpp
void on_key_preview(KeyEvent& event) override
```

Public SplitContainer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.

### `on_semantic_action`

```cpp
bool on_semantic_action(SemanticAction action, std::string_view value) override
```

Public SplitContainer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
