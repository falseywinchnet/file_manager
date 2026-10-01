# Windows slash/question shortcut audit

Date: 2026-10-01. **OBSERVED** source gap; no source change or installed support
claimed. Consumer request: SwiftEdit wildcard toggle via Ctrl+? plus a working
context-menu route. Consumer synthetic events currently use HID usage `0x38`.

The active Windows application host's `physical_key(WPARAM)` in
`src/host/windows/application/windows_host.cpp` handles letters, digits and
selected control/navigation/function keys; `VK_OEM_2` falls through to zero.
`WindowsHostState::key` calls this function for normal key events. The public
`events/input_events/input_events.hpp` names USB HID physical usages but omits
a slash constant (`0x38`). This confirms the consumer report for this path.

**CANDIDATE narrow next-SDK correction:** name the slash HID usage and translate
the applicable native slash key with focused down/up and modifier tests. Shared
host/input ownership and semantics require parent/registry agreement before
implementation. No D3a or dynamic-window approval grants that host edit itself.

Mapping `VK_OEM_2` to `PhysicalKey::slash` alone follows the current host's
virtual-key-derived approach; it does not prove physical-position invariance
or the character shortcut Ctrl+? across keyboard layouts. The adapter currently
does not use the event scan code in this translation. Adding a character-valued
shortcut identity or revising physical-key normalization is a distinct broader
contract, not silently part of this one-key fix.

Needed scope choice: a documented/tested US-layout slash/question development
route, or a separately negotiated layout-aware shortcut mechanism. Keep the
consumer's menu/context action usable; do not claim native Ctrl+? acceptance
from a synthetic HID event alone. Native tests must verify the active layout,
modifier state, down/up/repeat, unrelated OEM keys and whether translated text
is also emitted. Other host mappings need their own evidence. Any accepted
change goes into a new matched SDK, never the frozen existing prefixes.

## Subsequent bounded source correction

**OBSERVED:** parent authorized the small development correction under the current
Windows virtual-key-derived convention, explicitly limited to the supported US
Ctrl+Shift+slash profile. **OBSERVED:** `PhysicalKey::slash` now names HID `0x38`;
the actual host translator adds `VK_OEM_2` and is extracted into private
`src/host/windows/input/windows_key_translation.hpp` for direct testing. The
Windows host calls that same helper; down/up, repeat and modifier dispatch
remain unchanged. All existing letter/digit/control/navigation/function mappings
were preserved. Unknown OEM and keypad divide remain unadmitted as before.

**MEASURED:** GCC 16.2 Windows Release build of `gui_forms_host_windows` and
`gui_forms_windows_key_translation_tests` succeeded. Focused CTest passed 1/1
in 0.04 seconds (0.07 seconds total). Tests invoke the actual extracted mapper,
check the new slash usage and preserve existing mappings. Parent review removed
synthetic event-construction assertions because they did not exercise dispatch.
These tests do **not** exercise OS keyboard delivery, actual
layout state, WM_CHAR suppression or SwiftEdit's command dispatch.

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake --build gui_forms/.build/shadow-windows `
  --target gui_forms_windows_key_translation_tests gui_forms_host_windows --parallel 2
ctest --test-dir gui_forms/.build/shadow-windows `
  -R '^gui_forms_windows_key_translation_tests$' --output-on-failure
```

Full `planning/PROGRAMMING_HOUSE_STYLE.md` semantic review covered the extracted
helper and test, the single public constant, the host include/call replacement
and removed duplicate function, and the four CMake test lines. Explicit types,
bounded arithmetic/narrowing, named behavior, unchanged input event ownership
and absent retained callbacks/storage were checked. No remaining violation was
identified in these edits; the surrounding legacy event/host code is not claimed
compliant. Two-file spelling scan returned zero; strict authored syntax check
passed with public dependency headers treated as system includes to exclude
their previously recorded unrelated warnings. `git diff --check` passed.

Parent independently compared the extracted helper with the old switch and
reviewed the authored scope above. After removing synthetic construction-only
assertions, the parent rebuilt the test and host targets, reran focused CTest
(1/1 passed, 0.05 seconds total), and reran the two-file spelling scan (zero).

No SDK installation occurred. Matching package/consumer rebuild and native
US-layout shortcut acceptance remain pending; universal character Ctrl+? remains
outside this correction. The initial audit above is preserved as pre-change
evidence, not a claim that the missing case remains missing in current source.
