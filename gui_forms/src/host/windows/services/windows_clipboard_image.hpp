#pragma once
#include "gui_forms/host.hpp"
#include <windows.h>
namespace gui_forms::host::detail {
[[nodiscard]] HostClipboardImageResult read_windows_clipboard_image(HWND owner);
[[nodiscard]] HostServiceStatus write_windows_clipboard_image(HWND owner, HostImageView image);
}
