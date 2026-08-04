#pragma once

#include <cstdint>
#include <atomic>
#include <cstddef>
#include <string>
#include <string_view>

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
    std::uint64_t scheduled_frame_requests{};
    std::uint64_t scheduler_wakes{};
    std::uint64_t frame_deadlines_fired{};
    std::uint64_t active_surface_ticks{};
    std::uint64_t frame_requests_coalesced{};
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

class Metrics {
public:
    [[nodiscard]] MetricsSnapshot snapshot() const;
    void reset_activity() noexcept;

    void set_population(std::uint64_t controls, std::uint64_t stable_ids) noexcept;
    void record_mutation() noexcept;
    void record_dirty_mark(double requested_damage_area) noexcept;
    void record_measure(std::uint64_t nodes_visited, std::uint64_t callbacks) noexcept;
    void record_arrange(std::uint64_t nodes_visited, std::uint64_t callbacks) noexcept;
    void record_paint(std::uint64_t nodes_visited,
                      std::uint64_t controls,
                      std::uint64_t invalidations_consumed,
                      std::uint64_t chunks_rebuilt,
                      std::uint64_t chunks_reused,
                      std::uint64_t commands_replayed,
                      double area,
                      bool full_window) noexcept;
    void set_display_cache(std::uint64_t entries, std::uint64_t generation) noexcept;
    void record_input() noexcept;
    void record_focus_transition() noexcept;
    void record_activation() noexcept;
    void record_disposal(std::uint64_t count = 1) noexcept;
    void record_wrong_thread_rejection() noexcept;
    void record_subscription_connected() noexcept;
    void record_subscription_disconnected() noexcept;
    void record_callback_emitted(std::uint64_t count = 1) noexcept;
    void record_focus_revocation() noexcept;
    void record_capture_revocation() noexcept;
    void record_press_revocation() noexcept;
    void record_undeclared_mutation() noexcept;
    void record_damage_region(std::size_t rectangle_count,
                              std::uint64_t compactions,
                              std::uint64_t collapses) noexcept;
    void enter_update_scope() noexcept;
    void leave_update_scope() noexcept;
    void record_flush(bool read_barrier) noexcept;
    void record_pass_limit_hit() noexcept;
    void record_frame_request() noexcept;
    void record_frame_poll(std::uint64_t deadlines_fired,
                           std::uint64_t active_surface_ticks,
                           std::uint64_t coalesced_requests) noexcept;
    void set_active_surface_count(std::size_t count) noexcept;
    void record_occlusion_transition(bool occluded) noexcept;
    void record_occluded_frame_poll() noexcept;
    void record_present(std::uint64_t duration_nanoseconds) noexcept;
    void set_renderer(std::string_view name, bool cpu_only) ;

private:
    MetricsSnapshot values_;
    std::atomic<std::uint64_t> wrong_thread_rejections_{};
};

} // namespace gui_forms
