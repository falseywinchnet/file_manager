#include "gui_forms/metrics/types/metrics_types.hpp"

#include <sstream>
#include <string_view>

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
           << "\"callback_arbitration_retries\":"
           << callback_arbitration_retries << ','
           << "\"callback_arbitration_limit_hits\":"
           << callback_arbitration_limit_hits << ','
           << "\"scheduled_frame_requests\":" << scheduled_frame_requests << ','
           << "\"scheduler_wakes\":" << scheduler_wakes << ','
           << "\"frame_deadlines_fired\":" << frame_deadlines_fired << ','
           << "\"active_surface_ticks\":" << active_surface_ticks << ','
           << "\"frame_requests_coalesced\":" << frame_requests_coalesced << ','
           << "\"frame_callback_faults\":" << frame_callback_faults << ','
           << "\"reentrant_frame_polls_deferred\":"
           << reentrant_frame_polls_deferred << ','
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

} // namespace gui_forms
