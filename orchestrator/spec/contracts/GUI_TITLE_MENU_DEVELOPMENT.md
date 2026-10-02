# ORC-GUI-001 — Dynamic title and menu opening development

Date: 2026-10-02. Status: **reconciled source candidate; native conformance and
matching SDK consumption pending**. No stable ABI or daemon operation is added.

## Meaning and ownership

`ApplicationWindowHandle::set_title` is an owning-UI-thread request. It borrows
UTF-8 only for the call. Empty titles are admitted; malformed UTF-8, embedded
NUL, and titles larger than 65,536 bytes are rejected before native mutation.
Closed handles report `after_shutdown`; absent callbacks and temporarily
unready native windows remain distinguishable. Acceptance means the host
accepted a title request, not that pixels or an accessibility client observed
it. A callable remains alive through reentrant handle revocation.

Native callbacks retain weak host lifetime state, validate thread affinity
before mutable host access, and refuse use after native closure or host return.
Windows revokes its HWND state before destruction; macOS uses weak native
window/view state and session phase; Linux uses weak native window state and
updates the accessibility name together with X11 title properties. Native
failure and allocation/encoding failure must not escape the public status path.

`MenuOpenMode` distinguishes keyboard opening (first enabled row selected) from
pointer opening (focus on the accessible popup container, no row selected).
Navigation can subsequently select an enabled row; Escape closes an otherwise
empty or all-disabled popup. Checked command state is independent of focus.
Invalid opening modes are rejected before closing or reusing an existing menu.
Pointer hover between top-level menus preserves pointer opening behavior.

## Compatibility and acceptance

Host option layouts and menu method signatures change. All C++ consumers must
rebuild against one coherent set of headers and libraries; an old installed
SDK cannot be repaired by copying individual new libraries into it. Frozen
FM0 manifests and executable availability remain unchanged.

The reviewed scope and local tests are recorded in the
[provider receipt](../../../gui_forms/docs/TITLE_MENU_DEVELOPMENT_2026-10-02.md).
Native Windows/macOS/Linux tests, an independently rebuilt consumer, and
visible/accessible title and menu behavior remain acceptance gates before
claiming portable delivery. Source review does not establish those results.
This source-development addition is reversible before SDK publication; after
publication, removal requires a documented migration and rebuilt consumers.
