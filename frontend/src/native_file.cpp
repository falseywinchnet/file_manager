#include "native_file.hpp"

#include <cstring>
#include <limits>

#if !defined(_WIN32)
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace file_manager {

#if defined(_WIN32)
ObjectIdentity identity_from_handle(const HANDLE handle) {
    BY_HANDLE_FILE_INFORMATION basic{};
    FILE_ID_INFO identifier{};
    if (!GetFileInformationByHandle(handle, &basic) ||
        !GetFileInformationByHandleEx(handle, FileIdInfo, &identifier,
                                     sizeof(identifier))) return {};
    ObjectIdentity result;
    result.device = identifier.VolumeSerialNumber;
    std::memcpy(&result.inode, identifier.FileId.Identifier, sizeof(result.inode));
    std::memcpy(&result.inode_high, identifier.FileId.Identifier + 8,
                sizeof(result.inode_high));
    result.size = (static_cast<std::uint64_t>(basic.nFileSizeHigh) << 32U) |
                  basic.nFileSizeLow;
    const std::uint64_t ticks =
        (static_cast<std::uint64_t>(basic.ftLastWriteTime.dwHighDateTime) << 32U) |
        basic.ftLastWriteTime.dwLowDateTime;
    // Convert the Windows 1601 epoch to Unix nanoseconds before multiplication.
    constexpr std::uint64_t epoch_ticks = 116444736000000000ULL;
    result.modified_nanoseconds = (ticks - epoch_ticks) * 100U;
    if ((basic.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
        result.type = std::filesystem::file_type::symlink;
    } else if ((basic.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        result.type = std::filesystem::file_type::directory;
    } else {
        result.type = std::filesystem::file_type::regular;
    }
    return result;
}
#endif

NativeReadFile::NativeReadFile(const std::filesystem::path& path) {
#if defined(_WIN32)
    handle_ = CreateFileW(path.c_str(), GENERIC_READ,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         nullptr, OPEN_EXISTING,
                         FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                         nullptr);
    if (!available()) error_ = std::error_code(GetLastError(), std::system_category());
#else
    handle_ = ::open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (!available()) error_ = std::error_code(errno, std::generic_category());
#endif
}

NativeReadFile::~NativeReadFile() {
    if (!available()) return;
#if defined(_WIN32)
    CloseHandle(handle_);
#else
    ::close(handle_);
#endif
}

bool NativeReadFile::available() const noexcept {
#if defined(_WIN32)
    return handle_ != INVALID_HANDLE_VALUE;
#else
    return handle_ >= 0;
#endif
}

ObjectIdentity NativeReadFile::identity() const {
#if defined(_WIN32)
    return identity_from_handle(handle_);
#else
    struct stat observed{};
    if (::fstat(handle_, &observed) != 0) return {};
    ObjectIdentity result;
    result.device = static_cast<std::uint64_t>(observed.st_dev);
    result.inode = static_cast<std::uint64_t>(observed.st_ino);
    result.size = static_cast<std::uint64_t>(observed.st_size);
#if defined(__APPLE__)
    result.modified_nanoseconds = static_cast<std::uint64_t>(observed.st_mtimespec.tv_sec) *
        1'000'000'000ULL + static_cast<std::uint64_t>(observed.st_mtimespec.tv_nsec);
#else
    result.modified_nanoseconds = static_cast<std::uint64_t>(observed.st_mtim.tv_sec) *
        1'000'000'000ULL + static_cast<std::uint64_t>(observed.st_mtim.tv_nsec);
#endif
    if (S_ISREG(observed.st_mode)) result.type = std::filesystem::file_type::regular;
    else if (S_ISDIR(observed.st_mode)) result.type = std::filesystem::file_type::directory;
    else result.type = std::filesystem::file_type::unknown;
    return result;
#endif
}

std::ptrdiff_t NativeReadFile::read(std::byte* const destination, const std::size_t count) {
#if defined(_WIN32)
    if (count > std::numeric_limits<DWORD>::max()) {
        error_ = std::make_error_code(std::errc::value_too_large);
        return -1;
    }
    DWORD bytes{};
    if (!ReadFile(handle_, destination, static_cast<DWORD>(count), &bytes, nullptr)) {
        error_ = std::error_code(GetLastError(), std::system_category());
        return -1;
    }
    return static_cast<std::ptrdiff_t>(bytes);
#else
    ssize_t bytes{};
    do { bytes = ::read(handle_, destination, count); } while (bytes < 0 && errno == EINTR);
    if (bytes < 0) error_ = std::error_code(errno, std::generic_category());
    return bytes;
#endif
}

std::string NativeReadFile::error_message() const { return error_.message(); }

} // namespace file_manager
