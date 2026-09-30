#pragma once
#import <AppKit/AppKit.h>
#include "gui_forms/host.hpp"
namespace gui_forms::host::detail {
[[nodiscard]] HostClipboardImageResult read_appkit_clipboard_image(NSPasteboard* pasteboard);
[[nodiscard]] HostServiceStatus write_appkit_clipboard_image(NSPasteboard* pasteboard, HostImageView image);
} // namespace gui_forms::host::detail
