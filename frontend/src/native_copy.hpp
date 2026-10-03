#pragma once

#include "file_manager/filesystem_model.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <span>
#include <system_error>

namespace file_manager {

enum class NativeCopyTerminal : std::uint8_t {
    complete, cancelled, source_changed, failed, callback_failed,
};

struct NativeCopyResult final {
    NativeCopyTerminal terminal{NativeCopyTerminal::failed};
    std::error_code error{};
    std::error_code source_close_error{};
    std::error_code stage_close_error{};
    std::uint64_t copied_bytes{};
    bool stage_created{};
    ObjectIdentity stage_identity{};
};

struct NativeCopyProgress final {
    std::uint64_t copied_bytes{};
    std::uint64_t total_bytes{};
};

using NativeCopyProgressObserver = std::function<void(NativeCopyProgress)>;

struct DirectoryCopyMetadataResult final {
    bool applied{};
    bool identity_changed{};
    std::error_code error{};
    std::error_code source_close_error{};
    std::error_code stage_close_error{};

    [[nodiscard]] bool succeeded() const noexcept {
        const bool success = applied && !identity_changed && !error && !source_close_error && !stage_close_error;
        return success;
    }
};

// Exclusively creates a writable traversal stage: POSIX requests owner-only
// 0700 (subject to umask); Windows inherits the destination parent's security.
// No source attributes are applied until children have been copied.
[[nodiscard]] std::error_code create_copy_directory_stage(
    const std::filesystem::path& stage) noexcept;

// Finalize only after child creation. Opens no-follow source/stage handles,
// checks source revision and stage identity, then applies modification time and
// ordinary permissions. Never creates, publishes or deletes. Failure can leave
// metadata partly applied; caller retains cleanup ownership. Paths are borrowed
// synchronously; no handle survives return and both close errors are reported.
[[nodiscard]] DirectoryCopyMetadataResult finish_copy_directory_metadata(
    const std::filesystem::path& source, const std::filesystem::path& stage,
    const ObjectIdentity& expected_source, const ObjectIdentity& expected_stage) noexcept;

// Construct once before a batch; allocation failure throws before any I/O.
// A workspace is exclusively borrowed for one synchronous invocation at a time.
class NativeCopyWorkspace final {
public:
    static constexpr std::size_t capacity = 256U * 1024U;
    NativeCopyWorkspace();
    NativeCopyWorkspace(const NativeCopyWorkspace&) = delete;
    NativeCopyWorkspace& operator=(const NativeCopyWorkspace&) = delete;
    [[nodiscard]] std::span<std::byte> bytes() noexcept;
private:
    std::unique_ptr<std::byte[]> storage_{};
};

// Private, synchronous stage writer. Inputs/callback are borrowed only during
// this call. Never publishes, unlinks, or cleans up paths. A refused create
// returns stage_created=false and an unavailable stage_identity. After a create,
// callers must revalidate stage identity before their own cleanup/publication;
// an unavailable identity means preserve the uncertain path. Identity revision
// is the last handle observation, not a post-close timestamp guarantee.
// complete requires exact extent, source revision/path checks, and clean closes.
// Successful regular files receive the opened source's modification time after
// writing, with the destination filesystem's supported timestamp precision.
// Access/creation/status-change timestamps are not cloned. Existing permission
// handling is unchanged; this does not promise complete metadata preservation.
// Nonpositive Windows native times are refused before staging because they
// cannot be passed as ordinary absolute times to the basic-information setter.
// Close errors are retained separately even when cancellation/failure wins.
// Windows/Linux poll between bounded I/O calls; macOS uses native write
// callbacks. No hard I/O deadline, parent-route pin, or filesystem snapshot.
// Progress is optional and synchronously borrowed, never retained or copied.
// The caller keeps its target alive and must not reenter with this workspace.
// Observations start at zero after source validation and stage creation, then
// report confirmed bytes monotonically within the validated expected extent.
// Empty files report zero of zero. Duplicate counts are permitted. No event
// establishes revision validation, clean closes, publication, or durability.
// Observer exceptions stop copying as callback_failed; identity and close
// reporting still run. Delivery performs no per-buffer callable allocation.
[[nodiscard]] NativeCopyResult copy_regular_file_to_stage(
    const std::filesystem::path& source,
    const std::filesystem::path& stage,
    const ObjectIdentity& expected,
    const CancellationCheck& cancelled,
    NativeCopyWorkspace& workspace,
    const NativeCopyProgressObserver& progress = {}) noexcept;

} // namespace file_manager
