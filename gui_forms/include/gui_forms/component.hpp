#pragma once

#include <memory>
#include <span>
#include <vector>

namespace gui_forms {

namespace detail {

class Revocable {
public:
    virtual ~Revocable() = default;
    virtual void disconnect() noexcept = 0;
    [[nodiscard]] virtual bool connected() const noexcept = 0;
};

} // namespace detail

enum class ComponentState {
    alive,
    disposing,
    disposed,
};

class Component {
public:
    using Ptr = std::shared_ptr<Component>;

    Component() = default;
    virtual ~Component();
    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    void dispose();
    [[nodiscard]] ComponentState component_state() const noexcept { return state_; }
    [[nodiscard]] bool is_alive() const noexcept { return state_ == ComponentState::alive; }
    [[nodiscard]] bool is_disposed() const noexcept {
        return state_ == ComponentState::disposed;
    }

    // Used by tokenized events and, later, timers/queued work. The component
    // owns only revocation authority, never the subscription itself.
    void own_revocable(const std::weak_ptr<detail::Revocable>& revocable);

protected:
    virtual void verify_dispose_thread();
    virtual void on_dispose() noexcept;
    void revoke_owned_work() noexcept;

private:
    ComponentState state_{ComponentState::alive};
    std::vector<std::weak_ptr<detail::Revocable>> owned_revocables_;
};

class ComponentContainer final {
public:
    ComponentContainer() = default;
    ~ComponentContainer();
    ComponentContainer(const ComponentContainer&) = delete;
    ComponentContainer& operator=(const ComponentContainer&) = delete;

    void add(Component::Ptr component);
    [[nodiscard]] Component::Ptr remove(const Component& component);
    [[nodiscard]] std::span<const Component::Ptr> components() const noexcept {
        return components_;
    }
    [[nodiscard]] bool contains(const Component& component) const noexcept;
    void dispose();
    [[nodiscard]] bool is_disposed() const noexcept { return disposed_; }

private:
    std::vector<Component::Ptr> components_;
    bool disposed_{};
};

} // namespace gui_forms
