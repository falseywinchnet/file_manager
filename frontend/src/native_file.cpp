#include "native_file.hpp"

#include <cstring>
#include <array>
#include <limits>

#if defined(_WIN32)
#include <winioctl.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace file_manager {

std::optional<std::filesystem::path> resolve_native_directory(
    const std::filesystem::path& path) {
#if defined(_WIN32)
    // Allocate before acquiring the handle. No throwing work occurs until it is
    // closed. Verify identity when converting the returned extended namespace
    // into the DOS/UNC spelling used by the admitted-root path model.
    constexpr DWORD capacity = 32'768;
    std::wstring target(capacity, L'\0');
    const HANDLE handle = CreateFileW(path.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (handle == INVALID_HANDLE_VALUE) return {};
    BY_HANDLE_FILE_INFORMATION information{};
    const BOOL observed = GetFileInformationByHandle(handle, &information);
    const ObjectIdentity identity = identity_from_handle(handle);
    DWORD length = 0;
    if (observed && (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        length = GetFinalPathNameByHandleW(handle, target.data(), capacity,
                                         FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    }
    CloseHandle(handle);
    if (length == 0 || length >= capacity || !identity.available()) return {};
    target.resize(length);
    if (target.starts_with(L"\\\\?\\UNC\\")) {
        target.replace(0, 8, L"\\\\");
    } else if (target.starts_with(L"\\\\?\\") && target.size() >= 7 &&
               target[5] == L':' && target[6] == L'\\') {
        target.erase(0, 4);
    } else {
        return {};
    }
    const std::filesystem::path resolved(target);
    const ObjectIdentity normalized_identity = observe_identity(resolved);
    if (!normalized_identity.available() || !(normalized_identity == identity)) return {};
    return resolved;
#else
    std::error_code error{};
    const std::filesystem::path resolved = std::filesystem::canonical(path, error);
    if (error) return {};
    const bool directory = std::filesystem::is_directory(resolved, error);
    if (error || !directory) return {};
    return resolved;
#endif
}

#if defined(_WIN32)
namespace {

// The caller validates the fixed 20-byte header before borrowing these fields.
WORD reparse_word(const std::span<const std::byte> data, const std::size_t offset) {
    WORD value{};
    std::memcpy(&value, data.data() + offset, sizeof(value));
    return value;
}

DWORD reparse_dword(const std::span<const std::byte> data, const std::size_t offset) {
    DWORD value{};
    std::memcpy(&value, data.data() + offset, sizeof(value));
    return value;
}

bool valid_reparse_range(const std::size_t path_bytes, const std::size_t offset,
                         const std::size_t length) {
    const bool valid = offset % 2 == 0 && length % 2 == 0 &&
        offset <= path_bytes && length <= path_bytes - offset;
    return valid;
}

} // namespace

std::filesystem::path parse_windows_symlink_target(
    const std::span<const std::byte> data, std::error_code& error) {
    error = std::make_error_code(std::errc::invalid_argument);
    // REPARSE_DATA_BUFFER: 8-byte common header, then the 12-byte symbolic
    // link header. Offsets and lengths are bytes relative to PathBuffer.
    // https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_reparse_data_buffer
    if (data.size() < 20 || data.size() > MAXIMUM_REPARSE_DATA_BUFFER_SIZE) return {};
    if (reparse_dword(data, 0) != IO_REPARSE_TAG_SYMLINK) {
        error = std::make_error_code(std::errc::operation_not_supported);
        return {};
    }
    const std::size_t payload = reparse_word(data, 4);
    if (payload < 12 || payload > data.size() - 8) return {};
    const std::size_t path_bytes = payload - 12;
    const std::size_t target_offset = reparse_word(data, 8);
    const std::size_t target_bytes = reparse_word(data, 10);
    const std::size_t print_offset = reparse_word(data, 12);
    const std::size_t print_bytes = reparse_word(data, 14);
    if (!valid_reparse_range(path_bytes, target_offset, target_bytes) ||
        !valid_reparse_range(path_bytes, print_offset, print_bytes) ||
        target_bytes == 0) return {};
    const DWORD flags = reparse_dword(data, 16);
    if ((flags & ~DWORD{1}) != 0) return {};
    static_assert(sizeof(wchar_t) == 2);
    std::wstring target(target_bytes / 2, L'\0');
    std::memcpy(target.data(), data.data() + 20 + target_offset, target_bytes);
    if (target.find(L'\0') != std::wstring::npos) return {};
    if ((flags & 1) != 0) {
        const std::filesystem::path relative_target(target);
        if (relative_target.has_root_path()) return {};
    } else {
        // Preserve namespace semantics, including long paths and UNC names,
        // while translating the NT prefix to its Win32 extended equivalent.
        // MinGW path::is_absolute does not recognize the extended prefix.
        if (!target.starts_with(L"\\??\\")) return {};
        const std::wstring_view name(target.data() + 4, target.size() - 4);
        const bool drive = name.size() >= 3 &&
            ((name[0] >= L'A' && name[0] <= L'Z') ||
             (name[0] >= L'a' && name[0] <= L'z')) &&
            name[1] == L':' && name[2] == L'\\';
        const bool unc = name.starts_with(L"UNC\\") && name.size() > 4;
        if (!drive && !unc) {
            error = std::make_error_code(std::errc::operation_not_supported);
            return {};
        }
        target.replace(0, 4, L"\\\\?\\");
    }
    error.clear();
    const std::filesystem::path result(std::move(target));
    return result;
}

ObjectIdentity identity_from_handle(const HANDLE handle) {
    BY_HANDLE_FILE_INFORMATION basic{};
    FILE_ID_INFO identifier{};
    if (!GetFileInformationByHandle(handle, &basic) ||
        !GetFileInformationByHandleEx(handle, FileIdInfo, &identifier,
                                     sizeof(identifier))) return {};
    ObjectIdentity result{};
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

NativeSymlink read_native_symlink(const std::filesystem::path& path,
                                 std::error_code& error) {
    error.clear();
#if defined(_WIN32)
    const HANDLE handle = CreateFileW(path.c_str(), 0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        error = std::error_code(static_cast<int>(GetLastError()), std::system_category());
        return {};
    }
    BY_HANDLE_FILE_INFORMATION info{};
    std::array<std::byte, MAXIMUM_REPARSE_DATA_BUFFER_SIZE> buffer{};
    DWORD returned{};
    const bool read = GetFileInformationByHandle(handle, &info) &&
        DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT, nullptr, 0,
            buffer.data(), static_cast<DWORD>(buffer.size()), &returned, nullptr);
    const DWORD read_error = read ? ERROR_SUCCESS : GetLastError();
    CloseHandle(handle);
    if (!read) {
        error = std::error_code(static_cast<int>(read_error), std::system_category());
        return {};
    }
    if (returned > buffer.size()) {
        error = std::make_error_code(std::errc::invalid_argument);
        return {};
    }
    const std::span<const std::byte> reparse_data(buffer.data(), returned);
    NativeSymlink result{};
    result.target = parse_windows_symlink_target(reparse_data, error);
    result.directory = (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    return result;
#else
    NativeSymlink result{};
    result.target = std::filesystem::read_symlink(path, error);
    return result;
#endif
}

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
    const bool result = handle_ != INVALID_HANDLE_VALUE;
#else
    const bool result = handle_ >= 0;
#endif
    return result;
}

ObjectIdentity NativeReadFile::identity() const {
#if defined(_WIN32)
    const ObjectIdentity result = identity_from_handle(handle_);
    return result;
#else
    struct stat observed{};
    if (::fstat(handle_, &observed) != 0) return {};
    ObjectIdentity result{};
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
    const std::ptrdiff_t result = static_cast<std::ptrdiff_t>(bytes);
    return result;
#else
    ssize_t bytes{};
    do { bytes = ::read(handle_, destination, count); } while (bytes < 0 && errno == EINTR);
    if (bytes < 0) error_ = std::error_code(errno, std::generic_category());
    return bytes;
#endif
}

std::string NativeReadFile::error_message() const {
    const std::string result = error_.message();
    return result;
}

} // namespace file_manager
