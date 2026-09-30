#include "file_manager/platform_commands.hpp"

#include <cerrno>
#include <cstring>
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#else
#include <spawn.h>
#include <sys/wait.h>

extern char** environ;
#endif

namespace file_manager {
namespace {

PlatformCommandResult refuse(std::string code, std::string message) {
    const PlatformCommandResult result{false, std::move(code), std::move(message), 0};
    return result;
}

PlatformCommandResult validate(const PlatformCommandKind kind,
                               const std::filesystem::path& root,
                               const std::filesystem::path& path,
                               const ObjectIdentity& expected,
                               std::filesystem::path& canonical_root,
                               std::filesystem::path& lexical_path) {
    try {
        canonical_root = canonical_existing_directory(root);
    } catch (const std::exception& error) {
        const PlatformCommandResult refused = refuse("root-unavailable", error.what());
        return refused;
    }
    std::error_code absolute_error{};
    const std::filesystem::path supplied_root = std::filesystem::absolute(
        root, absolute_error).lexically_normal();
    const std::filesystem::path supplied_path = (path.is_absolute() ? path : supplied_root / path)
        .lexically_normal();
    lexical_path = (!absolute_error &&
                    path_is_within(supplied_root, supplied_path))
        ? (canonical_root / supplied_path.lexically_relative(supplied_root))
              .lexically_normal()
        : supplied_path;
    if (lexical_path.filename().empty() && lexical_path != lexical_path.root_path()) {
        lexical_path = lexical_path.parent_path();
    }
    if (!path_is_within(canonical_root, lexical_path)) {
        const PlatformCommandResult refused = refuse("outside-protected-root",
                      "launch path is outside the protected root");
        return refused;
    }
    if (path_route_has_symlink(canonical_root, lexical_path)) {
        const PlatformCommandResult refused = refuse("symlink-refused",
                      "native launch never follows a symbolic-link route");
        return refused;
    }
    const ObjectIdentity current = observe_identity(lexical_path);
    if (!current.available()) {
        const PlatformCommandResult refused = refuse("object-unavailable", "selected object is unavailable");
        return refused;
    }
    if (expected.available() && !expected.same_revision(current)) {
        const PlatformCommandResult refused = refuse("selection-changed",
                      "selected object changed before native launch");
        return refused;
    }
    if (kind == PlatformCommandKind::open_terminal_here &&
        current.type != std::filesystem::file_type::directory) {
        const PlatformCommandResult refused = refuse("not-directory", "Terminal Here requires a directory");
        return refused;
    }
    if (kind == PlatformCommandKind::open_default &&
        current.type != std::filesystem::file_type::regular &&
        current.type != std::filesystem::file_type::directory) {
        const PlatformCommandResult refused = refuse("unsupported-object",
                      "default Open accepts a regular file or directory");
        return refused;
    }
    const PlatformCommandResult result{false, "ok", "native launch plan validated", 0};
    return result;
}

} // namespace

PlatformCommandResult make_platform_command_plan(
    const PlatformCommandKind kind,
    const std::filesystem::path& protected_root,
    const std::filesystem::path& selected_path,
    const ObjectIdentity& expected_identity,
    PlatformCommandPlan& plan) {
    std::filesystem::path canonical_root{};
    std::filesystem::path path{};
    const PlatformCommandResult checked = validate(kind, protected_root, selected_path,
                            expected_identity, canonical_root, path);
    if (checked.code != "ok") return checked;

    plan = {};
    plan.kind = kind;
    plan.protected_root = std::move(canonical_root);
    plan.selected_path = std::move(path);
    plan.expected_identity = expected_identity;
#if defined(_WIN32)
    if (kind == PlatformCommandKind::open_terminal_here) {
        wchar_t system_directory[MAX_PATH]{};
        const UINT length = GetSystemDirectoryW(system_directory, MAX_PATH);
        if (length == 0 || length >= MAX_PATH) {
            const PlatformCommandResult refused = refuse("terminal-unavailable", "Windows system directory is unavailable");
        return refused;
        }
        const std::filesystem::path terminal =
            std::filesystem::path(system_directory) / L"cmd.exe";
        plan.executable = terminal;
        plan.arguments = {"/D"};
    } else {
        plan.executable.clear();
        plan.arguments.clear();
    }
#elif defined(__APPLE__)
    plan.executable = "/usr/bin/open";
    if (kind == PlatformCommandKind::open_terminal_here) {
        plan.arguments = {"/usr/bin/open", "-a", "Terminal",
                          plan.selected_path.string()};
    } else {
        plan.arguments = {"/usr/bin/open", plan.selected_path.string()};
    }
#else
    if (kind == PlatformCommandKind::open_terminal_here) {
        const PlatformCommandResult refused = refuse("terminal-unavailable", "A Linux terminal launcher has not been configured");
        return refused;
    }
    plan.executable = "/usr/bin/xdg-open";
    plan.arguments = {"/usr/bin/xdg-open", plan.selected_path.string()};
#endif
    return checked;
}

PlatformCommandResult execute_platform_command(
    const PlatformCommandPlan& plan) {
    std::filesystem::path canonical_root{};
    std::filesystem::path path{};
    const PlatformCommandResult checked = validate(plan.kind, plan.protected_root, plan.selected_path,
                            plan.expected_identity, canonical_root, path);
    if (checked.code != "ok") return checked;
#if defined(_WIN32)
    PlatformCommandPlan expected_plan{};
    const PlatformCommandResult planned = make_platform_command_plan(
        plan.kind, plan.protected_root, plan.selected_path,
        plan.expected_identity, expected_plan);
    if (planned.code != "ok") return planned;
    if (plan.executable != expected_plan.executable ||
        plan.arguments != expected_plan.arguments) {
        const PlatformCommandResult refused = refuse("invalid-plan", "native launch plan failed closed");
        return refused;
    }
    if (plan.kind == PlatformCommandKind::open_terminal_here) {
        STARTUPINFOW startup{};
        startup.cb = static_cast<DWORD>(sizeof(startup));
        PROCESS_INFORMATION process{};
        // The selected path is supplied only as lpCurrentDirectory. /D disables
        // cmd AutoRun; no user path or command is inserted into shell input.
        std::wstring command = L"\"" + plan.executable.wstring() + L"\" /D";
        const BOOL created = CreateProcessW(plan.executable.c_str(), command.data(),
            nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, path.c_str(),
            &startup, &process);
        if (!created) {
            const PlatformCommandResult refused = refuse("spawn-failed", "Windows terminal launch failed: " +
                          std::to_string(GetLastError()));
        return refused;
        }
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        const PlatformCommandResult result{true, "ok", "Command Prompt opened at the selected directory", 0};
    return result;
    }
    const HRESULT apartment = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED |
                                                      COINIT_DISABLE_OLE1DDE);
    if (FAILED(apartment) && apartment != RPC_E_CHANGED_MODE) {
        const PlatformCommandResult refused = refuse("launcher-unavailable", "Windows shell initialization failed");
        return refused;
    }
    SHELLEXECUTEINFOW launch{};
    launch.cbSize = static_cast<DWORD>(sizeof(launch));
    launch.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC;
    launch.lpVerb = L"open";
    launch.lpFile = path.c_str();
    launch.nShow = SW_SHOWNORMAL;
    const BOOL launched = ShellExecuteExW(&launch);
    const DWORD launch_error = launched ? ERROR_SUCCESS : GetLastError();
    if (SUCCEEDED(apartment)) CoUninitialize();
    if (!launched) {
        const PlatformCommandResult refused = refuse("launcher-rejected", "Windows default Open failed: " +
                      std::to_string(launch_error));
        return refused;
    }
    const PlatformCommandResult result{true, "ok", "Object handed to its default Windows application", 0};
    return result;
#else
    PlatformCommandPlan expected_plan{};
    const PlatformCommandResult planned = make_platform_command_plan(
        plan.kind, plan.protected_root, plan.selected_path,
        plan.expected_identity, expected_plan);
    if (planned.code != "ok") return planned;
    if (plan.executable != expected_plan.executable ||
        plan.arguments != expected_plan.arguments) {
        const PlatformCommandResult refused = refuse("invalid-plan", "native launch plan failed closed");
        return refused;
    }

    std::vector<char*> argv{};
    argv.reserve(plan.arguments.size() + 1U);
    for (const std::string& argument : plan.arguments) {
        argv.push_back(const_cast<char*>(argument.c_str()));
    }
    argv.push_back(nullptr);
    pid_t child{};
    const int spawn_error = ::posix_spawn(
        &child, plan.executable.c_str(), nullptr, nullptr, argv.data(), environ);
    if (spawn_error != 0) {
        const PlatformCommandResult refused = refuse("spawn-failed",
                      std::string("native launcher failed: ") +
                          std::strerror(spawn_error));
        return refused;
    }
    int status{};
    while (::waitpid(child, &status, 0) < 0) {
        if (errno == EINTR) continue;
        const PlatformCommandResult refused = refuse("wait-failed",
                      std::string("native launcher wait failed: ") +
                          std::strerror(errno));
        return refused;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        PlatformCommandResult result = refuse(
            "launcher-rejected", "The desktop launcher rejected the requested object");
        result.exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        return result;
    }
    const PlatformCommandResult result{true, "ok",
            plan.kind == PlatformCommandKind::open_terminal_here
                ? "Terminal opened at the selected directory"
                : "Object handed to its default desktop application",
            0};
    return result;
#endif
}

} // namespace file_manager
