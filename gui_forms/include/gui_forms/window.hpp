#pragma once

#include "gui_forms/control.hpp"
#include "gui_forms/display.hpp"
#include "gui_forms/event.hpp"
#include "gui_forms/metrics.hpp"
#include "gui_forms/resources.hpp"
#include "gui_forms/scheduler.hpp"

#include <chrono>
#include <array>
#include <memory>
#include <optional>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

namespace gui_forms {

class UpdateScope;

struct PointerCaptureChange final {
    bool captured{};
    RuntimeId control_id{};
    std::string stable_id;
    std::uint64_t pointer_id{};
};

class Window {
public:
    explicit Window(Control::Ptr root, Size client_size = {});
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    [[nodiscard]] Control::Ptr root() const noexcept { return root_; }
    [[nodiscard]] Size client_size() const noexcept { return client_size_; }
    void resize(Size client_size);
    void set_scale(double scale);
    [[nodiscard]] double scale() const noexcept { return scale_; }

    [[nodiscard]] UpdateScope begin_update();
    void perform_layout();
    void flush();
    void paint(Painter& painter, Rect requested_damage = {});

    [[nodiscard]] DamageRegion take_damage();
    [[nodiscard]] DamageRegion take_damage(PaintPlane plane);
    [[nodiscard]] bool needs_frame() const noexcept;
    [[nodiscard]] std::optional<FrameTime> next_wake() const noexcept;
    [[nodiscard]] FrameRequestToken schedule_paint(const Control::Ptr& control,
                                                   FrameTime deadline);
    [[nodiscard]] FrameRequestToken activate_surface(const Control::Ptr& control,
                                                     FrameInterval interval,
                                                     FrameTime first_deadline);
    [[nodiscard]] FramePollResult poll_frame_schedule(FrameTime now);
    void cancel_frame_requests();
    void set_occluded(bool occluded, FrameTime transition_time);
    [[nodiscard]] bool occluded() const noexcept { return occluded_; }

    [[nodiscard]] ImageLoadResult load_png(std::span<const std::byte> encoded);
    [[nodiscard]] ImageLoadResult replace_png(ImageId image,
                                               std::span<const std::byte> encoded);
    [[nodiscard]] ImageLoadResult replace_png(ImageId image,
                                               std::span<const std::byte> encoded,
                                               Control& consumer);
    [[nodiscard]] bool remove_image(ImageId image);
    [[nodiscard]] const ImageRegistry& image_resources() const noexcept {
        return image_resources_;
    }
    [[nodiscard]] ImageRegistrySnapshot image_resource_snapshot() const noexcept {
        return image_resources_.snapshot();
    }

    [[nodiscard]] Control::Ptr find(std::string_view stable_id) const;
    [[nodiscard]] Control::Ptr hit_test(Point position);
    bool request_focus(const Control::Ptr& control);
    [[nodiscard]] Control::Ptr focused_control() const noexcept { return focused_.lock(); }
    void capture_pointer(const Control::Ptr& control, std::uint64_t pointer_id = 1);
    void release_pointer();
    [[nodiscard]] Control::Ptr captured_control() const noexcept { return captured_.lock(); }
    [[nodiscard]] std::uint64_t captured_pointer_id() const noexcept {
        return captured_pointer_id_;
    }
    [[nodiscard]] Event<const PointerCaptureChange&>& pointer_capture_changed() noexcept {
        return pointer_capture_changed_;
    }
    [[nodiscard]] Control::Ptr pressed_control() const noexcept { return pressed_.lock(); }

    bool dispatch_pointer(PointerEvent event);
    bool dispatch_key(KeyEvent event);
    bool dispatch_text(TextInputEvent event);
    [[nodiscard]] DragDispatchResult dispatch_drag(DragEvent event);
    void cancel_drag() noexcept;

    [[nodiscard]] MetricsSnapshot metrics_snapshot() const { return metrics_.snapshot(); }
    Metrics& metrics() noexcept { return metrics_; }
    void reset_activity_metrics() noexcept { metrics_.reset_activity(); }

private:
    friend class Control;
    friend class UpdateScope;

    void attach_subtree(const Control::Ptr& control, const Control::WeakPtr& parent);
    void detach_subtree(const Control::Ptr& control);
    void dispose_subtree(const Control::Ptr& control) noexcept;
    void revoke_interaction_for_subtree(const Control::Ptr& control,
                                        bool notify_focus);
    void change_pointer_capture(const Control::Ptr& control,
                                std::uint64_t pointer_id,
                                bool revoked);
    void on_eligibility_changed(const Control::Ptr& control);
    void register_subtree(const Control::Ptr& control);
    void unregister_subtree(const Control::Ptr& control);
    void mark_dirty(Control& control, Dirty dirty);
    void mark_subtree_dirty(Control& control, Dirty dirty);
    void change_paint_plane(Control& control, PaintPlane plane);
    void add_damage(Rect damage, PaintPlane plane);
    void add_damage_all_planes(Rect damage);
    void add_subtree_damage(const Control::Ptr& control);
    void ensure_layout(bool read_barrier);
    void leave_update_scope();
    void flush_if_outermost();
    [[nodiscard]] std::vector<Control::Ptr> route_to(const Control::Ptr& target) const;
    [[nodiscard]] Control::Ptr drop_target_at(Point position);
    [[nodiscard]] DragDispatchResult route_drag(const Control::Ptr& target,
                                                DragEvent event);
    [[nodiscard]] Control::Ptr hit_test_recursive(const Control::Ptr& control,
                                                  Point window_position) const;
    void paint_recursive(const Control::Ptr& control,
                         Painter& painter,
                         Rect window_damage,
                         PaintPlane plane,
                         std::uint64_t& visited_nodes,
                         std::uint64_t& painted_controls,
                         std::uint64_t& consumed_invalidations,
                         std::uint64_t& chunks_rebuilt,
                         std::uint64_t& chunks_reused,
                         std::uint64_t& commands_replayed);
    void measure_dirty_recursive(const Control::Ptr& control,
                                 Size available,
                                 std::uint64_t& visited_nodes,
                                 std::uint64_t& callbacks);
    void arrange_dirty_recursive(const Control::Ptr& control,
                                 Rect final_bounds,
                                 std::uint64_t& visited_nodes,
                                 std::uint64_t& callbacks);
    [[nodiscard]] Dirty recompute_subtree_dirty(const Control::Ptr& control) noexcept;
    void clear_layout_dirty_subtree(const Control::Ptr& control) noexcept;
    void clear_paint_dirty_subtree(const Control::Ptr& control) noexcept;
    [[nodiscard]] std::uint64_t display_cache_entries(
        const Control::Ptr& control) const noexcept;
    void update_display_cache_metrics() noexcept;
    void compact_frame_requests() noexcept;
    [[nodiscard]] std::size_t active_surface_count() const noexcept;
    void update_frame_schedule_metrics() noexcept;
    [[nodiscard]] Rect absolute_bounds_of(const Control& control) const;
    [[nodiscard]] bool eligible(const Control::Ptr& control) const noexcept;
    void require_ui_thread(std::string_view operation);

    Control::Ptr root_;
    Size client_size_{};
    double scale_{1.0};
    std::unordered_map<std::string, Control::WeakPtr> stable_ids_;
    Control::WeakPtr focused_;
    Control::WeakPtr captured_;
    std::uint64_t captured_pointer_id_{};
    Event<const PointerCaptureChange&> pointer_capture_changed_;
    Control::WeakPtr pressed_;
    Control::WeakPtr hovered_;
    Control::WeakPtr drag_target_;
    std::uint64_t drag_session_id_{};
    std::array<DamageRegion, paint_plane_count> plane_damage_;
    Metrics metrics_;
    ImageRegistry image_resources_;
    std::uint64_t display_generation_{};
    std::vector<std::shared_ptr<detail::ScheduledFrameRequest>> frame_requests_;
    std::thread::id ui_thread_;
    std::uint64_t update_depth_{};
    bool layout_dirty_{true};
    bool paint_dirty_{true};
    bool hit_test_dirty_{true};
    bool in_layout_{};
    bool in_paint_{};
    bool in_lifecycle_notification_{};
    bool second_layout_pass_requested_{};
    bool occluded_{};
};

class UpdateScope {
public:
    explicit UpdateScope(Window& window) noexcept : window_(&window) {}
    ~UpdateScope();
    UpdateScope(UpdateScope&& other) noexcept;
    UpdateScope& operator=(UpdateScope&& other) noexcept;
    UpdateScope(const UpdateScope&) = delete;
    UpdateScope& operator=(const UpdateScope&) = delete;

    void perform_layout();
    void close();

private:
    Window* window_{};
};

} // namespace gui_forms
