# M2 durability and update-cost measurement 002

Status: **MEASURED logical crash/partial-write evidence and update baseline;
M2 remains in progress and is not promoted**.

Date: 2026-08-05.

Source manifest (Go/C files, `go.mod`, and `Makefile`):
`867a0ae4cda413064b93909132fca2314887b2a3c9cb4cb95b19d7c10e63cd51`.

## Claim boundary

This result extends `M2_GENERATION_001.md` with evidence-preserving quarantine,
damaged-both-manifests recovery, authenticated schema-version rejection,
deterministic partial segment/manifest writes, a generation high-water mark,
publication allocation/write accounting, and the bounded streaming diff needed
for the next delta experiment.

It does **not** establish physical power-loss or physical ENOSPC behavior,
native NTFS behavior, hardware ext4 durability, schema migration execution,
delta publication, compaction, the copy-on-write-tree update control, isolated
idle-service RSS, ten-million scale, or HDD behavior.

## Environment

- macOS 14.8.7 (23J520), Apple M3 arm64, 8 logical CPUs, 8 GiB RAM;
- Go 1.26.5, `darwin/arm64`, APFS SSD, 129 GiB reported free;
- warm filesystem/process state with no cache-clearing method;
- host load near final measurement: 2.00 / 2.72 / 3.28;
- other host work was present, so maximum stalls are retained.

Compatibility tests used Wine 11.10 for Windows/amd64 API behavior on host APFS
and Lima Linux/arm64 kernel 7.0.0-28 with guest `/var/tmp` on ext4. They are not
NTFS or physical Linux deployment evidence.

## Quarantine and rebuild campaign

**OBSERVED:** recovery problems retain their exact manifest slot, sequence,
generation, and engine-owned segment name. Quarantine moves only validated
targets into an engine-owned `quarantine/` directory using the same durable
rename primitives as publication. It never overwrites prior evidence.

**MEASURED:**

- a corrupt newest segment fell back to generation 1; its segment and manifest
  were moved, and a subsequent checked recovery reported no rejected candidate;
- a corrupt newest manifest fell back to generation 1; the corrupt manifest
  and now-unreferenced segment were preserved;
- quarantine refused to remove the only root-bearing invalid generation before
  a checked replacement existed;
- after the only segment became invalid, service startup retained its safe
  manifest/header, rebuild committed generation 2, and two artifacts were then
  quarantined;
- after both manifest checksums were damaged, startup inferred no root. After
  the approved root was explicitly reapplied, rebuild committed generation 3.
  The overwritten corrupt slot was copied before publication, the other corrupt
  manifest was moved, and both orphan segments were preserved (four artifacts);
- authenticated manifest/segment headers establish a high-water mark, so
  fallback plus quarantine advanced from rejected generation 2 to generation 3
  rather than reusing generation 2. Quarantined segment headers remain in this
  high-water scan across restart through a checksummed 64-byte
  `quarantine/HIGHWATER` record. Publication does not scan historical evidence;
- an interrupted corrupt-manifest quarantine preserved the orphan segment
  first, restarted with the corrupt manifest still diagnostic, then moved the
  remaining manifest;
- if recovery publication overwrote a corrupt slot and the process restarted
  before quarantine, an atomic `quarantine/PENDING` marker surfaced synthetic
  recovery work. The orphan segment was preserved before the marker cleared.

Status reports rejected candidates while degraded, preserved-artifact count
after successful quarantine, and any incomplete-quarantine error. Quarantine
state remains internal; no public destructive administration method was added.

## Partial-write and schema campaign

**MEASURED:** deterministic write limits interrupted generation 2 at five
locations: the first segment byte, segment-header tail, first component,
manifest first byte, and manifest body. Every case reopened generation 1 only.
Explicit reclamation removed temporary/orphan publication debris without
removing the committed baseline.

This is a short-write/disk-full injection control. It does not claim the host
filesystem actually returned ENOSPC.

**OBSERVED and tested:** checksums are verified before version fields are
trusted. A checksum-authenticated newer major or minor returns
`ErrNewerFormat`; an authenticated older major returns
`ErrMigrationRequired`. A flipped, unauthenticated version bit remains ordinary
corruption. Persistent startup refuses a newer store without quarantining or
modifying it. Format v1 is the only readable major today; migration execution
remains open before a later major can ship.

## One-million publication and update control

The generated corpus contains 1,000,000 files, 100,000 directories, the root,
and 1,100,000 bindings. One metadata timestamp changed in the update case.

| Measurement | Result |
|---|---:|
| reference build | 1.821 s |
| initial durable publish | 224.434 ms |
| bounded manifest/header probe | 34.250 us |
| full checked recovery | 128.036 ms |
| segment bytes | 164,200,576 |
| publication total allocation | 1,740,832 B |
| publication transient heap growth | 1,740,832 B |
| publication retained heap growth after GC | 0 B |
| publication mallocs | 156 |
| initial live-store bytes | 164,200,926 B |
| initial write lower bound | 164,201,438 B |

The write lower bound counts the complete temporary segment, the 512-byte
header rewrite, and manifest bytes. It excludes filesystem metadata writes, so
it is deliberately conservative.

| Warm query workload | Samples | p50 | p95 | p99 | max |
|---|---:|---:|---:|---:|---:|
| exact path | 20,000 | 1.291 us | 1.625 us | 2.792 us | 57.875 us |
| direct name, first 100 | 2,000 | 127.417 us | 138.292 us | 234.750 us | 1.347 ms |
| exact pipeline, first 100 | 2,000 | 47.791 us | 53.458 us | 132.500 us | 171.542 us |

The one-record replacement shard took 1.433 s to build and 212.959 ms to
publish. It wrote at least 164,201,438 bytes: 2,565,647.47x the 64-byte canonical
changed object record. Keeping current and previous manifest generations grew
the live store from 164,200,926 to 328,401,852 bytes.

**REJECTED:** full-snapshot publication is not the ordinary small-update path.
It remains the authoritative rebuild and compaction-output control.

## Streaming diff

`generation.Diff` performs a cancellable path-ordered merge between the checked
segment and replacement shard. It includes exact object metadata, object
identity, and parent identity, so directory replacement updates child binding
authority even if lexical paths do not change.

**MEASURED:** it found one update among 1,100,000 bindings in 260.420 ms,
allocated 55,924,224 bytes over the scan, and retained 6,324,224 bytes after GC
as bounded cache growth. It holds only the current before/after rows and streams
changes to a sink. Allocation profiling/optimization remains available, but no
complete change array was hidden in this result.

The measured full-snapshot amplification admits a delta experiment; it does not
select delta format, run count, shadow representation, or compaction trigger.
Those candidates and gates are frozen in `docs/M2_DELTA_COMPACTION_SLICE.md`.

## Verification matrix

- **PASS:** `go test ./...` on macOS;
- **PASS:** `go test -race ./...` on macOS;
- **PASS:** `go vet ./...`;
- **PASS:** Linux/amd64 and Windows/amd64 `go build ./...`;
- **PASS compatibility oracle:** every Windows/amd64 package test executable
  under Wine 11.10;
- **PASS compatibility oracle:** every Linux/arm64 package test executable in
  Lima with `TMPDIR=/var/tmp` on guest ext4.

The final measured command had 830,406,656 B maximum RSS because it held and
rebuilt the exhaustive million-record reference shards in the same process. It
is not an isolated idle-service result. The reported macOS peak-footprint field
was 17,090,112 B; the discrepancy is retained rather than reinterpreted.

## Remaining M2 promotion gates

1. standalone candidate delta-run encoding/reader and identical update controls;
2. manifest integration only after run-count/merge/compaction evidence;
3. copy-on-write-tree update control and SQLite update campaign;
4. physical ENOSPC, power-loss, APFS, NTFS, and ext4 durability oracles;
5. executable current-plus-previous-major migration/rebuild path;
6. isolated startup/RSS/idle CPU/write measurements;
7. ten-million and HDD query/compaction measurements.
