# M2 immutable generation measurement 001

Status: **MEASURED component and one-root-service evidence; M2 remains in
progress and is not promoted**.

Date: 2026-08-05.

Source manifest (Go/C files, `go.mod`, and `Makefile`):
`3b9bc4608501d226743c259734aeae5cde1e6dba2b47c88685a6553d6c3d526b`.

## Claim boundary

This result tests the first immutable-generation vertical slice: deterministic
object-plus-binding storage, exact name/path/identity order, atomic dual-slot
publication, checked recovery and fallback, pinned readers, bounded caching,
one-root service restart, and a SQLite exact-store control.

It does **not** promote M2. It does not establish physical power-loss behavior,
NTFS or physical ext4 identity semantics, quarantine, disk-full safety,
delta/compaction write amplification, schema migration, ten-million scale, idle
CPU, isolated service RSS, framed GUI IPC, lexical/fuzzy behavior, or Kolmogrov
transfer readiness.

## Environment

- macOS 14.8.7 (23J520), Darwin 23.6.0, APFS on SSD;
- Apple M3, 8 logical CPUs, 8 GiB RAM;
- Go 1.26.5, `darwin/arm64`; system SQLite 3.43.2;
- 129 GiB reported free on `/`; AC/thermal state was not instrumented;
- warm filesystem/process state; no cache-clearing method;
- host load near final measurement: 1.68 / 2.03 / 2.38;
- other host work was present, so maximum stalls are retained.

Compatibility tests also executed as Linux/arm64 under Lima and Windows/amd64
under Wine. Lima test stores used guest `/tmp` unless explicitly rerun with
`TMPDIR=/var/tmp`; Wine files remained on host APFS. These are API/test oracles,
not deployment or NTFS performance evidence.

## Format and correctness

**OBSERVED:** format v1 contains six separately checksummed components:
objects, bindings, name bytes, path order/data, name order, and identity order.
The manifest binds root, generation, counts, light-mode count, catalogue digest,
segment size, and whole-file SHA-256. The frozen correctness segment is 1,978
bytes with SHA-256
`3f4554f5cea457874e21e10be268616325ef0c0d524af309050de37669f03ac3`.

**MEASURED:** the immutable reader and system-SQLite control both consume the
frozen `correctness-v1` digest
`dba2ace881e9893ab08b6c919f27087ba291c37f62c8d7f6efc53a989983de18`.
Both preserve the hard-linked object's two bindings, the symlink kind, Unicode
path bytes, case distinction, metadata, and canonical order. Durable exact
queries, alternate metadata sorts, cursors, and path-qualified hard-link
inspection match the exhaustive reference.

## Publication and recovery campaign

Publication boundaries are segment sync, segment rename, segment-directory
durability, manifest sync, manifest rename, and manifest-directory durability.
On Unix, rename publication is followed by directory `fsync`. On Windows, the
file is flushed and the rename uses `MoveFileExW(MOVEFILE_WRITE_THROUGH)`.

**MEASURED:** injected failures and abrupt subprocess exits at all six
boundaries recover only generation 1 or complete generation 2. Before manifest
rename, generation 1 remains selected; after manifest rename, generation 2 is
selected. Truncated segments, component corruption, whole-file digest mismatch,
corrupt newest manifests, and corrupt newest segments are rejected. A valid
older generation is selected and the recovery problem is reported to service
status. A reader pinned across two later publications stays queryable; its
unreferenced segment is reclaimed after the pin closes.

If no segment validates but a safe manifest/header still identifies the
approved root, the service starts degraded instead of making recovery
unreachable. `projection.rebuild` performs a full scan, atomically publishes a
new generation, and clears the recovery warning after success.

This is process-exit/crash-state evidence, not a physical power-cut result.

## One-million custom generation

The generated corpus contains 1,000,000 files, 100,000 directories, the root
object, and 1,100,000 bindings. One hundred thousand files share the basename
`repeated`.

| Measurement | Result |
|---|---:|
| exhaustive reference build | 1.482 s |
| durable publish | 429.543 ms |
| bounded manifest/header probe | 75.542 us |
| full checked recovery | 133.706 ms |
| segment bytes | 164,200,576 |
| bytes/binding | 149.27 |
| reopened heap delta before queries | 20,696 B |
| warmed heap delta | 2,268,248 B |
| populated read-cache bytes / limit | 2,064,384 / 8,388,608 B |

The `/usr/bin/time` maximum RSS was 842,399,744 B, dominated by constructing the
exhaustive M1 reference and transient publication input in the same process. It
is not an idle-service memory result and cannot establish the 48 MiB gate.

| Warm custom workload | Samples | p50 | p95 | p99 | max |
|---|---:|---:|---:|---:|---:|
| exact path, complete stored record | 20,000 | 1.333 us | 1.750 us | 2.834 us | 60.833 us |
| direct name, first 100 complete records | 2,000 | 133.084 us | 215.125 us | 452.625 us | 40.137 ms |
| exact pipeline, first 100 with ranks/evidence | 2,000 | 47.375 us | 56.209 us | 125.250 us | 176.209 us |

The path digest was `94065664886a96d8`; the direct-name and public-pipeline
digests were both `a69b9113a32327fd`.

## SQLite full-record control

The benchmark-only C control uses system SQLite 3.43.2,
`synchronous=FULL`, rollback journal, an 8 MiB page cache, mmap disabled,
separate object/binding tables, path primary key, and `(name,path)` index. Every
timed row consumes object ID, path, name, kind, size, mode, and modification
time. It does not construct Go evidence objects, so the process boundary remains
a named difference.

| Measurement | Result |
|---|---:|
| durable build/index | 2.166 s |
| database bytes | 130,555,904 |
| bytes/binding | 118.69 |
| combined build/query maximum RSS | 119,619,584 B |

| Warm SQLite workload | Samples | p50 | p95 | p99 | max |
|---|---:|---:|---:|---:|---:|
| exact path, full stored columns | 20,000 | 4 us | 4 us | 4 us | 19 us |
| exact name, first 100 full stored rows | 20,000 | 50 us | 53 us | 56 us | 164 us |

**MEASURED comparison:** the custom candidate publishes about five times faster
in these runs and returns exact paths faster. SQLite remains 20.5% smaller per
binding and narrowly wins exact-name p95 (53 us versus 56.209 us). Both are far
inside the one-million exact-query latency constitution at their measured
boundaries. Neither result establishes update write amplification or idle RSS.

## Accepted-gate comparison

- **PASS component proxy:** 149.27 bytes/binding is below 384 bytes/object for
  this representative name/basic-metadata workload.
- **PASS component proxy:** exact path and exact-name pipeline p95/p99 are below
  8/20 ms at one million.
- **PASS manifest-ready proxy:** bounded manifest/header probe is below 100 ms.
- **FAIL if full validation is required before readiness:** checked recovery is
  133.706 ms. Background validation or block-authenticated lazy reads require a
  separate correctness decision; the target is not weakened.
- **UNMEASURED:** isolated idle private memory, idle CPU/writes, GUI-to-result,
  write amplification, HDD behavior, 10m scale, and foreground compaction.

## Rejected/retained alternatives

- Generic Windows directory-handle flush was rejected after Wine returned
  access denied; see `rejected/WINDOWS_DIRECTORY_FLUSH_001.md`.
- Exhaustively reading a 100,000-item exact-name range for an unfiltered first
  page was rejected as the default plan; see
  `rejected/EXHAUSTIVE_EXACT_NAME_PAGE_001.md`.
- The exhaustive M1 heap remains the correctness control and remains rejected
  as the durable representation.
- SQLite remains a strong control. Its size and exact-name result are evidence
  that can still reverse low-level custom-layout choices; it is not a shipped
  dependency.

## Open M2 gates

1. add quarantine with status/API coverage;
2. run disk-full and damaged-both-manifests campaigns;
3. implement bounded deltas/compaction and measure <3x write amplification;
4. compare the declared copy-on-write-tree control under the same campaign;
5. define migration/current-plus-previous-major behavior;
6. isolate service startup/RSS/idle CPU and write measurements;
7. run physical APFS, NTFS, and ext4 power-loss/identity oracles;
8. measure 10m scale and HDD query/compaction behavior.
