#pragma once

#include <windows.h>

namespace gui_forms::host::detail {

// Alternate input-first retrieval with an ordinary queue turn. The caller
// bounds each batch and presents afterward, so neither input nor posted work
// can monopolize the pump. Preserve native ordering within each filtered lane.
[[nodiscard]] inline bool take_window_message(MSG& message, bool input_first) noexcept {
    if (input_first &&
        PeekMessageW(&message, nullptr, 0U, 0U, PM_REMOVE | PM_QS_INPUT)) {
        return true;
    }
    // Queue-category filtering alone excludes posted keyboard/mouse messages.
    // This contiguous range also retains the relative order of translated
    // characters, IME composition, commands and timers between those ranges.
    // Application-private render/managed wakes remain in the ordinary lane.
    if (input_first &&
        PeekMessageW(&message, nullptr, WM_KEYFIRST, WM_MOUSELAST, PM_REMOVE)) {
        return true;
    }
    const BOOL available = PeekMessageW(&message, nullptr, 0U, 0U, PM_REMOVE);
    return available != FALSE;
}

} // namespace gui_forms::host::detail
