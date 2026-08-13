#include "application.hpp"
#include "about_view.hpp"
#include "file_manager/document_picker_view.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace {

std::filesystem::path default_root() {
    if (const char* configured = std::getenv("FILE_MANAGER_ROOT")) {
        return configured;
    }
    if (const char* home = std::getenv("HOME")) {
        return home;
    }
    return std::filesystem::current_path();
}

} // namespace

int main(const int argc, char** argv) {
    try {
        auto root = default_root();
        std::optional<std::filesystem::path> quarantine;
        std::string engine_root_id;
        bool allow_mutations = false;
        for (int index = 1; index < argc; ++index) {
            const std::string_view argument(argv[index]);
            if (argument == "--root" && index + 1 < argc) {
                root = argv[++index];
            } else if (argument == "--quarantine" && index + 1 < argc) {
                quarantine = argv[++index];
            } else if (argument == "--allow-mutations") {
                allow_mutations = true;
            } else if (argument == "--engine-root-id" && index + 1 < argc) {
                engine_root_id = argv[++index];
            } else {
                std::cerr << "usage: File Manager [--root DIRECTORY] "
                             "[--engine-root-id ID] "
                             "[--allow-mutations --quarantine DIRECTORY]\n";
                return 2;
            }
        }
        if (quarantine && !allow_mutations) {
            std::cerr << "File Manager: --quarantine requires --allow-mutations\n";
            return 2;
        }

        auto application = std::make_shared<file_manager::Application>(
            root, quarantine, allow_mutations, std::move(engine_root_id));

        file_manager::DocumentPickerRequest picker_request;
        picker_request.profile = file_manager::DocumentPickerProfile::open_file;
        picker_request.protected_root = root;
        picker_request.initial_location = root;
        picker_request.owner_application_id = "file-manager";
        picker_request.maximum_selection = 1;
        auto picker = std::make_shared<file_manager::DocumentPickerView>(
            std::move(picker_request));
        auto about = std::make_shared<file_manager::AboutView>();

        auto show_picker = std::make_shared<std::function<void()>>();
        auto hide_picker = std::make_shared<std::function<void()>>();
        auto show_about = std::make_shared<std::function<void()>>();
        auto hide_about = std::make_shared<std::function<void()>>();
        application->bind_secondary_surfaces(
            [picker, show_picker](const std::filesystem::path& location) {
                picker->present(location);
                if (*show_picker) (*show_picker)();
            },
            [show_about] {
                if (*show_about) (*show_about)();
            });
        about->bind_hide([hide_about] {
            if (*hide_about) (*hide_about)();
        });
        const gui_forms::SubscriptionToken picker_completed =
            picker->completed().subscribe(
                [application, hide_picker](
                    const file_manager::DocumentPickerResult& result) {
                    application->document_picker_completed(result);
                    if (result.terminal ==
                            file_manager::DocumentPickerTerminal::accepted ||
                        result.terminal ==
                            file_manager::DocumentPickerTerminal::cancelled) {
                        if (*hide_picker) (*hide_picker)();
                    }
                });

        gui_forms::host::MacApplicationWindow primary;
        primary.stable_id = "file-manager.window";
        primary.model = application->make_window();
        primary.options.title = "File Manager";
        primary.options.initial_size = {1340, 850};
        primary.options.minimum_size = {150, 150};
        primary.options.titlebar_presentation =
            gui_forms::host::MacTitlebarPresentation::
                transparent_full_size_content;
        for (const std::string_view stable_id :
             web_forms_generated_file_manager_sapphire::NativeForm::
                 window_drag_region_ids) {
            primary.options.window_drag_region_ids.emplace_back(stable_id);
        }
        primary.options.print_metrics_on_close = true;
        primary.options.host_ready = [application](std::function<void()> wake,
                                           std::function<void()> request_close,
                                           auto, auto, auto, auto, auto) {
            application->bind_host(std::move(wake), std::move(request_close));
        };
        primary.options.dispatch_pending =
            [application] { application->drain_ui(); };
        primary.options.closed = [application] { application->stop(); };

        gui_forms::host::MacApplicationWindow picker_window;
        picker_window.stable_id = "file-manager.open-picker";
        picker_window.owner_id = primary.stable_id;
        picker_window.model = std::make_unique<gui_forms::Window>(
            picker->root_control(), gui_forms::Size{760, 560});
        picker_window.options.title = "Open — File Manager";
        picker_window.options.initial_size = {760, 560};
        picker_window.options.minimum_size = {620, 460};
        picker_window.options.titlebar_presentation =
            gui_forms::host::MacTitlebarPresentation::
                transparent_full_size_content;
        picker_window.options.window_drag_region_id =
            "file-manager.picker.title";
        picker_window.options.initially_visible = false;
        picker_window.options.hide_on_close = true;
        picker_window.options.minimizable = false;
        picker_window.options.print_metrics_on_close = false;
        picker_window.options.visibility_ready =
            [show_picker, hide_picker](std::function<void()> show,
                                       std::function<void()> hide) {
                *show_picker = std::move(show);
                *hide_picker = std::move(hide);
            };
        picker_window.tool_window = false;

        gui_forms::host::MacApplicationWindow about_window;
        about_window.stable_id = "file-manager.about-window";
        about_window.owner_id = primary.stable_id;
        about_window.model = std::make_unique<gui_forms::Window>(
            about->root_control(), gui_forms::Size{540, 330});
        about_window.options.title = "About File Manager";
        about_window.options.initial_size = {540, 330};
        about_window.options.minimum_size = {500, 300};
        about_window.options.titlebar_presentation =
            gui_forms::host::MacTitlebarPresentation::
                transparent_full_size_content;
        about_window.options.window_drag_region_id =
            "file-manager.about.title";
        about_window.options.initially_visible = false;
        about_window.options.hide_on_close = true;
        about_window.options.minimizable = false;
        about_window.options.print_metrics_on_close = false;
        about_window.options.visibility_ready =
            [show_about, hide_about](std::function<void()> show,
                                     std::function<void()> hide) {
                *show_about = std::move(show);
                *hide_about = std::move(hide);
            };
        about_window.tool_window = false;

        std::vector<gui_forms::host::MacApplicationWindow> windows;
        windows.push_back(std::move(primary));
        windows.push_back(std::move(picker_window));
        windows.push_back(std::move(about_window));
        const int result =
            gui_forms::host::run_macos_application(std::move(windows));
        static_cast<void>(picker_completed);
        return result;
    } catch (const std::exception& error) {
        std::cerr << "File Manager: " << error.what() << '\n';
        return 1;
    }
}
