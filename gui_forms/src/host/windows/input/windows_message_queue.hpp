#pragma once

#include <windows.h>

namespace gui_forms::host::detail {

// Synchronous borrowed reader; neither the callback nor context is retained.
// The native adapter and deterministic queue-category controls use this seam.
using WindowMessageReader = bool(void*, MSG&, UINT, UINT, UINT);

[[nodiscard]] inline bool read_native_window_message(void*, MSG& message,
    const UINT first, const UINT last, const UINT flags) noexcept {
    const BOOL available = PeekMessageW(&message, nullptr, first, last, flags);
    const bool found = available != FALSE;
    return found;
}

// Alternate input-range retrieval with an ordinary queue turn. The caller
// bounds batches and presents afterward; individual callbacks are not timed
// out. Preserve posted-character precedence within the keyboard/mouse range.
[[nodiscard]] inline bool take_window_message_with_reader(MSG& message, const bool input_first,
    WindowMessageReader& read, void* const context) {
    // TranslateMessage posts WM_CHAR. A hardware-only read first could take a
    // later Backspace before that character when private wakes fill the posted
    // queue. Range filtering keeps native category precedence for these input
    // messages while excluding application-private render/managed wakes.
    if (input_first &&
        read(context, message, WM_KEYFIRST, WM_MOUSELAST, PM_REMOVE)) {
        return true;
    }
    // Other hardware input (for example non-client input outside the range)
    // still gets a preferred retrieval opportunity before ordinary work.
    if (input_first &&
        read(context, message, 0U, 0U, PM_REMOVE | PM_QS_INPUT)) {
        return true;
    }
    const bool available = read(context, message, 0U, 0U, PM_REMOVE);
    return available;
}

[[nodiscard]] inline bool take_window_message(MSG& message, const bool input_first) {
    const bool available = take_window_message_with_reader(
        message, input_first, read_native_window_message, nullptr);
    return available;
}

} // namespace gui_forms::host::detail
