#pragma once

#include "file_manager/filesystem_model.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

namespace file_manager {

enum class ChecksumTerminal : std::uint8_t {
    completed,
    cancelled,
    refused,
    unavailable,
    changed,
};

struct ChecksumProgress final {
    std::uint64_t bytes_read{};
    std::uint64_t total_bytes{};
};

struct ChecksumResult final {
    ChecksumTerminal terminal{ChecksumTerminal::refused};
    std::string code;
    std::string message;
    std::string algorithm{"SHA-256"};
    std::string digest_hex;
    std::filesystem::path path;
    ObjectIdentity identity;
    std::uint64_t bytes_read{};

    [[nodiscard]] bool succeeded() const noexcept {
        return terminal == ChecksumTerminal::completed;
    }
};

using ChecksumProgressCallback = std::function<void(ChecksumProgress)>;

// Reads one regular file through a bounded 256 KiB buffer. The selected path,
// opened descriptor, and post-read path must all retain the expected revision.
// Symbolic links and paths outside the protected root are never followed.
[[nodiscard]] ChecksumResult checksum_sha256(
    const std::filesystem::path& protected_root,
    const std::filesystem::path& selected_path,
    const ObjectIdentity& expected_identity,
    const CancellationCheck& cancelled = {},
    const ChecksumProgressCallback& progress = {});

} // namespace file_manager
