#include "gui_forms/metrics.hpp"

#include <algorithm>
#include <sstream>

namespace gui_forms {

namespace {

std::string escaped_json(std::string_view value) {
    std::string result;
    result.reserve(value.size());
    for (const char character : value) {
        switch (character) {
        case '\\': result += "\\\\"; break;
        case '"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += character; break;
        }
    }
    return result;
}

} // namespace

std::string MetricsSnapshot::to_json() const {
    std::ostringstream output;
    output << '{'
           << "\"control_count\":" << control_count << ','
           << "\"stable_id_count\":" << stable_id_count << ','
           << "\"mutations\":" << mutations << ','
           << "\"dirty_marks\":" << dirty_marks << ','
           << "\"measure_passes\":" << measure_passes << ','
           << "\"arrange_passes\":" << arrange_passes << ','
           << "\"paint_passes\":" << paint_passes << ','
           << "\"controls_measured\":" << controls_measured << ','
           << "\"controls_arranged\":" << controls_arranged << ','
           << "\"controls_painted\":" << controls_painted << ','
           << "\"measure_nodes_visited\":" << measure_nodes_visited << ','
           << "\"arrange_nodes_visited\":" << arrange_nodes_visited << ','
           << "\"paint_nodes_visited\":" << paint_nodes_visited << ','
           << "\"paint_invalidations_consumed\":" << paint_invalidations_consumed << ','
           << "\"display_chunks_rebuilt\":" << display_chunks_rebuilt << ','
           << "\"display_chunks_reused\":" << display_chunks_reused << ','
           << "\"display_commands_replayed\":" << display_commands_replayed << ','
           << "\"display_cache_entries\":" << display_cache_entries << ','
           << "\"display_generation\":" << display_generation << ','
           << "\"requested_damage_area\":" << requested_damage_area << ','
           << "\"painted_damage_area\":" << painted_damage_area << ','
           << "\"full_window_paints\":" << full_window_paints << ','
           << "\"partial_paints\":" << partial_paints << ','
           << "\"input_events\":" << input_events << ','
           << "\"focus_transitions\":" << focus_transitions << ','
           << "\"focus_scopes_opened\":" << focus_scopes_opened << ','
           << "\"focus_scopes_closed\":" << focus_scopes_closed << ','
           << "\"focus_scope_restorations\":" << focus_scope_restorations << ','
           << "\"focus_scope_rejections\":" << focus_scope_rejections << ','
           << "\"focus_scope_depth\":" << focus_scope_depth << ','
           << "\"maximum_focus_scope_depth\":" << maximum_focus_scope_depth << ','
           << "\"activations\":" << activations << ','
           << "\"disposals\":" << disposals << ','
           << "\"rejected_wrong_thread_operations\":" << rejected_wrong_thread_operations << ','
           << "\"subscriptions_connected\":" << subscriptions_connected << ','
           << "\"subscriptions_disconnected\":" << subscriptions_disconnected << ','
           << "\"callbacks_emitted\":" << callbacks_emitted << ','
           << "\"focus_revocations\":" << focus_revocations << ','
           << "\"capture_revocations\":" << capture_revocations << ','
           << "\"press_revocations\":" << press_revocations << ','
           << "\"undeclared_mutations\":" << undeclared_mutations << ','
           << "\"damage_region_compactions\":" << damage_region_compactions << ','
           << "\"damage_region_collapses\":" << damage_region_collapses << ','
           << "\"maximum_damage_rectangles\":" << maximum_damage_rectangles << ','
           << "\"update_scopes_started\":" << update_scopes_started << ','
           << "\"update_scope_depth\":" << update_scope_depth << ','
           << "\"maximum_update_scope_depth\":" << maximum_update_scope_depth << ','
           << "\"flush_count\":" << flush_count << ','
           << "\"read_barrier_flushes\":" << read_barrier_flushes << ','
           << "\"bounded_pass_limit_hits\":" << bounded_pass_limit_hits << ','
           << "\"scheduled_frame_requests\":" << scheduled_frame_requests << ','
           << "\"scheduler_wakes\":" << scheduler_wakes << ','
           << "\"frame_deadlines_fired\":" << frame_deadlines_fired << ','
           << "\"active_surface_ticks\":" << active_surface_ticks << ','
           << "\"frame_requests_coalesced\":" << frame_requests_coalesced << ','
           << "\"active_surface_count\":" << active_surface_count << ','
           << "\"maximum_active_surface_count\":" << maximum_active_surface_count << ','
           << "\"occlusion_suspensions\":" << occlusion_suspensions << ','
           << "\"occlusion_resumes\":" << occlusion_resumes << ','
           << "\"occluded_frame_polls\":" << occluded_frame_polls << ','
           << "\"frames_presented\":" << frames_presented << ','
           << "\"present_duration_nanoseconds\":" << present_duration_nanoseconds << ','
           << "\"worst_present_duration_nanoseconds\":" << worst_present_duration_nanoseconds << ','
           << "\"renderer_name\":\"" << escaped_json(renderer_name) << "\","
           << "\"cpu_only\":" << (cpu_only ? "true" : "false")
           << '}';
    return output.str();
}

MetricsSnapshot Metrics::snapshot() const {
    MetricsSnapshot result = values_;
    result.rejected_wrong_thread_operations =
        wrong_thread_rejections_.load(std::memory_order_relaxed);
    return result;
}

void Metrics::reset_activity() noexcept {
    const auto controls = values_.control_count;
    const auto stable_ids = values_.stable_id_count;
    const auto depth = values_.update_scope_depth;
    const auto maximum_depth = values_.maximum_update_scope_depth;
    const auto focus_scope_depth = values_.focus_scope_depth;
    const auto maximum_focus_scope_depth = values_.maximum_focus_scope_depth;
    const auto renderer = values_.renderer_name;
    const auto cpu_only = values_.cpu_only;
    const auto display_cache_entries = values_.display_cache_entries;
    const auto display_generation = values_.display_generation;
    const auto active_surfaces = values_.active_surface_count;
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
