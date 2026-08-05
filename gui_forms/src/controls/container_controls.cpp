#include "gui_forms/container_controls.hpp"

#include "gui_forms/window.hpp"

#include <utility>

namespace gui_forms {

ContainerControl::ContainerControl(StableId stable_id)
    : Control(std::move(stable_id)) {}

bool ContainerControl::contains_descendant(const Control::Ptr& control) const noexcept {
    if (!control || control.get() == this) {
        return false;
    }
    for (Control::Ptr ancestor = control->parent(); ancestor;
         ancestor = ancestor->parent()) {
        if (ancestor.get() == this) {
            return true;
        }
    }
    return false;
}

Control::Ptr ContainerControl::active_control() const noexcept {
    const Window* owner = window();
    if (owner == nullptr) {
        return {};
    }
    Control::Ptr focused = owner->focused_control();
    return contains_descendant(focused) ? focused : Control::Ptr{};
}

bool ContainerControl::request_active_control(const Control::Ptr& control) {
    require_mutable();
    if (!contains_descendant(control) || window() == nullptr) {
        return false;
    }
    return window()->request_focus(control);
}

bool ContainerControl::clear_active_control() {
    require_mutable();
    if (window() == nullptr || !active_control()) {
        return false;
    }
    return window()->request_focus({});
}

UserControl::UserControl(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {}

void UserControl::on_attached_to_window() {
    attached_ = true;
    if (!loaded_) {
        loaded_ = true;
        loaded_event_.emit();
    }
}

void UserControl::on_attachment_committed() noexcept {
    ++attachment_count_;
}

void UserControl::on_detached_from_window() noexcept {
    attached_ = false;
}

} // namespace gui_forms
