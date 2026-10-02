# Live filename/path matching: first search optimization

**GIVEN:** the owner prioritizes search speed, everyday operations, and
responsiveness. This slice changes the existing live query, not catalogue
capabilities or ranking. Baseline source: `9ec6998`.

## Implementation and correctness

**OBSERVED:** `internal/live/path_matcher.go` gives each locked query session
owned normalization scratch. ASCII paths are lowercased and converted to slash
separators in reusable storage before standard byte substring search. A path
contains its complete filename, so the old separate filename match is redundant.
Non-ASCII paths retain the existing `strings.ToLower(filepath.ToSlash(...))`
behavior. This does not introduce Unicode normalization or EqualFold semantics.
Scratch starts at 512 bytes and grows before processing a larger path; later
shorter paths search only their initialized extent. Query text is at most the
existing 4096 bytes. Storage is session-local, not a global/shared workspace.

Exact observations, root ownership, traversal order, result ranks, work ceilings,
continuation, expiry and cancellation keep their existing authority. No source
file writes or catalogue construction are added. The new pagination fixture
forces metadata-budget deferral and confirms 70 unique results, 70 visits,
continuous rank, and no remaining session. Cancellation checks handle cleanup.

**MEASURED:** full `go test ./...`, `go vet ./...`, and `go test -race ./...`
passed on Shadow Windows amd64 / Go 1.27.1. A 15-second, two-worker fuzz run
completed 15,850 differential executions without a mismatch. Unicode, combining
marks, invalid UTF-8, separators, empty strings and long paths have fixed cases.
Fuzzing is bounded evidence, not exhaustive proof. Native CI now runs Engine
tests and vet on all three package platforms; those new runs are pending.

## Component measurements

**MEASURED:** AMD EPYC 9354, Shadow Windows amd64, Go 1.27.1, 8 reported Go
benchmark processors. Generated in-memory strings, warm component workload;
no filesystem timing is included in this table. Ranges are two repetitions of
300 ms Go benchmarks, not p50/p95/p99 distributions or CPU service measurements.

| Case | Previous ns/op | Reused ns/op | Previous B/op, allocations | Reused B/op, allocations |
|---|---:|---:|---:|---:|
| Uppercase path, no match | 270.5–282.9 | 59.28–61.44 | 176, 4 | 0, 0 |
| Filename hit | 81.45–83.43 | 56.17–58.54 | 32, 1 | 0, 0 |
| Lowercase path, no match | 126.1–128.5 | 56.42–60.21 | 96, 2 | 0, 0 |
| Unicode path, no match | 361.7–401.9 | 218.7–221.3 | 88, 3 | 64, 2 |

Reproduce: `go test -run '^$' -bench '^BenchmarkLivePathMatching$' -benchmem
-benchtime=300ms -count=2 ./internal/live`.

## Retained negative results and limits

**REJECTED:** a 32-entry ReadDir batch was implemented, tested and removed from
the production path after its 1000-file native directory control regressed.
Single-entry enumeration: 0.968–0.998 ms, about 145 KB, 3004 allocations.
32-entry enumeration: 1.007–1.162 ms, about 161 KB, 2194 allocations. Lower
allocation count did not compensate for greater byte allocation and elapsed
time. The benchmark remains as `BenchmarkLiveDirectoryBatches`; no read-ahead
buffer or changed work-budget semantics remain in the shipped path.

**MEASURED, not a speedup claim:** the existing generated 10,000-file all-match
campaign returned all records over 80 pages with 10,000 visits/stat calls and no
catalogue generation changes in each attempt:

| Attempt | First result | Full traversal | Total allocated bytes | Allocations |
|---|---:|---:|---:|---:|
| Baseline | 9.0001 ms | 2.1226435 s | 30,144,400 | 486,662 |
| Matcher plus rejected batch | 4.9993 ms | 1.8952012 s | 30,298,832 | 478,521 |
| Final matcher, no batch | 4.9959 ms | 3.1508658 s | 29,036,176 | 486,762 |

The final full traversal was slower in this unpaired run. These independent
temporary NTFS fixtures do not control host load/cache and cannot support a
whole-search latency claim. Metadata/identity I/O remains in this all-match
path; it has not been attributed with a CPU/I/O profile. The harness's `/dev/fd`
count is not a Windows handle measurement. RSS, physical I/O, energy, latency
distributions and million-record targets were not measured here.

## House-style review and outstanding work

Reviewed scope: all of `path_matcher.go` and `search_work_test.go`, plus the
session-field, constructor, entry-name and matching changes in `manager.go`.
Review covered explicit types, named benchmark/fuzz targets, session ownership
and lock protection, initialized scratch extent, conversions, failure cleanup,
budget-preserving order and repeated-loop allocation. No remaining violation
was found in that scope. Legacy timer closures and other unchanged Engine code
are not certified by this review. The CI addition contains only ordered test
commands and no retained callback state.

The source-atlas check is blocked by an existing missing manual package entry
for `filemanager/engine/internal/windowssecure`; this change does not create
that package or claim the generated atlas is current.

Indexed substring retrieval, its capability negotiation, durable generation
binding, off-heap candidate structure, and end-to-end GUI latency remain the
next substantial search work. This live matcher is not that index.
