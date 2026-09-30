#pragma once

#include <filesystem>
#include <iostream>
#include <system_error>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#undef near
#undef far
#endif

// False skips only assertions requiring this fixture. Unexpected setup failures
// still fail the suite. No elevation or machine configuration is attempted.
inline bool create_fixture_link(const std::filesystem::path& target,
                                const std::filesystem::path& link,
                                const bool directory = false) {
#if defined(_WIN32)
    const std::filesystem::path resolved = target.is_absolute() ? target : link.parent_path() / target;
    DWORD flags = (directory || std::filesystem::is_directory(resolved)) ?
        SYMBOLIC_LINK_FLAG_DIRECTORY : 0;
    flags |= SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
    if (CreateSymbolicLinkW(link.c_str(), target.c_str(), flags)) return true;
    const DWORD error = GetLastError();
    if (error == ERROR_PRIVILEGE_NOT_HELD || error == ERROR_NOT_SUPPORTED ||
        error == ERROR_CALL_NOT_IMPLEMENTED) {
        std::cout << "SKIP symlink fixture assertions: Windows error " << error << '\n';
        return false;
    }
    throw std::system_error(static_cast<int>(error), std::system_category(), "create fixture symlink");
#else
    if (directory) std::filesystem::create_directory_symlink(target, link);
    else std::filesystem::create_symlink(target, link);
    return true;
#endif
}
