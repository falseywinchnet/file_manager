# Shadow Windows toolchain and validation — 2026-09-29

Status: **MEASURED development build/test evidence; no platform promotion**.

Base revision: `7ce5cf44d5fa35b89b182f106ca79c8ab3f44dde`, with the engine-only
fixture/test corrections described below. Other sibling edits were preserved.
Host: Windows 11 Home 10.0.22621, windows/amd64, NTFS on C:, reported CPU
AMD EPYC 9354 32-Core Processor. This session exposes eight logical processors
and approximately 16 GB RAM; concurrent sibling native builds were active.
Go package parallelism and GOMAXPROCS were capped at two. Tests used generated
temporary roots, not personal-file scans. No service was installed.

## Toolchain receipt

**OBSERVED:** official stable metadata at <https://go.dev/dl/?mode=json> selected
`go1.27.1.windows-amd64.zip` (78,931,360 bytes). It was downloaded from
<https://go.dev/dl/go1.27.1.windows-amd64.zip> and verified with PowerShell
`Get-FileHash -Algorithm SHA256` before extraction:

```text
a3911b5e0e1b1053f25ed0675f4c1c6aad1e2bfcf253df2b9be4caabd2edd95d
```

The user-local executable is:

```text
C:\Users\Shadow\AppData\Local\Programs\Go\go1.27.1\go\bin\go.exe
go version go1.27.1 windows/amd64
```

User environment variable `FILE_MANAGER_GO_BIN` contains its bin directory.
The persistent user/machine PATH was not changed. Race builds consumed the
Paint sibling's existing compiler, without installing another C++ stack:
`C:\Users\Shadow\plan-paint\build-deps\msys64\mingw64\bin\gcc.exe`.

Local ignored receipts: `.build/windows-validation/go-downloads.json`,
`go-selected.json`, `go-bin.txt`, the verified ZIP, and each run's
`environment.json`. Logs remain under that directory rather than Git.

## Baseline and correction

**MEASURED baseline:** build and vet passed. Ordinary and race suites each
failed four tests:

- `TestProtocolGolden` and `TestSegmentMatchesReferencePathAndName`: Git's CRLF
  checkout conversion changed text fixtures used in byte-exact LF comparisons.
- `TestChangedInstalledExclusionsReconcileBeforeServingRecoveredGeneration`
  and `TestManifestAdmissionCommitterAdvancesSuccessfulReconciliation`: the
  macOS installed-profile tests invoked directory sync on Windows. The first
  failure also left an open generation handle until process exit, causing a
  temporary-directory cleanup failure.

**OBSERVED correction:** `engine/.gitattributes` pins testdata to LF. The two
installed-profile test bodies moved unchanged into `main_darwin_test.go`,
matching `deployment/host_other.go`'s explicit macOS-only installed profile and
its existing Darwin-only deployment test suite. A non-Darwin regression checks
that `CurrentHost` rejects the installed profile and returns no host identity.
No production implementation, identity, containment, authentication, permission,
or durability behavior changed. Assertions and fixture content were retained.

## Final native checks

Commands ran from `engine/`, with `GOTOOLCHAIN=local`, `GOMAXPROCS=2`, and
`CGO_ENABLED=1`; the Go and shared MinGW bin directories were prepended only to
the command process PATH.

| Command | Result |
|---|---|
| `go build -p 2 ./...` | PASS, exit 0 |
| `go test -p 2 -count=1 -json ./...` | PASS, exit 0; 165 test/subtest passes, 20 skips, 17 passing test packages |
| `go vet -p 2 ./...` | PASS, exit 0 |
| `go test -race -p 2 -count=1 -json ./...` | PASS, exit 0; 165 test/subtest passes, 20 skips; no reported data races |
| `go test -p 2 -run '^$' -bench . -benchtime=1x -benchmem ./...` | PASS, exit 0; all four benchmarks executed |
| `go build -p 2 -trimpath -o .build/windows-validation/fileman-engine.exe ./cmd/fileman-engine` | PASS, exit 0 |
| Built executable with no arguments | Expected refusal, exit 1: sandbox root is required |
| Built executable, fresh temporary sandbox, JSONL `engine.version` then `engine.shutdown` | PASS, exit 0; development capabilities returned |
| `GOOS=linux GOARCH=amd64 CGO_ENABLED=0 go build -p 2 ./...` | PASS, exit 0; cross-compilation only |
| `GOOS=darwin GOARCH=arm64 CGO_ENABLED=0 go test -p 2 -c -o .build/windows-validation/command-darwin-arm64.test ./cmd/fileman-engine` | PASS, exit 0; includes relocated test bodies; no native execution |

Native passes include rename/hard-link identity, immutable-generation recovery
and publication interruption, bounded query behavior, the disposable-root
`ReadDirectoryChangesW` adapter create/stop test, and the service watcher test
that deliberately retains incomplete-currentness status. These are specific
fixture observations, not the complete NTFS identity/currentness oracle.

The 20 skips comprise 13 opt-in measurement campaigns, an unavailable SQLite
control, two subprocess-only crash helpers, one symlink-creation privilege
limitation, and three explicit Windows reparse/symlink identity gates. Darwin
installed transport/profile tests and platform-specific macOS code are excluded
by build constraints. The full mixed rename/move/replace/hard-link/symlink
oracle remains skipped on Windows; the separate rename and hard-link tests pass.

The benchmark run is a one-iteration smoke check under competing builds. It is
not a latency distribution, controlled baseline comparison, resource budget
result, or performance claim. Raw outputs are retained in `benchmarks.log`.

## Reproduction on this host

```powershell
Set-Location C:\Users\Shadow\file_manager\engine
$goBin = [Environment]::GetEnvironmentVariable('FILE_MANAGER_GO_BIN', 'User')
$env:PATH = "$goBin;C:\Users\Shadow\plan-paint\build-deps\msys64\mingw64\bin;" + $env:PATH
$env:CC = 'C:\Users\Shadow\plan-paint\build-deps\msys64\mingw64\bin\gcc.exe'
$env:CGO_ENABLED = '1'
$env:GOMAXPROCS = '2'
$env:GOTOOLCHAIN = 'local'
go build -p 2 ./...
go test -p 2 -count=1 ./...
go vet -p 2 ./...
go test -race -p 2 -count=1 ./...
go test -p 2 -run '^$' -bench . -benchtime=1x -benchmem ./...
go build -p 2 -trimpath -o .build/windows-validation/fileman-engine.exe ./cmd/fileman-engine
```

Fresh clones honor the LF attributes automatically. For a pre-existing CRLF
checkout, normalize only `engine/testdata/` to LF, preserving content and other
edits, before executing the byte-exact fixture tests. The local logging driver
is `.build/windows-validation/validate.ps1`; `baseline/` and `final/` contain
separate build/test/vet/race logs and exit/timing summaries.

## Remaining gates

**OBSERVED/UNRESOLVED:** no Windows authenticated installed IPC or SCM service
has been admitted; the installed root manifest remains macOS-only. Reparse-point
identity, root-replacement/journal coverage, persisted watermarks, the complete
native NTFS/ext4 oracle, million-entry live-query resource distributions,
blocked-output/permission campaigns, and release packaging remain open. Native
macOS rerunning of the relocated tests is still required. Existing historical
compatibility-only capability descriptions were not silently promoted by this
limited native fixture run. Linux/macOS cross-builds establish compilation only.
