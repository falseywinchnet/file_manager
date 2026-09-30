#pragma once
#include "gui_forms/application.hpp"
#include <thread>

namespace gui_forms::detail {
struct ApplicationWindowState final {
    std::thread::id thread{std::this_thread::get_id()};
    bool ready{};
    bool closed{};
    bool can_hide{};
    std::function<void()> request_close;
    std::function<void()> show;
    std::function<void()> hide;
    std::function<void()> toggle_full_screen;
};
struct ApplicationHandleAccess final {
    [[nodiscard]] static ApplicationWindowHandle make(
        const std::shared_ptr<ApplicationWindowState>& state) noexcept {
        return ApplicationWindowHandle(state);
    }
};
} // namespace gui_forms::detail
