#include "file_manager/platform_paths.hpp"

#include <cstdlib>
#include <cctype>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shlobj.h>
#endif

namespace file_manager {

bool valid_platform_basename(const std::string_view name) {
    if (name.empty() || name == "." || name == ".." || name.size() > 255U ||
        name.find('/') != std::string_view::npos ||
        name.find('\0') != std::string_view::npos) return false;
#if defined(_WIN32)
    if (name.find_first_of("\\:<>\"|?*") != std::string_view::npos ||
        name.back() == '.' || name.back() == ' ') return false;
    std::string stem;
    for (const unsigned char value : name) {
        if (value < 32U) return false;
        if (value == '.') break;
        stem.push_back(static_cast<char>(std::toupper(value)));
    }
    if (stem == "CON" || stem == "PRN" || stem == "AUX" || stem == "NUL") return false;
    if (stem.size() == 4U && (stem.starts_with("COM") || stem.starts_with("LPT")) &&
        stem[3] >= '1' && stem[3] <= '9') return false;
#endif
    return true;
}

std::filesystem::path user_home_directory() {
#if defined(_WIN32)
    PWSTR value = nullptr;
    const HRESULT status = SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &value);
    if (SUCCEEDED(status)) {
        const std::filesystem::path result(value);
        CoTaskMemFree(value);
        return result;
    }
#else
    const char* const value = std::getenv("HOME");
    if (value != nullptr && *value != '\0') return value;
#endif
    return std::filesystem::current_path();
}

std::vector<std::filesystem::path> local_volume_roots() {
    std::vector<std::filesystem::path> roots;
#if defined(_WIN32)
    const DWORD mask = GetLogicalDrives();
    for (unsigned index = 0; index < 26; ++index) {
        if ((mask & (1UL << index)) == 0) continue;
        wchar_t root[] = L"A:\\";
        root[0] = static_cast<wchar_t>(L'A' + index);
        const UINT type = GetDriveTypeW(root);
        // Do not probe mapped remote drives or spin up removable media here.
        if (type == DRIVE_FIXED || type == DRIVE_RAMDISK) roots.emplace_back(root);
    }
#elif defined(__APPLE__)
    std::error_code error;
    if (std::filesystem::is_directory("/Volumes", error)) roots.emplace_back("/Volumes");
#else
    // Linux mounts form one real filesystem hierarchy. Expose its root without
    // assuming a desktop-specific removable-media directory or scanning it.
    roots.emplace_back("/");
#endif
    return roots;
}

std::string path_utf8(const std::filesystem::path& path) {
    const std::u8string value = path.u8string();
    std::string result;
    result.reserve(value.size());
    for (const char8_t byte : value) result.push_back(static_cast<char>(byte));
    return result;
}

std::string path_generic_utf8(const std::filesystem::path& path) {
    const std::u8string value = path.generic_u8string();
    std::string result;
    result.reserve(value.size());
    for (const char8_t byte : value) result.push_back(static_cast<char>(byte));
    return result;
}

std::filesystem::path path_from_utf8(const std::string_view text) {
    const std::u8string bytes(text.begin(), text.end());
    const std::filesystem::path result(bytes);
    return result;
}

} // namespace file_manager
