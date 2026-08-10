#include "../state/dispatcher_state.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {

namespace {

bool dispatch_terminal(DispatchOperationState state) noexcept {
    return state == DispatchOperationState::completed ||
           state == DispatchOperationState::cancelled ||
           state == DispatchOperationState::faulted;
}
} // namespace

std::uint64_t DispatchOperation::sequence() const noexcept {
    return work_ ? work_->sequence : 0U;
}

DispatchOperationState DispatchOperation::state() const noexcept {
    return work_ ? work_->state.load(std::memory_order_acquire)
                 : DispatchOperationState::invalid;
}

bool DispatchOperation::pending() const noexcept {
    return state() == DispatchOperationState::pending;
}

bool DispatchOperation::cancel() noexcept {
    if (!work_) return false;
    DispatchOperationState expected = DispatchOperationState::pending;
    const bool cancelled = work_->state.compare_exchange_strong(
        expected, DispatchOperationState::cancelled,
        std::memory_order_acq_rel, std::memory_order_acquire);
    if (cancelled) work_->completion.notify_all();
    return cancelled;
}

std::exception_ptr DispatchOperation::exception() const noexcept {
    if (!work_) return {};
    std::scoped_lock lock(work_->fault_mutex);
    return work_->fault;
}

void DispatchOperation::wait_and_rethrow() const {
    if (!work_) {
        throw std::logic_error(
            "GUI.Forms cannot wait for an invalid dispatch operation");
    }
    {
        std::unique_lock lock(work_->completion_mutex);
        work_->completion.wait(lock, [this] {
            return dispatch_terminal(
                work_->state.load(std::memory_order_acquire));
        });
    }
    const DispatchOperationState final_state = state();
    if (final_state == DispatchOperationState::completed) return;
    if (final_state == DispatchOperationState::faulted) {
        const std::exception_ptr fault = exception();
        if (fault) std::rethrow_exception(fault);
        throw std::runtime_error(
            "GUI.Forms synchronous Invoke faulted without an exception");
    }
    throw DispatchCancelledError();
}

} // namespace gui_forms
