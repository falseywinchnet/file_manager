#pragma once

#include <cstdint>
#include <string>

namespace gui_forms {

struct MetricsSnapshot {
    std::uint64_t control_count{};
    std::uint64_t stable_id_count{};
    std::uint64_t mutations{};
    std::uint64_t dirty_marks{};
    std::uint64_t measure_passes{};
    std::uint64_t arrange_passes{};
    std::uint64_t paint_passes{};
    std::uint64_t controls_measured{};
    std::uint64_t controls_arranged{};
    std::uint64_t controls_painted{};
    std::uint64_t measure_nodes_visited{};
    std::uint64_t arrange_nodes_visited{};
    std::uint64_t paint_nodes_visited{};
    std::uint64_t paint_invalidations_consumed{};
    std::uint64_t display_chunks_rebuilt{};
    std::uint64_t display_chunks_reused{};
    std::uint64_t display_commands_replayed{};
    std::uint64_t display_cache_entries{};
    std::uint64_t display_generation{};
    double requested_damage_area{};
    double painted_damage_area{};
    std::uint64_t full_window_paints{};
    std::uint64_t partial_paints{};
    std::uint64_t input_events{};
    std::uint64_t focus_transitions{};
    std::uint64_t focus_scopes_opened{};
    std::uint64_t focus_scopes_closed{};
    std::uint64_t focus_scope_restorations{};
    std::uint64_t focus_scope_rejections{};
    std::uint64_t focus_scope_depth{};
    std::uint64_t maximum_focus_scope_depth{};
    std::uint64_t activations{};
    std::uint64_t disposals{};
    std::uint64_t rejected_wrong_thread_operations{};
    std::uint64_t subscriptions_connected{};
    std::uint64_t subscriptions_disconnected{};
    std::uint64_t callbacks_emitted{};
    std::uint64_t focus_revocations{};
    std::uint64_t capture_revocations{};
    std::uint64_t press_revocations{};
    std::uint64_t undeclared_mutations{};
    std::uint64_t damage_region_compactions{};
    std::uint64_t damage_region_collapses{};
    std::uint64_t maximum_damage_rectangles{};
    std::uint64_t update_scopes_started{};
    std::uint64_t update_scope_depth{};
    std::uint64_t maximum_update_scope_depth{};
    std::uint64_t flush_count{};
    std::uint64_t read_barrier_flushes{};
    std::uint64_t bounded_pass_limit_hits{};
    std::uint64_t callback_arbitration_retries{};
    std::uint64_t callback_arbitration_limit_hits{};
    std::uint64_t scheduled_frame_requests{};
    std::uint64_t scheduler_wakes{};
    std::uint64_t frame_deadlines_fired{};
    std::uint64_t active_surface_ticks{};
    std::uint64_t frame_requests_coalesced{};
    std::uint64_t frame_callback_faults{};
    std::uint64_t reentrant_frame_polls_deferred{};
    std::uint64_t active_surface_count{};
    std::uint64_t maximum_active_surface_count{};
    std::uint64_t occlusion_suspensions{};
    std::uint64_t occlusion_resumes{};
    std::uint64_t occluded_frame_polls{};
    std::uint64_t frames_presented{};
    std::uint64_t present_duration_nanoseconds{};
    std::uint64_t worst_present_duration_nanoseconds{};
    std::string renderer_name{"unbound"};
    bool cpu_only{true};

    [[nodiscard]] std::string to_json() const;
};

} // namespace gui_forms
