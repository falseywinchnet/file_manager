#pragma once

#include "file_manager/filesystem_model.hpp"

#include <cstddef>
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
// Close errors are retained separately even when cancellation/failure wins.
// Windows/Linux poll between bounded I/O calls; macOS uses native write
// callbacks. No hard I/O deadline, parent-route pin, or filesystem snapshot.
[[nodiscard]] NativeCopyResult copy_regular_file_to_stage(
    const std::filesystem::path& source,
    const std::filesystem::path& stage,
    const ObjectIdentity& expected,
    const CancellationCheck& cancelled,
    NativeCopyWorkspace& workspace) noexcept;

} // namespace file_manager
