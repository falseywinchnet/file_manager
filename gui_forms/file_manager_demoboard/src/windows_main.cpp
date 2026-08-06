#include "file_manager_demoboard/demoboard.hpp"
#include "gui_forms/platform/windows_host.hpp"

#include <utility>
#include <vector>
#include <string_view>

int main(int argc, const char* argv[]) {
    auto product_model = file_manager_demoboard::make_product_window();
    for (int index = 1; index < argc; ++index) {
        constexpr std::string_view prefix = "--capture-state=";
        const std::string_view argument(argv[index]);
        if (argument.starts_with(prefix)) {
            static_cast<void>(file_manager_demoboard::apply_capture_state(
                *product_model, argument.substr(prefix.size())));
        }
    }
    auto controller_model =
        file_manager_demoboard::make_controller_window(product_model.get());
    gui_forms::host::WindowsApplicationWindow product;
    product.stable_id = "fm.window.primary";
    product.model = std::move(product_model);
    product.options.title = "File Manager — Demoboard";
    product.options.initial_size = {1450.0, 850.0};
    product.options.minimum_size = {150.0, 150.0};

    gui_forms::host::WindowsApplicationWindow controller;
    controller.stable_id = "demo.controller.window";
    controller.owner_id = product.stable_id;
    controller.model = std::move(controller_model);
    controller.options.title = "Demoboard Controller";
    controller.options.initial_size = {470.0, 700.0};
    controller.options.minimum_size = {360.0, 320.0};
    controller.tool_window = true;

    std::vector<gui_forms::host::WindowsApplicationWindow> windows;
    windows.push_back(std::move(product));
    windows.push_back(std::move(controller));
    return gui_forms::host::run_windows_application(std::move(windows));
}
