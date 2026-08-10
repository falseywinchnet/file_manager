# Metrics

- Status: **OBSERVED: bundle 010 metrics accumulator split; focused M4 core tests pass**
- Kind: **class**
- Hierarchy: `Metrics`
- Declaration: `include/gui_forms/metrics/metrics/metrics.hpp:12`
- Definition: `src/core/metrics/metrics/metrics.cpp`

Metrics is the structured counter accumulator used by Window and retained controls; only wrong-thread rejection is atomic because that rejection may originate off-thread.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `snapshot` (public)

```cpp
[[nodiscard]] MetricsSnapshot snapshot() const
```

Copies retained counters and samples the atomic wrong-thread rejection total.

### `reset_activity` (public)

```cpp
void reset_activity() noexcept
```

Clears transient activity while preserving population, nesting depths, renderer, display cache, and active-surface baselines.

### `set_population` (public)

```cpp
void set_population(std::uint64_t controls, std::uint64_t stable_ids) noexcept
```

Sets live control and stable-ID counts.

### `record_mutation` (public)

```cpp
void record_mutation() noexcept
```

Counts one retained mutation.

### `record_dirty_mark` (public)

```cpp
void record_dirty_mark(double requested_damage_area) noexcept
```

Counts a dirty mark and accumulates requested area.

### `record_measure` (public)

```cpp
void record_measure(std::uint64_t nodes_visited, std::uint64_t callbacks) noexcept
```

Counts one measure pass, visited nodes, and callbacks.

### `record_arrange` (public)

```cpp
void record_arrange(std::uint64_t nodes_visited, std::uint64_t callbacks) noexcept
```

Counts one arrange pass, visited nodes, and callbacks.

### `record_paint` (public)

```cpp
void record_paint(std::uint64_t nodes_visited, std::uint64_t controls, std::uint64_t invalidations_consumed, std::uint64_t chunks_rebuilt, std::uint64_t chunks_reused, std::uint64_t commands_replayed, double area, bool full_window) noexcept
```

Counts paint traversal, invalidations, display cache work, replay, area, and full/partial classification.

### `set_display_cache` (public)

```cpp
void set_display_cache(std::uint64_t entries, std::uint64_t generation) noexcept
```

Sets current cache entries and generation.

### `record_input` (public)

```cpp
void record_input() noexcept
```

Counts one input event.

### `record_focus_transition` (public)

```cpp
void record_focus_transition() noexcept
```

Counts one focus transition.

### `record_focus_scope_opened` (public)

```cpp
void record_focus_scope_opened(std::size_t depth) noexcept
```

Counts an open, sets depth, and raises maximum depth.

### `record_focus_scope_closed` (public)

```cpp
void record_focus_scope_closed(bool restored_focus, std::size_t depth) noexcept
```

Counts a close, optional restoration, and resulting depth.

### `record_focus_scope_rejection` (public)

```cpp
void record_focus_scope_rejection() noexcept
```

Counts one rejected focus-scope operation.

### `record_activation` (public)

```cpp
void record_activation() noexcept
```

Counts one activation transition.

### `record_disposal` (public)

```cpp
void record_disposal(std::uint64_t count = 1) noexcept
```

Adds one or more disposed controls.

### `record_wrong_thread_rejection` (public)

```cpp
void record_wrong_thread_rejection() noexcept
```

Atomically counts a rejected cross-thread operation.

### `record_subscription_connected` (public)

```cpp
void record_subscription_connected() noexcept
```

Counts one connected subscription.

### `record_subscription_disconnected` (public)

```cpp
void record_subscription_disconnected() noexcept
```

Counts one disconnected subscription.

### `record_callback_emitted` (public)

```cpp
void record_callback_emitted(std::uint64_t count = 1) noexcept
```

Adds emitted callback count.

### `record_focus_revocation` (public)

```cpp
void record_focus_revocation() noexcept
```

Counts focus revocation.

### `record_capture_revocation` (public)

```cpp
void record_capture_revocation() noexcept
```

Counts pointer-capture revocation.

### `record_press_revocation` (public)

```cpp
void record_press_revocation() noexcept
```

Counts press-state revocation.

### `record_undeclared_mutation` (public)

```cpp
void record_undeclared_mutation() noexcept
```

Counts a mutation outside declared static-tree policy.

### `record_damage_region` (public)

```cpp
void record_damage_region(std::size_t rectangle_count, std::uint64_t compactions, std::uint64_t collapses) noexcept
```

Accumulates compaction/collapse work and tracks maximum rectangle cardinality.

### `enter_update_scope` (public)

```cpp
void enter_update_scope() noexcept
```

Counts and enters one nested update scope.

### `leave_update_scope` (public)

```cpp
void leave_update_scope() noexcept
```

Leaves one update scope without underflow.

### `record_flush` (public)

```cpp
void record_flush(bool read_barrier) noexcept
```

Counts a flush and optional read-barrier cause.

### `record_pass_limit_hit` (public)

```cpp
void record_pass_limit_hit() noexcept
```

Counts bounded layout-pass exhaustion.

### `record_callback_arbitration_retry` (public)

```cpp
void record_callback_arbitration_retry(bool limit_hit) noexcept
```

Counts callback arbitration retry and optional limit exhaustion.

### `record_frame_request` (public)

```cpp
void record_frame_request() noexcept
```

Counts a scheduled frame request.

### `record_frame_poll` (public)

```cpp
void record_frame_poll(std::uint64_t deadlines_fired, std::uint64_t active_surface_ticks, std::uint64_t coalesced_requests) noexcept
```

Accumulates deadlines, active ticks, coalescing, and nonempty scheduler wakes.

### `record_frame_callback_fault` (public)

```cpp
void record_frame_callback_fault() noexcept
```

Counts a contained frame callback exception.

### `record_reentrant_frame_poll` (public)

```cpp
void record_reentrant_frame_poll() noexcept
```

Counts a deferred reentrant poll.

### `set_active_surface_count` (public)

```cpp
void set_active_surface_count(std::size_t count) noexcept
```

Sets active surfaces and raises the maximum.

### `record_occlusion_transition` (public)

```cpp
void record_occlusion_transition(bool occluded) noexcept
```

Counts occlusion suspension or resumption.

### `record_occluded_frame_poll` (public)

```cpp
void record_occluded_frame_poll() noexcept
```

Counts a poll suppressed by occlusion.

### `record_present` (public)

```cpp
void record_present(std::uint64_t duration_nanoseconds) noexcept
```

Counts presentation, total duration, and worst duration.

### `set_renderer` (public)

```cpp
void set_renderer(std::string_view name, bool cpu_only)
```

Sets renderer identity and the explicit CPU-only assertion.
