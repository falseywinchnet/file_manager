# ErrorProvider

- Status: **OBSERVED: bundle 005 split and icon-size enhancement; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class**
- Hierarchy: `Component → ErrorProvider`
- Declaration: `include/gui_forms/components/error_provider/error_provider.hpp:72`
- Definition: `src/controls/guidance/error_provider/error_provider.cpp`

ErrorProvider is a nonvisual validation-adornment provider bound to a Window lifetime. It tokenizes messages and per-target alignment/padding, composes independent source-private glyph overlays and tooltip text, schedules blink through the shared frame scheduler, mirrors policy for right-to-left layout, optionally imports binding errors, and exposes caller-configurable icon size and exact snapshots.

## Visual evidence

![ErrorProvider](../captures/tool_tip_error_provider.png)

## Declared methods

### `ErrorProvider` (public)

```cpp
explicit ErrorProvider(Window& window)
```

Binds the provider to a live Window and subscribes to target availability, presentation, and root-geometry changes.

### `~ErrorProvider` (public)

```cpp
~ErrorProvider() override
```

Closes glyph popups and disconnects target, binding, tooltip, scheduler, and lifetime state before destruction.

### `can_extend` (public)

```cpp
[[nodiscard]] bool can_extend(const std::shared_ptr<Control>& target) const
```

Reports whether a shared control is live, belongs to the bound Window, and can accept provider metadata.

### `set_error` (public)

```cpp
void set_error(const std::shared_ptr<Control>& target, std::string error)
```

Validates a target, commits or clears its message, refreshes semantic and visual adornment, and publishes a real transition.

### `error` (public)

```cpp
[[nodiscard]] std::string error(const Control& target) const
```

Returns the current retained message for a target.

### `clear` (public)

```cpp
void clear()
```

Clears every mapped and bound error, closes glyph overlays, and preserves provider policy.

### `has_errors` (public)

```cpp
[[nodiscard]] bool has_errors() const noexcept
```

Reports whether any live entry retains a nonempty effective error.

### `set_icon_alignment` (public)

```cpp
void set_icon_alignment(const std::shared_ptr<Control>& target, ErrorIconAlignment alignment)
```

Commits per-target six-way edge alignment and immediately repositions a presented glyph.

### `icon_alignment` (public)

```cpp
[[nodiscard]] ErrorIconAlignment icon_alignment(const Control& target) const
```

Returns the target's retained alignment or the provider default.

### `set_icon_padding` (public)

```cpp
void set_icon_padding(const std::shared_ptr<Control>& target, double padding)
```

Validates and commits per-target logical separation from the aligned target edge.

### `icon_padding` (public)

```cpp
[[nodiscard]] double icon_padding(const Control& target) const
```

Returns retained per-target logical glyph padding.

### `icon_size` (public)

```cpp
[[nodiscard]] double icon_size() const noexcept
```

Returns the provider-wide logical glyph square size.

### `set_icon_size` (public)

```cpp
void set_icon_size(double size)
```

Validates a bounded size and remeasures/repositions all presented glyphs.

### `blink_rate` (public)

```cpp
[[nodiscard]] std::chrono::milliseconds blink_rate() const noexcept
```

Returns the scheduler cadence used while blinking is active.

### `set_blink_rate` (public)

```cpp
void set_blink_rate(std::chrono::milliseconds rate)
```

Validates positive cadence and refreshes every live blink schedule.

### `blink_style` (public)

```cpp
[[nodiscard]] ErrorBlinkStyle blink_style() const noexcept
```

Returns conditional, continuous, or disabled blink policy.

### `set_blink_style` (public)

```cpp
void set_blink_style(ErrorBlinkStyle style)
```

Commits blink policy and restarts or suppresses schedules coherently.

### `right_to_left` (public)

```cpp
[[nodiscard]] bool right_to_left() const noexcept
```

Reports whether left/right alignment policies are mirrored.

### `set_right_to_left` (public)

```cpp
void set_right_to_left(bool value)
```

Commits mirroring, repositions glyphs, and publishes a real policy transition.

### `right_to_left_changed` (public)

```cpp
[[nodiscard]] Event<bool>& right_to_left_changed() noexcept
```

Returns the event published after right-to-left policy changes.

### `icon` (public)

```cpp
[[nodiscard]] std::optional<ImageId> icon() const noexcept
```

Returns the optional caller-owned ImageId used instead of the default error mark.

### `set_icon` (public)

```cpp
void set_icon(std::optional<ImageId> icon)
```

Rebinds glyph imagery and refreshes every live adornment.

### `container_control` (public)

```cpp
[[nodiscard]] std::shared_ptr<Control> container_control() const noexcept
```

Returns the Window root control that bounds provider extension and overlay geometry.

### `data_source` (public)

```cpp
[[nodiscard]] std::shared_ptr<BindingSource> data_source() const noexcept
```

Returns the optional weak BindingSource whose completion errors feed this provider.

### `set_data_source` (public)

```cpp
void set_data_source(std::shared_ptr<BindingSource> source)
```

Rebinds source subscriptions, clears stale bound errors, and refreshes imported error state.

### `data_member` (public)

```cpp
[[nodiscard]] const std::string& data_member() const noexcept
```

Returns the optional member filter applied to binding errors.

### `set_data_member` (public)

```cpp
void set_data_member(std::string member)
```

Commits the member filter and re-evaluates bound errors.

### `bind_to_data_and_errors` (public)

```cpp
void bind_to_data_and_errors(std::shared_ptr<BindingSource> source, std::string member =
```

Atomically binds source and member policy before importing current completion errors.

### `update_binding` (public)

```cpp
void update_binding()
```

Rebuilds provider errors from current BindingSource completion state without disturbing authored mappings.

### `data_source_changed` (public)

```cpp
[[nodiscard]] Event<>& data_source_changed() noexcept
```

Returns the event published after source ownership changes.

### `data_member_changed` (public)

```cpp
[[nodiscard]] Event<const std::string&>& data_member_changed() noexcept
```

Returns the event carrying the committed member filter after change.

### `tag` (public)

```cpp
[[nodiscard]] const std::any& tag() const noexcept
```

Returns caller-owned opaque provider metadata.

### `set_tag` (public)

```cpp
void set_tag(std::any tag)
```

Replaces caller-owned opaque provider metadata without affecting validation state.

### `error_changed` (public)

```cpp
[[nodiscard]] Event<const ErrorProviderChange&>& error_changed() noexcept
```

Returns the event published after each target error commit.

### `snapshot` (public)

```cpp
[[nodiscard]] ErrorProviderSnapshot snapshot() const
```

Returns deterministic icon records and live/presented counts for tests and diagnostics.

### `verify_dispose_thread` (protected)

```cpp
void verify_dispose_thread() override
```

Executes ErrorProvider's verify dispose thread operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Executes ErrorProvider's on dispose operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

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

Executes ErrorProvider's find entry operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `find_entry` (private)

```cpp
[[nodiscard]] const Entry* find_entry(const Control& target) const
```

Reports the current find entry value without mutation.

### `require_entry` (private)

```cpp
[[nodiscard]] Entry& require_entry(const std::shared_ptr<Control>& target)
```

Executes ErrorProvider's require entry operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `refresh_visual` (private)

```cpp
void refresh_visual(Entry& entry, bool error_changed)
```

Executes ErrorProvider's refresh visual operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `close_visual` (private)

```cpp
void close_visual(Entry& entry) noexcept
```

Executes ErrorProvider's close visual operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `refresh_all_visuals` (private)

```cpp
void refresh_all_visuals(bool restart_blink)
```

Executes ErrorProvider's refresh all visuals operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `position_visual` (private)

```cpp
void position_visual(Entry& entry)
```

Executes ErrorProvider's position visual operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `icon_bounds` (private)

```cpp
[[nodiscard]] Rect icon_bounds(const Entry& entry) const noexcept
```

Reports the current icon bounds value without mutation.

### `erase_if_empty` (private)

```cpp
void erase_if_empty(std::uint64_t runtime_id)
```

Executes ErrorProvider's erase if empty operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `clear_bound_errors` (private)

```cpp
void clear_bound_errors() noexcept
```

Removes the explicit bound errors value and restores fallback behavior.

### `binding_completed` (private)

```cpp
void binding_completed(BindingCompleteEvent& event)
```

Executes ErrorProvider's binding completed operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
