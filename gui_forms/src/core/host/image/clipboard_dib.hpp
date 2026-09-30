#pragma once
#include "gui_forms/host/types/host_types.hpp"
namespace gui_forms::detail {
// Native clipboard DIB interchange only; not an application file codec.
[[nodiscard]] std::vector<std::byte> encode_clipboard_dib(HostImageView image);
[[nodiscard]] HostClipboardImageResult decode_clipboard_dib(std::span<const std::byte> bytes);
}
