#pragma once
#include "gui_forms/host/types/host_types.hpp"
namespace gui_forms::render {
std::vector<std::byte> encode_clipboard_png(HostImageView image);
HostClipboardImageResult decode_clipboard_png(std::span<const std::byte> bytes);
} // namespace gui_forms::render
