# Native Windows frontend development receipt — 2026-09-29

Status: **MEASURED native build and 11/11 CTest executables passed; OBSERVED
initial native UI launch; final visual navigation acceptance remains open.**

## Authority and scope

**GIVEN:** direct development in the Shadow checkout, native Windows navigation,
retained GUI.Forms, existing Web.Forms authorship, component ownership, and
truthful degraded services. This work owns `frontend/` only. The parent chat
owns the GUI.Forms SDK/build setup; the Orchestrator chat owns its C++ client.
No services were installed, personal files mutated, commits made, or additional
chats/workers created by this frontend work.

The existing accepted visual composition and macOS host branch remain. The
Windows adapter uses the portable public Application API with the parent-added
wake/drain bridge. Native Windows outer chrome is standard; the authored
application content is retained. The app remains `0.001-alpha`; the separate
Document Picker CMake package retains its 1.0 contract version.

## Artifact and reproducible build

From the repository root:

```powershell
./tools/Build-Windows.ps1 -Component Toolkit -Jobs 2 -Test
./tools/Build-Windows.ps1 -Component Frontend -Jobs 2 -Test
```

**MEASURED:** GNU C++ 16.2.0 / MinGW64, CMake/Ninja, Windows x64 on Shadow.
The installed SDK is `gui_forms/.build/shadow-sdk`, identified by
`gui-forms-shadow-windows-x64-2026-09-29`.

Build product: `frontend/.build/shadow-windows/File Manager.exe`.
Its PE subsystem is **Windows GUI**, so the final target does not require a
companion console. CMake stages the dependency closure, fonts and notices in
the same directory. This is a development layout, not a public installer.

| Artifact | SHA-256 |
|---|---|
| File Manager.exe | `8cd5973217c5a208c1668bf0adf3c61499b526fa733a0a5e63b2bbb21a5d59ae` |
| libgui_forms_application.dll | `82d9d2e31d11ee4a0c848835b5f36a9ab558c7a7dd14eb8736bc46bdfcc8664a` |

The final staged GUI.Forms hash matches the parent's accessibility-fixed SDK.
The additional staged runtime DLLs are `libgcc_s_seh-1.dll`, `libstdc++-6.dll`
and `libwinpthread-1.dll`. GUI.Forms and runtime license files are staged under
`licenses/`; font notices accompany `fonts/`.

## Implementation evidence

**OBSERVED:** Windows identity uses an opened no-follow handle, the volume
identifier and all 128 file-ID bits. Preview and checksum readers inspect the
same handle before and after reading and revalidate the selected path. All
reparse points are classified as links, including junctions. Hidden attributes
are respected. Unsupported identity observation remains unavailable.

**OBSERVED:** Home uses the Windows profile folder; native fixed-drive roots
are individually admitted. Explicit launch roots remain independent roots even
when Windows TEMP lies below Home. UI path text is UTF-8 and native OS calls
use UTF-16. Picker/rename basename validation excludes Windows separators,
alternate-stream syntax and reserved device names. Removable and remote drive
discovery are not promoted by this tranche.

**OBSERVED:** default Open uses `ShellExecuteExW` with an exact selected path.
Terminal Here uses the system `cmd.exe /D` with the directory passed separately
as `lpCurrentDirectory`; no user path enters shell input. Plans revalidate
identity and reject tampering. These are implementation and plan-test facts;
actual handler/terminal behavior was not visually accepted in this run.

**OBSERVED:** the Orchestrator Windows C++ client explicitly reports live IPC
unavailable. The application does not synthesize bootstrap success. Local
navigation remains available; indexed search, settings and service controls
retain their provider-dependent degraded states. Mutation remains the existing
explicit protected-root opt-in; this is not normal recycle-bin/undo promotion.

## Tests and retained failures

**MEASURED:** final combined CTest: **11/11 executables passed in 3.15 seconds**.
The complete per-executable output is [ctest.txt](ctest.txt).

Coverage includes model navigation, protected operations, internal drag,
checksum, launch plans, preview, picker controller/view, House art, and retained
application interaction. The Windows-specific suite additionally covers Unicode
path round trips/enumeration, UTF-8 preview, SHA-256, rename-stable identity,
hardlinks, upper identity bits, hidden attributes, real NTFS junction refusal,
launch-plan tampering and invalid Windows basenames.

The test log reports **10 skipped symlink fixture setups** with Windows error
1314 (`ERROR_PRIVILEGE_NOT_HELD`). Only assertions dependent on those fixtures
are bypassed; the rest of each suite executes. A separate real junction fixture
runs without elevation and verifies no-follow navigation and identity. File
symlink-copy/undo behavior is therefore not measured on this host. No privileges
or machine settings were changed to make a test pass.

Earlier runs retained useful failures:

- MinGW `std::filesystem::create_symlink` reported unimplemented. Tests now use
  the native Windows creation API and explicitly disclose privilege absence.
- TEMP inside Home prevented the explicit launch root from being retained as a
  tree root. Admission now retains that explicit root.
- Joining a root to `.` left a trailing separator that MinGW's directory
  equivalence check rejected. Native launch plans now remove that redundant
  separator while retaining drive roots.
- A test tried to expand Help while Edit owned the popup focus scope. It now
  dismisses Edit before invoking Help, preserving the toolkit's focus boundary.
- Exception cleanup could release an application on its worker thread and
  attempt a self-join. Main owns an explicit stop guard; interaction tests stop
  workers during unwinding before releasing their application owner.

**MEASURED diagnostic only:** directly enumerating the exact reported
`C:/Users/Shadow` → `C:/Users/Shadow/file_manager` location returned 22 entries,
no error, in 12 ms. This single observation is not a performance benchmark.

## Native UI observation and unresolved edge

**OBSERVED by the parent chat:** the earlier candidate launched with the
Sapphire/House authored interface, 16 real Home folders, explicit Orchestrator
unavailability and no indexed-root claim. The first attempted nested navigation
then appeared stuck; the process consumed CPU and stopped responding. That
failure is retained rather than counted as successful navigation acceptance.

The Paint chat separately captured a shared GUI.Forms MSAA self-focus loop.
The parent integrated its `CHILDID_SELF` fix, reran the toolkit's 64 native tests
successfully, and supplied the final DLL staged here. This is a plausible shared
cause of the observed frontend hang, **not a proved post-fix frontend workflow**.
The user stopped Computer Use with Escape; no further desktop automation or
visual relaunch was performed by this frontend work.

Final build/tests pass, but post-fix visual navigation, repeated owned-window
opening/closing, default handler launch, Terminal Here, clipboard/drop and
cross-platform visual/accessibility parity remain unverified. macOS paths are
preserved in source but were not rebuilt on the M4 during this Windows run.
