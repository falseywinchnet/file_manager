#pragma once

#include "gui_forms/component/revocable/revocable.hpp"
#include "gui_forms/component/types/component_types.hpp"
#include "gui_forms/event/subscription_token/subscription_token.hpp"

#include <memory>
#include <vector>

namespace gui_forms {

class Component {
public:
    using Ptr = std::shared_ptr<Component>;

    Component() = default;
    virtual ~Component();
    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    void dispose();
    [[nodiscard]] ComponentState component_state() const noexcept { return state_; }
    [[nodiscard]] bool is_alive() const noexcept {
        const bool alive = state_ == ComponentState::alive;
        return alive;
    }
    [[nodiscard]] bool is_disposed() const noexcept {
        const bool disposed = state_ == ComponentState::disposed;
        return disposed;
    }

    // Observes revocation authority; the caller retains its subscription token.
    void own_revocable(const std::weak_ptr<detail::Revocable>& revocable);

    // Transfers the token into this component. Disposal/destruction disconnects
    // it; no ownership of the event publisher or callback target is introduced.
    // A dead component disconnects immediately. Failure also disconnects the
    // transferred token. Like event dispatch, this is execution-thread confined.
    void own_subscription(SubscriptionToken subscription);

protected:
    virtual void verify_dispose_thread();
    virtual void on_dispose() noexcept;
    void revoke_owned_work() noexcept;

private:
    ComponentState state_{ComponentState::alive};
    std::vector<std::weak_ptr<detail::Revocable>> owned_revocables_{};
    // Components that only use caller-owned tokens do not allocate this store.
    std::unique_ptr<std::vector<SubscriptionToken>> owned_subscriptions_{};
};

} // namespace gui_forms
