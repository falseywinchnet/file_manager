# EasingPreview

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Control → EasingPreview`  
Declaration: `include/gui_forms/diagnostic_controls.hpp:77`  
Definition: `src/controls/diagnostic_controls.cpp`

EasingPreview is a visual retained control declared in include/gui_forms/diagnostic_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `EasingPreview`

```cpp
explicit EasingPreview(StableId stable_id)
```

Constructs or tears down the retained EasingPreview object according to its ownership contract.

### `title`

```cpp
[[nodiscard]] const std::string& title() const noexcept
```

Reports the current title value without mutation.

### `set_title`

```cpp
void set_title(std::string title)
```

Synchronously updates the retained title property. Validation, typed invalidation, and notifications are defined by the implementation.

### `style`

```cpp
[[nodiscard]] const BasicControlStyle& style() const noexcept
```

Reports the current style value without mutation.

### `set_style`

```cpp
void set_style(BasicControlStyle style)
```

Synchronously updates the retained style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `specification`

```cpp
[[nodiscard]] const AnimationSpec& specification() const noexcept
```

Reports the current specification value without mutation.

### `set_specification`

```cpp
void set_specification(AnimationSpec specification)
```

Synchronously updates the retained specification property. Validation, typed invalidation, and notifications are defined by the implementation.

### `tracks`

```cpp
[[nodiscard]] const std::vector<EasingPreviewTrack>& tracks() const noexcept
```

Reports the current tracks value without mutation.

### `set_tracks`

```cpp
void set_tracks(std::vector<EasingPreviewTrack> tracks)
```

Synchronously updates the retained tracks property. Validation, typed invalidation, and notifications are defined by the implementation.

### `motion_policy`

```cpp
[[nodiscard]] MotionPolicy motion_policy() const noexcept
```

Reports the current motion policy value without mutation.

### `effective_motion_policy`

```cpp
[[nodiscard]] MotionPolicy effective_motion_policy() const noexcept
```

Reports the current effective motion policy value without mutation.

### `set_motion_policy`

```cpp
void set_motion_policy(MotionPolicy policy)
```

Synchronously updates the retained motion policy property. Validation, typed invalidation, and notifications are defined by the implementation.

### `phase`

```cpp
[[nodiscard]] double phase() const noexcept
```

Reports the current phase value without mutation.

### `on_frame`

```cpp
void on_frame(FrameTime now) override
```

Public EasingPreview operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_paint`

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records renderer-neutral paint operations for the damaged local region.

### `hit_test_local`

```cpp
[[nodiscard]] bool hit_test_local(Point local_point) const override
```

Reports the current hit test local value without mutation.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.
