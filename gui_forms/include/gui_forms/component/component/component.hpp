#pragma once

#include "gui_forms/component/revocable/revocable.hpp"
#include "gui_forms/component/types/component_types.hpp"

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
        return state_ == ComponentState::alive;
    }
    [[nodiscard]] bool is_disposed() const noexcept {
        return state_ == ComponentState::disposed;
    }

    // The component owns revocation authority, never the subscription itself.
    void own_revocable(const std::weak_ptr<detail::Revocable>& revocable);

protected:
    virtual void verify_dispose_thread();
    virtual void on_dispose() noexcept;
    void revoke_owned_work() noexcept;

private:
    ComponentState state_{ComponentState::alive};
    std::vector<std::weak_ptr<detail::Revocable>> owned_revocables_;
};

} // namespace gui_forms
