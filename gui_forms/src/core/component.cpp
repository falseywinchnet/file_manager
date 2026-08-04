#include "gui_forms/component.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace gui_forms {

Component::~Component() = default;

void Component::dispose() {
    if (state_ != ComponentState::alive) {
        return;
    }
    verify_dispose_thread();
    state_ = ComponentState::disposing;
    revoke_owned_work();
    on_dispose();
    state_ = ComponentState::disposed;
}

void Component::own_revocable(const std::weak_ptr<detail::Revocable>& revocable) {
    if (!is_alive()) {
        if (auto work = revocable.lock()) {
            work->disconnect();
        }
        return;
    }
    owned_revocables_.erase(
        std::remove_if(owned_revocables_.begin(), owned_revocables_.end(),
                       [](const auto& candidate) { return candidate.expired(); }),
        owned_revocables_.end());
    owned_revocables_.push_back(revocable);
}

void Component::verify_dispose_thread() {}

void Component::on_dispose() noexcept {}

void Component::revoke_owned_work() noexcept {
    auto owned = std::exchange(owned_revocables_, {});
    for (const auto& weak : owned) {
        if (auto revocable = weak.lock()) {
            revocable->disconnect();
        }
    }
}

ComponentContainer::~ComponentContainer() {
    dispose();
}

void ComponentContainer::add(Component::Ptr component) {
    if (disposed_) {
        throw std::logic_error("GUI.Forms cannot add to a disposed component container");
    }
    if (!component) {
        throw std::invalid_argument("GUI.Forms cannot own a null component");
    }
    if (contains(*component)) {
        return;
    }
    components_.push_back(std::move(component));
}

Component::Ptr ComponentContainer::remove(const Component& component) {
    const auto found = std::find_if(components_.begin(), components_.end(),
                                    [&component](const Component::Ptr& candidate) {
                                        return candidate.get() == &component;
                                    });
    if (found == components_.end()) {
        return {};
    }
    Component::Ptr removed = std::move(*found);
    components_.erase(found);
    return removed;
}

bool ComponentContainer::contains(const Component& component) const noexcept {
    return std::any_of(components_.begin(), components_.end(),
                       [&component](const Component::Ptr& candidate) {
                           return candidate.get() == &component;
                       });
}

void ComponentContainer::dispose() {
    if (disposed_) {
        return;
    }
    disposed_ = true;
    auto owned = std::exchange(components_, {});
    for (const auto& component : owned) {
        component->dispose();
    }
}

} // namespace gui_forms
