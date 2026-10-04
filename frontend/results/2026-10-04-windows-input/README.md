# Windows input-turn integration

**OBSERVED:** sibling commit `c3b9d10d0ed708da2527fcb2697b29b8b2cd9007`
was imported as `607d7624` without merging its older branch. It alternates
input-preferred and ordinary retrieval, retaining the 32-message batch bound
and checking the four-millisecond budget after a pair. It also unions the
complete native `BeginPaint` device-pixel rectangle with logical damage in
both paint paths. Those changes modify private Windows hosting only.

**REJECTED ordering:** its first `PM_QS_INPUT` lookup can take later Backspace
before an earlier translated character when ordinary turns consume private
render wakes. `TranslateMessage` posts characters; queue category filtering
and message-range filtering are different. See the primary
[TranslateMessage](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage)
and [PeekMessageW](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-peekmessagew)
contracts. These source observations motivated the experiment rather than
being treated as measured desktop behavior.

The correction checks the contiguous `WM_KEYFIRST..WM_MOUSELAST` range first,
then other input-category messages, then ordinary work. The range includes
intervening command/timer/IME messages; it is not a global chronological merge
or a hard latency guarantee. The private synchronous reader seam retains no
callback or context and introduces no public API.

## Native evidence on Shadow

**MEASURED:** native Windows x64, MSYS2 GCC 16.2.0, C++20 Release. The explicit
local `gui_forms_windows_message_queue_native_probe` creates its own visible
window and requires foreground/focus ownership and no relevant held key. It
posts 64 private wakes, injects A down/up, retrieves those two native messages,
calls `TranslateMessage` to post the character, then injects Backspace down/up.
It compares the exact earlier category-first policy with the production helper.

| Policy | First retrieved message | Minimal edit oracle after character/Backspace | Private wakes | Key-up receipts |
|---|---|---:|---:|---|
| Rejected category-first control | `WM_KEYDOWN` (256) | One character remains | 64 | Both, owned HWND |
| Corrected range-first | `WM_CHAR` (258) | Empty | 64 | Both, owned HWND |

Both control processes exited zero because each matched its explicitly named
expected outcome. The negative control is not a passing product behavior.
Raw logs are `native-category-first.log` and `native-range-first.log`.
The oracle handles editing Backspace on key-down and ignores translated control
characters, matching the relevant host semantics. This verifies actual native
queue competition with injected input; it is not an actual File Manager field,
physical keyboard, IME, accessibility input, or latency-distribution result.

The probe has an owner for partial injection: an unmatched accepted down gets
a compensating release, and cleanup drains owned-window keyboard messages with
a bounded wait before restoring the previous foreground. Normal acceptance
requires both releases to have been observed. Cleanup failures are reported.
Foreground checks cannot make input routing atomic against desktop interference.
It is deliberately outside CTest and is invoked explicitly with `--native`
or `--rejected-control`.

The earlier preparation incorrectly required every key-up to remain retrievable
through `PM_QS_INPUT`. That failed here while a range lookup could see the key-up;
the rejected diagnostic is retained. Corrected preparation uses range retrieval
and does not claim that the initial A receipt established a queue category.

The deterministic reference-category fixture independently fails the old order
and passes the correction. The existing native posted-message/quit tests remain.
Four focused tests passed against the rebuilt provider: queue, key translation,
host images and native accessibility. The updated SDK was then installed into
the File Manager-owned build tree and the complete Windows consumer suite
passed **15/15 in 7.19 seconds**. That suite covers existing application/picker,
preview, operation and search behavior; it is not native typing-under-load
evidence. Cross-platform integration remains pending at this record's creation.

## Source review and remaining scope

The complete house style was reviewed for the new helper, deterministic/native
fixtures and CMake additions, plus the two paint-bound and two pump edits.
Initial ordering, const-input and native-input cleanup findings were corrected.
No blanket compliance claim is made for the unchanged Windows host or legacy
tests. The spelling scanner passed all three complete new C++ files. Paint
union source acceptance is not complete fractional-DPI visual verification.

Separate source audit: `frontend/src/platform_commands.cpp` still waits for a
POSIX launcher with blocking `waitpid`; `Application::PlatformCommandWork` runs
on the shared read worker and `Application::stop` joins that worker. A launcher
that remains alive therefore holds up later read jobs and shutdown. That path
is unchanged here. A repair needs explicit launch-process lifetime, error
reporting and reaping semantics; it must not terminate the user's application.
