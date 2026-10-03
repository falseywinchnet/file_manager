#include "native_copy.hpp"
#include "native_file.hpp"

#include <algorithm>
#include <limits>
#include <new>
#include <utility>

#if !defined(_WIN32)
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#endif
#if defined(__APPLE__)
#include <copyfile.h>
#endif

namespace file_manager {

NativeCopyWorkspace::NativeCopyWorkspace()
    : storage_(std::make_unique<std::byte[]>(capacity)) {}

std::span<std::byte> NativeCopyWorkspace::bytes() noexcept {
    const std::span<std::byte> result(storage_.get(), capacity);
    return result;
}

namespace {

[[nodiscard]] std::error_code native_error() noexcept {
#if defined(_WIN32)
    const DWORD value = GetLastError();
    const std::error_code result(static_cast<int>(value), std::system_category());
#else
    const int value = errno;
    const std::error_code result(value, std::generic_category());
#endif
    return result;
}

void fail(NativeCopyResult& result, const std::error_code error) noexcept {
    result.terminal = NativeCopyTerminal::failed;
    result.error = error;
    if (!result.error) result.error = std::make_error_code(std::errc::io_error);
}

[[nodiscard]] bool stop_requested(const CancellationCheck& cancelled, NativeCopyResult& result) noexcept {
    try {
        if (!cancelled || !cancelled()) return false;
        result.terminal = NativeCopyTerminal::cancelled;
        result.error = std::make_error_code(std::errc::operation_canceled);
    } catch (...) {
        result.terminal = NativeCopyTerminal::callback_failed;
        result.error = std::make_error_code(std::errc::operation_canceled);
    }
    return true;
}

[[nodiscard]] bool report_progress(const NativeCopyProgressObserver& observer,
                                   const std::uint64_t expected_size,
                                   NativeCopyResult& result) noexcept {
    if (!observer) return true;
    const NativeCopyProgress progress{result.copied_bytes, expected_size};
    try {
        observer(progress);
    } catch (...) {
        result.terminal = NativeCopyTerminal::callback_failed;
        result.error = std::make_error_code(std::errc::operation_canceled);
        return false;
    }
    return true;
}

// Handles stay with this invocation. Explicit closes report errors; lifetime
// cleanup is only the fallback during an unexpected exception. POSIX close is
// never retried after EINTR, because retry can close a reused descriptor.
class CopyFile final {
public:
    CopyFile() = default;
    CopyFile(const CopyFile&) = delete;
    CopyFile& operator=(const CopyFile&) = delete;
    ~CopyFile() { const std::error_code ignored = close(); (void)ignored; }

    [[nodiscard]] std::error_code open_source(const std::filesystem::path& path) noexcept {
#if defined(_WIN32)
        handle = CreateFileW(path.c_str(), GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
            OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
#else
        do { handle = ::open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK); }
        while (handle < 0 && errno == EINTR);
#endif
        if (!available()) {
            const std::error_code error = native_error();
            return error;
        }
        return {};
    }

    [[nodiscard]] std::error_code create_stage(const std::filesystem::path& path) noexcept {
#if defined(_WIN32)
        handle = CreateFileW(path.c_str(), GENERIC_WRITE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
#else
        do { handle = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600); }
        while (handle < 0 && errno == EINTR);
#endif
        if (!available()) {
            const std::error_code error = native_error();
            return error;
        }
        return {};
    }

    [[nodiscard]] bool available() const noexcept {
#if defined(_WIN32)
        const bool result = handle != INVALID_HANDLE_VALUE;
#else
        const bool result = handle >= 0;
#endif
        return result;
    }

    [[nodiscard]] std::error_code open_directory_stage(const std::filesystem::path& path) noexcept {
#if defined(_WIN32)
        handle = CreateFileW(path.c_str(), FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
            OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
#else
        do { handle = ::open(path.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW); }
        while (handle < 0 && errno == EINTR);
#endif
        if (!available()) {
            const std::error_code error = native_error();
            return error;
        }
        return {};
    }

    [[nodiscard]] NativeObjectObservation observe() const noexcept {
#if defined(_WIN32)
        if (GetFileType(handle) != FILE_TYPE_DISK) {
            NativeObjectObservation failure{};
            failure.error = std::make_error_code(std::errc::operation_not_supported);
            return failure;
        }
        const NativeObjectObservation result = observation_from_handle(handle);
#else
        struct stat status{};
        if (::fstat(handle, &status) != 0) {
            NativeObjectObservation failure{};
            failure.error = native_error();
            return failure;
        }
        const NativeObjectObservation result = observation_from_stat(status, NativeIdentityProjection::read_descriptor);
#endif
        return result;
    }

    [[nodiscard]] std::error_code close() noexcept {
        if (!available()) return {};
#if defined(_WIN32)
        const HANDLE closing = handle;
        handle = INVALID_HANDLE_VALUE;
        if (!CloseHandle(closing)) {
            const std::error_code error = native_error();
            return error;
        }
#else
        const int closing = handle;
        handle = -1;
        if (::close(closing) != 0) {
            const std::error_code error = native_error();
            return error;
        }
#endif
        return {};
    }

#if defined(_WIN32)
    HANDLE handle{INVALID_HANDLE_VALUE};
#else
    int handle{-1};
#endif
};

struct CopyMetadata final {
#if defined(_WIN32)
    DWORD attributes{FILE_ATTRIBUTE_NORMAL};
    LARGE_INTEGER modified{};
#else
    mode_t mode{};
    timespec modified{};
#endif
};

[[nodiscard]] std::error_code read_metadata(const CopyFile& source, CopyMetadata& metadata) noexcept {
#if defined(_WIN32)
    FILE_BASIC_INFO information{};
    if (!GetFileInformationByHandleEx(source.handle, FileBasicInfo, &information, sizeof(information))) {
        const std::error_code error = native_error();
        return error;
    }
    if ((information.FileAttributes & FILE_ATTRIBUTE_READONLY) != 0) {
        metadata.attributes = FILE_ATTRIBUTE_READONLY;
    }

    metadata.modified = information.LastWriteTime;
    // Do not pass zero/negative timestamps as setter control values. This
    // writer supports positive absolute Windows times; refuse before staging.
    if (metadata.modified.QuadPart <= 0) {
        const std::error_code error = std::make_error_code(std::errc::operation_not_supported);
        return error;
    }
#else
    struct stat information{};
    if (::fstat(source.handle, &information) != 0) {
        const std::error_code error = native_error();
        return error;
    }
    metadata.mode = information.st_mode & 0777;
#if defined(__APPLE__)
    metadata.modified = information.st_mtimespec;
#else
    metadata.modified = information.st_mtim;
#endif
#endif
    return {};
}

// Source metadata was captured through its validated handle. Apply only after
// data writes and source revalidation, through the still-owned stage handle.
// Access/creation/status-change times keep their native new-object behavior.
[[nodiscard]] std::error_code apply_metadata(const CopyFile& stage, const CopyMetadata& metadata) noexcept {
#if defined(_WIN32)
    FILE_BASIC_INFO information{};
    information.FileAttributes = metadata.attributes;
    information.LastWriteTime = metadata.modified;
    if (!SetFileInformationByHandle(stage.handle, FileBasicInfo, &information, sizeof(information))) {
        const std::error_code error = native_error();
        return error;
    }
#else
    const timespec times[2]{{0, UTIME_OMIT}, metadata.modified};
    if (::futimens(stage.handle, times) != 0) {
        const std::error_code error = native_error();
        return error;
    }
    if (::fchmod(stage.handle, metadata.mode) != 0) {
        const std::error_code error = native_error();
        return error;
    }
#endif
    return {};
}

#if !defined(__APPLE__)
void copy_bounded(const CopyFile& source, const CopyFile& stage, const std::uint64_t expected_size,
                  const CancellationCheck& cancelled, NativeCopyWorkspace& workspace,
                  const NativeCopyProgressObserver& progress, NativeCopyResult& result) noexcept {
    const std::span<std::byte> bytes = workspace.bytes();
#if defined(_WIN32)
    static_assert(NativeCopyWorkspace::capacity <= std::numeric_limits<DWORD>::max());
#else
    static_assert(NativeCopyWorkspace::capacity <= static_cast<std::size_t>(std::numeric_limits<ssize_t>::max()));
#endif
    while (result.copied_bytes < expected_size) {
        if (stop_requested(cancelled, result)) return;
        const std::uint64_t remaining = expected_size - result.copied_bytes;
        const std::uint64_t bounded = std::min(remaining, static_cast<std::uint64_t>(bytes.size()));
        const std::size_t requested = static_cast<std::size_t>(bounded);
        std::size_t received{};
#if defined(_WIN32)
        DWORD count{};
        const BOOL read = ReadFile(source.handle, bytes.data(), static_cast<DWORD>(requested), &count, nullptr);
        if (!read) { fail(result, native_error()); return; }
        received = static_cast<std::size_t>(count);
#else
        const ssize_t count = ::read(source.handle, bytes.data(), requested);
        if (count < 0) {
            if (errno == EINTR) continue;
            fail(result, native_error());
            return;
        }
        received = static_cast<std::size_t>(count);
#endif
        if (received == 0 || received > requested) {
            result.terminal = NativeCopyTerminal::source_changed;
            result.error = std::make_error_code(std::errc::io_error);
            return;
        }
        std::size_t offset{};
        while (offset < received) {
            if (stop_requested(cancelled, result)) return;
            const std::size_t pending = received - offset;
            std::size_t written{};
#if defined(_WIN32)
            DWORD count_written{};
            const BOOL write = WriteFile(stage.handle, bytes.data() + offset,
                static_cast<DWORD>(pending), &count_written, nullptr);
            if (!write) { fail(result, native_error()); return; }
            written = static_cast<std::size_t>(count_written);
#else
            const ssize_t count_written = ::write(stage.handle, bytes.data() + offset, pending);
            if (count_written < 0) {
                if (errno == EINTR) continue;
                fail(result, native_error());
                return;
            }
            written = static_cast<std::size_t>(count_written);
#endif
            if (written == 0 || written > pending) {
                fail(result, std::make_error_code(std::errc::io_error));
                return;
            }
            offset += written;
            result.copied_bytes += written;
            if (!report_progress(progress, expected_size, result)) return;
        }
    }
    if (stop_requested(cancelled, result)) return;
    result.terminal = NativeCopyTerminal::complete;
}
#else
struct MacCopyContext final {
    const CancellationCheck& cancelled;
    const NativeCopyProgressObserver& progress;
    NativeCopyResult& result;
    std::uint64_t expected_size{};
};

[[nodiscard]] bool mac_copied_count(const copyfile_state_t state, NativeCopyResult& result) noexcept {
    off_t copied{};
    if (copyfile_state_get(state, COPYFILE_STATE_COPIED, &copied) != 0) {
        fail(result, native_error());
        return false;
    }
    if (!std::in_range<std::uint64_t>(copied)) {
        fail(result, std::make_error_code(std::errc::value_too_large));
        return false;
    }
    const std::uint64_t count = static_cast<std::uint64_t>(copied);
    if (count < result.copied_bytes) {
        fail(result, std::make_error_code(std::errc::io_error));
        return false;
    }
    result.copied_bytes = count;
    return true;
}

// Only synchronous fcopyfile borrows this stack context. Never throw across C.
int mac_copy_progress(const int what, const int phase, const copyfile_state_t state,
                      const char*, const char*, void* const opaque) noexcept {
    MacCopyContext& context = *static_cast<MacCopyContext*>(opaque);
    try {
        if (phase == COPYFILE_ERR) {
            fail(context.result, native_error());
            return COPYFILE_QUIT;
        }
        if (what != COPYFILE_COPY_DATA) return COPYFILE_CONTINUE;
        if (!mac_copied_count(state, context.result)) return COPYFILE_QUIT;
        if (context.result.copied_bytes > context.expected_size) {
            context.result.terminal = NativeCopyTerminal::source_changed;
            context.result.error = std::make_error_code(std::errc::io_error);
            return COPYFILE_QUIT;
        }
        if (!report_progress(context.progress, context.expected_size, context.result)) return COPYFILE_QUIT;
        if (stop_requested(context.cancelled, context.result)) return COPYFILE_QUIT;
        return COPYFILE_CONTINUE;
    } catch (...) {
        context.result.terminal = NativeCopyTerminal::callback_failed;
        context.result.error = std::make_error_code(std::errc::operation_canceled);
        return COPYFILE_QUIT;
    }
}

void copy_mac(const CopyFile& source, const CopyFile& stage, const std::uint64_t expected_size,
              const CancellationCheck& cancelled, const NativeCopyProgressObserver& progress,
              NativeCopyResult& result) noexcept {
    const copyfile_state_t state = copyfile_state_alloc();
    if (state == nullptr) { fail(result, native_error()); return; }
    MacCopyContext context{cancelled, progress, result, expected_size};
    // copyfile_state_set takes the callback address itself, not its address of
    // storage; this cast is confined to Apple's documented foreign interface.
    const int callback_set = copyfile_state_set(state, COPYFILE_STATE_STATUS_CB,
        reinterpret_cast<const void*>(mac_copy_progress));
    if (callback_set != 0) {
        fail(result, native_error());
    } else {
        const int context_set = copyfile_state_set(state, COPYFILE_STATE_STATUS_CTX, &context);
        if (context_set != 0) {
            fail(result, native_error());
        } else {
            result.terminal = NativeCopyTerminal::complete;
            const int copied = fcopyfile(source.handle, stage.handle, state, COPYFILE_DATA);
            const std::error_code copy_error = copied == 0 ? std::error_code{} : native_error();
            NativeCopyResult count_result{};
            count_result.copied_bytes = result.copied_bytes;
            const bool count_available = mac_copied_count(state, count_result);
            if (count_available) result.copied_bytes = count_result.copied_bytes;
            else if (result.terminal == NativeCopyTerminal::complete) fail(result, count_result.error);
            else result.error = count_result.error;
            if (count_available && result.terminal == NativeCopyTerminal::complete) {
                if (copied != 0) fail(result, copy_error);
                else if (result.copied_bytes != expected_size) {
                    result.terminal = NativeCopyTerminal::source_changed;
                    result.error = std::make_error_code(std::errc::io_error);
                } else {
                    const bool reported = report_progress(progress, expected_size, result);
                    if (reported) {
                        const bool stopped = stop_requested(cancelled, result);
                        (void)stopped;
                    }
                }
            }
        }
    }
    if (copyfile_state_free(state) != 0 && result.terminal == NativeCopyTerminal::complete) {
        fail(result, native_error());
    }
}
#endif

void perform_copy(const std::filesystem::path& source_path,
                  const std::filesystem::path& stage_path, const ObjectIdentity& expected,
                  const CancellationCheck& cancelled, NativeCopyWorkspace& workspace,
                  const NativeCopyProgressObserver& progress,
                  CopyFile& source, CopyFile& stage, NativeCopyResult& result) {
    if (stop_requested(cancelled, result)) return;
    result.error = source.open_source(source_path);
    if (result.error) return;
    const NativeObjectObservation before = source.observe();
    if (before.error) { fail(result, before.error); return; }
    if (!expected.available() || before.identity.type != std::filesystem::file_type::regular ||
        !before.identity.same_revision(expected)) {
        result.terminal = NativeCopyTerminal::source_changed;
        return;
    }
    CopyMetadata metadata{};
    result.error = read_metadata(source, metadata);
    if (result.error) return;
    if (stop_requested(cancelled, result)) return;
    result.error = stage.create_stage(stage_path);
    if (result.error) return;
    result.stage_created = true;
    const NativeObjectObservation created = stage.observe();
    result.stage_identity = created.identity;
    if (created.error) { fail(result, created.error); return; }
    if (!report_progress(progress, expected.size, result)) return;
#if defined(__APPLE__)
    (void)workspace;
    copy_mac(source, stage, expected.size, cancelled, progress, result);
#else
    copy_bounded(source, stage, expected.size, cancelled, workspace, progress, result);
#endif
    if (result.terminal != NativeCopyTerminal::complete) return;
    const NativeObjectObservation after = source.observe();
    if (after.error) { fail(result, after.error); return; }
    CopyFile binding{};
    const std::error_code binding_open = binding.open_source(source_path);
    NativeObjectObservation bound{};
    if (!binding_open) bound = binding.observe();
    const std::error_code binding_close = binding.close();
    if (binding_close) { fail(result, binding_close); return; }
    if (binding_open || bound.error || !after.identity.same_revision(expected) ||
        !bound.identity.same_revision(expected)) {
        result.terminal = NativeCopyTerminal::source_changed;
        result.error = binding_open ? binding_open : bound.error;
        return;
    }
    const NativeObjectObservation destination = stage.observe();
    if (destination.error) { fail(result, destination.error); return; }
    if (result.copied_bytes != expected.size || destination.identity.size != expected.size) {
        fail(result, std::make_error_code(std::errc::io_error));
        return;
    }
    const std::error_code metadata_error = apply_metadata(stage, metadata);
    if (metadata_error) fail(result, metadata_error);
}

void apply_directory_metadata(const std::filesystem::path& source_path,
                              const std::filesystem::path& stage_path,
                              const ObjectIdentity& expected_source,
                              const ObjectIdentity& expected_stage,
                              CopyFile& source, CopyFile& stage,
                              DirectoryCopyMetadataResult& result) noexcept {
    result.error = source.open_source(source_path);
    if (result.error) return;
    const NativeObjectObservation source_observation = source.observe();
    result.error = source_observation.error;
    if (result.error) return;
    if (!expected_source.available() || source_observation.identity.type != std::filesystem::file_type::directory ||
        !source_observation.identity.same_revision(expected_source)) {
        result.identity_changed = true;
        return;
    }
    CopyMetadata metadata{};
    result.error = read_metadata(source, metadata);
    if (result.error) return;
    result.error = stage.open_directory_stage(stage_path);
    if (result.error) return;
    const NativeObjectObservation stage_observation = stage.observe();
    result.error = stage_observation.error;
    if (result.error) return;
    if (!expected_stage.available() || stage_observation.identity.type != std::filesystem::file_type::directory ||
        stage_observation.identity != expected_stage) {
        result.identity_changed = true;
        return;
    }
    result.error = apply_metadata(stage, metadata);
    result.applied = !result.error;
}
} // namespace

std::error_code create_copy_directory_stage(const std::filesystem::path& stage) noexcept {
#if defined(_WIN32)
    const bool created = CreateDirectoryW(stage.c_str(), nullptr) != FALSE;
#else
    const bool created = ::mkdir(stage.c_str(), 0700) == 0;
#endif
    if (!created) {
        const std::error_code error = native_error();
        return error;
    }
    return {};
}

DirectoryCopyMetadataResult finish_copy_directory_metadata(
    const std::filesystem::path& source_path, const std::filesystem::path& stage_path,
    const ObjectIdentity& expected_source, const ObjectIdentity& expected_stage) noexcept {
    DirectoryCopyMetadataResult result{};
    CopyFile source{};
    CopyFile stage{};
    apply_directory_metadata(source_path, stage_path, expected_source, expected_stage, source, stage, result);
    result.stage_close_error = stage.close();
    result.source_close_error = source.close();
    return result;
}

NativeCopyResult copy_regular_file_to_stage(
    const std::filesystem::path& source_path, const std::filesystem::path& stage_path,
    const ObjectIdentity& expected, const CancellationCheck& cancelled,
    NativeCopyWorkspace& workspace, const NativeCopyProgressObserver& progress) noexcept {
    NativeCopyResult result{};
    CopyFile source{};
    CopyFile stage{};
    try {
        perform_copy(source_path, stage_path, expected, cancelled, workspace, progress, source, stage, result);
    } catch (const std::bad_alloc&) {
        fail(result, std::make_error_code(std::errc::not_enough_memory));
    } catch (...) {
        fail(result, std::make_error_code(std::errc::io_error));
    }
    if (result.stage_created) {
        const NativeObjectObservation final_stage = stage.observe();
        if (!final_stage.error) result.stage_identity = final_stage.identity;
        else if (result.terminal == NativeCopyTerminal::complete) fail(result, final_stage.error);
    }
    result.stage_close_error = stage.close();
    result.source_close_error = source.close();
    if (result.terminal == NativeCopyTerminal::complete) {
        if (result.stage_close_error) fail(result, result.stage_close_error);
        else if (result.source_close_error) fail(result, result.source_close_error);
    }
    return result;
}
} // namespace file_manager
