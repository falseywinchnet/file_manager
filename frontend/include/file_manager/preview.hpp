#pragma once

#include "file_manager/filesystem_model.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace file_manager {

enum class PreviewKind : std::uint8_t {
    text,
    png,
    unsupported,
    refused,
    unavailable,
    changed,
};

struct PreviewResult final {
    PreviewKind kind{PreviewKind::unsupported};
    std::string code;
    std::string message;
    std::filesystem::path path;
    ObjectIdentity identity;
    std::string text_utf8;
    std::vector<std::byte> png_bytes;
};

inline constexpr std::size_t maximum_text_preview_bytes = 64U * 1024U;
inline constexpr std::size_t maximum_png_preview_bytes = 16U * 1024U * 1024U;

// Reads only bounded first-party formats: UTF-8 text and PNG. The opened
// descriptor and visible path must retain the selected revision; links are
// never followed and unsupported formats stay explicit.
[[nodiscard]] PreviewResult load_preview(
    const std::filesystem::path& protected_root,
    const std::filesystem::path& selected_path,
    const ObjectIdentity& expected_identity,
    const CancellationCheck& cancelled = {});

} // namespace file_manager
