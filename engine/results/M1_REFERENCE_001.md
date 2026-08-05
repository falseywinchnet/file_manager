# M1 exact reference measurement 001

Status: **MEASURED reference evidence; not a production-engine promotion**.

Date: 2026-08-05.

Source manifest (Go files, `go.mod`, and `Makefile`):
`753377ba103ef6531dd2b8453882ae8e7e4be91237ac4d88656293a093494a1e`.

Post-measurement audit manifest:
`65e802b97cd0591e9ed971a47ddddb54776ada9ea829b9071353b42b78ef0d17`.
The only intervening code change replaces an unreachable canonical-encoder
panic with explicit error propagation; the full correctness/race/vet/cross-build
suite was rerun, but the distributions below remain tied to the first digest.

## Claim and rejection boundary

The run tests whether the M1 object-plus-binding reference can publish an
immutable in-memory generation and answer bounded exact names/paths with stable
evidence and cursors. It does **not** test the M2 durable-segment store, crash
recovery, watcher ingestion, cold service open, one-million-record gates,
lexical/fuzzy quality, or GUI/IPC latency.

**GIVEN:** this complete Go-heap reference is a correctness control. It may not
become the production storage strategy. ADR-001 ENG-06 requires the complete
catalogue to remain off the Go heap in the durable engine.

## Environment

- macOS 14.8.7 (23J520), Darwin 23.6.0, APFS on solid-state storage;
- Apple M3, 8 logical CPUs, 8 GiB RAM;
- Go 1.26.5, `darwin/arm64`;
- AC power; 133 GiB reported free on `/`;
- warm process/filesystem state; generated fixtures were created immediately
  before scanning and no cache-clearing method was used;
- host load average near measurement time: 2.53 / 2.99 / 3.16. Other host work
  was present, so worst stalls are retained rather than characterized as
  isolated-engine noise.

## Commands

```text
go test ./...
go test -race ./...
go vet ./...
make cross-build
GOOS=linux GOARCH=amd64 go test -exec=/usr/bin/true ./...
GOOS=windows GOARCH=amd64 go test -exec=/usr/bin/true ./...
FILEMAN_ENGINE_MEASURE=1 go test -v -count=1 ./benchmarks
go test -run '^$' -bench 'Benchmark(ReferenceShardBuild100k|ExactNameUnique100k|ExactNameRepeated10kOf100k|OwnerAmongNestedRoots)' -benchmem -count=5 ./internal/catalog ./internal/exact ./internal/ownership
```

All correctness, race, vet, native tests, Linux/Windows builds, and Linux/Windows
test compilations passed. Cross-target tests were compiled, not executed.

## Correctness digest and oracle coverage

**MEASURED:** canonical object/binding serialization matches
`testdata/catalog/v1/object-binding.hex`. JSONL version fixtures match exactly.
The shared logical `correctness-v1` workload digest is
`dba2ace881e9893ab08b6c919f27087ba291c37f62c8d7f6efc53a989983de18`.
The macOS sandbox oracle passed:

- hard links share one intrinsic object and retain two bindings;
- a symlink is distinct from its target and is not traversed outside the root;
- rename and same-volume move preserve platform object identity;
- replace-at-path receives a distinct platform object identity;
- the moved old object retains its prior identity;
- most-specific child roots suppress parent results before and after rescan;
- old reader generations remain immutable and old cursors fail against a newer
  generation.

Unique-name correctness digest: `0bd6ad4ceab8ccf3`.
Ten-thousand-candidate correctness digest: `a69b9113a32327fd`.
The service digest is deliberately run-specific because APFS object IDs enter
the digest.

## Distribution measurement

The virtual corpus contains 100,000 file objects in 10,000 directories, plus
the root: 110,001 total objects and 110,000 bindings. Ten thousand files share
the exact basename `repeated`, one per directory.

| Workload | Samples | p50 | p95 | p99 | max |
|---|---:|---:|---:|---:|---:|
| exact unique name, reference boundary | 20,000 | 1.459 us | 1.834 us | 2.750 us | 112.333 us |
| exact basename with 10,000 candidates, first 100 | 2,000 | 229.667 us | 272.708 us | 372.542 us | 821.292 us |
| service exact name after 10k-file sandbox scan | 5,000 | 2.250 us | 3.334 us | 8.791 us | 79.500 us |

**MEASURED:** the 10,000-file metadata-only APFS reconciliation committed 10,000
bindings in 34.700 ms (288,183 bindings/s) in that warm generated fixture.
Bytes read/written and CPU time were not captured, so this is not an indexing
economy claim.

## Allocation and footprint measurement

- Virtual generation construction retained 17,401,312 heap bytes, or 158.19
  bytes per total object, after a forced GC. Build elapsed time was 127.538 ms.
- The isolated 100k direct-child shard build used 39,263,928–39,265,609 B/op and
  532 allocs/op across the five runs; time ranged from 32.433 to 34.806 ms.
- Unique exact-name lookup used 1,156 B/op and 11 allocs/op.
- The 10,000-candidate/100-result lookup used 83,470–83,474 B/op and 114
  allocs/op; most returned-result allocations are exact path/evidence material.
- The independent 512-nested-root router used zero allocations and
  1.822–2.203 us/op, versus 10.023 us/op in `BASELINE_000.md`.

**REJECTED as a production representation:** a resident reference heap at this
density. A linear extrapolation is only a **HYPOTHESIS**, but it would be about
151 MiB at one million objects, well above the accepted 48 MiB idle-private
budget. M2 must use immutable off-heap/on-disk blocks and bounded caches while
remaining digest-equivalent to this reference.

## Accepted performance constitution comparison

- The run does not contain one million or ten million objects, so it does not
  establish the accepted exact/prefix p95/p99 gates.
- It does not measure cold manifest open or idle service state.
- It does not measure index file bytes/object, write amplification, unchanged
  idle writes/CPU, HDD behavior, foreground/query overlap, or compaction.
- The exact 100k reference numbers are an encouraging control only; no target is
  weakened or promoted from them.

## Unresolved correctness gates

- Windows reparse-point identity is refused until a root-relative no-follow
  adapter exists; it is not approximated with target identity.
- Linux `statx` birth/incarnation evidence is not yet implemented.
- `st_dev`/volume serial are current adapter observations, not proven eternal
  volume identities across mount/reboot/restore.
- Cross-volume move, clone/snapshot behavior, event gaps, scan races, and
  unavailable volumes remain unproved.
- No durable manifest, checksum block, quarantine, crash injection, migration,
  or last-valid-generation recovery exists yet.

The next falsifiable step is to feed the now-frozen `correctness-v1` workload to
the immutable-segment candidate and SQLite control, then begin crash-publication
experiments—not a page-codec optimization in isolation.
