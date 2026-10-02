# File Manager frontend

Status: **0.001-alpha under active repair. The owner rejected the premature
File Manager 1.0 claim; real-root navigation and House Composite fidelity are
being dogfooded before any later promotion**.

This directory contains the C++ application called **File Manager**. It
normally consumes Orchestrator as the integration/policy authority while using
GUI.Forms in-process. The systemwide engine remains available through
Orchestrator and through a registered degraded fallback:

- [`../gui_forms/`](../gui_forms/) — retained cross-platform GUI framework;
- [`../engine/`](../engine/) — systemwide exact/lexical/Kolmogrov-ready Go file
  index and degraded fallback;
- [`../orchestrator/`](../orchestrator/) — Orchestrator, Rust contract authority,
  control plane, hives, settings, CLI, handlers, plugins and integration broker.

Orchestrator Core 1.0 is ready, GUI.Forms published the named
`gui-forms-fm0-macos-arm64-2026-08-10` installed snapshot, and the grand
architect supplied explicit implementation/M4 dogfood direction on 2026-08-10.
Product startup uses the live Orchestrator; deterministic Core fixtures remain
test doubles only. Engine and later providers open through their own negotiated
contracts. Orchestrator itself has no GUI.

The current protected-root prototype compiles the checked-in browser-valid
Web.Forms source into a retained public GUI.Forms tree and has no browser or
Python runtime. It supplies asynchronous navigation, paged installed-Engine
search through Orchestrator, typed settings/service control, bounded text/PNG
previews, identity-checked default Open, fixed-argv Terminal Here, streamed
SHA-256 with cancellation/expected-digest comparison, clipboard path/digest
copy, and explicit-opt-in recoverable operations. Read-only remains the
default. The mutation profile requires a separate same-volume quarantine and
implements New Folder, inline Rename, internal drag, staged no-overwrite Copy,
same-volume Move, two-step recoverable Delete, and one-step identity-checked
Undo under ADR-017 and ADR-020.

The repair removes the rejected synthetic navigation and rebuilds toward the
accepted dense House Composite geography. `Places` and `Recent locations` are
absent from both authored and native trees; Home and Volumes are the honest
daily navigation roots. The rejected build remains documented under
[`planning/OWNER_CORRECTION_2026-08-11.md`](planning/OWNER_CORRECTION_2026-08-11.md)
and the first dogfood record; it is not counted as 1.0 evidence.

`FileManager::DocumentPicker`, `FileManager::DocumentPickerView`, and their
shared `FileManager::FrontendModel` are independently installed CMake targets.
The bounded GUI.Forms picker supports open-one, open-many, folder, save-as,
import, and export profiles without linking the File Manager executable.
Ordinary picker browsing does not require Engine; acceptance revalidates both
the filesystem observation and the Orchestrator selection-session state.

Native Windows development build (Shadow, 2026-09-29):

```powershell
./tools/Build-Windows.ps1 -Component Toolkit -Jobs 2 -Test
./tools/Build-Windows.ps1 -Component Frontend -Jobs 2 -Test
```

Run `frontend/.build/shadow-windows/File Manager.exe` from the resulting
build directory. CMake stages the GUI.Forms DLL, its MinGW runtime dependency
closure, fonts, and dependency notices beside the executable. This is a
local development layout, not an installer or a release-ready distribution.
The current ergonomic changes and native verification record are in
[`planning/DAILY_BROWSING_2026-09-29.md`](planning/DAILY_BROWSING_2026-09-29.md).
The measured Windows performance repair is staged separately at
`frontend/.build/shadow-windows-latency/File Manager.exe` so an already-running
older build can remain open. See
[`results/2026-09-29-shadow-windows/LATENCY.md`](results/2026-09-29-shadow-windows/LATENCY.md)
for the same-application before/after results and remaining performance limits.
The Windows manifest is
`gui-forms-shadow-windows-x64-2026-09-29`; the original macOS snapshot remains
separate. The application version remains `0.001-alpha`, while the independently
consumed Document Picker CMake package retains its existing 1.0 contract version.

The Windows adapter uses the profile directory for Home and exposes actual
fixed-drive roots. Paths cross the UI boundary as UTF-8 and the OS boundary as
UTF-16. File identity retains the Windows volume identifier and all 128 file-ID
bits; reparse routes are refused by bounded reads and navigation. Default Open
uses the Windows association API. Terminal Here opens Command Prompt with
AutoRun disabled and the selected directory supplied as the process working
directory, without inserting a path into shell command text. Open/terminal
launch-plan tests do not establish native handler or terminal workflow acceptance.

Windows has an explicit-process authenticated named-pipe route for Orchestrator
and Engine, including the packaged `launch_windows_search.ps1` launcher. Bundling
those components does not activate a service or admit an indexing root; ordinary
launch still reports actual negotiated availability. See the
[Windows Engine deployment profile](../engine/docs/WINDOWS_LOCAL_DEPLOYMENT.md)
and [Orchestrator projection](../orchestrator/spec/WINDOWS_LOCAL_PROJECTION.md).
Read-only remains the default and the protected mutation profile is explicit
opt-in. Installed service management, settings and platform promotion retain
their documented gates.
The authored Web.Forms controls and retained GUI.Forms composition are preserved;
Windows currently uses its standard native outer frame. Native visual and
accessibility parity, removable-drive handling, and daily mutation promotion
remain outstanding. Detailed evidence and exclusions are in
[`results/2026-09-29-shadow-windows/README.md`](results/2026-09-29-shadow-windows/README.md).

Build on the M4 after installing the named GUI.Forms package:

```sh
/Users/ultimussecundai/.local/bin/m4build -- /bin/sh -c '
  /opt/homebrew/bin/cmake -S frontend -B frontend/build \
    -DCMAKE_BUILD_TYPE=Release \
    -DGUIForms_DIR="$PWD/gui_forms/.build/fm0-install/lib/cmake/GUIForms"
  /opt/homebrew/bin/cmake --build frontend/build --parallel 10
  /opt/homebrew/bin/ctest --test-dir frontend/build --output-on-failure
'
```

After a green M4 build, promote the exact bundle into the non-repository
dogfood area. This preserves earlier candidates, ad-hoc signs and verifies the
new copy, records its executable hash in an atomic current-candidate manifest,
and installs three plainly separated launchers:

```sh
/bin/sh frontend/tools/stage_m4_dogfood.sh \
  "frontend/build/File Manager.app"
```

Run `~/Developer/CodexRuns/run-file-manager-daily.command` from Terminal inside
the M4 Screen Sharing desktop for ordinary read-only Home and Volumes browsing.
Use `run-file-manager-protected-read-only.command` for the contained
`fmsandbox` plus `fm1-contained` Engine profile. Each launcher refuses a
missing, hash-mismatched, or invalidly signed candidate before asking
LaunchServices to open it.

A disposable mutation run must use the separately named
`run-file-manager-mutation-sandbox.command`, or opt in explicitly at the
command line:

```sh
open -n "frontend/build/File Manager.app" --args \
  --root /absolute/disposable/root \
  --allow-mutations \
  --quarantine /absolute/separate/same-volume/quarantine
```

Launching the bundle, rather than invoking its binary through a symlink, is
required so the host resolves bundled fonts and resources correctly.

An external first-party picker consumer can use the installed package:

```cmake
find_package(FileManagerDocumentPicker 1.0 REQUIRED CONFIG)
target_link_libraries(my_app PRIVATE FileManager::DocumentPickerView)
```

Corrective control audits, cross-component test results, and M4 Screen Sharing
observations are recorded under
[`results/2026-08-11-m4-dogfood/`](results/2026-08-11-m4-dogfood/). The
[`2026-08-10 record`](results/2026-08-10-m4-dogfood/) remains the rejected
baseline and negative evidence.

The exact first slice and gate are in
[`planning/FRONTEND_001.md`](planning/FRONTEND_001.md) and
[`planning/DEPENDENCY_GATES.md`](planning/DEPENDENCY_GATES.md). Its visual
constitution is
[`planning/visual/DESIGN_DNA_006.md`](planning/visual/DESIGN_DNA_006.md).
The total implementation and integrated dogfood program is
[`planning/TOTAL_IMPLEMENTATION_PLAN.md`](planning/TOTAL_IMPLEMENTATION_PLAN.md).
