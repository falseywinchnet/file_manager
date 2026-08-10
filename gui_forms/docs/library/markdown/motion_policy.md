# MotionPolicy

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `MotionPolicy`  
Declaration: `include/gui_forms/animation.hpp:53`  
Definition: `inline/header-only`

MotionPolicy is a struct declared in include/gui_forms/animation.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `active`

```cpp
[[nodiscard]] constexpr bool active() const noexcept
```

Reports the current active value without mutation.

### `quiescent`

```cpp
[[nodiscard]] constexpr bool quiescent() const noexcept
```

Reports the current quiescent value without mutation.

### `frame_interval`

```cpp
[[nodiscard]] constexpr FrameInterval frame_interval( FrameInterval full_motion_interval) const noexcept
```

Reports the current frame interval value without mutation.

### `speed_scale`

```cpp
[[nodiscard]] constexpr double speed_scale() const noexcept
```

Reports the current speed scale value without mutation.

### `presentation_phase`

```cpp
[[nodiscard]] constexpr double presentation_phase(double phase) const noexcept
```

Reports the current presentation phase value without mutation.

### `operator==`

```cpp
friend constexpr bool operator==(const MotionPolicy&, const MotionPolicy&) noexcept = default
```

Public MotionPolicy operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
