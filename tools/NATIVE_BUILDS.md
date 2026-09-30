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
