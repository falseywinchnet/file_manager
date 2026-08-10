#include "../../dispatcher/state/dispatcher_state.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {

namespace {
void record_inline_invoke(
    const std::shared_ptr<detail::DispatcherState>& state) {
    std::scoped_lock lock((*state).mutex);
    if (!(*state).accepting) {
        throw std::logic_error("GUI.Forms dispatcher is shut down");
    }
    ++(*state).synchronous_invocations;
    ++(*state).inline_invocations;
}

} // namespace

bool Control::invoke_required() const noexcept {
    std::shared_ptr<detail::DispatcherState> state;
    {
        std::scoped_lock lock(dispatcher_mutex_);
        state = dispatcher_state_;
    }
    return state && std::this_thread::get_id() != (*state).ui_thread;
}

DispatchOperation Control::begin_invoke(std::function<void()> callback) {
    std::shared_ptr<detail::DispatcherState> state;
    {
        std::scoped_lock lock(dispatcher_mutex_);
        state = dispatcher_state_;
    }
    return detail::post_dispatch(state, weak_from_this(), true,
                                 std::move(callback));
}

void Control::invoke(std::function<void()> callback) {
    if (!callback) {
        throw std::invalid_argument("GUI.Forms Invoke requires a callback");
    }
    std::shared_ptr<detail::DispatcherState> state;
    {
        std::scoped_lock lock(dispatcher_mutex_);
        state = dispatcher_state_;
    }
    if (!state) {
        throw std::logic_error(
            "GUI.Forms Invoke requires an attached live dispatcher");
    }
    if (std::this_thread::get_id() == (*state).ui_thread) {
        if (!is_alive()) {
            throw std::logic_error(
                "GUI.Forms Invoke requires an attached live control");
        }
        record_inline_invoke(state);
        callback();
        return;
    }
    DispatchOperation operation = detail::post_dispatch(
        state, weak_from_this(), true, std::move(callback), true);
    operation.wait_and_rethrow();
}

} // namespace gui_forms
