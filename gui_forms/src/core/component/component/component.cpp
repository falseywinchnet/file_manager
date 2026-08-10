#include "gui_forms/component/component/component.hpp"

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

void Component::own_revocable(
    const std::weak_ptr<detail::Revocable>& revocable) {
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
    // Revocables form an acquisition stack, so tear them down in strict
    // reverse order and never callback into a disposing owner.
    for (auto item = owned.rbegin(); item != owned.rend(); ++item) {
        if (auto revocable = item->lock()) {
            revocable->disconnect();
        }
    }
}

} // namespace gui_forms
