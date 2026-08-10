#include "gui_forms/metrics/metrics/metrics.hpp"

#include <algorithm>

namespace gui_forms {

MetricsSnapshot Metrics::snapshot() const {
    MetricsSnapshot result = values_;
    result.rejected_wrong_thread_operations =
        wrong_thread_rejections_.load(std::memory_order_relaxed);
    return result;
}

void Metrics::reset_activity() noexcept {
    const std::uint64_t controls = values_.control_count;
    const std::uint64_t stable_ids = values_.stable_id_count;
    const std::uint64_t depth = values_.update_scope_depth;
    const std::uint64_t maximum_depth = values_.maximum_update_scope_depth;
    const std::uint64_t focus_scope_depth = values_.focus_scope_depth;
    const std::uint64_t maximum_focus_scope_depth = values_.maximum_focus_scope_depth;
    const std::string renderer = values_.renderer_name;
    const bool cpu_only = values_.cpu_only;
    const std::uint64_t display_cache_entries = values_.display_cache_entries;
    const std::uint64_t display_generation = values_.display_generation;
    const std::uint64_t active_surfaces = values_.active_surface_count;
    values_ = {};
    values_.control_count = controls;
    values_.stable_id_count = stable_ids;
    values_.update_scope_depth = depth;
    values_.maximum_update_scope_depth = maximum_depth;
    values_.focus_scope_depth = focus_scope_depth;
    values_.maximum_focus_scope_depth = maximum_focus_scope_depth;
    values_.renderer_name = renderer;
    values_.cpu_only = cpu_only;
    values_.display_cache_entries = display_cache_entries;
    values_.display_generation = display_generation;
    values_.active_surface_count = active_surfaces;
    values_.maximum_active_surface_count = active_surfaces;
    wrong_thread_rejections_.store(0, std::memory_order_relaxed);
}

void Metrics::set_population(std::uint64_t controls, std::uint64_t stable_ids) noexcept {
    values_.control_count = controls;
    values_.stable_id_count = stable_ids;
}
void Metrics::record_mutation() noexcept { ++values_.mutations; }
void Metrics::record_dirty_mark(double area) noexcept {
    ++values_.dirty_marks;
    values_.requested_damage_area += area;
}
void Metrics::record_measure(std::uint64_t nodes_visited,
                             std::uint64_t callbacks) noexcept {
    ++values_.measure_passes;
    values_.measure_nodes_visited += nodes_visited;
    values_.controls_measured += callbacks;
}
void Metrics::record_arrange(std::uint64_t nodes_visited,
                             std::uint64_t callbacks) noexcept {
    ++values_.arrange_passes;
    values_.arrange_nodes_visited += nodes_visited;
    values_.controls_arranged += callbacks;
}
void Metrics::record_paint(std::uint64_t nodes_visited,
                           std::uint64_t controls,
                           std::uint64_t invalidations_consumed,
                           std::uint64_t chunks_rebuilt,
                           std::uint64_t chunks_reused,
                           std::uint64_t commands_replayed,
                           double area, bool full_window) noexcept {
    ++values_.paint_passes;
    values_.paint_nodes_visited += nodes_visited;
    values_.controls_painted += controls;
    values_.paint_invalidations_consumed += invalidations_consumed;
    values_.display_chunks_rebuilt += chunks_rebuilt;
    values_.display_chunks_reused += chunks_reused;
    values_.display_commands_replayed += commands_replayed;
    values_.painted_damage_area += area;
    ++(full_window ? values_.full_window_paints : values_.partial_paints);
}
void Metrics::set_display_cache(std::uint64_t entries,
                                std::uint64_t generation) noexcept {
    values_.display_cache_entries = entries;
    values_.display_generation = generation;
}
void Metrics::record_input() noexcept { ++values_.input_events; }
void Metrics::record_focus_transition() noexcept { ++values_.focus_transitions; }
void Metrics::record_focus_scope_opened(std::size_t depth) noexcept {
    ++values_.focus_scopes_opened;
    values_.focus_scope_depth = static_cast<std::uint64_t>(depth);
    values_.maximum_focus_scope_depth =
        std::max(values_.maximum_focus_scope_depth,
                 static_cast<std::uint64_t>(depth));
}
void Metrics::record_focus_scope_closed(bool restored_focus,
                                        std::size_t depth) noexcept {
    ++values_.focus_scopes_closed;
    values_.focus_scope_depth = static_cast<std::uint64_t>(depth);
    if (restored_focus) {
        ++values_.focus_scope_restorations;
    }
}
void Metrics::record_focus_scope_rejection() noexcept {
    ++values_.focus_scope_rejections;
}
void Metrics::record_activation() noexcept { ++values_.activations; }
void Metrics::record_disposal(std::uint64_t count) noexcept { values_.disposals += count; }
void Metrics::record_wrong_thread_rejection() noexcept {
    wrong_thread_rejections_.fetch_add(1, std::memory_order_relaxed);
}
void Metrics::record_subscription_connected() noexcept {
    ++values_.subscriptions_connected;
}
void Metrics::record_subscription_disconnected() noexcept {
    ++values_.subscriptions_disconnected;
}
void Metrics::record_callback_emitted(std::uint64_t count) noexcept {
    values_.callbacks_emitted += count;
}
void Metrics::record_focus_revocation() noexcept { ++values_.focus_revocations; }
void Metrics::record_capture_revocation() noexcept { ++values_.capture_revocations; }
void Metrics::record_press_revocation() noexcept { ++values_.press_revocations; }
void Metrics::record_undeclared_mutation() noexcept { ++values_.undeclared_mutations; }
void Metrics::record_damage_region(std::size_t rectangle_count,
                                   std::uint64_t compactions,
                                   std::uint64_t collapses) noexcept {
    values_.damage_region_compactions += compactions;
    values_.damage_region_collapses += collapses;
    values_.maximum_damage_rectangles =
        std::max(values_.maximum_damage_rectangles,
                 static_cast<std::uint64_t>(rectangle_count));
}
void Metrics::enter_update_scope() noexcept {
    ++values_.update_scopes_started;
    ++values_.update_scope_depth;
    values_.maximum_update_scope_depth =
        std::max(values_.maximum_update_scope_depth, values_.update_scope_depth);
}
void Metrics::leave_update_scope() noexcept {
    if (values_.update_scope_depth > 0) {
        --values_.update_scope_depth;
    }
}
void Metrics::record_flush(bool read_barrier) noexcept {
    ++values_.flush_count;
    if (read_barrier) {
        ++values_.read_barrier_flushes;
    }
}
void Metrics::record_pass_limit_hit() noexcept { ++values_.bounded_pass_limit_hits; }
void Metrics::record_callback_arbitration_retry(bool limit_hit) noexcept {
    ++values_.callback_arbitration_retries;
    if (limit_hit) ++values_.callback_arbitration_limit_hits;
}
void Metrics::record_frame_request() noexcept { ++values_.scheduled_frame_requests; }
void Metrics::record_frame_poll(std::uint64_t deadlines_fired,
                                std::uint64_t active_surface_ticks,
                                std::uint64_t coalesced_requests) noexcept {
    if (deadlines_fired != 0 || active_surface_ticks != 0) {
        ++values_.scheduler_wakes;
    }
    values_.frame_deadlines_fired += deadlines_fired;
    values_.active_surface_ticks += active_surface_ticks;
    values_.frame_requests_coalesced += coalesced_requests;
}
void Metrics::record_frame_callback_fault() noexcept {
    ++values_.frame_callback_faults;
}
void Metrics::record_reentrant_frame_poll() noexcept {
    ++values_.reentrant_frame_polls_deferred;
}
void Metrics::set_active_surface_count(std::size_t count) noexcept {
    values_.active_surface_count = static_cast<std::uint64_t>(count);
    values_.maximum_active_surface_count =
        std::max(values_.maximum_active_surface_count, values_.active_surface_count);
}
void Metrics::record_occlusion_transition(bool occluded) noexcept {
    ++(occluded ? values_.occlusion_suspensions : values_.occlusion_resumes);
}
void Metrics::record_occluded_frame_poll() noexcept {
    ++values_.occluded_frame_polls;
}
void Metrics::record_present(std::uint64_t duration) noexcept {
    ++values_.frames_presented;
    values_.present_duration_nanoseconds += duration;
    values_.worst_present_duration_nanoseconds =
        std::max(values_.worst_present_duration_nanoseconds, duration);
}
void Metrics::set_renderer(std::string_view name, bool cpu_only) {
    values_.renderer_name.assign(name);
    values_.cpu_only = cpu_only;
}

} // namespace gui_forms
