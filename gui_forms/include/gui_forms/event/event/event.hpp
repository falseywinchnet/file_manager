#pragma once

#include "gui_forms/component.hpp"
#include "gui_forms/delegate.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace gui_forms {

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
        SubscriptionToken token = subscribe_impl(nullptr, std::move(callback));
        return token;
    }

    [[nodiscard]] SubscriptionToken subscribe(DelegateCallback callback) {
        SubscriptionToken token = subscribe_impl(nullptr, callback);
        return token;
    }

    [[nodiscard]] SubscriptionToken subscribe(Component& owner, Callback callback) {
        SubscriptionToken token = subscribe_impl(&owner, std::move(callback));
        return token;
    }

    [[nodiscard]] SubscriptionToken subscribe(Component& owner,
                                              DelegateCallback callback) {
        SubscriptionToken token = subscribe_impl(&owner, callback);
        return token;
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
    template <typename Owner, typename Target, typename... EventArguments>
    friend void on(Event<EventArguments...>& event, Owner& owner,
                   void (Target::*method)(EventArguments...));
    template <typename Owner, typename Target, typename... EventArguments>
    friend void on(Event<EventArguments...>& event, Owner& owner,
                   void (Target::*method)(EventArguments...) const);

    struct State;

    struct Slot : detail::Revocable {
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

        virtual void invoke(Arguments&... arguments) {
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

        std::weak_ptr<State> state{};
        Callback callback{};
        DelegateCallback delegate{};
        CallbackKind kind{CallbackKind::owning};
        bool connected_{};
    };

    // The event retains this named member-binding state in the same allocation
    // as its slot. emit() holds the slot across revocation, so invoking the
    // member neither copies an owning callable nor allocates. The owner is
    // borrowed: registration never extends its lifetime or forms a cycle.
    template <typename Owner, typename Method>
    struct MemberSlot final : Slot {
        MemberSlot(const std::shared_ptr<State>& event_state, Owner& owner,
                   const Method method)
            : Slot(event_state, DelegateCallback{}), owner_(owner),
              method_(method) {}

        void invoke(Arguments&... arguments) override {
            (owner_.*method_)(arguments...);
        }

        Owner& owner_;
        Method method_;
    };

    struct State final {
        std::vector<std::shared_ptr<Slot>> slots{};
        EventStatistics statistics{};
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
        SubscriptionToken token(slot);
        connect(owner, slot);
        return token;
    }

    [[nodiscard]] SubscriptionToken subscribe_impl(Component* owner,
                                                   DelegateCallback callback) {
        if (!callback) {
            return {};
        }
        const std::shared_ptr<Slot> slot =
            std::make_shared<Slot>(state_, callback);
        SubscriptionToken token(slot);
        connect(owner, slot);
        return token;
    }

    template <typename Owner, typename Method>
    [[nodiscard]] SubscriptionToken subscribe_member(Owner& owner,
                                                      const Method method) {
        const std::shared_ptr<Slot> slot =
            std::make_shared<MemberSlot<Owner, Method>>(state_, owner, method);
        SubscriptionToken token(slot);
        connect(nullptr, slot);
        return token;
    }

    void connect(Component* owner, const std::shared_ptr<Slot>& slot) {
        compact(*state_);
        (*state_).slots.push_back(slot);
        (*slot).connected_ = true;
        ++(*state_).statistics.subscriptions_connected;
        if (owner != nullptr) {
            (*owner).own_revocable(slot);
        }
    }

    struct SlotDisconnected final {
        [[nodiscard]] bool operator()(
            const std::shared_ptr<Slot>& slot) const noexcept {
            const bool disconnected = !(*slot).connected_;
            return disconnected;
        }
    };

    static void compact(State& state) noexcept {
        if (state.emission_depth != 0U) return;
        std::vector<std::shared_ptr<Slot>>& slots = state.slots;
        const typename std::vector<std::shared_ptr<Slot>>::iterator retained_end =
            std::remove_if(slots.begin(), slots.end(), SlotDisconnected{});
        slots.erase(retained_end, slots.end());
    }

    std::shared_ptr<State> state_;
};

// Owner-held named member subscription. Owner must explicitly participate in
// Component lifecycle; Target may be Owner or one of its bases. A disposed
// owner is a no-op; a null member is rejected even for a disposed owner.
// Registration may allocate/throw. Emission adds no allocation or idle work.
// Owner and publisher obey the existing Event execution-thread contract.
template <typename Owner, typename Target, typename... Arguments>
void on(Event<Arguments...>& event, Owner& owner,
        void (Target::*method)(Arguments...)) {
    static_assert(std::is_base_of_v<Component, Owner>,
                  "gui_forms::on requires a Component owner");
    static_assert(std::is_base_of_v<Target, Owner>,
                  "the handler must belong to the owner or one of its bases");
    if (method == nullptr) {
        throw std::invalid_argument("gui_forms::on requires a non-null member");
    }
    Component& component = owner;
    if (!component.is_alive()) {
        return;
    }
    SubscriptionToken token = event.subscribe_member(owner, method);
    component.own_subscription(std::move(token));
}

template <typename Owner, typename Target, typename... Arguments>
void on(Event<Arguments...>& event, Owner& owner,
        void (Target::*method)(Arguments...) const) {
    static_assert(std::is_base_of_v<Component, Owner>,
                  "gui_forms::on requires a Component owner");
    static_assert(std::is_base_of_v<Target, Owner>,
                  "the handler must belong to the owner or one of its bases");
    if (method == nullptr) {
        throw std::invalid_argument("gui_forms::on requires a non-null member");
    }
    Component& component = owner;
    if (!component.is_alive()) {
        return;
    }
    SubscriptionToken token = event.subscribe_member(owner, method);
    component.own_subscription(std::move(token));
}

} // namespace gui_forms
