#pragma once

#include "gui_forms/metrics/types/metrics_types.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace gui_forms {

class Metrics {
public:
    [[nodiscard]] MetricsSnapshot snapshot() const;
    void reset_activity() noexcept;

    void set_population(std::uint64_t controls, std::uint64_t stable_ids) noexcept;
    void record_mutation() noexcept;
    void record_dirty_mark(double requested_damage_area) noexcept;
    void record_measure(std::uint64_t nodes_visited,
                        std::uint64_t callbacks) noexcept;
    void record_arrange(std::uint64_t nodes_visited,
                        std::uint64_t callbacks) noexcept;
    void record_paint(std::uint64_t nodes_visited,
                      std::uint64_t controls,
                      std::uint64_t invalidations_consumed,
                      std::uint64_t chunks_rebuilt,
                      std::uint64_t chunks_reused,
                      std::uint64_t commands_replayed,
                      double area,
                      bool full_window) noexcept;
    void set_display_cache(std::uint64_t entries,
                           std::uint64_t generation) noexcept;
    void record_input() noexcept;
    void record_focus_transition() noexcept;
    void record_focus_scope_opened(std::size_t depth) noexcept;
    void record_focus_scope_closed(bool restored_focus,
                                   std::size_t depth) noexcept;
    void record_focus_scope_rejection() noexcept;
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
    void record_callback_arbitration_retry(bool limit_hit) noexcept;
    void record_frame_request() noexcept;
    void record_frame_poll(std::uint64_t deadlines_fired,
                           std::uint64_t active_surface_ticks,
                           std::uint64_t coalesced_requests) noexcept;
    void record_frame_callback_fault() noexcept;
    void record_reentrant_frame_poll() noexcept;
    void set_active_surface_count(std::size_t count) noexcept;
    void record_occlusion_transition(bool occluded) noexcept;
    void record_occluded_frame_poll() noexcept;
    void record_present(std::uint64_t duration_nanoseconds) noexcept;
    void set_renderer(std::string_view name, bool cpu_only);

private:
    MetricsSnapshot values_;
    std::atomic<std::uint64_t> wrong_thread_rejections_{};
};

} // namespace gui_forms
