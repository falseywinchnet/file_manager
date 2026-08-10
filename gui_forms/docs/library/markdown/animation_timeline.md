# AnimationTimeline

- Status: **OBSERVED: bundle 011 isolated animation state-machine review**
- Kind: **class**
- Hierarchy: `AnimationTimeline`
- Declaration: `include/gui_forms/animation/animation_timeline/animation_timeline.hpp:86`
- Definition: `src/core/animation/animation_timeline/animation_timeline.cpp`

AnimationTimeline validates one specification and deterministically transitions stopped, running, and paused time into iteration-aware eased samples.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `AnimationTimeline` (public)

```cpp
AnimationTimeline() = default
```

Constructs or tears down the retained AnimationTimeline object according to its ownership contract.

### `AnimationTimeline` (public)

```cpp
explicit AnimationTimeline(AnimationSpec specification)
```

Constructs or tears down the retained AnimationTimeline object according to its ownership contract.

### `set_specification` (public)

```cpp
void set_specification(AnimationSpec specification)
```

Synchronously updates the retained specification property. Validation, typed invalidation, and notifications are defined by the implementation.

### `specification` (public)

```cpp
[[nodiscard]] const AnimationSpec& specification() const noexcept
```

Reports the current specification value without mutation.

### `start` (public)

```cpp
void start(FrameTime start_time) noexcept
```

Transitions AnimationTimeline into its active state while preserving accumulated state.

### `pause` (public)

```cpp
void pause(FrameTime pause_time) noexcept
```

Suspends AnimationTimeline's active progression without discarding its current position.

### `resume` (public)

```cpp
void resume(FrameTime resume_time) noexcept
```

Transitions AnimationTimeline into its active state while preserving accumulated state.

### `stop` (public)

```cpp
void stop() noexcept
```

Returns AnimationTimeline to its stopped baseline and clears active progression.

### `running` (public)

```cpp
[[nodiscard]] bool running() const noexcept
```

Reports the current running value without mutation.

### `paused` (public)

```cpp
[[nodiscard]] bool paused() const noexcept
```

Reports the current paused value without mutation.

### `sample` (public)

```cpp
[[nodiscard]] AnimationSample sample(FrameTime now) const noexcept
```

Reports the current sample value without mutation.
