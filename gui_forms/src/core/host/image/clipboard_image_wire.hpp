#pragma once

#include "gui_forms/host/types/host_types.hpp"

namespace gui_forms::detail {
// Private lossless clipboard representation. 16-byte little-endian header:
// magic GFIM, version 1, width, height; followed by tight straight RGBA8 rows.
[[nodiscard]] std::vector<std::byte> encode_clipboard_image(HostImageView image);
[[nodiscard]] HostClipboardImageResult decode_clipboard_image(
    std::span<const std::byte> bytes, bool allow_allocation_padding = false);
[[nodiscard]] HostImage copy_host_image(HostImageView image);
} // namespace gui_forms::detail
