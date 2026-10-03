#pragma once

#include <filesystem>
#include <system_error>

namespace file_manager {

// Private synchronous adapters. Paths are borrowed only for each call.
// A successful preflight is an observation, never a reservation or authority.
// Vacancy returns no error, an existing leaf returns file_exists, and an
// unobservable leaf returns its error. Symlink leaves count as occupied.
[[nodiscard]] std::error_code check_destination_vacant(
    const std::filesystem::path& destination);

// Publish a same-volume rename without replacing any existing destination.
// No replace/copy fallback. Unsupported kernels/filesystems fail explicitly.
// The service separately owns source identity and parent-route validation;
// this operation does not make those earlier observations race-free.
[[nodiscard]] std::error_code rename_no_replace(
    const std::filesystem::path& source,
    const std::filesystem::path& destination);

} // namespace file_manager
