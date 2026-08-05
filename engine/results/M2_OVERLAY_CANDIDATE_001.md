# M2 base-plus-run exact overlay measurement 001

Status: **MEASURED component evidence; one-run candidate is not admitted to the
live manifest, service query path, run-count policy, or compaction policy**.

Date: 2026-08-05.

Source manifest (Go/C files, `go.mod`, and `Makefile`):
`f9a4841923fbd50459d7f6c9026b3ffc93ace592162c531b2382affc06d3a3a2`.

## Claim boundary

`internal/generation/overlay_candidate.go` presents one checked immutable base
segment plus one checked standalone delta run through the existing exact-index
interface. Construction verifies root, base generation, target digest, delta
checksum, exact add/update/delete preconditions, and a caller-supplied maximum
change count. The base remains off heap. Only changed live rows, path shadows,
shadowed base ordinals, and name/identity orders are retained.

This is deliberately a one-run experiment. It does not modify manifest schema
v1, publish a delta from the service, merge multiple runs, compact, reclaim,
consume native events, or provide an ephemeral live-query overlay. Therefore it
does not establish an admitted maximum run count or pass the complete M2 gate.

## Environment and workload

- macOS 14.8.7 (23J520), Apple M3 arm64, 8 logical CPUs, 8 GiB RAM;
- Go 1.26.5, `darwin/arm64`, internal APFS SSD;
- generated `scale-v1`: 1,000,000 files, 100,000 directories, 1,100,000 base
  bindings;
- warm process/filesystem state; no cache-clearing method;
- each batch is exactly 25% metadata updates, 25% deletions, 12.5% renames
  encoded as delete plus add, 12.5% creates, and 12.5% hard-link additions;
- 20,000 unchanged exact-path queries and 2,000 repeated-name top-100 queries
  per base/run distribution.

## One-million-record result

| Measurement | 4,096 changes | 10,000 changes |
|---|---:|---:|
| delta file bytes | 695,667 | 1,697,871 |
| write lower bound incl. header rewrite | 695,923 | 1,698,127 |
| canonical logical bytes | 601,088 | 1,467,500 |
| run-only write amplification | 1.1578x | 1.1572x |
| delta write elapsed | 330.236 ms | 372.446 ms |
| delta write total allocation | 58,779,072 B | 51,524,040 B |
| overlay open elapsed | 13.183 ms | 29.742 ms |
| overlay open total allocation | 4,444,496 B | 10,410,960 B |
| overlay retained heap after GC | 997,368 B | 2,270,032 B |
| retained live rows | 2,560 | 6,250 |
| exact-path base/run p99 | 4.625 / 6.208 us | 5.916 / 7.625 us |
| exact-path p99 ratio | 1.342x | 1.289x |
| repeated-name base/run p99 | 206.917 / 79.625 us | 160.083 / 63.041 us |
| repeated-name p99 ratio | 0.385x | 0.394x |

**MEASURED component result:** one base plus one committed run stayed below the
provisional 2x p99 bound in both measured query families and both batch sizes.
The run encoding stayed below 3x before manifest and compaction bytes. The
complete write gate includes amortized compaction and cannot yet pass.

The lower repeated-name ratios do not imply the overlay is faster. Tail timing
in a shared warm process is noisy; the base distribution had larger outliers in
this run. The relevant bounded claim is only that the candidate did not exceed
2x in this named run. Repetition and overlap measurements remain required.

**OBSERVED:** exact-path lookup initially normalized an unchanged absolute path
twice and decoded a base row merely to discover whether its ordinal was
shadowed. A bounded relative-path lookup helper plus a change-count-sized set of
shadowed base ordinals removed those costs. This keeps lookup work proportional
to one base lookup plus one change-set probe without mirroring base records.

## Correctness evidence

- mixed add/update/delete/rename/replacement/hard-link state matches the exact
  reference shard by row and record;
- deleted and replaced base ordinals cannot be read through the overlay;
- exact path, name, metadata, ordering, and generation-bound pagination agree
  with the reference query path;
- ambiguous hard-link identity inspection returns the same result/error code;
- wrong base generation, over-budget change count, and cancellation are
  rejected; the underlying checked delta reader rejects corruption/truncation.

Test locators: `internal/generation/overlay_candidate_test.go` and the opt-in
measurement in `benchmarks/m2_overlay_test.go`.

## Verification matrix

- **PASS:** `go test ./...` on macOS;
- **PASS:** `go test -race ./...` on macOS;
- **PASS:** `go vet ./...`;
- **PASS:** Linux/amd64 and Windows/amd64 `go build ./...`;
- **PASS compatibility oracle:** every Windows/amd64 package test executable
  under Wine 11.10 on host APFS;
- **PASS compatibility oracle:** every Linux/arm64 package test executable in
  Lima with the executable copied onto guest ext4.

Wine and Lima are not native NTFS/ext4 durability evidence and make no
power-loss claim.

## Remaining falsification work

1. repeat distributions and measure query p99 during concurrent publication
   and compaction rather than only quiescent warm lookup;
2. admit no multi-run bound until 2, 4, 8, and 16-run controls measure exact
   query, retained memory, and shadow work;
3. add light-directory threshold churn and hard-link removal/intrinsic-change
   cases to the scale workload;
4. include manifest bytes, crash boundaries, and amortized compaction in write
   amplification;
5. compare the identical update/query transcript with copy-on-write tree and
   SQLite controls;
6. retain the full snapshot as recovery/compaction authority until a numbered
   decision admits a live delta schema.
