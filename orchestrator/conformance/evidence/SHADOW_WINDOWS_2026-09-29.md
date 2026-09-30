# Shadow Windows development verification

Date: 2026-09-29 (America/Los_Angeles).
Status: **MEASURED native development build; Windows Core 1.0 promotion closed**.

## Environment and tool provenance

**OBSERVED:** Windows NT 10.0.22621.0, x86-64, authoritative working checkout
`C:\Users\Shadow\file_manager`. No remote mirror, worktree, or second C++
installation was used for this receipt.

Rust was absent from PATH and the user-local Cargo bin directory. The official
[Rust installation instructions](https://rust-lang.org/tools/install/) and
[rustup Windows instructions](https://rust-lang.github.io/rustup/installation/windows.html)
were consulted. Downloaded
`https://static.rust-lang.org/rustup/dist/x86_64-pc-windows-gnu/rustup-init.exe`
and its `.sha256` sidecar; compared SHA-256 before execution:

```text
6d5b5709addc0122c916d8c810da8d8a7b086a5d64fa805ef404d506392aadc8
```

The checksum comparison verifies agreement with the official HTTPS sidecar;
it is not an independent signature claim. Downloads remain under
`%LOCALAPPDATA%\FileManagerTooling\rust`. Installation used
`-y --default-host x86_64-pc-windows-gnu --profile default --no-modify-path`.
Rustup installed user-local binaries under `%USERPROFILE%\.cargo\bin` and
toolchains under `%USERPROFILE%\.rustup`; no persistent PATH change was made.

Measured versions:

- rustup 1.29.1 (`d95a37b6a`, 2026-08-13).
- rustc 1.98.1 (`48a229cea`, 2026-09-01), LLVM 22.1.8.
- cargo 1.98.1 (`797e8a9bc`, 2026-08-05).
- Host/target: `x86_64-pc-windows-gnu`.
- Shared Plan Paint MinGW: GCC 16.2.0, CMake 4.4.3, Ninja 1.13.2 under
  `C:\Users\Shadow\plan-paint\build-deps\msys64\mingw64\bin`.

Cargo used the existing lockfile with `--locked`. This run does not establish
the declared Rust 1.87 minimum-version gate.

## Retained failures and repairs

**MEASURED baseline:** Windows Rust compilation failed because non-Unix
`call_local` accepted an owned `Request` while the call site and Unix adapter
use `&Request`. The fallback signature now agrees. Unix-only test imports and
the timeout specimen are gated with their Unix-only test.

The next native run passed 49 of 52 unit tests. One test used Unix absolute
path literals; it now derives paths from the host temporary directory. Two
durable-settings tests failed opening/fsyncing a directory after initial file
publication. The non-Unix adapter also lacked the Unix private-owner and
no-follow guarantees. `SettingsService::open` now returns explicit unavailable
before filesystem access on non-Unix. A native test verifies that a missing
store is not created and an existing document remains unchanged. Unix durable
recovery tests remain intact and Unix-gated. In-memory validation, transactions,
and concurrency tests continue on Windows.

**MEASURED:** the C++ client already compiled unchanged through its existing
non-Unix fallback. Its settings/service snapshot lookup helpers incorrectly
returned null on non-Unix. Those pure value lookups now share a platform-neutral
implementation. A C++ check covers found/missing values, explicit unavailable
connection, and the blocked Windows bootstrap gate. The Unix transport and
public C++ signatures remain unchanged.

**OBSERVED:** release provenance embeds raw specification/fixture bytes.
Windows CRLF checkout conversion changed those bytes. Component-local
`.gitattributes` now pins embedded specification files and JSON fixtures to LF;
expected release/bootstrap digests were regenerated after normalization and
the Windows development registry receipt. No readiness field, authentication
rule, or wire operation was changed. An intermediate verification correctly
rejected stale digests; the final fixture check uses the regenerated values.

## Reproduction and scope

From the repository root in PowerShell:

```powershell
./orchestrator/tools/verify_windows.ps1
```

The script accepts `-MingwBin` and `-RustBin`, changes PATH only for its own
process, fails at the first nonzero command, runs format/test/Clippy/fixtures,
builds release binaries, and builds/tests the independent C++ client. It does
not install or start a service.

Native Rust tests pass: 69 total (51 unit, six CLI, four Engine semantic
fixtures, six route-policy tests, two registry/golden-fixture tests).
The complete verification script exited zero: format check, warning-free
all-target/all-feature Clippy, fixture check, locked release build, independent
C++ build, and one CTest platform check passed. Native release `status --json`
succeeded; Rust `call-local status` and C++ `probe` each exited 1 with the
explicit platform-unavailable diagnostic. The status command describes a
fresh in-process laboratory kernel, not a connection to a running daemon.
Unix transport, hostile-session, installed lifecycle, live Engine cross-process,
and Unix C++ daemon conformance tests compile out on Windows; zero executed
tests in those suites are not evidence of Windows transport support.

Build/check logs are retained in ignored `orchestrator/.build/`:
`windows-baseline.log`, `windows-test.log`, `windows-clippy.log`,
`windows-verification.log`, and `windows-verification-final.log`.

Produced native artifacts (not installed services or distributable packages):

| Artifact relative to Orchestrator | SHA-256 |
|---|---|
| `target/release/orchestrator.exe` | `bd1c75b340e02b8b3c6baf9723fdf662880b94bf9aa69429f61f89322c4a944d` |
| `target/release/orchestrator-fixtures.exe` | `a572b0397db08768d99366ffb0831d8d3ab72a3ae2f520b2349920259270ab1b` |
| `target/release/orchestrator-worker-bench.exe` | `d4a3e2482d1360e55b58a55d83a2ff90e22ecbe7fc4208a887c0c8d9d6fa7529` |
| `.build/windows-cpp/orchestrator-cpp-client.exe` | `3d527ce5725b7dd108c4eddc1ee6dd9323fad1db7cb2899e4fc8defd404d1afa` |

The C++ static library is `.build/windows-cpp/libfileman_orchestrator_client.a`.
The C++ executable was run with the shared MinGW bin directory on PATH;
standalone runtime DLL packaging is not claimed.

## Remaining unsupported operations

- `serve-local`, `call-local`, and every C++ live service operation: no Windows
  named-pipe adapter, discovery/authentication, or supervisor lifecycle yet.
- `serve-launchd` and `launchd-plist`: macOS only.
- Durable settings open/commit/recovery on Windows: unavailable before access.
- Installed authenticated Engine query/admin integration on Windows: not
  demonstrated by these tests. JSONL remains a development laboratory.
- End-to-end Windows live frontend bootstrap: not satisfied by compilation.

The CLI's release record still explicitly describes `first_platform: macos`;
its historical readiness is not current-host Windows readiness. Named-pipe
transport remains the ADR-009 direction, with native identity, private endpoint,
bounded session, restart, and independent client evidence still required.
