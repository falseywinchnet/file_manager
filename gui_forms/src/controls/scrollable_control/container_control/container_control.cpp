#include "gui_forms/controls/scrollable_control/container_control/container_control.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

namespace gui_forms {

ContainerControl::ContainerControl(StableId stable_id)
    : ScrollableControl(std::move(stable_id)) {}

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

AutoValidate ContainerControl::effective_auto_validate() const noexcept {
    for (const Control* current = this; current != nullptr;) {
        if (const auto* container = dynamic_cast<const ContainerControl*>(current);
            container != nullptr && container->auto_validate_ != AutoValidate::inherit) {
            return container->auto_validate_;
        }
        const Control::Ptr owner = current->parent();
        current = owner.get();
    }
    return AutoValidate::enable_prevent_focus_change;
}

void ContainerControl::set_auto_validate(AutoValidate value) {
    require_mutable();
    if (value != AutoValidate::inherit && value != AutoValidate::disable &&
        value != AutoValidate::enable_prevent_focus_change &&
        value != AutoValidate::enable_allow_focus_change) {
        throw std::invalid_argument("GUI.Forms AutoValidate value is invalid");
    }
    if (auto_validate_ == value) return;
    auto_validate_ = value;
    publish_change(auto_validate_changed_, value);
}

bool ContainerControl::validate(bool check_auto_validate) {
    require_mutable();
    if (window() == nullptr) return true;
    if (check_auto_validate &&
        effective_auto_validate() == AutoValidate::disable) return true;
    Control::Ptr current = active_control();
    bool accepted = true;
    while (current && current.get() != this) {
        accepted = window()->validate_control(current, this, false) && accepted;
        current = current->parent();
    }
    return accepted;
}

bool ContainerControl::validate_children(ValidationConstraints constraints) {
    require_mutable();
    return window() == nullptr
        ? true : window()->validate_children(shared_from_this(), constraints);
}

SemanticDescriptor ContainerControl::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = !descriptor.name.empty() ||
                         !descriptor.description.empty();
    return descriptor;
}

} // namespace gui_forms
