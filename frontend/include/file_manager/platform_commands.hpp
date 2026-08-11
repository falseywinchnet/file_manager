#pragma once

#include "file_manager/filesystem_model.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace file_manager {

enum class PlatformCommandKind : std::uint8_t {
    open_default,
    open_terminal_here,
};

struct PlatformCommandPlan final {
    PlatformCommandKind kind{PlatformCommandKind::open_default};
    std::filesystem::path protected_root;
    std::filesystem::path selected_path;
    ObjectIdentity expected_identity;
    std::filesystem::path executable;
    std::vector<std::string> arguments;
};

struct PlatformCommandResult final {
    bool launched{};
    std::string code;
    std::string message;
    int exit_status{};
};

// Produces an argv-only macOS launch plan. No shell command, interpolation,
// environment injection, elevation, or repository command is admitted.
[[nodiscard]] PlatformCommandResult make_platform_command_plan(
    PlatformCommandKind kind,
    const std::filesystem::path& protected_root,
    const std::filesystem::path& selected_path,
    const ObjectIdentity& expected_identity,
    PlatformCommandPlan& plan);

// Revalidates the exact path revision immediately before spawning /usr/bin/open.
[[nodiscard]] PlatformCommandResult execute_platform_command(
    const PlatformCommandPlan& plan);

} // namespace file_manager
