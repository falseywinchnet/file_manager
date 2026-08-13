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
