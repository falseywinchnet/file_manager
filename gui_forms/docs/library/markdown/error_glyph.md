# ErrorGlyph

- Status: **OBSERVED: bundle 005 source-private animation split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Control → ErrorGlyph`
- Declaration: `src/controls/guidance/error_glyph/error_glyph.hpp:8`
- Definition: `src/controls/guidance/error_glyph/error_glyph.cpp`

ErrorGlyph is ErrorProvider's source-private retained visual and blink state machine. It renders caller imagery or the default error mark, exposes input-transparent alert semantics, gates animation on provider policy, presentation, occlusion, and reduced motion, and schedules only through the owning Window frame clock.

## Visual evidence

![ErrorGlyph](../captures/tool_tip_error_provider.png)

## Declared methods

### `ErrorGlyph` (public)

```cpp
explicit ErrorGlyph(StableId stable_id)
```

Constructs an input-transparent alert visual for one target and retains its frame-scheduler callback.

### `set_error` (public)

```cpp
void set_error(std::string error)
```

Commits displayed/semantic error text and refreshes the glyph.

### `set_icon` (public)

```cpp
void set_icon(std::optional<ImageId> icon)
```

Rebinds optional caller imagery and refreshes paint.

### `configure_blink` (public)

```cpp
void configure_blink(ErrorBlinkStyle style, std::chrono::milliseconds rate, bool restart)
```

Commits style, cadence, and restart policy, then reconciles scheduler registration.

### `refresh_motion_policy` (public)

```cpp
void refresh_motion_policy()
```

Re-evaluates animation against current Window reduced-motion, occlusion, and presentation state.

### `blink_active` (public)

```cpp
[[nodiscard]] bool blink_active() const noexcept
```

Reports whether the glyph currently owns an active blink schedule.

### `phase_visible` (public)

```cpp
[[nodiscard]] bool phase_visible() const noexcept
```

Reports whether the current blink phase paints the glyph.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records caller imagery or the default error mark only when the current phase is visible.

### `hit_test_local` (public)

```cpp
[[nodiscard]] bool hit_test_local(Point point) const override
```

Rejects input so the glyph remains a pure adornment.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects an alert with the target error while remaining nonfocusable.

### `on_frame` (public)

```cpp
void on_frame(FrameTime now) override
```

Advances blink phase at cadence and requests only the next necessary frame.

### `on_attachment_committed` (protected)

```cpp
void on_attachment_committed() noexcept override
```

Public ErrorGlyph operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_detached_from_window` (protected)

```cpp
void on_detached_from_window() noexcept override
```

Public ErrorGlyph operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `refresh_schedule` (private)

```cpp
void refresh_schedule()
```

Public ErrorGlyph operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
