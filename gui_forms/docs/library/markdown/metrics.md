# Metrics

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Metrics`  
Declaration: `include/gui_forms/metrics.hpp:85`  
Definition: `src/core/metrics.cpp`

Metrics is a class declared in include/gui_forms/metrics.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `snapshot`

```cpp
[[nodiscard]] MetricsSnapshot snapshot() const
```

Reports the current snapshot value without mutation.

### `reset_activity`

```cpp
void reset_activity() noexcept
```

Returns activity to its inherited or default policy.

### `set_population`

```cpp
void set_population(std::uint64_t controls, std::uint64_t stable_ids) noexcept
```

Synchronously updates the retained population property. Validation, typed invalidation, and notifications are defined by the implementation.

### `record_mutation`

```cpp
void record_mutation() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_dirty_mark`

```cpp
void record_dirty_mark(double requested_damage_area) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_measure`

```cpp
void record_measure(std::uint64_t nodes_visited, std::uint64_t callbacks) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_arrange`

```cpp
void record_arrange(std::uint64_t nodes_visited, std::uint64_t callbacks) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_paint`

```cpp
void record_paint(std::uint64_t nodes_visited, std::uint64_t controls, std::uint64_t invalidations_consumed, std::uint64_t chunks_rebuilt, std::uint64_t chunks_reused, std::uint64_t commands_replayed, double area, bool full_window) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_display_cache`

```cpp
void set_display_cache(std::uint64_t entries, std::uint64_t generation) noexcept
```

Synchronously updates the retained display cache property. Validation, typed invalidation, and notifications are defined by the implementation.

### `record_input`

```cpp
void record_input() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_focus_transition`

```cpp
void record_focus_transition() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_focus_scope_opened`

```cpp
void record_focus_scope_opened(std::size_t depth) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_focus_scope_closed`

```cpp
void record_focus_scope_closed(bool restored_focus, std::size_t depth) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_focus_scope_rejection`

```cpp
void record_focus_scope_rejection() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_activation`

```cpp
void record_activation() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_disposal`

```cpp
void record_disposal(std::uint64_t count = 1) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_wrong_thread_rejection`

```cpp
void record_wrong_thread_rejection() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_subscription_connected`

```cpp
void record_subscription_connected() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_subscription_disconnected`

```cpp
void record_subscription_disconnected() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_callback_emitted`

```cpp
void record_callback_emitted(std::uint64_t count = 1) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_focus_revocation`

```cpp
void record_focus_revocation() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_capture_revocation`

```cpp
void record_capture_revocation() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_press_revocation`

```cpp
void record_press_revocation() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_undeclared_mutation`

```cpp
void record_undeclared_mutation() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_damage_region`

```cpp
void record_damage_region(std::size_t rectangle_count, std::uint64_t compactions, std::uint64_t collapses) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `enter_update_scope`

```cpp
void enter_update_scope() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `leave_update_scope`

```cpp
void leave_update_scope() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_flush`

```cpp
void record_flush(bool read_barrier) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_pass_limit_hit`

```cpp
void record_pass_limit_hit() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_callback_arbitration_retry`

```cpp
void record_callback_arbitration_retry(bool limit_hit) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_frame_request`

```cpp
void record_frame_request() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_frame_poll`

```cpp
void record_frame_poll(std::uint64_t deadlines_fired, std::uint64_t active_surface_ticks, std::uint64_t coalesced_requests) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_frame_callback_fault`

```cpp
void record_frame_callback_fault() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_reentrant_frame_poll`

```cpp
void record_reentrant_frame_poll() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_active_surface_count`

```cpp
void set_active_surface_count(std::size_t count) noexcept
```

Synchronously updates the retained active surface count property. Validation, typed invalidation, and notifications are defined by the implementation.

### `record_occlusion_transition`

```cpp
void record_occlusion_transition(bool occluded) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_occluded_frame_poll`

```cpp
void record_occluded_frame_poll() noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `record_present`

```cpp
void record_present(std::uint64_t duration_nanoseconds) noexcept
```

Public Metrics operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_renderer`

```cpp
void set_renderer(std::string_view name, bool cpu_only)
```

Synchronously updates the retained renderer property. Validation, typed invalidation, and notifications are defined by the implementation.
