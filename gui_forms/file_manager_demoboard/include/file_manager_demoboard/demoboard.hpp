#pragma once

#include "gui_forms/window.hpp"

#include <memory>
#include <string_view>

namespace file_manager_demoboard {

[[nodiscard]] std::unique_ptr<gui_forms::Window> make_product_window();
[[nodiscard]] std::unique_ptr<gui_forms::Window> make_controller_window(
    gui_forms::Window* product = nullptr);
// Deterministic review/capture states use the same public semantic actions as
// assistive technology and tests. They do not bypass product controls or add a
// second mutation path.
[[nodiscard]] bool apply_capture_state(gui_forms::Window& product,
                                       std::string_view state);

} // namespace file_manager_demoboard
