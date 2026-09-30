# Shadow Go house-style correction — 2026-09-30

**GIVEN:** apply the language-neutral rules in the owner's `programming-house-style (2).md` to the authored Shadow engine work, preserving admitted behavior and protocol. This is a scoped correction, not a claim that the entire pre-existing Go engine follows that style.

## Scope and baseline

**OBSERVED:** baseline source is commit `9ec42b1` (`Record the coordinated Windows editor SDK checkpoint`). The audit compares complete touched files, including shared code that predates the Windows work. Vendor source, dependency versions, query semantics, manifest schema, and public protocol are unchanged.

The 16 corrected files are:

- `cmd/fileman-engine/main.go`
- `cmd/fileman-engine/main_darwin_test.go`
- `cmd/fileman-engine/main_test.go`
- `cmd/fileman-engine/windows.go`
- `cmd/fileman-engine/windows_other.go`
- `cmd/fileman-engine/windows_test.go`
- `internal/deployment/host_other_test.go`
- `internal/deployment/manifest_windows.go`
- `internal/transport/local.go`
- `internal/transport/local_notwindows.go`
- `internal/transport/local_windows.go`
- `internal/transport/local_windows_test.go`
- `internal/transport/peer_other.go`
- `internal/transport/peer_windows.go`
- `internal/windowssecure/state_windows.go`
- `internal/windowssecure/state_windows_test.go`

The new regression/benchmark file is `internal/transport/frame_storage_test.go`. This evidence record is the only new documentation file. The ignored local driver `.build/windows-validation/validate.ps1` was also corrected: typed initialized locals, named check function with explicit arguments, separate native command invocation and exit capture, aggregate nonzero failure status. No other component is edited by this engine task.

## Structural audit

**MEASURED:** Go parser traversal over the baseline 16 files and resulting 17 files gives:

| AST property | Baseline | Result |
| --- | ---: | ---: |
| Short declarations, including range clauses | 425 | 0 |
| Anonymous function expressions | 14 | 0 |
| Explicit `var` declarations missing their type | 0 | 0 |
| Explicit `var` declarations missing initialization | 33 | 0 |
| Return expressions other than named values, literals, or direct field/element access | 139 | 0 |

The missing-type row counts `var` declarations; inferred `:=` declarations are counted separately. Named callback types and method bindings are not anonymous functions. These are structural checks, not a substitute for reviewing ownership, sequencing, or behavior.

The final source SHA256 inventory is `.build/style-review/source-hashes.json`. Local parser source and receipts: `.build/style-review/audit.go`, `baseline-ast.json`, `result-ast.json`, `scope.txt`, and `result-scope.txt`. The baseline is read directly with `git show 9ec42b1:<path>`; no baseline tests were rerun before editing. Previously recorded Windows validation remains in `WINDOWS_TOOLCHAIN_VALIDATION_2026-09-29.md`.

## Ownership and execution corrections

**OBSERVED:** endpoint and connection goroutines now run named owner methods with explicit listener, service, registry, slot, cancellation, and completion fields. The server cancels, closes listeners, joins acceptors, closes registered connections, and joins handlers before its borrowed state is released. Existing query/admin connection limits remain 32/4.

**OBSERVED:** the admission callback is a bound method on immutable manifest metadata. Windows client cancellation has a named owner; revocation either prevents the callback or joins a callback already closing the connection. Test servers likewise own their cancellation and completion channels and are joined before temporary directory cleanup. A private context-taking manifest-serving helper permits fixture cleanup even after readiness failure; normal CLI serving retains its background context.

**OBSERVED:** a frame reader now owns connection-local header and payload scratch. It validates the byte bound before allocation and reuses capacity across frames. JSON decoding completes before the next reuse. Frame writes marshal once, then write a fixed header and borrowed payload in order, handling short writes. There is no shared mutable frame workspace. Root projection arrays have their final length before filling; the ACL entry bound is read before traversal. Foreign SID borrowing remains confined to the synchronous Windows security-descriptor check.

## Validation

**MEASURED:** final build, test, vet, race, offline build, Linux cross-build, and Darwin test compilation all pass. Both the ordinary and race suites report 177 passing tests/subtests and 20 skips, with zero failures. Commands use Go 1.27.1 on Windows amd64, GOMAXPROCS=2, build parallelism 2, and the existing MinGW GCC only for the race runtime. Tests use temporary fixture roots.

- `go build -p 2 ./...`
- `go test -p 2 -count=1 -json ./...`
- `go vet -p 2 ./...`
- `go test -race -p 2 -count=1 -json ./...`
- `go test -p 2 ./internal/transport -run '^$' -bench BenchmarkFrameStorage -benchmem -count 3`
- Offline `go build -mod=vendor -p 2` with GOPROXY/GOSUMDB off, CGO disabled.
- Linux amd64 service cross-build and Darwin arm64 main/transport test compilation.

New regressions cover exact ENG1 bytes with short writes, zero-progress writes, smaller-frame scratch reuse, oversized-frame rejection before growth, revocation before cancellation, and joining an already-running close callback. Existing Windows tests retain wrong-PID and cross-authority rejection, idle-client shutdown, live-only behavior, indexed restart with changed exclusions, host/SID/root binding, private-state ACLs, and exclusive state locks.

**MEASURED:** the initial three-repeat microbenchmark on this shared AMD EPYC 9354 Windows host reported reusable reads at 496 B/op and 5 allocs/op; one-shot reads at 552 B/op and 7 allocs/op. The workload is one encoded JSON string, `fixture payload`, repeatedly decoded from an in-memory reader. It compares the resulting reusable reader with its one-shot wrapper, not the entire old server. Timing was noisy and did not establish a speed improvement; no latency, CPU, RSS, or end-to-end transport performance claim is made. Raw outputs are retained under `.build/windows-validation/house-style/` and `house-style-final/`.

**Unresolved limits:** Darwin tests are compiled but not executed on this Windows host. Existing platform skips remain explicit. Older Go packages and upstream vendor source are outside this audit. No running installed process or frozen package is replaced; build products use separate style-validation names. No commit, push, or publication is performed by this task.

**MEASURED:** the separate offline validation executable is `.build/windows-validation/fileman-engine-style.exe`, SHA256 `3de7ae7cba8834b4de76ab80d77fe5a57a8c2a332e9bdb5a3e7c63ba888c233c`. The prior pipe executable was not overwritten. Final command receipts, JSON test streams, environment, benchmark repetitions, and cross-build receipt are under `.build/windows-validation/house-style-final/`. `gofmt -l` over the 17 files and `git diff --check -- engine` report no formatting/whitespace defects.
