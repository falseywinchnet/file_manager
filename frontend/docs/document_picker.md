# Document Picker public consumer workflow

Status: **OBSERVED implementation; MEASURED focused Windows controller/view tests**
(2026-09-29). This is a source package rebuilt with the same GUI.Forms SDK as its
consumer, not a frozen C++ binary ABI. The application release remains alpha.

Link only the installed package:

```cmake
find_package(FileManagerDocumentPicker 1.0 REQUIRED CONFIG)
target_link_libraries(my_app PRIVATE FileManager::DocumentPickerView)
```

`DocumentPicker` is the controller-only target. `DocumentPickerView` adds the
public retained GUI.Forms view. Neither links the File Manager executable.
The standalone probe in `../examples/document_picker_consumer` has no source-tree
include or private dependency. Configure it with both installed prefixes:

```powershell
cmake -S frontend/examples/document_picker_consumer -B frontend/.build/picker-consumer -G Ninja -DCMAKE_PREFIX_PATH="C:/path/to/frontend-sdk;C:/path/to/gui-sdk"
cmake --build frontend/.build/picker-consumer --parallel 2
ctest --test-dir frontend/.build/picker-consumer --output-on-failure
```

On Windows, put the matching GUI.Forms and MinGW runtime DLL directories on PATH
or stage that dependency closure beside the consumer executable. The build owner
publishes the exact prefix for the current coherent SDK checkpoint.

## Request and authority

Create a `DocumentPickerRequest` with nonempty `owner_application_id`, explicit
`protected_root`, purpose/profile and initial location. `admitted_roots` contains
additional roots explicitly admitted by the host, such as its chosen local drive
roots. The view does not infer drive authority. `home_location` is only a navigation
hint and must resolve within those roots; otherwise Home is disabled.

**DECIDED in the current Orchestrator reconciliation:** a trusted first-party host
may explicitly set `authority = DocumentPickerAuthority::trusted_local_host` for
an owned, in-process local selection lifetime. This does not claim a live daemon,
Engine, plugin capability, access grant or file I/O. Unspecified authority remains
unavailable. The historical `orchestrator_session_valid` bool defaults false and
maps only an explicit verified caller input to the daemon-session source.

The caller sets visibility independently for each application using `show_hidden`.
`allow_hidden_toggle` controls whether a session may change it. A toggle does not
persist a preference or change another application's policy. Empty filters (or an
empty extensions list) admit all files, including extensionless names. Save As
with an empty default extension and all-files filter does not append an extension.
The filename filter supports ASCII-insensitive `*` and byte-sized `?`, combined
with the type filter. Directories remain visible for navigation.

## Owned dialog lifecycle

Keep the `DocumentPickerView` alive longer than its host `Window`. Construct that
window with `view.root_control()`, then call `view.attach_dialog(window)` to set
its default/cancel actions and initial focus. Use a minimum size of 540 x 400.
The host supplies a GUI.Forms `ApplicationWindow` with its real `owner_id`;
GUI.Forms owns native window mechanics. The host suppresses its document controls
while the picker is open, retains the previous focused control, and restores it
when hiding/closing the picker. Sharing the picker does not authorize persistent
panels in the host application.

For each new owned presentation:

```cpp
view.set_authority_valid(true); // same explicit source and scope, new host lifetime
(void)view.controller().set_filename(proposed_name); // Save As only
view.present(initial_directory);
// Show the host-owned picker window.
```

`present` refreshes navigation and initial focus, but never renews an invalid grant.
Owner close or picker-window close calls `view.cancel()`. Accepted/cancelled results
revoke the presentation's authority and are emitted once. Restoring authority
alone cannot reopen a completed presentation. Destroy/hide the host dialog only
for accepted/cancelled results; validation and overwrite requests remain open.

`completed()` also reports validation errors and
`overwrite_confirmation_required`. For the latter, the host presents its owned
confirmation UI showing the returned destination. Only an affirmative decision
calls `view.confirm_overwrite()`. The controller compares the newly observed
identity and revision with the observation that requested confirmation. A changed
destination requires a fresh decision.

Ctrl+L focuses location, Alt+Up enters its parent, Enter activates a file/folder or
the default action, Escape cancels through GUI.Forms' dialog routing, and Tab uses
GUI.Forms retained focus traversal. A path edit resolves relative to the current
folder. A selected directory in Open/Save navigates instead of returning a file.

## Filesystem and save boundary

Acceptance rechecks the current directory identity and selected object revision,
refuses link routes/out-of-root paths, enforces profile cardinality and validates
the final save basename after extension application. A selected file copied into
the Save As filename field remains an observation, not a write reservation.
For an explicit, current `trusted_local_host` grant, directory navigation may
resolve a symbolic-link route to an existing canonical directory within an
already admitted root. Folder-link rows remain visible through file filters and
can be entered using Open. The browser then displays the canonical location;
selection and save observations refer to that location, not the alias. Broken
links, targets outside admitted roots and revoked grants do
not gain this behavior. Orchestrator-session navigation retains its no-link
policy. This directory-navigation exception does not permit following a save
target link or weaken overwrite identity checks.
For the same current trusted-local grant, Open File, Open Files and Import Files
may select a visible file alias. Acceptance rechecks the alias revision, resolves
an existing regular target within the admitted roots and returns the canonical
target path and freshly observed target identity. Filters and hidden-file policy
apply to the displayed alias; a target's different basename does not silently
change that selection. Broken, cyclic, outside-root or changed aliases refuse.
One failed item refuses the whole selection. Save/Export and the daemon-session
projection continue to refuse file-link leaves. No result reserves a file or
allows a consumer to fall back to the unresolved alias during its own I/O.
The host revalidates immediately before reading/replacing and owns parsing,
encoding, atomic-write behavior and its final collision policy. No file contents
are read or written by the picker.

## Evidence and remaining work

**MEASURED:** focused controller/view tests on the Shadow Windows MinGW build
pass for local-host/default-unavailable/revoked authority; ordinary open, multiple
and folder profiles; stale selection; filename/type/hidden policy; extension and
path validation; overwrite revision binding; owned presentation reopen/cancel;
folder navigation; Ctrl+L, Alt+Up, Escape; and compact Save As geometry.

These headless tests do not establish native window ownership/focus, screen-reader
acceptance, high contrast or visual parity on three platforms. Native consumer
verification belongs with newNotepad and PlanPaint. Enumeration still uses the
existing synchronous directory snapshot: slow/network or very large directories
need a separately measured cancellable/paged read lane before latency claims.
Create-folder, previews, search, recents and native fallback execution remain
absent. `allow_create_folder` and `allow_native_fallback` are not implementation
claims. The local-host route requires neither indexing nor a running daemon.

## Installed consumer receipt (Shadow, 2026-09-29)

**MEASURED:** the standalone `frontend/examples/document_picker_consumer`
configured from scratch using only
`C:/Users/Shadow/file_manager/.build/native-windows-x64/frontend-sdk` and
`C:/Users/Shadow/file_manager/gui_forms/.build/shadow-sdk`, linked successfully
with MinGW GCC 16.2.0, and passed its independent CTest (1/1). The generated
consumer has no frontend source include paths and does not link File Manager.
The probe exercised extensionless open, host identity revalidation, reopen,
Escape cancellation and duplicate terminal suppression. Its generated evidence
is `frontend/.build/picker-consumer/Testing/Temporary/LastTest.log`.
This proves independent installed consumption on this Windows environment,
not native owner-window behavior or a cross-platform release.
