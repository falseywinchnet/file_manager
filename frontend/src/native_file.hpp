#pragma once

#include "file_manager/filesystem_model.hpp"

#include <cstddef>
#include <span>
#include <system_error>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace file_manager {

#if defined(_WIN32)
// Observes the opened object, including all 128 file-ID bits. Reparse points
// remain links even when they target an ordinary file or directory.
[[nodiscard]] ObjectIdentity identity_from_handle(HANDLE handle);
// Parse only symbolic-link reparse data, never reinterpret another tag as one.
[[nodiscard]] std::filesystem::path parse_windows_symlink_target(
    std::span<const std::byte> data, std::error_code& error);
#endif

struct NativeSymlink final {
    std::filesystem::path target;
    bool directory{}; // Windows link attribute, independent of target existence.
};
[[nodiscard]] NativeSymlink read_native_symlink(
    const std::filesystem::path& path, std::error_code& error);

// Owns one no-follow read handle. The byte reader and identity observation use
// the same handle so replacing the path cannot switch the file being read.
class NativeReadFile final {
public:
    explicit NativeReadFile(const std::filesystem::path& path);
    ~NativeReadFile();
    NativeReadFile(const NativeReadFile&) = delete;
    NativeReadFile& operator=(const NativeReadFile&) = delete;
    [[nodiscard]] bool available() const noexcept;
    [[nodiscard]] ObjectIdentity identity() const;
    [[nodiscard]] std::ptrdiff_t read(std::byte* destination, std::size_t count);
    [[nodiscard]] std::string error_message() const;
private:
#if defined(_WIN32)
    HANDLE handle_{INVALID_HANDLE_VALUE};
#else
    int handle_{-1};
#endif
    std::error_code error_;
};

} // namespace file_manager
