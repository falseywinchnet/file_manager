# Independent source reviews

Latest review first. Runtime measurements were performed by the integrating
thread and are preserved separately; these reports are source reviews only.

**Source acceptance: the injected-key cleanup blocker is resolved. No remaining correctness or house-style blockers found in the current helper, queue fixtures, native probe and CMake additions.**

The revised cleanup has explicit ownership and ordering:

- `InjectedKeys` is constructed after `NativeWindow`, so it cleans up before window destruction and foreground restoration.
- Each `SendInput` result is recorded before completeness is checked. An accepted down without its paired up creates a compensating-release obligation.
- Cleanup attempts those releases, drains pending keyboard messages addressed to the probe window with a bounded wait, and reports injection or receipt failures.
- Normal success now requires both admitted key pairs and both key-up receipts addressed to the owned HWND. The message loop cannot pass merely because it counted a character, Backspace and all 64 wakes.

The revised preparation and evidence should be described precisely. A-down and A-up are retrieved by **message-range filtering**, then `TranslateMessage` posts the character, and the Backspace pair is injected afterward. This does **not** establish that the initial A receipt used `PM_QS_INPUT`. It does establish a native queued-character/later-input competition for the two retrieval policies, using injected input rather than manually posted key messages.

The production helper remains range-first, category-fallback, ordinary-fallback. Its deterministic fixture and actual posted/quit fixtures retain their distinct roles. The native probe’s minimal edit oracle remains useful for the identified defect without claiming actual File Manager text-control or physical-keyboard conformance.

The reported native outcomes—category-first `256` with one text unit remaining versus range-first `258` with empty text, both with all wakes and key-up receipts—are consistent with the source assertions. **They remain root-reported measurements**, as does the focused 4/4 result; I did not execute the probe or inspect its retained logs.

Exact final reviewed scope:

- Full [windows_message_queue.hpp](C:/Users/Shadow/file_manager/gui_forms/src/host/windows/input/windows_message_queue.hpp).
- Full [windows_message_queue_tests.cpp](C:/Users/Shadow/file_manager/gui_forms/tests/windows_message_queue_tests.cpp).
- Full [windows_message_queue_native_probe.cpp](C:/Users/Shadow/file_manager/gui_forms/tests/windows_message_queue_native_probe.cpp).
- Both queue targets and their test-registration distinction in [gui_forms/CMakeLists.txt](C:/Users/Shadow/file_manager/gui_forms/CMakeLists.txt).

The earlier source acceptance of the paint-bound unions is unchanged. Interactive foreground ownership still cannot be made atomic with input injection; cleanup reports failures rather than guaranteeing recovery from external desktop interference. The native probe appropriately remains an explicit local target outside CTest.

No edits, builds, tests or Git mutations were performed.

---

**The native ordering experiment is useful, but I would not accept the probe yet: its injected-key cleanup is incomplete on failure.** The queue comparison itself is appropriately scoped to real native message categories plus a minimal edit oracle.

**Blocking — partial injection can leave a key pressed.** In [windows_message_queue_native_probe.cpp](C:/Users/Shadow/file_manager/gui_forms/tests/windows_message_queue_native_probe.cpp), `inject_sequence` throws immediately when `SendInput` returns fewer than four events. If only the A-down event, or the sequence through Backspace-down, was accepted, the corresponding release was not injected. `NativeWindow` then destroys the window and may restore the previous foreground window without repairing that keyboard state.

Add an explicit owner for the injected sequence’s cleanup obligations. Record the accepted event count before checking completeness, and ensure any unmatched injected key-down receives a compensating key-up on failure. Keep that cleanup separate from window destruction and perform it before foreground restoration. Cleanup failure must be reported rather than silently treated as a clean refusal.

**Also tighten normal/failure drain evidence.** The probe explicitly consumes A-up, but does not require observation of Backspace-up. Its bounded loop can end after 512 turns and pass the final counts without proving that release was retrieved. Track and validate the expected key-up, including its target window. Ensure the failure path handles remaining probe-owned keyboard messages before restoring foreground where possible. Destroying the window alone is not evidence that the complete injected sequence was consumed.

The remaining design is sound within its limits:

- Foreground and focus are checked immediately before injection, and relevant held keys cause refusal. Those checks reduce interference but cannot make foreground ownership atomic with `SendInput`; retain this as an explicit interactive-desktop limitation.
- The first A key is obtained through the actual input-category filter, passed to `TranslateMessage`, and its resulting character is verified as pending. The subsequent competition therefore exercises a boundary the deterministic fixture could not.
- The rejected control reproduces the original category-first ordering, while the positive mode calls the production helper. Private wakes, character delivery and Backspace handling are counted separately.
- The text oracle intentionally handles Backspace at key-down and ignores its translated control character. That is a declared minimal edit-order oracle, not an actual File Manager text-control test.
- There is no retained custom window callback: the registered procedure is `DefWindowProcW`. Native window/class ownership is explicit and noncopyable. Sample state and message records are initialized, and the small text buffer is reserved before retrieval.
- The CMake target is Windows-only and is not registered with CTest, which matches the requirement for deliberate local interactive execution.

Exact reviewed scope: full [windows_message_queue_native_probe.cpp](C:/Users/Shadow/file_manager/gui_forms/tests/windows_message_queue_native_probe.cpp) and its target/include/language/link additions in [gui_forms/CMakeLists.txt](C:/Users/Shadow/file_manager/gui_forms/CMakeLists.txt). Applying the complete house style, the remaining issue is **failure cleanup and explicit ownership of injected input**, rather than spelling or retained callback state.

No edits, builds, tests or Git mutations were performed. Compilation/scanner success and native control outcomes remain root-owned evidence.

---

**Source acceptance for the corrected helper and deterministic fixture. The category-first ordering blocker and previously identified const-input/style issues are resolved. Native text-order conformance remains pending.**

Reviewed against `607d7624`:

- The input-preferred turn now consults the contiguous `WM_KEYFIRST..WM_MOUSELAST` range **before** `PM_QS_INPUT`. The ordinary fallback remains last. This corrects the identified path where a later hardware Backspace could bypass an already-posted translated character.
- The category fallback still gives input outside that range a preferred opportunity. The deterministic `WM_INPUT` case covers that branch.
- The new reader seam has named behavior and an explicit synchronous, non-retained callback/context contract. The native adapter uses no context; the reference adapter borrows its stack-owned queue only during the calls. No callback allocation or retained queue borrow is introduced.
- The reference fixture checks the intended sequence: earlier character, paired ordinary wake, then later Backspace ahead of unrelated remaining wakes. Its fixed records, initialized state and bounded traversal make the modeled ordering inspectable.
- Existing native posted-message and quit tests remain in place. Read-only inputs and named Boolean results address the prior style findings.

The deterministic fixture establishes that the helper implements the declared category policy and can distinguish the old and corrected retrieval orders. **It does not establish that real `TranslateMessage` output, hardware delivery, focus and text-control handling behave correctly together on a native desktop.** Its comments and output preserve that distinction. The rejected/accepted executions are root-reported; I did not inspect or reproduce those logs.

Exact follow-up scope: the diff to [windows_message_queue.hpp](C:/Users/Shadow/file_manager/gui_forms/src/host/windows/input/windows_message_queue.hpp) and [windows_message_queue_tests.cpp](C:/Users/Shadow/file_manager/gui_forms/tests/windows_message_queue_tests.cpp). No remaining house-style violations were identified in this follow-up scope. The earlier source acceptance of the two paint-bound union edits is unchanged.

No edits, builds, tests or Git mutations were performed.

---

**Do not accept `607d7624` as File Manager input conformance yet. The input-first helper has the ordering defect you identified, and its new fixture does not exercise that defect. The paint-bound changes are sound within the reviewed scope.**

1. **Blocking: `PM_QS_INPUT` is consulted before pending translated characters.**
   In [windows_message_queue.hpp](C:/Users/Shadow/file_manager/gui_forms/src/host/windows/input/windows_message_queue.hpp:10), the first successful category-filtered retrieval returns immediately. A pending hardware Backspace can therefore be dispatched before an earlier translated `WM_CHAR` that remains behind posted render wakes. The intervening ordinary turn does not repair this: it may consume another render wake rather than the character.

   `TranslateMessage` posts character messages into the thread queue; the category and range filters are not interchangeable. Microsoft documents the posted-character behavior and the distinct queue-category filtering. [TranslateMessage documentation](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage), [PeekMessageW documentation](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-peekmessagew).

   **The proposed range-first ordering addresses this specific defect:** consult the unrestricted-category `WM_KEYFIRST..WM_MOUSELAST` range before `PM_QS_INPUT`, then retain the ordinary fallback. This lets the matching posted character be retrieved before falling through to hardware-input category prioritization, while bypassing application-private render wakes. Preserve the broad range as one retrieval rather than splitting character, key and mouse messages into independently prioritized subranges.

   This is not a guarantee of global chronology across every Windows message category. The range intentionally also includes messages such as commands, timers and IME composition; the helper comment should describe that policy precisely.

2. **Blocking evidence gap: the fixture tests posted messages only.**
   [windows_message_queue_tests.cpp](C:/Users/Shadow/file_manager/gui_forms/tests/windows_message_queue_tests.cpp) uses `PostThreadMessageW` for both keyboard and mouse messages. Its manually posted `WM_CHAR` ordering check exercises the range fallback, not competition between `QS_INPUT` hardware input and an already-posted translated character. Consequently, the current test can pass with the defective category-first implementation.

   Retain the existing posted-message, ordinary-turn and quit checks, but add evidence for the actual mixed-category case before claiming text-order conformance. The relevant outcome is that typing a character followed by Backspace under render-wake backlog leaves the same text as the normal pump. Merely posting another `WM_KEYDOWN` does not establish that behavior.

3. **Remaining house-style corrections:**
   Mark read-only scalar inputs `const` in the new helper and fixture:
   - `take_window_message`: `input_first`.
   - `require`: `condition`, and the pointer value in `const char* const message`.
   - `post`: `message` and `value`.

   The types, initialized message records, named execution, fixed loop bounds and absence of retained borrows otherwise meet the reviewed requirements.

**Paint-bound assessment:** both rendering branches now add the complete `BeginPaint` update rectangle to existing logical damage before calling the retained painter. This preserves existing damage while including native exposures and device-pixel edges previously omitted whenever logical damage was already nonempty. The change retains each branch’s existing presentation/acknowledgement conditions for clearing damage.

The transactional branch’s early front-buffer/recovery presentation remains an existing bypass; it presents the native rectangle without entering the newly changed repaint path. I found no new blocker in these two damage-union edits. This is source acceptance of the union, not visual proof that all fractional-scale artifacts are fixed.

The pump still bounds retrieval to 32 turns and checks elapsed time after input/ordinary pairs. That provides scheduling opportunities, **not a hard latency ceiling**: individual handlers and synchronous message dispatch can exceed the nominal budget.

Exact reviewed scope is the four-file diff in `607d7624`:

- Queue-test target additions in [gui_forms/CMakeLists.txt](C:/Users/Shadow/file_manager/gui_forms/CMakeLists.txt).
- Both paint-bound edits and both pump edits in [windows_host.cpp](C:/Users/Shadow/file_manager/gui_forms/src/host/windows/application/windows_host.cpp).
- Full new [windows_message_queue.hpp](C:/Users/Shadow/file_manager/gui_forms/src/host/windows/input/windows_message_queue.hpp).
- Full new [windows_message_queue_tests.cpp](C:/Users/Shadow/file_manager/gui_forms/tests/windows_message_queue_tests.cpp).

I applied the complete house style and inspected surrounding paint, pump and key/character dispatch code for context. No edits, builds, tests or Git mutations were performed.
