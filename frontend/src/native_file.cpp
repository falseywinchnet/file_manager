#include "native_file.hpp"

#include <cstring>
#include <array>
#include <limits>
#include <utility>

#if defined(_WIN32)
#include <winioctl.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace file_manager {

std::optional<ObservedFileTime> decode_native_time(
    const std::int64_t seconds, const std::int64_t nanoseconds) noexcept {
    if (nanoseconds < 0 || nanoseconds >= 1'000'000'000) return {};
    const ObservedFileTime result{seconds, static_cast<std::uint32_t>(nanoseconds)};
    return result;
}

ObservedFileTime decode_windows_file_time(const std::uint64_t ticks) noexcept {
    constexpr std::uint64_t ticks_per_second = 10'000'000U;
    constexpr std::int64_t epoch_seconds = 11'644'473'600LL;
    const std::uint64_t quotient = ticks / ticks_per_second;
    static_assert(std::numeric_limits<std::uint64_t>::max() / ticks_per_second <
                  static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()));
    const std::int64_t seconds = static_cast<std::int64_t>(quotient) - epoch_seconds;
    const std::uint64_t remainder = ticks % ticks_per_second;
    const std::uint32_t nanoseconds = static_cast<std::uint32_t>(remainder * 100U);
    const ObservedFileTime result{seconds, nanoseconds};
    return result;
}

#if !defined(_WIN32)
NativeObjectObservation observation_from_stat(
    const struct stat& observed, const NativeIdentityProjection projection) noexcept {
    NativeObjectObservation result{};
    ObjectIdentity& identity = result.identity;
    identity.device = static_cast<std::uint64_t>(observed.st_dev);
    identity.inode = static_cast<std::uint64_t>(observed.st_ino);
    identity.size = static_cast<std::uint64_t>(observed.st_size);
#if defined(__APPLE__)
    const timespec modified = observed.st_mtimespec;
#else
    const timespec modified = observed.st_mtim;
#endif
    identity.modified_nanoseconds = static_cast<std::uint64_t>(modified.tv_sec) *
        1'000'000'000ULL + static_cast<std::uint64_t>(modified.tv_nsec);
    identity.type = std::filesystem::file_type::unknown;
    if (S_ISREG(observed.st_mode)) identity.type = std::filesystem::file_type::regular;
    else if (S_ISDIR(observed.st_mode)) identity.type = std::filesystem::file_type::directory;
    else if (projection == NativeIdentityProjection::path) {
        if (S_ISLNK(observed.st_mode)) identity.type = std::filesystem::file_type::symlink;
        else if (S_ISBLK(observed.st_mode)) identity.type = std::filesystem::file_type::block;
        else if (S_ISCHR(observed.st_mode)) identity.type = std::filesystem::file_type::character;
        else if (S_ISFIFO(observed.st_mode)) identity.type = std::filesystem::file_type::fifo;
        else if (S_ISSOCK(observed.st_mode)) identity.type = std::filesystem::file_type::socket;
    }
    if (!identity.available()) {
        result.error = std::make_error_code(std::errc::operation_not_supported);
        return result;
    }
    if (S_ISREG(observed.st_mode) && std::in_range<std::uint64_t>(observed.st_size)) {
        result.facts.logical_size = static_cast<std::uint64_t>(observed.st_size);
    }
    if (std::in_range<std::int64_t>(modified.tv_sec) && std::in_range<std::int64_t>(modified.tv_nsec)) {
        result.facts.modified = decode_native_time(static_cast<std::int64_t>(modified.tv_sec),
                                                  static_cast<std::int64_t>(modified.tv_nsec));
    }
    return result;
}
#endif

NativeObjectObservation observe_native_object(const std::filesystem::path& path) {
#if defined(_WIN32)
    const HANDLE handle = CreateFileW(path.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        const DWORD error = GetLastError();
        NativeObjectObservation failure{};
        failure.error = std::error_code(static_cast<int>(error), std::system_category());
        return failure;
    }
    // Fixed-record decoding is nonthrowing; close precedes any display allocation.
    const NativeObjectObservation result = observation_from_handle(handle);
    CloseHandle(handle);
#else
    struct stat observed{};
    const int status = ::lstat(path.c_str(), &observed);
    if (status != 0) {
        const int error = errno;
        NativeObjectObservation failure{};
        failure.error = std::error_code(error, std::generic_category());
        return failure;
    }
    const NativeObjectObservation result = observation_from_stat(observed, NativeIdentityProjection::path);
#endif
    return result;
}

namespace {

std::optional<std::filesystem::path> resolve_native_target(
    const std::filesystem::path& path, const std::filesystem::file_type expected) {
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
    const ObjectIdentity identity = identity_from_handle(handle);
    DWORD length = 0;
    if (identity.available() && identity.type == expected) {
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
    const std::filesystem::file_status status = std::filesystem::status(resolved, error);
    if (error || status.type() != expected) return {};
    return resolved;
#endif
}
} // namespace

std::optional<std::filesystem::path> resolve_native_directory(
    const std::filesystem::path& path) {
    const std::optional<std::filesystem::path> result =
        resolve_native_target(path, std::filesystem::file_type::directory);
    return result;
}

std::optional<std::filesystem::path> resolve_native_file(
    const std::filesystem::path& path) {
    const std::optional<std::filesystem::path> result =
        resolve_native_target(path, std::filesystem::file_type::regular);
    return result;
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

NativeObjectObservation observation_from_windows_information(
    const BY_HANDLE_FILE_INFORMATION& basic, const std::uint64_t volume,
    const std::span<const unsigned char, 16U> identifier) noexcept {
    NativeObjectObservation result{};
    ObjectIdentity& identity = result.identity;
    identity.device = volume;
    std::memcpy(&identity.inode, identifier.data(), sizeof(identity.inode));
    std::memcpy(&identity.inode_high, identifier.data() + 8U, sizeof(identity.inode_high));
    identity.size = (static_cast<std::uint64_t>(basic.nFileSizeHigh) << 32U) | basic.nFileSizeLow;
    const std::uint64_t ticks =
        (static_cast<std::uint64_t>(basic.ftLastWriteTime.dwHighDateTime) << 32U) |
        basic.ftLastWriteTime.dwLowDateTime;
    // Preserve the existing modulo-2^64 revision fingerprint exactly.
    constexpr std::uint64_t epoch_ticks = 116444736000000000ULL;
    identity.modified_nanoseconds = (ticks - epoch_ticks) * 100U;
    if ((basic.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
        identity.type = std::filesystem::file_type::symlink;
    } else if ((basic.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        identity.type = std::filesystem::file_type::directory;
    } else {
        identity.type = std::filesystem::file_type::regular;
    }
    if (!identity.available()) {
        result.error = std::make_error_code(std::errc::operation_not_supported);
        return result;
    }
    if (identity.type == std::filesystem::file_type::regular) result.facts.logical_size = identity.size;
    result.facts.modified = decode_windows_file_time(ticks);
    return result;
}

NativeObjectObservation observation_from_handle(const HANDLE handle) noexcept {
    BY_HANDLE_FILE_INFORMATION basic{};
    FILE_ID_INFO identifier{};
    const BOOL basic_read = GetFileInformationByHandle(handle, &basic);
    if (!basic_read) {
        const DWORD error = GetLastError();
        NativeObjectObservation failure{};
        failure.error = std::error_code(static_cast<int>(error), std::system_category());
        return failure;
    }
    const BOOL id_read = GetFileInformationByHandleEx(handle, FileIdInfo, &identifier, sizeof(identifier));
    if (!id_read) {
        const DWORD error = GetLastError();
        NativeObjectObservation failure{};
        failure.error = std::error_code(static_cast<int>(error), std::system_category());
        return failure;
    }
    const std::span<const unsigned char, 16U> id_bytes(identifier.FileId.Identifier);
    const NativeObjectObservation result = observation_from_windows_information(basic, identifier.VolumeSerialNumber, id_bytes);
    return result;
}

ObjectIdentity identity_from_handle(const HANDLE handle) {
    const NativeObjectObservation observed = observation_from_handle(handle);
    return observed.identity;
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

NativeObjectObservation NativeReadFile::observation() const {
#if defined(_WIN32)
    const NativeObjectObservation result = observation_from_handle(handle_);
#else
    struct stat observed{};
    const int status = ::fstat(handle_, &observed);
    if (status != 0) {
        const int error = errno;
        NativeObjectObservation failure{};
        failure.error = std::error_code(error, std::generic_category());
        return failure;
    }
    const NativeObjectObservation result = observation_from_stat(observed, NativeIdentityProjection::read_descriptor);
#endif
    return result;
}

ObjectIdentity NativeReadFile::identity() const {
    const NativeObjectObservation observed = observation();
    return observed.identity;
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
