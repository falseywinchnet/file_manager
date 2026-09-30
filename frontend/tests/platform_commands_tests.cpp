#include "fixture_links.hpp"
#include "file_manager/platform_commands.hpp"
#include "file_manager/platform_paths.hpp"

#include <cstdlib>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unistd.h>

namespace {

struct FixtureDirectory final {
    explicit FixtureDirectory(const std::filesystem::path& value) : path(value) {
        const bool created = std::filesystem::create_directory(path);
        if (!created) throw std::runtime_error("fixture directory already exists");
    }
    ~FixtureDirectory() {
        std::error_code ignored{};
        std::filesystem::remove_all(path, ignored);
    }
    FixtureDirectory(const FixtureDirectory&) = delete;
    FixtureDirectory& operator=(const FixtureDirectory&) = delete;
    std::filesystem::path path{};
};

void require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << "platform command test failed: " << message << '\n';
        throw std::runtime_error(std::string(message));
    }
}

} // namespace

int main() {
    try {
        #if defined(__linux__)
            require(file_manager::local_volume_roots() == std::vector<std::filesystem::path>{"/"},
                    "Linux navigation must expose its actual filesystem root without a synthetic Volumes folder");
        #endif
            const std::filesystem::path root = std::filesystem::temp_directory_path() /
                ("file-manager-platform-command-" + std::to_string(::getpid()));
            const FixtureDirectory fixture(root);
            std::filesystem::create_directories(root / u8"space Ω ' -- folder");
            std::ofstream(root / u8"space Ω ' -- folder" / "-- file.txt") << "safe";
            const std::filesystem::path directory = root / u8"space Ω ' -- folder";
            const std::filesystem::path file = directory / "-- file.txt";

            file_manager::PlatformCommandPlan plan{};
            file_manager::PlatformCommandResult result = file_manager::make_platform_command_plan(
                file_manager::PlatformCommandKind::open_default, root, file,
                file_manager::observe_identity(file), plan);
            const std::filesystem::path canonical_file = std::filesystem::canonical(file);
            const std::filesystem::path canonical_directory = std::filesystem::canonical(directory);
        #if defined(_WIN32)
            require(result.code == "ok" && !result.launched && plan.executable.empty() &&
                        plan.arguments.empty() && plan.selected_path == canonical_file,
                    "Windows default Open must preserve the exact path without shell input");
        #elif defined(__APPLE__)
            require(result.code == "ok" && !result.launched &&
                        plan.executable == "/usr/bin/open" &&
                        plan.arguments.size() == 2U &&
                        plan.arguments.back() == canonical_file.string(),
                    "default Open must preserve a hostile-looking path as one argv value");
        #else
            require(result.code == "ok" && !result.launched &&
                        plan.executable == "/usr/bin/xdg-open" &&
                        plan.arguments == std::vector<std::string>{"/usr/bin/xdg-open", canonical_file.string()},
                    "Linux Open must pass one absolute path to xdg-open without shell parsing");
        #endif

            result = file_manager::make_platform_command_plan(
                file_manager::PlatformCommandKind::open_terminal_here, root, directory,
                file_manager::observe_identity(directory), plan);
        #if defined(_WIN32)
            require(result.code == "ok" && plan.executable.filename() == L"cmd.exe" &&
                        plan.arguments == std::vector<std::string>{"/D"} &&
                        plan.selected_path == canonical_directory,
                    "Windows Terminal Here must pass the directory independently of shell input");
        #elif defined(__APPLE__)
            require(result.code == "ok" && plan.arguments.size() == 4U &&
                        plan.arguments[1] == "-a" &&
                        plan.arguments[2] == "Terminal" &&
                        plan.arguments[3] == canonical_directory.string(),
                    "Terminal Here must use a fixed application and one directory argv");
        #else
            require(result.code == "terminal-unavailable" && !result.launched,
                    "Linux must not claim the macOS Terminal application is available");
        #endif

            result = file_manager::make_platform_command_plan(
                file_manager::PlatformCommandKind::open_terminal_here, root, file,
                file_manager::observe_identity(file), plan);
            require(result.code == "not-directory",
                    "Terminal Here must refuse a regular file");

            const file_manager::ObjectIdentity stale = file_manager::observe_identity(file);
            std::filesystem::remove(file);
            std::ofstream(file) << "replacement";
            result = file_manager::make_platform_command_plan(
                file_manager::PlatformCommandKind::open_default, root, file, stale, plan);
            require(result.code == "selection-changed",
                    "replaced selections must fail before launch planning");

            if (create_fixture_link(directory, root / "linked-directory")) {

            result = file_manager::make_platform_command_plan(
                file_manager::PlatformCommandKind::open_terminal_here, root,
                root / "linked-directory",
                file_manager::observe_identity(root / "linked-directory"), plan);
            require(result.code == "symlink-refused",
                    "Terminal Here must never traverse a symbolic link");
            }

            std::error_code ignored{};
            std::filesystem::remove_all(root, ignored);
            std::cout << "platform command tests passed\n";
            return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
