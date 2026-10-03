#pragma once

#include "../src/native_file.hpp"

#if !defined(_WIN32)
#include <fcntl.h>
#include <unistd.h>
#endif

namespace copy_time_fixture {

// Only call for an already-created fixture-owned file. The fixed old instant
// includes a fraction representable by NTFS, APFS and the native Linux fixture.
// No assertion or throwing work occurs between native acquisition and close.
[[nodiscard]] inline bool set_modified(const std::filesystem::path& path) {
#if defined(_WIN32)
    constexpr std::uint64_t seconds = 946'684'800U + 11'644'473'600U;
    constexpr std::uint64_t ticks = seconds * 10'000'000U + 1'234'567U;
    const FILETIME modified{static_cast<DWORD>(ticks & 0xffffffffU),
        static_cast<DWORD>(ticks >> 32U)};
    const HANDLE handle = CreateFileW(path.c_str(), FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (handle == INVALID_HANDLE_VALUE) return false;
    const BOOL changed = SetFileTime(handle, nullptr, nullptr, &modified);
    const BOOL closed = CloseHandle(handle);
    const bool success = changed != FALSE && closed != FALSE;
#else
    const int descriptor = ::open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (descriptor < 0) return false;
    const timespec times[2]{{0, UTIME_OMIT}, {946'684'800, 123'456'700}};
    const int changed = ::futimens(descriptor, times);
    const int closed = ::close(descriptor);
    const bool success = changed == 0 && closed == 0;
#endif
    return success;
}

[[nodiscard]] inline bool same_modified(const std::filesystem::path& source,
                                       const std::filesystem::path& copied) {
    const file_manager::NativeObjectObservation before = file_manager::observe_native_object(source);
    const file_manager::NativeObjectObservation after = file_manager::observe_native_object(copied);
    if (before.error || after.error || !before.facts.modified || !after.facts.modified) return false;
    const file_manager::ObservedFileTime expected = *before.facts.modified;
    const file_manager::ObservedFileTime actual = *after.facts.modified;
    const bool equal = expected.unix_seconds == actual.unix_seconds &&
        expected.nanoseconds == actual.nanoseconds;
    return equal;
}

} // namespace copy_time_fixture
