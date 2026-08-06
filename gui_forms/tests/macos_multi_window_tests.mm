#import <AppKit/AppKit.h>

#include "gui_forms/gui_forms.hpp"
#include "gui_forms/platform/macos_host.hpp"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

std::unique_ptr<gui_forms::Window> make_window(std::string stable_id) {
    auto root = std::make_shared<gui_forms::Panel>(
        gui_forms::StableId(std::move(stable_id)));
    root->set_requested_bounds({0.0, 0.0, 240.0, 140.0});
    return std::make_unique<gui_forms::Window>(root,
                                               gui_forms::Size{240.0, 140.0});
}

} // namespace

int main() {
    using gui_forms::host::MacApplicationWindow;

    if (gui_forms::host::run_macos_application({}) != 2) return 1;

    std::uint64_t product_closed{};
    std::uint64_t controller_closed{};
    std::string product_snapshot;
    std::string controller_snapshot;
    std::function<void()> close_product;

    MacApplicationWindow product;
    product.stable_id = "test.window.product";
    product.model = make_window("test.root.product");
    product.options.title = "GUI.Forms Product Root";
    product.options.initial_size = {240.0, 140.0};
    product.options.minimum_size = {160.0, 100.0};
    product.options.closed = [&] { ++product_closed; };
    product.options.host_ready = [&](auto, auto request_close, auto, auto,
                                     auto, auto, auto) {
        close_product = std::move(request_close);
    };
    product.options.final_snapshot = [&](std::string_view metrics,
                                         std::string_view host) {
        product_snapshot = std::string(metrics) + std::string(host);
    };

    MacApplicationWindow controller;
    controller.stable_id = "test.window.controller";
    controller.owner_id = product.stable_id;
    controller.model = make_window("test.root.controller");
    controller.options.title = "GUI.Forms Tool Root";
    controller.options.initial_size = {220.0, 130.0};
    controller.options.minimum_size = {160.0, 100.0};
    controller.options.closed = [&] { ++controller_closed; };
    controller.options.host_ready = [&](auto, auto request_close, auto, auto,
                                        auto, auto, auto) {
        const auto primary_close = close_product;
        request_close();
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_MSEC),
                       dispatch_get_main_queue(), ^{
                           if (primary_close) primary_close();
                       });
    };
    controller.options.final_snapshot = [&](std::string_view metrics,
                                            std::string_view host) {
        controller_snapshot = std::string(metrics) + std::string(host);
    };
    controller.tool_window = true;

    std::vector<MacApplicationWindow> windows;
    windows.push_back(std::move(product));
    windows.push_back(std::move(controller));
    const int result = gui_forms::host::run_macos_application(std::move(windows));
    if (result != 0 || product_closed != 1U || controller_closed != 1U ||
        product_snapshot.empty() || controller_snapshot.empty()) {
        return 2;
    }
    return 0;
}
