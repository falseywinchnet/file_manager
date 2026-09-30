#pragma once

#include "gui_forms/component.hpp"
#include "gui_forms/delegate.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace gui_forms {

class SubscriptionToken final {
public:
    SubscriptionToken() = default;
    ~SubscriptionToken() { disconnect(); }
    SubscriptionToken(SubscriptionToken&& other) noexcept
        : revocable_(std::move(other.revocable_)) {}
    SubscriptionToken& operator=(SubscriptionToken&& other) noexcept {
        if (this != &other) {
            disconnect();
            revocable_ = std::move(other.revocable_);
        }
        return *this;
    }
    SubscriptionToken(const SubscriptionToken&) = delete;
    SubscriptionToken& operator=(const SubscriptionToken&) = delete;

    void disconnect() noexcept {
        if (revocable_) {
            (*revocable_).disconnect();
            revocable_.reset();
        }
    }
    [[nodiscard]] bool connected() const noexcept {
        return revocable_ != nullptr && (*revocable_).connected();
    }

private:
    template <typename... Arguments>
    friend class Event;
    explicit SubscriptionToken(std::shared_ptr<detail::Revocable> revocable)
        : revocable_(std::move(revocable)) {}

    std::shared_ptr<detail::Revocable> revocable_;
};

struct EventStatistics {
    std::uint64_t subscriptions_connected{};
    std::uint64_t subscriptions_disconnected{};
    std::uint64_t callbacks_emitted{};
};

// Emission fixes a registration-order boundary. Handlers added during an
// emission wait for the next emission. A handler disconnected before its turn
// is skipped. Nested emissions establish their own boundary. Slots remain in
// place until the outermost emission finishes, avoiding a heap snapshot for
// each notification. The emission retains State, not the Event object: a
// callback may destroy the event owner without invalidating dispatch.
template <typename... Arguments>
class Event final {
public:
    using Callback = std::function<void(Arguments...)>;
    using DelegateCallback = Delegate<Arguments...>;

    Event() : state_(std::make_shared<State>()) {}
    ~Event() { disconnect_all(); }
    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    [[nodiscard]] SubscriptionToken subscribe(Callback callback) {
        return subscribe_impl(nullptr, std::move(callback));
    }

    [[nodiscard]] SubscriptionToken subscribe(DelegateCallback callback) {
        return subscribe_impl(nullptr, callback);
    }

    [[nodiscard]] SubscriptionToken subscribe(Component& owner, Callback callback) {
        return subscribe_impl(&owner, std::move(callback));
    }

    [[nodiscard]] SubscriptionToken subscribe(Component& owner,
                                              DelegateCallback callback) {
        return subscribe_impl(&owner, callback);
    }

    void emit(Arguments... arguments) {
        const std::shared_ptr<State> state = state_;
        const std::size_t boundary = (*state).slots.size();
        EmissionScope emission(state);
        for (std::size_t index = 0; index < boundary; ++index) {
            // A callback can grow the slots vector or disconnect this slot.
            // Keep one slot alive without retaining an iterator into the vector.
            const std::shared_ptr<Slot> slot = (*state).slots[index];
            if (!(*slot).connected_) {
                continue;
            }
            ++(*state).statistics.callbacks_emitted;
            (*slot).invoke(arguments...);
        }
    }

    void disconnect_all() noexcept {
        const std::shared_ptr<State> state = state_;
        const std::size_t boundary = (*state).slots.size();
        EmissionScope emission(state);
        for (std::size_t index = 0; index < boundary; ++index) {
            // Releasing an owning callback can itself run application teardown.
            const std::shared_ptr<Slot> slot = (*state).slots[index];
            (*slot).disconnect();
        }
    }

    [[nodiscard]] EventStatistics statistics() const noexcept {
        return (*state_).statistics;
    }

private:
    struct State;

    struct Slot final : detail::Revocable {
        Slot(std::weak_ptr<State> event_state, Callback event_callback)
            : state(std::move(event_state)), callback(std::move(event_callback)),
              kind(CallbackKind::owning) {}

        Slot(std::weak_ptr<State> event_state, DelegateCallback event_delegate)
            : state(std::move(event_state)), delegate(event_delegate),
              kind(CallbackKind::delegate) {}

        void disconnect() noexcept override {
            if (!connected_) {
                return;
            }
            connected_ = false;
            callback = {};
            delegate = {};
            const std::shared_ptr<State> event_state = state.lock();
            if (event_state != nullptr) {
                ++(*event_state).statistics.subscriptions_disconnected;
            }
        }

        [[nodiscard]] bool connected() const noexcept override { return connected_; }

        void invoke(Arguments&... arguments) {
            if (kind == CallbackKind::delegate) {
                const DelegateCallback local_delegate = delegate;
                local_delegate(arguments...);
                return;
            }
            // The local owning callable keeps its target alive if this callback
            // disposes its owner and revokes the slot while it is executing.
            const Callback local_callback = callback;
            local_callback(arguments...);
        }

        enum class CallbackKind : std::uint8_t {
            owning,
            delegate,
        };

        std::weak_ptr<State> state;
        Callback callback;
        DelegateCallback delegate;
        CallbackKind kind{CallbackKind::owning};
        bool connected_{true};
    };

    struct State final {
        std::vector<std::shared_ptr<Slot>> slots;
        EventStatistics statistics;
        std::size_t emission_depth{};
    };

    class EmissionScope final {
    public:
        explicit EmissionScope(std::shared_ptr<State> state) noexcept
            : state_(std::move(state)) {
            ++(*state_).emission_depth;
        }
        ~EmissionScope() {
            --(*state_).emission_depth;
            compact(*state_);
        }
        EmissionScope(const EmissionScope&) = delete;
        EmissionScope& operator=(const EmissionScope&) = delete;

    private:
        std::shared_ptr<State> state_;
    };

    [[nodiscard]] SubscriptionToken subscribe_impl(Component* owner, Callback callback) {
        if (!callback) {
            return {};
        }
        const std::shared_ptr<Slot> slot =
            std::make_shared<Slot>(state_, std::move(callback));
        connect(owner, slot);
        return SubscriptionToken(slot);
    }

    [[nodiscard]] SubscriptionToken subscribe_impl(Component* owner,
                                                   DelegateCallback callback) {
        if (!callback) {
            return {};
        }
        const std::shared_ptr<Slot> slot =
            std::make_shared<Slot>(state_, callback);
        connect(owner, slot);
        return SubscriptionToken(slot);
    }

    void connect(Component* owner, const std::shared_ptr<Slot>& slot) {
        (*state_).slots.push_back(slot);
        ++(*state_).statistics.subscriptions_connected;
        if (owner != nullptr) {
            (*owner).own_revocable(slot);
        }
    }

    struct SlotDisconnected final {
        [[nodiscard]] bool operator()(
            const std::shared_ptr<Slot>& slot) const noexcept {
            return !(*slot).connected_;
        }
    };

    static void compact(State& state) noexcept {
        if (state.emission_depth != 0U) return;
        std::vector<std::shared_ptr<Slot>>& slots = state.slots;
        slots.erase(std::remove_if(slots.begin(), slots.end(),
                                   SlotDisconnected{}),
                    slots.end());
    }

    std::shared_ptr<State> state_;
};

} // namespace gui_forms
