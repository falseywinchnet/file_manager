# Dynamic titles and menu opening development receipt

Status: reviewed source candidate; native CI and matching public SDK acceptance
remain pending. This is not an exported capability or a stable C++ ABI promise.
Provider integration and canonical registration belong to the coordinator.

## Reviewed source scope

The bounded patch comprises these 19 files (relative to `gui_forms/`):

1. `include/gui_forms/application/application.hpp`
2. `src/core/application/application_state.hpp`
3. `src/core/application/application.cpp`
4. `src/runtime/application/application.cpp`
5. `include/gui_forms/platform/windows_host.hpp`
6. `include/gui_forms/platform/macos_host.hpp`
7. `include/gui_forms/platform/linux_host.hpp`
8. `src/host/windows/application/windows_host.cpp`
9. `src/host/macos/application/macos_host.mm`
10. `src/host/linux/application/linux_host.cpp`
11. `src/host/linux/accessibility/linux_accessibility.hpp`
12. `src/host/linux/accessibility/linux_accessibility.cpp`
13. `tests/application_contract_tests.cpp`
14. `tests/application_native_tests.cpp`
15. `include/gui_forms/components/context_menu/context_menu.hpp`
16. `include/gui_forms/controls/menu_strip/menu_strip.hpp`
17. `src/controls/menu/context_menu/context_menu.cpp`
18. `src/controls/menu/menu_strip/menu_strip.cpp`
19. `tests/menu_controls_tests.cpp`

Unrelated Details, preview, prepared-window and build-system changes in the
shared checkout are outside this receipt.

## Source semantics and lifetime

`ApplicationWindowHandle::set_title` borrows UTF-8 for the call only. Empty
titles are allowed; malformed UTF-8, embedded NUL and more than 65,536 bytes
are refused before native mutation. Wrong-thread calls are refused before
mutable host access. Expired handles report `after_shutdown`; unavailable
native callbacks are distinguished from a ready supported host. The runtime
publishes the callback during initialization and revokes it during teardown.
Callback exceptions become `backend_failure`; success means native submission,
not proof that pixels were presented.

Low-level title callback copies do not own their host. Windows callbacks observe
a weak state carrying the UI thread and HWND, revoked before destruction; they
never retain a raw WindowsHostState. macOS observes weak window/view references,
checks the main thread first and checks the view's host lifecycle before use.
Linux observes a weak native window and checks the creating thread before
mutable window state. All three adapters validate the same title bound and
contain callback failures. Direct Windows tests retain the callback across
shutdown and exercise Unicode, empty/maximum titles, invalid input and worker
thread refusal. Those native tests have compiled locally but have not run here.

`MenuOpenMode` distinguishes pointer and keyboard opening. Pointer opening
focuses the accessible menu container without choosing a row. Keyboard opening
retains first-enabled-row focus; navigation can choose the first/last enabled
row from container focus. Pointer hover transitions preserve pointer mode.
Invalid mode values throw before closing or replacing an existing popup,
including same-index MenuStrip requests. Tests verify open state, focus and
open-change notifications remain unchanged after invalid requests.

## Consumer rebuild requirement

The new title callbacks change the C++ WindowsHostOptions, MacHostOptions and
LinuxHostOptions layouts. ContextMenu::show and MenuStrip::open gain a mode
parameter, changing their compiled signatures even where a default argument
preserves source calls. Rebuild all libraries and consumers against the same
exported headers and binaries. Mixing old compiled consumers with these new
libraries is unsupported. No frozen installed SDK was edited.

## Validation and pending gates

Local Windows/MinGW build directory: `gui_forms/.build/windows-prepared-dev`.
Targets `gui_forms_application_native_tests`,
`gui_forms_application_contract_tests` and `gui_forms_menu_controls_tests`
compiled. The two headless contract/menu tests passed (2/2, 0.11 seconds before
the final test-parameter const correction). After that correction, both targets
rebuilt and `ctest --test-dir gui_forms/.build/windows-prepared-dev
--output-on-failure -R 'application_contract|menu_controls'` passed 2/2 in
0.17 seconds. An earlier anchored filter matched no tests and is not evidence
of a pass. No local desktop launch occurred.

Source review covered explicit types, named callbacks, const input borrows,
callback ownership/revocation, thread checks, failure preservation and operation
order under `planning/PROGRAMMING_HOUSE_STYLE.md`. The scoped spelling audit
identified the existing macOS display-tick block's `strongSelf->_displayTickQueued`
expression; that unrelated legacy expression was left unchanged. This receipt
does not certify the entire legacy host files against the house style.

Required before public SDK acceptance:

- Compile and run native application/title and menu coverage on Windows,
  macOS ARM64 and Linux, including the retained Windows low-level callback test.
- Confirm native Unicode title changes and shutdown behavior on each host;
  headless bridge results alone do not prove native presentation.
- Produce matching SDK exports and rebuild installed-package consumers.
- In SwiftEdit, adopt the public title API, remove the duplicate filename row,
  and verify open/save/dirty/untitled title transitions and pointer/keyboard menu
  behavior with native evidence before claiming delivery.

No physical pointer/keyboard responsiveness or compositor latency claim follows
from these source, compile or headless checks.
