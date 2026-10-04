#include "windows_message_queue.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

void post(UINT message, WPARAM value = 0U) {
    const BOOL posted = PostThreadMessageW(GetCurrentThreadId(), message, value, 0);
    require(posted != FALSE, "post test queue message");
}

void test_queue() {
    MSG message{};
    // Establish this thread's queue; all fixtures stay on the test thread.
    PeekMessageW(&message, nullptr, 0U, 0U, PM_NOREMOVE);
    constexpr UINT render_wake = WM_APP + 0x42U;
    for (WPARAM index = 0U; index < 64U; ++index) { post(render_wake, index); }
    post(WM_KEYDOWN, 'H');
    post(WM_CHAR, 'h');
    post(WM_LBUTTONDOWN);
    post(WM_LBUTTONUP);
    require(gui_forms::host::detail::take_window_message(message, true), "queued key available");
    require(message.message == WM_KEYDOWN, "posted input precedes render backlog");
    require(gui_forms::host::detail::take_window_message(message, false), "ordinary turn available");
    require(message.message == render_wake && message.wParam == 0U, "ordinary work retains FIFO");
    require(gui_forms::host::detail::take_window_message(message, true), "translated character available");
    require(message.message == WM_CHAR && message.wParam == 'h', "translated character ordering preserved");
    require(gui_forms::host::detail::take_window_message(message, true), "mouse press available");
    require(message.message == WM_LBUTTONDOWN, "mouse press precedes release");
    require(gui_forms::host::detail::take_window_message(message, true), "mouse release available");
    require(message.message == WM_LBUTTONUP, "mouse release ordering preserved");
    for (WPARAM index = 1U; index < 64U; ++index) {
        require(gui_forms::host::detail::take_window_message(message, true), "no-input fallback available");
        require(message.message == render_wake && message.wParam == index, "fallback preserves posted work");
    }
    require(!gui_forms::host::detail::take_window_message(message, true), "empty queue does not fabricate work");
    PostQuitMessage(17);
    require(gui_forms::host::detail::take_window_message(message, true), "quit available during input turn");
    require(message.message == WM_QUIT && message.wParam == 17U, "quit exit code preserved");
    PostQuitMessage(23);
    require(gui_forms::host::detail::take_window_message(message, false), "quit available during ordinary turn");
    require(message.message == WM_QUIT && message.wParam == 23U, "ordinary quit exit code preserved");
}

} // namespace

int main() {
    try {
        test_queue();
        std::cout << "Windows queue fixtures passed; physical device delivery requires native dogfood\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
