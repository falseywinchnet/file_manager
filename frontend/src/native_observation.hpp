#pragma once

#include "file_manager/filesystem_model.hpp"
#include <system_error>

namespace file_manager {

// Portable owned observation. Native handles and platform headers stay in the
// adapter; GUI consumers must not inherit Windows header macros.
struct NativeObjectObservation final {
    ObjectIdentity identity{};
    FileMetadataFacts facts{};
    std::error_code error{};
};

[[nodiscard]] NativeObjectObservation observe_native_object(const std::filesystem::path& path);

} // namespace file_manager
