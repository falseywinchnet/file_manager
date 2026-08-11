#include "gui_forms/gui_forms.hpp"
#include "gui_forms/platform/macos_host.hpp"

#include <memory>
#include <string_view>

int main(int argc, char** argv) {
    auto root = std::make_shared<gui_forms::Panel>(
        gui_forms::StableId("installed.native.root"));
    auto label = std::make_shared<gui_forms::Label>(
        gui_forms::StableId("installed.native.label"),
        "GUI.Forms installed native application target");
    label->set_requested_bounds({24, 24, 360, 28});
    root->add_child(label);

    auto window = std::make_unique<gui_forms::Window>(root,
                                                      gui_forms::Size{520, 240});
    if (argc == 2 && std::string_view(argv[1]) == "--model-only") {
        window->perform_layout();
        return window->find("installed.native.label") == label ? 0 : 1;
    }

    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms Installed Native Consumer";
    options.initial_size = {520, 240};
    options.minimum_size = {360, 180};
    options.close_after_launch_for_testing = argc == 2 &&
        std::string_view(argv[1]) == "--close-after-launch";
    options.print_metrics_on_close = false;
    return gui_forms::host::run_macos(std::move(window), std::move(options));
}
