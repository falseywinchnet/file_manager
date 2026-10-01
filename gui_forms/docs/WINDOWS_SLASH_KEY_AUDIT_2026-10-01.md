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
