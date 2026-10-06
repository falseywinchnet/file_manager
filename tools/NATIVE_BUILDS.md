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

CI explicitly selects Clang for C/C++ on all three platforms (Apple Clang on
macOS). Windows uses MSYS2 **CLANG64**, LLVM/LLD, libc++ and UCRT. Install the
`mingw-w64-clang-x86_64-{clang,cmake,ninja,python}` packages in MSYS2, then set
`FILE_MANAGER_CLANG_BIN` to that installation's `clang64/bin` directory. The
PowerShell helper otherwise looks in the adjacent Plan Paint toolchain's
`build-deps/msys64/clang64/bin`; it does not modify that toolchain.
`Build-Windows.ps1` uses fresh `shadow-windows-clang` build directories and
`shadow-clang-sdk` so existing GCC artifacts are preserved. Rebuild all C++
consumers with the same toolchain: old GCC/libstdc++ SDKs are not ABI compatible.
Before reusing the native pipeline's existing `.build/native-windows-x64` or
Linux build directory, move that directory aside if it was compiled with GCC;
CMake caches compiler selection. NSIS is not used: Windows output remains a
portable ZIP.

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
python tools/build_native.py --jobs 2 --gui-forms-sdk gui_forms/.build/shadow-clang-sdk
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
- Windows: executable, GUI.Forms and CLANG64 DLL closure, fonts and notices.
  Unsigned portable folder; no installer or system registration.
- Linux: executable, GUI.Forms `.so`, fonts and notices. Ubuntu 24.04/glibc
  baseline with system X11/ATK dependencies, checked by `ldd`; `xdg-utils` supplies
  default Open. Terminal Here remains unavailable pending an admitted terminal
  launch contract. This is not a
  universal musl/AppImage package. No native Wayland host is claimed.
- Engine and Orchestrator executables are included; extraction does not install,
  activate or assign roots to them. Windows packages include the separately
  invoked `launch_windows_search.ps1` and its instructions. The user must choose
  a root; catalogue creation requires a separate opt-in. The launcher owns and
  stops only its new processes, and retains private local state/logs for review.
  Live negotiation remains the source of capability availability. Compilation
  alone does not promote search or durable settings.
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

**MEASURED Mac baseline:** [CI run 36674658657](https://github.com/falseywinchnet/file_manager/actions/runs/36674658657)
source `21a89b51f89333ee1dd9626e1eeb15b9c8208fd4` produced a clean macOS arm64
archive. GUI.Forms passed 73/73 CTests in 13.46 s and the frontend passed 11/11 in
1.38 s. Ad-hoc signature verification, generated-empty-root five-second startup,
archive readback and an independent downloaded-archive check of all 42 receipt
file hashes passed. Archive SHA-256:
`0951e0c7048acdf957a2559fdf9753f2e90fbe54f5847bd0606ee68eb133cce6`.

**OBSERVED in those actual Mach-O bytes:** the application, GUI.Forms libraries
and Engine each declare macOS 26.0 in `LC_BUILD_VERSION`; Orchestrator declares
11.0. The plist has no `LSMinimumSystemVersion`. Every executable/library is
arm64. The package therefore requires **macOS 26 or later** and startup was
measured on **macOS 26.6.2**; earlier OS compatibility is not established. Later
packages record the load-command/plist minimum explicitly in their receipt.
This baseline predates the subsequent DPI and service-launcher corrections.

## Published three-platform dogfood snapshot

**MEASURED:** [native run 36679615538](https://github.com/falseywinchnet/file_manager/actions/runs/36679615538)
passed at `342c42a48bc18532995d9bd5cf5854625a45bf98`:

| Native target | GUI.Forms | Frontend |
|---|---:|---:|
| Windows x64 | 66/66, 9.32 s | 11/11, 1.59 s |
| macOS arm64 | 73/73, 28.78 s | 11/11, 1.74 s |
| Ubuntu 24.04 x64 | 68/68, 5.17 s | 10/10, 0.63 s |

The [published prerelease](https://github.com/falseywinchnet/file_manager/releases/tag/v0.001-alpha.20260930)
contains clean archives at that common source revision. Mac/Linux bytes come
from native CI; Windows bytes come from the clean Shadow build with the tested
local SDK. That Windows archive passed 11/11 frontend tests, all 44 extracted
file hashes, a five-second startup with only Windows system directories on
PATH, and both live/indexed explicit service-launch checks. Independent
archive checks matched 42 Mac files and 40 Linux files and preserved executable
permissions. Uploaded GitHub asset digests match these SHA-256 values:

| Archive | SHA-256 |
|---|---|
| Windows x64 | `49fb74b224d71a8d0f99db1849550996f524642c873baaa8da38e6660a2bc856` |
| macOS arm64 | `d0a1c571f2dc30ebed83e445b90b50b2ad5587195a53812089b582c87197fda9` |
| Linux x64 | `3235fd722b6d648d247516b0e712c4ea80d82936ebfc616251040855d9ec49fa` |

**OBSERVED / retained failure:** Windows CI's archive reported a dirty checkout
and was excluded, never relabeled. Checkout used Git for Windows with system
`core.autocrlf=true`; MSYS Git used a separate system configuration. On the same
pristine checkout, Git for Windows reported zero changed paths, MSYS Git
reported 3,038, and MSYS Git with the matching policy reported zero. An
ignore-EOL comparison found no content differences. With actual tooling edits,
both implementations under the matching policy reported those same dirty
paths. The workflow now records the checkout-local policy before the MSYS
build. Build/package source-state JSON records the Git implementation, relevant
core policy, status and diff statistics; receipts retain a bounded status
summary. It does not suppress real modifications or query authentication
configuration. This follow-up does not change the published archive bytes.

**OBSERVED / corrected runtime gaps:** native Windows CI exposed MinGW's
incomplete symlink observation and unimplemented `read_symlink`. Operation-root
admission now observes native no-follow identity; link copying reads bounded
reparse data through a native handle and preserves dangling targets and
directory-link kind. Unknown reparse tags are refused explicitly. Unconditional
parser tests run without symlink privileges; the hosted runner passed the real
link-copy/collision/undo cases that Shadow's token cannot create. Linux tests
now reflect the admitted `/` root and explicitly unavailable Terminal Here;
default Open uses `xdg-open`.

These remain development archives, not production installers or general
mutation/platform-accessibility promotion. Mac minimum remains 26.0.

## House-style correction audit, 2026-09-30

**GIVEN:** the owner rejected the delegated implementation's failure to apply
`programming-house-style (2).md` and directed a complete correction within the
assigned build/export and API-reference tooling scope. This source correction
neither updates the published `342c42a` archives nor replaces the frozen
`dbe3766` SDKs. Their historical evidence above remains unchanged.

**OBSERVED baseline:** the seven original Python files (`build_native.py`,
`package_native.py`, `collect_diagnostics.py`, GUI.Forms `api_reference.py`,
`api_reference_render.py`, `verify_install.py`, and `api_reference_tests.py`)
contained 7 lambdas, 42 comprehensions/generator expressions and 43 procedure
interfaces without complete input/result annotations. Additional findings were
captured mutable maps in nested generator helpers, repeated header reads during
AST traversal, whole-member archive reads, child cleanup limited to the normal
timeout path, anonymous PowerShell filtering, and anonymous emitted JavaScript
search predicates. These are implementation findings, not merely formatting.

The correction gives synchronous command/JSON boundaries explicit types; moves
compiler extraction and reference rendering state onto named owners; preserves
UTF-8 byte-offset handling; uses named loops and bounded reusable hash scratch;
closes archive-member streams explicitly; and reaps only the acquired test child
on both normal and exceptional exits. PowerShell PATH selection now uses explicit
ordered collection traversal and named state. CI compiler-identity code lives in
a named script rather than an embedded untyped block. The new Python modules are
private tooling, not a new runtime or cross-project contract.

**MEASURED focused checks on Shadow:**

- `python -B tools/native_build_tests.py`: 8 tests passed. Generated fixtures
  cover fingerprint compatibility and changed bytes, bounded stream reads,
  ZIP/tar readback and negative hashes/permissions, diagnostic containment and
  exclusive report creation, timeout escalation, exception cleanup, and reaping
  a real owned Python child. No File Manager process or SDK was opened.
- `python -B gui_forms/tests/api_reference_tests.py`: 5 tests passed using the
  native Clang fixture. Against saved original generator source, the same fixture
  inventory and all 24 unchanged emitted artifacts were byte-identical, including
  reviewed contract sections and the UTF-8 source example. The checked-in golden
  hashes use canonical UTF-8 text/newlines so the fixture can run across hosts.
- `python -B gui_forms/tests/verify_install_tests.py`: 2 tests passed. Named
  command fakes verify configure/build/test/rejected-component order, relocation
  and emulator arguments. No consumer build or installed SDK write occurs.
- `node gui_forms/tests/api_reference_search_tests.js`: order, total match count,
  first-100 display bound, empty/no-match state, listener removal on page hide,
  reconnection on page show and final disposal passed in a generated DOM fixture.
- Both PowerShell files parse. Repeated toolchain entry preserves PATH and the
  explicitly selected compiler's precedence within the test shell.

**Authorized output delta:** the coordinator explicitly admitted rewriting only
emitted `search.js` to named behavior, explicit document-owned listener state and
bounded displayed-match storage. Inventory/HTML/CSS/search-data/coverage output
must otherwise match for an unchanged input fixture. Committed generated docs
are not rewritten until the coordinator confirms authoritative headers are
stable. Concurrent C++ header changes may legitimately change declarations;
fixture equivalence does not claim those live declarations are unchanged.

**Audit limits:** the owned Python source and tests have zero lambdas,
comprehensions/generator expressions or untyped procedure interfaces in the
focused AST inventory. That mechanical result is not a whole-project compliance
claim or a substitute for review of sequencing, ownership and bounds. No full
platform build, live GUI acceptance, native Mac signing execution or release
repackaging was performed for this correction. Full CI remains a coordinated
follow-up after source review. Existing negative CI and provenance results are
retained above.

**Regeneration boundary correction:** after the coordinator reported authoritative
headers stable, the compiler generator completed a full source extraction:
569 types, 5,061 members and 233 functions (5,920 symbols), with zero authored
contracts in its default destination. Inspection established that
`gui_forms/docs/reference/` had no tracked inventory or pages in this checkout;
`docs/library` is a different established projection. The new, verified-untracked
attempt was moved intact to `.build/house-style-tooling/reference-current`.
The coordinator explicitly declined admitting a new reference tree. No tracked
inventory, existing library atlas, SDK or published artifact was changed.
