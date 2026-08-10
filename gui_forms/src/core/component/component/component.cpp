#include "gui_forms/component/component/component.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

struct ExpiredRevocable final {
    [[nodiscard]] bool operator()(
        const std::weak_ptr<detail::Revocable>& candidate) const noexcept {
        return candidate.expired();
    }
};

} // namespace

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
        if (std::shared_ptr<gui_forms::detail::Revocable> work = revocable.lock()) {
            (*work).disconnect();
        }
        return;
    }
    owned_revocables_.erase(
        std::remove_if(owned_revocables_.begin(), owned_revocables_.end(),
                       ExpiredRevocable{}),
        owned_revocables_.end());
    owned_revocables_.push_back(revocable);
}

void Component::verify_dispose_thread() {}

void Component::on_dispose() noexcept {}

void Component::revoke_owned_work() noexcept {
    std::vector<std::weak_ptr<gui_forms::detail::Revocable>> owned = std::exchange(owned_revocables_, {});
    // Revocables form an acquisition stack, so tear them down in strict
    // reverse order and never callback into a disposing owner.
    for (std::vector<std::weak_ptr<gui_forms::detail::Revocable>>::reverse_iterator
             item = owned.rbegin();
         item != owned.rend(); ++item) {
        if (std::shared_ptr<gui_forms::detail::Revocable> revocable = (*item).lock()) {
            (*revocable).disconnect();
        }
    }
}

} // namespace gui_forms
