#include "fixture_links.hpp"
#include "file_manager/platform_commands.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unistd.h>

namespace {

void require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << "platform command test failed: " << message << '\n';
        std::exit(1);
    }
}

} // namespace

int main() {
    const auto root = std::filesystem::temp_directory_path() /
        ("file-manager-platform-command-" + std::to_string(::getpid()));
    std::filesystem::create_directories(root / u8"space Ω ' -- folder");
    std::ofstream(root / u8"space Ω ' -- folder" / "-- file.txt") << "safe";
    const auto directory = root / u8"space Ω ' -- folder";
    const auto file = directory / "-- file.txt";

    file_manager::PlatformCommandPlan plan;
    auto result = file_manager::make_platform_command_plan(
        file_manager::PlatformCommandKind::open_default, root, file,
        file_manager::observe_identity(file), plan);
    const auto canonical_file = std::filesystem::canonical(file);
    const auto canonical_directory = std::filesystem::canonical(directory);
#if defined(_WIN32)
    require(result.code == "ok" && !result.launched && plan.executable.empty() &&
                plan.arguments.empty() && plan.selected_path == canonical_file,
            "Windows default Open must preserve the exact path without shell input");
#else
    require(result.code == "ok" && !result.launched &&
                plan.executable == "/usr/bin/open" &&
                plan.arguments.size() == 2U &&
                plan.arguments.back() == canonical_file.string(),
            "default Open must preserve a hostile-looking path as one argv value");
#endif

    result = file_manager::make_platform_command_plan(
        file_manager::PlatformCommandKind::open_terminal_here, root, directory,
        file_manager::observe_identity(directory), plan);
#if defined(_WIN32)
    require(result.code == "ok" && plan.executable.filename() == L"cmd.exe" &&
                plan.arguments == std::vector<std::string>{"/D"} &&
                plan.selected_path == canonical_directory,
            "Windows Terminal Here must pass the directory independently of shell input");
#else
    require(result.code == "ok" && plan.arguments.size() == 4U &&
                plan.arguments[1] == "-a" &&
                plan.arguments[2] == "Terminal" &&
                plan.arguments[3] == canonical_directory.string(),
            "Terminal Here must use a fixed application and one directory argv");
#endif

    result = file_manager::make_platform_command_plan(
        file_manager::PlatformCommandKind::open_terminal_here, root, file,
        file_manager::observe_identity(file), plan);
    require(result.code == "not-directory",
            "Terminal Here must refuse a regular file");

    const auto stale = file_manager::observe_identity(file);
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

    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    std::cout << "platform command tests passed\n";
    return 0;
}
