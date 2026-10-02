#pragma once

#include "native_observation.hpp"

#include <cstddef>
#include <optional>
#include <span>
#include <system_error>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace file_manager {

// Fixed private decoder seams, also exercised by deterministic frontend tests.
[[nodiscard]] std::optional<ObservedFileTime> decode_native_time(
    const std::int64_t seconds, const std::int64_t nanoseconds) noexcept;
[[nodiscard]] ObservedFileTime decode_windows_file_time(const std::uint64_t ticks) noexcept;

#if defined(_WIN32)
[[nodiscard]] NativeObjectObservation observation_from_windows_information(
    const BY_HANDLE_FILE_INFORMATION& basic, const std::uint64_t volume,
    const std::span<const unsigned char, 16U> identifier) noexcept;
[[nodiscard]] NativeObjectObservation observation_from_handle(const HANDLE handle) noexcept;
#else
enum class NativeIdentityProjection : std::uint8_t { path, read_descriptor };
[[nodiscard]] NativeObjectObservation observation_from_stat(
    const struct stat& observed, const NativeIdentityProjection projection) noexcept;
#endif

// Follows directory aliases for navigation and returns the observed target path.
// This is not a retained I/O authority; callers recheck admitted roots and identity.
[[nodiscard]] std::optional<std::filesystem::path> resolve_native_directory(
    const std::filesystem::path& path);
// Resolves an existing regular file for a fresh selection observation only.
[[nodiscard]] std::optional<std::filesystem::path> resolve_native_file(
    const std::filesystem::path& path);

#if defined(_WIN32)
// Observes the opened object, including all 128 file-ID bits. Reparse points
// remain links even when they target an ordinary file or directory.
[[nodiscard]] ObjectIdentity identity_from_handle(const HANDLE handle);
// Parse only symbolic-link reparse data, never reinterpret another tag as one.
[[nodiscard]] std::filesystem::path parse_windows_symlink_target(
    std::span<const std::byte> data, std::error_code& error);
#endif

struct NativeSymlink final {
    std::filesystem::path target{};
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
    [[nodiscard]] NativeObjectObservation observation() const;
    [[nodiscard]] std::ptrdiff_t read(std::byte* destination, std::size_t count);
    [[nodiscard]] std::string error_message() const;
private:
#if defined(_WIN32)
    HANDLE handle_{INVALID_HANDLE_VALUE};
#else
    int handle_{-1};
#endif
    std::error_code error_{};
};

} // namespace file_manager
