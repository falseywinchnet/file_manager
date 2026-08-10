#include "dispatcher_state.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {
namespace detail {

DispatchOperation post_dispatch(const std::shared_ptr<DispatcherState>& state,
                                Control::WeakPtr owner,
                                bool requires_owner,
                                std::function<void()> callback,
                                bool synchronous) {
    if (!callback) {
        throw std::invalid_argument("GUI.Forms BeginInvoke requires a callback");
    }
    if (!state) {
        throw std::logic_error(
            "GUI.Forms BeginInvoke requires an attached live dispatcher");
    }

    std::function<void()> wake;
    std::shared_ptr<DispatchWork> work;
    {
        std::scoped_lock lock((*state).mutex);
        if (!(*state).accepting) {
            throw std::logic_error("GUI.Forms dispatcher is shut down");
        }
        if (synchronous && !(*state).wake) {
            throw std::logic_error(
                "GUI.Forms worker Invoke requires a running host dispatcher");
        }
        if ((*state).queue.size() >= maximum_posted_callbacks) {
            throw std::length_error("GUI.Forms posted callback limit reached");
        }
        work = std::make_shared<DispatchWork>();
        (*work).sequence = (*state).next_sequence++;
        if ((*state).next_sequence == 0U) (*state).next_sequence = 1U;
        (*work).callback = std::move(callback);
        (*work).owner = std::move(owner);
        (*work).requires_owner = requires_owner;
        (*work).synchronous = synchronous;
        (*work).dispatcher = state;
        (*state).queue.push_back(work);
        ++(*state).posted;
        if (synchronous) {
            ++(*state).synchronous_invocations;
            ++(*state).marshalled_invocations;
        }
        (*state).maximum_pending = std::max((*state).maximum_pending,
                                          (*state).queue.size());
        if ((*state).wake) {
            if (!(*state).wake_pending) {
                (*state).wake_pending = true;
                ++(*state).wake_requests;
                wake = (*state).wake;
            } else {
                ++(*state).coalesced_wakes;
            }
        }
    }
    if (wake) wake();
    return DispatchOperation(std::move(work));
}

} // namespace detail
} // namespace gui_forms
