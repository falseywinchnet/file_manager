# AnimationTimeline

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `AnimationTimeline`  
Declaration: `include/gui_forms/animation.hpp:86`  
Definition: `src/core/animation.cpp`

AnimationTimeline is a class declared in include/gui_forms/animation.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `AnimationTimeline`

```cpp
AnimationTimeline() = default
```

Constructs or tears down the retained AnimationTimeline object according to its ownership contract.

### `AnimationTimeline`

```cpp
explicit AnimationTimeline(AnimationSpec specification)
```

Constructs or tears down the retained AnimationTimeline object according to its ownership contract.

### `set_specification`

```cpp
void set_specification(AnimationSpec specification)
```

Synchronously updates the retained specification property. Validation, typed invalidation, and notifications are defined by the implementation.

### `specification`

```cpp
[[nodiscard]] const AnimationSpec& specification() const noexcept
```

Reports the current specification value without mutation.

### `start`

```cpp
void start(FrameTime start_time) noexcept
```

Public AnimationTimeline operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `pause`

```cpp
void pause(FrameTime pause_time) noexcept
```

Public AnimationTimeline operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `resume`

```cpp
void resume(FrameTime resume_time) noexcept
```

Public AnimationTimeline operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `stop`

```cpp
void stop() noexcept
```

Public AnimationTimeline operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `running`

```cpp
[[nodiscard]] bool running() const noexcept
```

Reports the current running value without mutation.

### `paused`

```cpp
[[nodiscard]] bool paused() const noexcept
```

Reports the current paused value without mutation.

### `sample`

```cpp
[[nodiscard]] AnimationSample sample(FrameTime now) const noexcept
```

Reports the current sample value without mutation.
