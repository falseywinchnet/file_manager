#include "gui_forms/window.hpp"
#include "../../dispatcher/state/dispatcher_state.hpp"
#include "../../scheduler/request/scheduled_frame_request.hpp"
#include "../accelerator/accelerator_attachment.hpp"
#include "../popup/popup_attachment.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <functional>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <unordered_set>

namespace gui_forms {

Window::Window(Control::Ptr root, Size client_size)
    : root_(std::move(root)), client_size_(client_size),
      theme_(default_theme()),
      lifetime_(std::make_shared<detail::WindowLifetime>()),
      ui_thread_(std::this_thread::get_id()),
      dispatcher_state_(std::make_shared<detail::DispatcherState>(ui_thread_)) {
    (*lifetime_).window = this;
    if (!root_) {
        throw std::invalid_argument("GUI.Forms window requires a retained root control");
    }
    if ((*root_).parent()) {
        throw std::logic_error("GUI.Forms window root may not already have a parent");
    }
    if (!(*root_).is_alive()) {
        throw std::logic_error("GUI.Forms window root may not be disposed");
    }
    attach_subtree(root_, {});
    const Rect initial_damage{0.0, 0.0, client_size_.width, client_size_.height};
    add_damage_all_planes(initial_damage);
    metrics_.record_dirty_mark(initial_damage.area());
}
Window::~Window() {
    paint_wake_handler_ = {};
    paint_wake_pending_ = false;
    abandon_deferred_input();
    shutdown_dispatcher();
    while (!accelerators_.empty()) {
        (*accelerators_.back()).disconnect();
    }
    while (!popups_.empty()) {
        (*popups_.back()).disconnect();
    }
    focus_scopes_.clear();
    for (const std::shared_ptr<gui_forms::detail::ScheduledFrameRequest>& request : frame_requests_) {
        (*request).disconnect();
    }
    frame_requests_.clear();
    update_frame_schedule_metrics();
    if (root_) {
        detach_subtree(root_);
    }
    (*lifetime_).window = nullptr;
}
void Window::resize(Size client_size) {
    require_ui_thread("window resize");
    if (client_size.width < 0.0 || client_size.height < 0.0) {
        throw std::invalid_argument("GUI.Forms client size may not be negative");
    }
    if (client_size_ == client_size) {
        return;
    }
    const Rect old_bounds{0.0, 0.0, client_size_.width, client_size_.height};
    client_size_ = client_size;
    ++surface_epoch_;
    if (surface_epoch_ == 0U) ++surface_epoch_;
    add_damage_all_planes(old_bounds);
    mark_subtree_dirty(*root_, invalidation::bounds);
    for (const std::shared_ptr<gui_forms::detail::PopupAttachment>& popup : popups_) {
        if (const Control::Ptr overlay = (*popup).popup()) {
            mark_subtree_dirty(*overlay, invalidation::bounds);
        }
    }
}

void Window::set_scale(double scale) {
    require_ui_thread("scale mutation");
    if (!std::isfinite(scale) || scale <= 0.0) {
        throw std::invalid_argument("GUI.Forms scale must be finite and positive");
    }
    if (scale_ == scale) {
        return;
    }
    scale_ = scale;
    ++surface_epoch_;
    if (surface_epoch_ == 0U) ++surface_epoch_;
    mark_subtree_dirty(*root_, invalidation::conservative_subtree);
    for (const std::shared_ptr<gui_forms::detail::PopupAttachment>& popup : popups_) {
        if (const Control::Ptr overlay = (*popup).popup()) {
            mark_subtree_dirty(*overlay, invalidation::conservative_subtree);
        }
    }
}

void Window::set_presentation_settings(PresentationSettings settings) {
    require_ui_thread("presentation settings mutation");
    if (!std::isfinite(settings.text_scale) ||
        settings.text_scale < 0.5 || settings.text_scale > 4.0) {
        throw std::invalid_argument(
            "GUI.Forms text scale must be finite and between 0.5 and 4.0");
    }
    if (presentation_settings_ == settings) {
        return;
    }
    const bool text_scale_changed =
        presentation_settings_.text_scale != settings.text_scale;
    presentation_settings_ = settings;
    // Transient overlays own geometry resolved for the opening text metrics.
    // A text-scale transition dismisses them deterministically; reopening
    // rebuilds rows, hit regions, focus scope, and screen-edge placement from
    // the new logical metrics.
    if (text_scale_changed) {
        while (!popups_.empty()) {
            (*popups_.back()).disconnect();
        }
    }
    mark_subtree_dirty(*root_, invalidation::conservative_subtree);
    for (const std::shared_ptr<gui_forms::detail::PopupAttachment>& popup : popups_) {
        if (const Control::Ptr overlay = (*popup).popup()) {
            mark_subtree_dirty(*overlay, invalidation::conservative_subtree);
        }
    }
    presentation_changed_.emit(presentation_settings_);
}

void Window::set_theme(std::shared_ptr<const Theme> theme) {
    require_ui_thread("theme mutation");
    if (!theme) throw std::invalid_argument("window theme may not be null");
    if (theme_ == theme) return;
    theme_ = std::move(theme);
    // Themes include structural spacing/geometry/type tokens as well as paint
    // recipes, so replacement must remeasure and re-hit-test inherited content.
    mark_subtree_dirty(*root_, invalidation::conservative_subtree);
    for (const std::shared_ptr<gui_forms::detail::PopupAttachment>& popup : popups_) {
        if (const Control::Ptr overlay = (*popup).popup()) {
            mark_subtree_dirty(*overlay,
                               Dirty::style | Dirty::paint | Dirty::semantics);
        }
    }
    theme_changed_.emit(*theme_);
}

void Window::set_active(bool active) {
    require_ui_thread("activation mutation");
    if (active_ == active) return;
    active_ = active;
    mark_subtree_dirty(*root_, Dirty::style | Dirty::paint | Dirty::semantics);
    for (const std::shared_ptr<gui_forms::detail::PopupAttachment>& popup : popups_) {
        if (const Control::Ptr overlay = (*popup).popup()) {
            mark_subtree_dirty(*overlay,
                               Dirty::style | Dirty::paint | Dirty::semantics);
        }
    }
    active_changed_.emit(active_);
}

void Window::set_text_scale(double text_scale) {
    PresentationSettings settings = presentation_settings_;
    settings.text_scale = text_scale;
    set_presentation_settings(settings);
}

UpdateScope Window::begin_update() {
    require_ui_thread("update scope");
    ++update_depth_;
    metrics_.enter_update_scope();
    return UpdateScope(*this);
}

void Window::perform_layout() {
    require_ui_thread("layout");
    ensure_layout(false);
}

void Window::flush() {
    require_ui_thread("flush");
    ensure_layout(false);
}
void Window::leave_update_scope() {
    require_ui_thread("update scope close");
    if (update_depth_ == 0) {
        throw std::logic_error("GUI.Forms update scope underflow");
    }
    --update_depth_;
    metrics_.leave_update_scope();
    flush_if_outermost();
}

void Window::flush_if_outermost() {
    if (update_depth_ == 0) {
        ensure_layout(false);
    }
}
} // namespace gui_forms
