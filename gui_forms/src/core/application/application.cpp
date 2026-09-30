#include "application_state.hpp"
#include "gui_forms/text/types/text_types.hpp"

#include <cmath>
#include <utility>

namespace gui_forms {
namespace {
bool valid_text(std::string_view value, std::size_t limit, bool allow_empty) {
    return (allow_empty || !value.empty()) && value.size() <= limit &&
        value.find('\0') == std::string_view::npos && validate_utf8(value).valid();
}
bool valid_size(Size size) noexcept {
    return std::isfinite(size.width) && std::isfinite(size.height) &&
        size.width >= 1.0 && size.height >= 1.0 &&
        size.width <= 32768.0 && size.height <= 32768.0;
}
HostServiceStatus request(const std::weak_ptr<detail::ApplicationWindowState>& weak,
                          std::function<void()> detail::ApplicationWindowState::* operation,
                          bool hiding) {
    const std::shared_ptr<detail::ApplicationWindowState> state = weak.lock();
    if (!state) return {HostServiceError::after_shutdown};
    if ((*state).thread != std::this_thread::get_id()) return {HostServiceError::wrong_thread};
    if ((*state).closed) return {HostServiceError::after_shutdown};
    if (!(*state).ready) return {HostServiceError::backend_failure};
    if (hiding && !(*state).can_hide) return {HostServiceError::invalid_argument};
    if (!((*state).*operation)) return {HostServiceError::unsupported};
    try {
        // A native request may synchronously close and clear its state. Keep
        // the active callable alive until its invocation has returned.
        const std::function<void()> action = (*state).*operation;
        action();
    } catch (...) {
        return {HostServiceError::backend_failure};
    }
    return {};
}
}

ApplicationWindowHandle::ApplicationWindowHandle(
    std::weak_ptr<detail::ApplicationWindowState> state) noexcept : state_(std::move(state)) {}

bool ApplicationWindowHandle::active() const noexcept {
    const std::shared_ptr<detail::ApplicationWindowState> state = state_.lock();
    return state && (*state).thread == std::this_thread::get_id() && (*state).ready && !(*state).closed;
}
HostServiceStatus ApplicationWindowHandle::request_close() const {
    return request(state_, &detail::ApplicationWindowState::request_close, false);
}
HostServiceStatus ApplicationWindowHandle::show() const {
    return request(state_, &detail::ApplicationWindowState::show, false);
}
HostServiceStatus ApplicationWindowHandle::hide() const {
    return request(state_, &detail::ApplicationWindowState::hide, true);
}

HostServiceStatus ApplicationWindowHandle::toggle_full_screen() const {
    return request(state_, &detail::ApplicationWindowState::toggle_full_screen, false);
}

ApplicationResult Application::validate(const std::vector<ApplicationWindow>& windows) {
    if (windows.empty() || windows.size() > maximum_windows) {
        return {ApplicationError::invalid_argument};
    }
    std::size_t primary_count = 0U;
    for (std::size_t index = 0U; index < windows.size(); ++index) {
        const ApplicationWindow& entry = windows[index];
        ApplicationResult invalid{ApplicationError::invalid_argument};
        invalid.window_index = index;
        if (!entry.model || !valid_text(entry.stable_id, 1024U, false) ||
            !valid_text(entry.owner_id, 1024U, true) ||
            !valid_text(entry.options.title, 65536U, true) ||
            !valid_size(entry.options.initial_size) || !valid_size(entry.options.minimum_size) ||
            entry.options.minimum_size.width > entry.options.initial_size.width ||
            entry.options.minimum_size.height > entry.options.initial_size.height ||
            entry.owner_id == entry.stable_id) return invalid;
        if (!(*entry.model).check_access()) {
            invalid.error = ApplicationError::wrong_thread;
            return invalid;
        }
        if (entry.owner_id.empty() && !entry.tool_window) {
            ++primary_count;
            if (!entry.options.initially_visible || entry.options.hide_on_close) return invalid;
        }
        for (std::size_t other = 0U; other < index; ++other) {
            if (windows[other].stable_id == entry.stable_id) return invalid;
        }
        std::string_view owner = entry.owner_id;
        for (std::size_t depth = 0U; !owner.empty(); ++depth) {
            if (depth >= windows.size()) return invalid;
            std::size_t parent = 0U;
            while (parent < windows.size() && windows[parent].stable_id != owner) ++parent;
            if (parent == windows.size()) return invalid;
            owner = windows[parent].owner_id;
        }
    }
    if (primary_count != 1U) return {ApplicationError::invalid_argument};
    return {};
}
} // namespace gui_forms
