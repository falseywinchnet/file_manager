# SpinButtons

- Status: **OBSERVED: bundle 004 source-private state-machine split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Control → SpinButtons`
- Declaration: `src/controls/panel/numeric_up_down/spin_buttons.hpp:7`
- Definition: `src/controls/panel/numeric_up_down/spin_buttons.cpp`

SpinButtons is NumericUpDown's source-private two-part command surface. It qualifies pointer capture independently for increment and decrement and publishes only directional intent.

## Visual evidence

![SpinButtons](../captures/numeric_up_down.png)

## Declared methods

### `SpinButtons` (public)

```cpp
explicit SpinButtons(StableId stable_id)
```

Constructs a non-tab-stop vertical two-command surface.

### `stepped` (public)

```cpp
[[nodiscard]] Event<int>& stepped() noexcept
```

Returns the event carrying +1 or -1 after a qualified release.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records framed upper/lower button states and directional chevrons.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Tracks hover, press, capture, cancellation, and same-part release before publishing a step.
