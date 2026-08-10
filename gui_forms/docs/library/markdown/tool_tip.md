# ToolTip

- Status: **OBSERVED: bundle 005 split and maximum-width enhancement; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class**
- Hierarchy: `Component → ToolTip`
- Declaration: `include/gui_forms/components/tool_tip/tool_tip.hpp:30`
- Definition: `src/controls/tool_tip/tool_tip.cpp`

ToolTip is a nonvisual window-bound provider that tokenizes help text to live controls and coordinates hover/focus/manual presentation through one scheduler-backed state machine. It owns delayed show, reshow, auto-pop, movement tracking, popup revocation, input-transparent overlay composition, configurable maximum width, and exact visibility events.

## Visual evidence

![ToolTip](../captures/tool_tip_error_provider.png)

## Declared methods

### `ToolTip` (public)

```cpp
explicit ToolTip(Window& window)
```

Binds a nonvisual tooltip provider to one live Window lifetime and creates its UI-thread scheduler.

### `~ToolTip` (public)

```cpp
~ToolTip() override
```

Closes overlays and releases target, timer, popup, and lifetime subscriptions before destruction.

### `set_tool_tip` (public)

```cpp
void set_tool_tip(const std::shared_ptr<Control>& target, std::string text)
```

Validates a live target, tokenizes or replaces its help text, installs movement/input subscriptions, and removes empty mappings.

### `tool_tip` (public)

```cpp
[[nodiscard]] std::string tool_tip(const Control& target) const
```

Returns authored help text for a currently mapped target.

### `remove_tool_tip` (public)

```cpp
bool remove_tool_tip(const Control& target)
```

Disconnects one target mapping and hides it first when it owns the visible or pending tooltip.

### `clear` (public)

```cpp
void clear()
```

Hides any overlay and disconnects every mapped target.

### `initial_delay` (public)

```cpp
[[nodiscard]] std::chrono::milliseconds initial_delay() const noexcept
```

Returns the hover/focus dwell required for a fresh tooltip.

### `set_initial_delay` (public)

```cpp
void set_initial_delay(std::chrono::milliseconds delay)
```

Validates nonnegative timing and updates future fresh-show scheduling.

### `reshow_delay` (public)

```cpp
[[nodiscard]] std::chrono::milliseconds reshow_delay() const noexcept
```

Returns the shorter delay admitted after a tooltip was recently hidden.

### `set_reshow_delay` (public)

```cpp
void set_reshow_delay(std::chrono::milliseconds delay)
```

Validates nonnegative timing and updates future reshow scheduling.

### `auto_pop_delay` (public)

```cpp
[[nodiscard]] std::chrono::milliseconds auto_pop_delay() const noexcept
```

Returns the default visible duration before automatic hiding.

### `set_auto_pop_delay` (public)

```cpp
void set_auto_pop_delay(std::chrono::milliseconds delay)
```

Validates positive timing and re-arms an active automatic hide.

### `show_on_hover` (public)

```cpp
[[nodiscard]] bool show_on_hover() const noexcept
```

Reports whether mapped pointer entry may schedule presentation.

### `set_show_on_hover` (public)

```cpp
void set_show_on_hover(bool value)
```

Toggles hover policy and cancels incompatible pending hover work.

### `show_on_focus` (public)

```cpp
[[nodiscard]] bool show_on_focus() const noexcept
```

Reports whether mapped keyboard focus may schedule presentation.

### `set_show_on_focus` (public)

```cpp
void set_show_on_focus(bool value)
```

Toggles focus policy and cancels incompatible pending focus work.

### `show_always` (public)

```cpp
[[nodiscard]] bool show_always() const noexcept
```

Reports whether inactive-window targets are still eligible for presentation.

### `set_show_always` (public)

```cpp
void set_show_always(bool value)
```

Toggles inactive-window eligibility and re-evaluates current presentation.

### `maximum_width` (public)

```cpp
[[nodiscard]] double maximum_width() const noexcept
```

Returns the logical wrapping ceiling used to measure tooltip bubbles.

### `set_maximum_width` (public)

```cpp
void set_maximum_width(double width)
```

Validates a bounded width, then remeasures and repositions any visible bubble.

### `show` (public)

```cpp
void show(const std::shared_ptr<Control>& target)
```

Immediately presents a mapped target with either default or caller-supplied duration using the common overlay path.

### `show` (public)

```cpp
void show(const std::shared_ptr<Control>& target, std::chrono::milliseconds duration)
```

Immediately presents a mapped target with either default or caller-supplied duration using the common overlay path.

### `hide` (public)

```cpp
void hide()
```

Cancels pending work and closes the active overlay with a hidden event.

### `visible` (public)

```cpp
[[nodiscard]] bool visible() const noexcept
```

Reports whether both visible target and connected popup lease remain live.

### `active_control` (public)

```cpp
[[nodiscard]] std::shared_ptr<Control> active_control() const noexcept
```

Returns the target currently owning visible presentation, if any.

### `visibility_changed` (public)

```cpp
[[nodiscard]] Event<const ToolTipEvent&>& visibility_changed() noexcept
```

Returns the event published after a tooltip becomes visible or hidden.

### `verify_dispose_thread` (protected)

```cpp
void verify_dispose_thread() override
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `bound_window` (private)

```cpp
[[nodiscard]] Window* bound_window() const noexcept
```

Reports the current bound window value without mutation.

### `require_access` (private)

```cpp
void require_access(std::string_view operation) const
```

Reports the current require access value without mutation.

### `find_entry` (private)

```cpp
[[nodiscard]] Entry* find_entry(const Control& target)
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `find_entry` (private)

```cpp
[[nodiscard]] const Entry* find_entry(const Control& target) const
```

Reports the current find entry value without mutation.

### `target_pointer` (private)

```cpp
void target_pointer(const std::shared_ptr<Control>& target, const PointerEvent& event)
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `target_focus` (private)

```cpp
void target_focus(const std::shared_ptr<Control>& target, bool focused)
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `target_moved` (private)

```cpp
void target_moved(const std::shared_ptr<Control>& target)
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `schedule_show` (private)

```cpp
void schedule_show(const std::shared_ptr<Control>& target, bool keyboard_initiated, Point pointer_position)
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `arm` (private)

```cpp
void arm(std::chrono::milliseconds delay, PendingAction action)
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `timer_tick` (private)

```cpp
void timer_tick()
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `show_now` (private)

```cpp
void show_now(const std::shared_ptr<Control>& target, bool keyboard_initiated, std::optional<std::chrono::milliseconds> duration)
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `position_overlay` (private)

```cpp
void position_overlay()
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `close_overlay` (private)

```cpp
void close_overlay(bool emit_change)
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `popup_revoked` (private)

```cpp
void popup_revoked()
```

Public ToolTip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
