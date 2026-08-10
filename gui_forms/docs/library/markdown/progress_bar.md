# ProgressBar

- Status: **OBSERVED: bundle 004 split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `RangeControl → ProgressBar`
- Declaration: `include/gui_forms/controls/range_control/progress_bar/progress_bar.hpp:39`
- Definition: `src/controls/range_control/progress_bar/progress_bar.cpp`

ProgressBar is a noninteractive RangeControl visualization with continuous, segmented, and marquee modes; optional overlays and stripes; centralized motion-policy resolution; retained frame scheduling; deterministic phase; reduced-motion behavior; and progress semantics.

## Visual evidence

![ProgressBar](../captures/range_controls.png)

## Declared methods

### `ProgressBar` (public)

```cpp
explicit ProgressBar(StableId stable_id)
```

Constructs a non-focusable, non-hit-testable range visualization with progress semantics.

### `visual_style` (public)

```cpp
[[nodiscard]] ProgressBarVisualStyle visual_style() const noexcept
```

Returns continuous, segmented, or marquee presentation.

### `set_visual_style` (public)

```cpp
void set_visual_style(ProgressBarVisualStyle style)
```

Validates style, updates animation registration, and invalidates size, paint, and semantics.

### `overlay_style` (public)

```cpp
[[nodiscard]] ProgressBarOverlayStyle overlay_style() const noexcept
```

Returns none, solid, or striped overlay policy.

### `set_overlay_style` (public)

```cpp
void set_overlay_style(ProgressBarOverlayStyle style)
```

Validates overlay vocabulary and invalidates painting.

### `stripe_width` (public)

```cpp
[[nodiscard]] double stripe_width() const noexcept
```

Returns the positive logical stripe width used by compatibility overlay painting.

### `set_stripe_width` (public)

```cpp
void set_stripe_width(double width)
```

Accepts a finite bounded positive width and synchronizes animation appearance.

### `animation_appearance` (public)

```cpp
[[nodiscard]] const ProgressBarAnimationAppearance& animation_appearance() const noexcept
```

Returns stripe width, gap, angle, and opacity as one value.

### `set_animation_appearance` (public)

```cpp
void set_animation_appearance(ProgressBarAnimationAppearance appearance)
```

Validates all animation geometry/opacity fields and commits them atomically.

### `animation_enabled` (public)

```cpp
[[nodiscard]] bool animation_enabled() const noexcept
```

Reports caller intent to animate marquee or striped presentation.

### `set_animation_enabled` (public)

```cpp
void set_animation_enabled(bool enabled)
```

Toggles animation intent and reconciles Window frame registration.

### `animation_paused` (public)

```cpp
[[nodiscard]] bool animation_paused() const noexcept
```

Reports whether application policy currently freezes phase.

### `set_animation_paused` (public)

```cpp
void set_animation_paused(bool paused)
```

Toggles pause state without discarding deterministic phase.

### `reduced_motion` (public)

```cpp
[[nodiscard]] bool reduced_motion() const noexcept
```

Reports the local compatibility override for reduced motion.

### `set_reduced_motion` (public)

```cpp
void set_reduced_motion(bool reduced)
```

Toggles the local override and reconciles effective animation registration.

### `set_motion_policy` (public)

```cpp
void set_motion_policy(bool paused, bool reduced_motion)
```

Installs or clears an explicit policy override instead of inheriting the Window policy.

### `set_motion_policy` (public)

```cpp
void set_motion_policy(MotionPolicy policy)
```

Installs or clears an explicit policy override instead of inheriting the Window policy.

### `motion_policy` (public)

```cpp
[[nodiscard]] MotionPolicy motion_policy() const noexcept
```

Returns the optional caller-authored motion-policy override.

### `effective_motion_policy` (public)

```cpp
[[nodiscard]] MotionPolicy effective_motion_policy() const noexcept
```

Resolves local compatibility state, explicit override, and attached Window policy into one behavior.

### `animation_period` (public)

```cpp
[[nodiscard]] FrameInterval animation_period() const noexcept
```

Returns the positive duration of one animation cycle.

### `set_animation_period` (public)

```cpp
void set_animation_period(FrameInterval period)
```

Accepts a positive duration and preserves phase continuity.

### `animation_phase` (public)

```cpp
[[nodiscard]] double animation_phase() const noexcept
```

Returns the normalized retained phase used for deterministic rendering and tests.

### `measure` (public)

```cpp
[[nodiscard]] Size measure(Size available) override
```

Provides stable progress-track desired thickness independent of phase.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records the selected track/fill/segment/marquee/overlay presentation from current range and phase.

### `on_frame` (public)

```cpp
void on_frame(FrameTime now) override
```

Advances phase from frame time under effective motion policy and invalidates only when pixels change.

### `hit_test_local` (public)

```cpp
[[nodiscard]] bool hit_test_local(Point local_point) const override
```

Always declines input because progress is informational.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects a progress role with numeric range/value and indeterminate state where applicable.

### `on_attached_to_window` (protected)

```cpp
void on_attached_to_window() override
```

Public ProgressBar operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_detached_from_window` (protected)

```cpp
void on_detached_from_window() noexcept override
```

Public ProgressBar operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `update_animation_registration` (private)

```cpp
void update_animation_registration()
```

Public ProgressBar operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `animated_style` (private)

```cpp
[[nodiscard]] bool animated_style() const noexcept
```

Reports the current animated style value without mutation.
