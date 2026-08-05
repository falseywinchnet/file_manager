# M2 standalone delta candidate measurement 001

Status: **MEASURED component evidence; candidate is not admitted to the live
manifest, query path, or compaction policy**.

Date: 2026-08-05.

Source manifest (Go/C files, `go.mod`, and `Makefile`):
`f8d8cf1cf012772ca8665ad28afb02f6c9008f97bd3ff1d54dc60cae2197f77c`.

## Claim boundary

`internal/generation/delta_candidate.go` writes a standalone path-ordered
add/update/delete run directly from `generation.Diff`. It has a checksummed
fixed header and payload, exact root/base/target/catalogue binding, bounded
string/record dimensions, strict path order, cancellation, no-op suppression,
and a streaming reader.

The candidate does not change schema v1, a production manifest, query lookup,
run merging, reader shadowing, compaction, or reclamation. Its batch results
measure run encoding only, not manifest bytes or amortized compaction. It does
not establish physical power-loss, ENOSPC, APFS, NTFS, or ext4 durability.

## Environment

- macOS 14.8.7 (23J520), Apple M3 arm64, 8 logical CPUs, 8 GiB RAM;
- Go 1.26.5, `darwin/arm64`, APFS SSD;
- generated `scale-v1`, one changed regular-file timestamp;
- warm process/filesystem state; no cache-clearing method.

## One-million result

The base contains 1,000,000 files, 100,000 directories, and 1,100,000
bindings. The deliberately conservative 64-byte canonical changed-object
record remains the denominator.

| Measurement | Standalone delta | Full-snapshot control |
|---|---:|---:|
| candidate/live output bytes | 536 | 164,200,576 segment bytes |
| write lower bound | 792 | 164,201,437 |
| amplification / 64-byte record | 12.38x | 2,565,647.45x |
| update encoding/publication elapsed | 307.414 ms | 1.487 s rebuild + 213.762 ms publish |
| total allocation during operation | 49,601,264 B | 1,767,336 B during snapshot publish; rebuild allocation not isolated |
| transient heap growth | 49,601,264 B | 1,767,336 B during publish |
| retained heap growth after GC | 0 B | 0 B during publish |

The delta lower bound counts the 536-byte file plus the 256-byte final header
rewrite. It excludes filesystem metadata, so it is conservative in the same
direction as the snapshot control.

**MEASURED:** the candidate writes over 207,000 times fewer application bytes
than the full-snapshot control for this case. It also satisfies the slice's
separate one-record requirement of at least 10x fewer bytes. It does not satisfy
or fail the less-than-3x mixed-batch gate because this is intentionally the
one-record worst case.

**OBSERVED:** most transient allocation comes from decoding path/name rows
while scanning the 1.1-million-binding base. It is bounded in retained memory
by the reader cache and returned to 0 additional retained bytes after GC, but
49.6 MB of allocation traffic is still an optimization target. The candidate
does not materialize a change array.

## Ten-thousand scale check

At 10,000 files plus 1,000 directories the same candidate was 537 bytes,
793-byte lower-bound, 12.39x amplification, 5.503 ms elapsed, and 497,264 bytes
allocated. The corresponding full snapshot wrote at least 1,643,438 bytes.
This scale point is a regression check, not a promotion gate.

## Representative update-run batches

The batch denominator is independent of the candidate encoding: one operation
tag, the fixed 64-byte object record, a 32-byte parent identity, two 32-bit
string lengths, and exact path/name bytes.

| Batch | Candidate bytes | Write lower bound | Canonical logical bytes | Run-only amplification | Elapsed | Total allocation | Retained after GC |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 4,096 metadata updates | 683,589 | 683,845 | 589,012 | 1.1610x | 322.199 ms | 58,706,984 B | 0 B |
| 10,000 metadata updates | 1,668,369 | 1,668,625 | 1,438,000 | 1.1604x | 391.712 ms | 51,348,032 B | 0 B |

**MEASURED component result:** the standalone run is below the 3x batch gate.
**NOT MEASURED:** amortized compaction and manifest publication. The complete
admission requirement explicitly includes both, so the format remains a
candidate.

## Correctness evidence

- exact update, addition, and deletion rows round-trip in strict path order;
- target catalogue digest and base/target generations round-trip;
- payload corruption fails checksum verification;
- truncation fails open;
- cancelled writes leave no candidate file; no-change writes never open the
  candidate path and leave an existing sentinel at that path untouched;
- a candidate root must match the expected exact root.

Test locators: `internal/generation/delta_candidate_test.go` and the opt-in
measurement in `benchmarks/m2_generation_test.go`.

## Verification matrix

- **PASS:** `go test ./...` on macOS;
- **PASS:** `go test -race ./...` on macOS;
- **PASS:** `go vet ./...`;
- **PASS:** Linux/amd64 and Windows/amd64 `go build ./...`;
- **PASS compatibility oracle:** every Windows/amd64 package test executable
  under Wine 11.10;
- **PASS compatibility oracle:** every Linux/arm64 package test executable in
  Lima with guest `/var/tmp` on ext4.

Wine used host APFS rather than NTFS. Lima used virtualized ext4 backed by the
host. Neither is a native power-loss result.

## Next falsification work

1. mixed create/update/delete/rename/hard-link versions of the 4,096- and
   10,000-operation batches;
2. exact base-plus-run query merge and p99 comparison at candidate run counts;
3. copy-on-write ordered-tree and SQLite update controls;
4. interrupted standalone publication and later manifest/compaction boundaries;
5. reduce path/name decode allocation without retaining a full heap mirror;
6. select no format, run bound, or compaction trigger before those results.
