# Shadow Windows build bring-up

Status: **MEASURED native component builds and tests passed; post-fix desktop dogfood and product release promotion remain open**.

## Owner direction

Work directly on the Shadow desktop, borrow the tools being established for
Plan Paint, install the additional File Manager tools, and backport reusable
GUI.Forms logic while preserving File Manager's different styling. Visible
sibling chats are authorized; workers/subagents are prohibited.

Ownership during this run: the parent chat owns GUI.Forms/shared scripts;
Engine, Orchestrator, frontend and Web.Forms each have a separate visible chat
working in their existing subtree. No worktrees are used.

## Tools

| Tool | Observed version | Location/selection |
|---|---|---|
| GCC / MinGW64 | 16.2.0 | adjacent `plan-paint/build-deps/msys64/mingw64/bin` |
| CMake | 4.4.3 | same shared MinGW64 bin |
| Ninja | 1.13.2 | same shared MinGW64 bin |
| GDB | 18.1 | same shared MinGW64 bin; installed by Paint toolchain owner |
| Python | 3.14.7 | same shared MinGW64 bin |
| Go | 1.27.1 windows/amd64 | user-local Programs/Go; `FILE_MANAGER_GO_BIN` user variable |
| Rust | rustc 1.98.1 / cargo 1.98.1 | user-local `.cargo/bin`, GNU toolchain |

Go's archive was verified against official release metadata. Rustup's installer
was verified against its official SHA-256 sidecar. Component receipts retain
their exact paths and checks. The shared C++ installation is owned by the Paint
bring-up; File Manager reads it without modifying that dependency tree.

```powershell
. .\tools\Enter-WindowsToolchain.ps1
.\tools\Build-Windows.ps1 -Component Toolkit -Jobs 3 -Test
.\tools\Build-Windows.ps1 -Component Frontend -Jobs 2 -Test
```

The environment helper changes the current process PATH only. The Go selection
variable is read from the user environment so an already-running shell can find
it. `FILE_MANAGER_MINGW_BIN` can override the adjacent toolchain location.

## Outputs and validation

- GUI.Forms build: `gui_forms/.build/shadow-windows`.
- File Manager SDK: `gui_forms/.build/shadow-sdk`.
- Frontend build: `frontend/.build/shadow-windows`.
- Engine receipts: `engine/.build/windows-validation`.
- Orchestrator verification is documented by its Windows bring-up chat.

GUI.Forms configure, application DLL build and SDK installation pass. All 64
native CTests pass after the accessibility repair (14.26 seconds). The frontend
application builds and its combined CTest run passes all 11 suites. The staged
application DLL matches the repaired SDK SHA-256 in the development manifest.
Symlink-creation privilege failures skip only link-dependent assertions;
Windows-specific junction/no-follow, Unicode, identity, preview and checksum
coverage passes. The executable is `frontend/.build/shadow-windows/File Manager.exe`.
The new development manifest is
`gui_forms/manifests/gui-forms-shadow-windows-x64-2026-09-29.json`; the historical
macOS FM0 manifest is unchanged. Source backport provenance and the additional
worker-wake/UI-dispatch bridge are documented in
`gui_forms/docs/PLAN_PAINT_BACKPORT_2026-09-29.md`.

Web.Forms initially passed 34 of 37 tests with three browser-discovery errors.
The repaired Windows harness passes all 40 tests with no skips (61.302 seconds),
including generated C++ compilation and actual Brave reference captures. See
`web_forms/experiments/WINDOWS_VALIDATION_001.md`. Native Windows raster parity
remains unmeasured; browser reference capture is a development tool only.

Orchestrator passes 69 native Rust tests, formatting, Clippy, fixture checks,
locked release compilation and one independent C++ CTest. Its platform-neutral
snapshot lookup now works on Windows; Windows local IPC and durable settings
still return unavailable. See
`orchestrator/conformance/evidence/SHADOW_WINDOWS_2026-09-29.md`.

Engine passes build, vet, ordinary tests and race checks. Both test runs report
165 passes and 20 explicit skips. Four one-iteration benchmarks run as smoke
checks only; no speed comparison is established. Linux amd64 compilation and
Darwin arm64 command-test compilation pass without native execution. See
`engine/results/WINDOWS_TOOLCHAIN_VALIDATION_2026-09-29.md` for skipped gates,
fixture repairs, checksums and reproduction.

The first native GUI run rendered the authored Home view with 16 actual local
folders and explicit unavailable service/search state. Opening the repository
folder stalled the UI, while a separate exact-path directory model probe
returned 22 entries in 12 ms. Paint subsequently reproduced an accessibility
focus-traversal loop: MSAA returned fresh self wrappers instead of CHILDID_SELF.
The shared source fix and focus/hit-test regression were backported; all 64
File Manager toolkit tests passed again in 14.26 seconds and the SDK was
reinstalled. This is a strong explanation for the apparent navigation stall,
but post-fix File Manager desktop verification remains unperformed because the
user stopped Computer Use. Rendered startup alone is not navigation acceptance.

## Explicit limits

These are Windows development builds. Linux/macOS code brought from Paint is
source reuse, not fresh native evidence on those systems. The initial Windows
renderer is Paint's GDI profile; custom macOS titlebar behavior remains behind
its platform adapter. The product's authored content styling is preserved.

Orchestrator's Windows native local IPC is still unavailable. Its C++ client
already has a non-Unix fallback, so Unix headers were not by themselves a
Windows compilation defect. Durable settings require real Windows atomicity,
owner/privacy and no-follow semantics; they may not silently claim success
through Unix assumptions. Engine installed transport and reparse-point
coverage retain their own gates. Compiling the application cannot promote
these capabilities to available.
