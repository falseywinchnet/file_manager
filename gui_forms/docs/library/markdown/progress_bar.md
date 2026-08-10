# ProgressBar

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `RangeControl → ProgressBar`  
Declaration: `include/gui_forms/range_controls.hpp:159`  
Definition: `src/controls/range_controls.cpp`

ProgressBar is a visual retained control declared in include/gui_forms/range_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `ProgressBar`

```cpp
explicit ProgressBar(StableId stable_id)
```

Constructs or tears down the retained ProgressBar object according to its ownership contract.

### `visual_style`

```cpp
[[nodiscard]] ProgressBarVisualStyle visual_style() const noexcept
```

Reports the current visual style value without mutation.

### `set_visual_style`

```cpp
void set_visual_style(ProgressBarVisualStyle style)
```

Synchronously updates the retained visual style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `overlay_style`

```cpp
[[nodiscard]] ProgressBarOverlayStyle overlay_style() const noexcept
```

Reports the current overlay style value without mutation.

### `set_overlay_style`

```cpp
void set_overlay_style(ProgressBarOverlayStyle style)
```

Synchronously updates the retained overlay style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `stripe_width`

```cpp
[[nodiscard]] double stripe_width() const noexcept
```

Reports the current stripe width value without mutation.

### `set_stripe_width`

```cpp
void set_stripe_width(double width)
```

Synchronously updates the retained stripe width property. Validation, typed invalidation, and notifications are defined by the implementation.

### `animation_appearance`

```cpp
[[nodiscard]] const ProgressBarAnimationAppearance& animation_appearance() const noexcept
```

Reports the current animation appearance value without mutation.

### `set_animation_appearance`

```cpp
void set_animation_appearance(ProgressBarAnimationAppearance appearance)
```

Synchronously updates the retained animation appearance property. Validation, typed invalidation, and notifications are defined by the implementation.

### `animation_enabled`

```cpp
[[nodiscard]] bool animation_enabled() const noexcept
```

Reports the current animation enabled value without mutation.

### `set_animation_enabled`

```cpp
void set_animation_enabled(bool enabled)
```

Synchronously updates the retained animation enabled property. Validation, typed invalidation, and notifications are defined by the implementation.

### `animation_paused`

```cpp
[[nodiscard]] bool animation_paused() const noexcept
```

Reports the current animation paused value without mutation.

### `set_animation_paused`

```cpp
void set_animation_paused(bool paused)
```

Synchronously updates the retained animation paused property. Validation, typed invalidation, and notifications are defined by the implementation.

### `reduced_motion`

```cpp
[[nodiscard]] bool reduced_motion() const noexcept
```

Reports the current reduced motion value without mutation.

### `set_reduced_motion`

```cpp
void set_reduced_motion(bool reduced)
```

Synchronously updates the retained reduced motion property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_motion_policy`

```cpp
void set_motion_policy(bool paused, bool reduced_motion)
```

Synchronously updates the retained motion policy property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_motion_policy`

```cpp
void set_motion_policy(MotionPolicy policy)
```

Synchronously updates the retained motion policy property. Validation, typed invalidation, and notifications are defined by the implementation.

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

### `animation_period`

```cpp
[[nodiscard]] FrameInterval animation_period() const noexcept
```

Reports the current animation period value without mutation.

### `set_animation_period`

```cpp
void set_animation_period(FrameInterval period)
```

Synchronously updates the retained animation period property. Validation, typed invalidation, and notifications are defined by the implementation.

### `animation_phase`

```cpp
[[nodiscard]] double animation_phase() const noexcept
```

Reports the current animation phase value without mutation.

### `measure`

```cpp
[[nodiscard]] Size measure(Size available) override
```

Computes desired size from the available constraint without arranging children.

### `on_paint`

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records renderer-neutral paint operations for the damaged local region.

### `on_frame`

```cpp
void on_frame(FrameTime now) override
```

Public ProgressBar operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

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
