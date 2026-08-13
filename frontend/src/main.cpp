#include "application.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>

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
        gui_forms::host::MacHostOptions options;
        options.title = "File Manager";
        options.initial_size = {1340, 850};
        options.minimum_size = {150, 150};
        options.titlebar_presentation =
            gui_forms::host::MacTitlebarPresentation::
                transparent_full_size_content;
        options.window_drag_region_id = "file-manager-app.shell.title";
        options.print_metrics_on_close = true;
        options.host_ready = [application](std::function<void()> wake,
                                           std::function<void()> request_close,
                                           auto, auto, auto, auto, auto) {
            application->bind_host(std::move(wake), std::move(request_close));
        };
        options.dispatch_pending = [application] { application->drain_ui(); };
        options.closed = [application] { application->stop(); };
        return gui_forms::host::run_macos(application->make_window(),
                                           std::move(options));
    } catch (const std::exception& error) {
        std::cerr << "File Manager: " << error.what() << '\n';
        return 1;
    }
}
