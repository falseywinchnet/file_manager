# Native development archives

**GIVEN:** the owner requested Windows, macOS and Linux builds for download and
manual diagnostics on 2026-09-29. These are File Manager `0.001-alpha` dogfood
archives, not a Malkuth production release or installer promotion.

The GitHub Actions workflow `native-builds.yml` runs native Windows x64,
macOS arm64 and Ubuntu 24.04 x64 jobs. Each builds GUI.Forms from this checkout,
runs its CTests, installs an isolated SDK, builds and tests the frontend, builds
the Go Engine and Rust Orchestrator, then packages and checks a generated-empty-
root startup. The jobs publish archives only after those steps pass. Failed job
test/configure logs are separate artifacts. A successful process-liveness check
does not establish rendering, interaction, search, accessibility or daily-root
acceptance. The exact build revision, dirty state, platform, packaged file hashes
and test/startup result travel in `build-receipt.json`.

The build fingerprints installed SDK headers, libraries, runtime, resources and
configuration. A changed or unknown fingerprint forces a clean frontend build:
installation can preserve header timestamps, so timestamp-only incremental
compilation is insufficient when a public C++ class layout changes. The
fingerprint is checked after tests and again before/after package verification;
an SDK replacement during the run rejects the package attempt.

The owner/source coordinator controls Git commits, pushes and release uploads.
This workflow uploads Actions artifacts and does not create a public release.

## Local reproduction

Use native CMake, Ninja, C++20 tools, Python 3.11+, Go (see `engine/go.mod`) and
Rust (see `orchestrator/Cargo.toml`). The workflow lists native OS prerequisites.
Linux needs X11/XWayland; headless test runs use `xvfb-run`.

```powershell
. ./tools/Enter-WindowsToolchain.ps1
python tools/build_native.py --jobs 2
```

```sh
python3 tools/build_native.py --jobs 2
# Linux headless CI:
xvfb-run -a python3 tools/build_native.py --jobs 2
```

To reuse an already tested native SDK on Shadow:

```powershell
python tools/build_native.py --jobs 2 --gui-forms-sdk gui_forms/.build/shadow-sdk
```

Build outputs remain under `.build/native-<platform>-<arch>/`. The independently
installed picker package is under `frontend-sdk/lib/cmake/FileManagerDocumentPicker`;
it still requires the matching GUI.Forms SDK. Distributable archives are under
`dist/` inside that platform directory. Packages are staged in a new folder on
each attempt, preserving earlier attempts. No existing running executable or
DLL is overwritten. `--skip-components` is an explicit frontend-only development
option, and the receipt then lists no bundled services.

The CMake install component `DocumentPicker` can install just the public picker
libraries, headers and package configuration after focused picker tests. It does
not install a potentially stale application executable:

```sh
cmake --install .build/native-windows-x64/frontend --component DocumentPicker
```

## Package boundaries

- macOS: complete `.app`, private dylib closure rewritten to bundle paths, fonts
  and notices, ad-hoc signature verified with `codesign`. No Developer ID or
  notarization claim. The arm64 build does not promise Intel compatibility.
- Windows: executable, GUI.Forms and MinGW DLL closure, fonts and notices.
  Unsigned portable folder; no installer or system registration.
- Linux: executable, GUI.Forms `.so`, fonts and notices. Ubuntu 24.04/glibc
  baseline with system X11/ATK dependencies, checked by `ldd`. This is not a
  universal musl/AppImage package. No native Wayland host is claimed.
- Engine and Orchestrator executables are included but not installed, activated,
  assigned roots or configured by this package. Live negotiation remains the
  source of capability availability. Compilation alone does not promote search
  or durable settings.
- Portsmouth remains owner-supplied evaluation font material; its production
  redistribution-rights gate remains open. Original attribution accompanies the
  requested development archives. No new license claim is made.

## Returning diagnostics

Extract the archive completely and run its `collect_diagnostics.py`:

```sh
python3 collect_diagnostics.py
python3 collect_diagnostics.py --launch
```

Windows can use `python` instead of `python3`. The default report records only
build identity, basic OS/architecture facts and package file hash comparisons.
The optional launch opens a separate five-second application instance on a
generated empty root and captures at most 64 KiB of startup output. It never
reads personal file contents or scans a personal root; known home, package and
fixture prefixes are redacted. Review the JSON before sharing, together with a
manual description of the observed behavior. There are no uploads, directory
listings, credential collection or automatic crash-log searches. Reports use
exclusive creation and never overwrite an existing report.

## First native execution evidence, 2026-09-29

**MEASURED:** [CI run 36673458473](https://github.com/falseywinchnet/file_manager/actions/runs/36673458473)
built source checkpoint `ddade5b2614b` on native hosted runners. This first run
did not produce a verified distribution. Its failures are retained here:

| Target | Passing evidence | Stopping result |
|---|---|---|
| macOS arm64 | GUI.Forms 72/72 CTests, 13.33 s | Frontend file-clock/system-clock duration conversion rejected by libc++; explicit `time_point_cast` required |
| Ubuntu 24.04 x64 | GUI.Forms 67/67 CTests, 4.29 s | Frontend structured-binding range loop copied pairs; GCC `-Werror=range-loop-construct` required a const reference |
| Windows x64 | GUI.Forms 64/65 CTests | API-reference Python fixture used locale-default text encoding for a Unicode lambda; explicit UTF-8 required |

**MEASURED:** the later local Shadow build from that revision plus working-tree
changes passed all 11 frontend CTests in 3.28 s after a clean SDK-bound rebuild.
SDK content fingerprint:
`bf3e9cf14dfab5aeefbff0bf7456bb57f7bdba9b3c41e6f4b87512072db00961`.
Its generated-root packaged startup stayed alive for five seconds and archive
CRC/file-hash readback passed. The dirty archive identity does not claim a clean
source release. A fresh native CI checkpoint remains necessary for Mac/Linux
application artifacts and a common source revision.

**OBSERVED and corrected build hazard:** a changed SDK could preserve header
timestamps, causing an incremental build to combine old TextBox allocations with
a new DLL. The mixed build failed the picker-view and application tests; the
clean rebuild passed. The SDK content stamp and automatic clean rule above retain
the cause and prevent that incremental route.
