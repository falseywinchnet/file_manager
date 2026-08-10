# MotionPolicy

- Status: **OBSERVED: bundle 011 animation-policy review**
- Kind: **struct**
- Hierarchy: `MotionPolicy`
- Declaration: `include/gui_forms/animation/animation_timeline/animation_timeline.hpp:53`
- Definition: `inline/header-only`

MotionPolicy is the resolved environment policy for active/quiescent cadence, speed scaling, and presentation phase.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `active` (public)

```cpp
[[nodiscard]] constexpr bool active() const noexcept
```

Reports the current active value without mutation.

### `quiescent` (public)

```cpp
[[nodiscard]] constexpr bool quiescent() const noexcept
```

Reports the current quiescent value without mutation.

### `frame_interval` (public)

```cpp
[[nodiscard]] constexpr FrameInterval frame_interval( FrameInterval full_motion_interval) const noexcept
```

Reports the current frame interval value without mutation.

### `speed_scale` (public)

```cpp
[[nodiscard]] constexpr double speed_scale() const noexcept
```

Reports the current speed scale value without mutation.

### `presentation_phase` (public)

```cpp
[[nodiscard]] constexpr double presentation_phase(double phase) const noexcept
```

Reports the current presentation phase value without mutation.

### `operator==` (public)

```cpp
friend constexpr bool operator==(const MotionPolicy&, const MotionPolicy&) noexcept = default
```

Compares the complete value identity used by deterministic retained-state decisions.
