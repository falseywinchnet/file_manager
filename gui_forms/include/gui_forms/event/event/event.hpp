#pragma once

#include "gui_forms/component.hpp"

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
            revocable_->disconnect();
            revocable_.reset();
        }
    }
    [[nodiscard]] bool connected() const noexcept {
        return revocable_ != nullptr && revocable_->connected();
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

// Emission uses a registration-order snapshot. Handlers added during an
// emission wait for the next emission. A handler disconnected before its turn
// is skipped. Each snapshot member can therefore run at most once.
template <typename... Arguments>
class Event final {
public:
    using Callback = std::function<void(Arguments...)>;

    Event() : state_(std::make_shared<State>()) {}
    ~Event() { disconnect_all(); }
    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    [[nodiscard]] SubscriptionToken subscribe(Callback callback) {
        return subscribe_impl(nullptr, std::move(callback));
    }

    [[nodiscard]] SubscriptionToken subscribe(Component& owner, Callback callback) {
        return subscribe_impl(&owner, std::move(callback));
    }

    void emit(Arguments... arguments) {
        const auto snapshot = state_->slots;
        for (const auto& slot : snapshot) {
            if (!slot->connected_) {
                continue;
            }
            ++state_->statistics.callbacks_emitted;
            // The local callable keeps its target alive if this callback
            // disposes its owner and revokes the slot while it is executing.
            Callback callback = slot->callback;
            callback(arguments...);
        }
        compact();
    }

    void disconnect_all() noexcept {
        for (const auto& slot : state_->slots) {
            slot->disconnect();
        }
        compact();
    }

    [[nodiscard]] EventStatistics statistics() const noexcept {
        return state_->statistics;
    }

private:
    struct State;

    struct Slot final : detail::Revocable {
        Slot(std::weak_ptr<State> event_state, Callback event_callback)
            : state(std::move(event_state)), callback(std::move(event_callback)) {}

        void disconnect() noexcept override {
            if (!connected_) {
                return;
            }
            connected_ = false;
            callback = {};
            if (const auto event_state = state.lock()) {
                ++event_state->statistics.subscriptions_disconnected;
            }
        }

        [[nodiscard]] bool connected() const noexcept override { return connected_; }

        std::weak_ptr<State> state;
        Callback callback;
        bool connected_{true};
    };

    struct State final {
        std::vector<std::shared_ptr<Slot>> slots;
        EventStatistics statistics;
    };

    [[nodiscard]] SubscriptionToken subscribe_impl(Component* owner, Callback callback) {
        if (!callback) {
            return {};
        }
        auto slot = std::make_shared<Slot>(state_, std::move(callback));
        state_->slots.push_back(slot);
        ++state_->statistics.subscriptions_connected;
        if (owner != nullptr) {
            owner->own_revocable(slot);
        }
        return SubscriptionToken(std::move(slot));
    }

    void compact() noexcept {
        state_->slots.erase(
            std::remove_if(state_->slots.begin(), state_->slots.end(),
                           [](const auto& slot) { return !slot->connected_; }),
            state_->slots.end());
    }

    std::shared_ptr<State> state_;
};

} // namespace gui_forms
