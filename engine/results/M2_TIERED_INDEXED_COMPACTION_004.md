# M2 disk-indexed tiered compaction control 004

Status: **MEASURED bounded implementation; write, retained-memory, exact-state,
and warm relative-query gates pass; CANDIDATE only**.

Date: 2026-08-05. Source manifest (Go/C files, `go.mod`, and `Makefile`):
`2237d809e89e2f4c303fb98ee19d0f8637179424e4cc1b70e42a5bff0e3cea97`.

## Question and implementation

Can the retained eight-run cohort schedule operate without the rejected full
changed-path heap overlay and without an oracle checkpoint, while charging its
exact query indexes to the write budget?

The implementation adds:

- a checked disposable sidecar bound to the delta payload digest, catalogue
  digest, generation, and record count;
- fixed record-offset, 64-bit path-fingerprint, exact-name-fingerprint, and
  identity-order tables at 36 bytes per change plus a 256-byte header;
- exact stored-row verification after path/name fingerprint selection;
- a tiered exact view retaining O(run count) structural state, an explicit
  aggregate 8 MiB immutable-sidecar cache budget, and one immutable exact-name
  result cache capped at 100,000 ordinals (400,000 bytes), a maximum 1 MiB
  exact-liveness proof, and a 1,000-row/256 KiB immutable page proof;
- path-ordered streaming fresh-run and cohort writers that compare checked
  composite views without materializing a full checkpoint or changed-path
  union;
- batch validation for high-frequency names, replacing one newest-path probe
  per candidate with one hash-ordered pass per run.

The sidecars are rebuildable projections. Fingerprints locate candidates; they
never decide exact identity, path, or name equality.

## Environment and workload

- macOS 14.8.7/Darwin 23.6.0, Apple M3 arm64, 8 GiB RAM;
- Go 1.26.5 darwin/arm64; warm internal APFS state;
- `scale-v1`, 250,000 files plus 25,000 directories;
- 22 disjoint mixed epochs at 4,096 or 10,000 exact operations per epoch;
- metadata update, delete, rename, create, and hard-link changes;
- two eight-run cohort compactions, leaving two cohort runs plus six fresh
  runs;
- exact generation/digest validation after each fresh run and cohort;
- 20,000 paired path samples, one separately timed cold repeated-name query,
  and 500 paired warmed repeated-name samples returning the first 100 of about
  25,000 matches.

Command:

```sh
FILEMAN_ENGINE_M2_MEASURE=1 go test ./benchmarks \
  -run '^TestTieredIndexedCompactionDistribution$' -count=1 -v
```

## Result

| Batch | Final runs | Live delta + index bytes | Total delta bytes written | Total index bytes written | Logical bytes | Final indexed WA | Peak cohort-boundary indexed WA | Retained heap growth |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 4,096 | 8 | 18,662,312 | 26,630,904 | 5,615,616 | 13,336,576 | **2.4179x** | **2.7994x** | **12,201,736 B** |
| 10,000 | 8 | 45,549,128 | 64,995,096 | 13,692,288 | 32,560,000 | **2.4167x** | **2.7982x** | **16,814,072 B** |

| Batch | Cohort 1 | Cohort 2 | Max compaction allocation | Cold path | Base path p99 | Tiered path p99 | Ratio | Cold repeated name | Base name p99 | Tiered name p99 | Ratio |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 4,096 | 446.477 ms | 513.520 ms | 40,368,648 B | 70.125 us | 1.542 us | 0.500 us | **0.324x** | 27.251 ms | 20.542 us | 3.250 us | **0.158x** |
| 10,000 | 881.613 ms | 1.036 s | 62,803,976 B | 93.625 us | 4.084 us | 0.500 us | **0.122x** | 50.315 ms | 20.000 us | 3.208 us | **0.160x** |

The compaction allocation column is total allocation churn during the streamed
operation, not retained heap. It remains a CPU/GC optimization target even
though the retained-memory constitution passes.

The first diagnostic implementation took 384.030 ms p99 for the 4,096-change
repeated-name query and saturated one CPU during 2,000 samples. That run was
interrupted and is retained here as a negative intermediate result. The
hash-ordered batch validator, one-entry immutable query proofs, and bounded
page proof reduce cold resolution to 27-50 ms and warmed p99 to 3.21-3.25 us.
Cold values remain visible and are not folded into the warm distribution.

## Gate disposition

- **PASS component write gate:** indexed delta plus amortized cohort output is
  below 3x at both required batch sizes, including sidecar headers and rewrites
  but excluding the still-unimplemented manifest and filesystem metadata.
- **PASS retained-memory gate:** 12.1/16.7 MB growth is below the 48 MiB full
  service constitution and replaces the rejected 37.1/79.2 MB heap overlay.
- **PASS exact-state gate:** every fresh and compacted state matches the
  authoritative catalogue digest with no checkpoint oracle in the writer.
- **PASS warm relative query gate:** path p99 is 0.12-0.32x base-only and warm
  name p99 is 0.16x base-only at the measured repeated-query point. Cold path
  and high-frequency-name costs are reported separately; diverse-path and
  background-overlap distributions remain required.
- **OBSERVED candidate durability/publication:** isolated `TIERED.*` atomic
  manifest mechanics now pass logical/subprocess crash, short-write, fallback,
  repair, schema, and reclamation tests. They are not referenced by the live
  service manifest; native power loss remains required. See
  `M2_TIERED_MANIFEST_RECOVERY_005.md`.
- **OPEN long horizon:** 22 epochs do not select the next tier or base rewrite
  threshold.

Disposition: retain the disk-indexed cohort implementation for further
dogfood and optimization, but do not promote it to the production manifest.
Optimize record decode/allocation, measure diverse queries under background
compaction, charge candidate-manifest bytes, and run the long-horizon and
native durability controls before an ADR selects bounds.

## Verification matrix

- **PASS:** macOS/arm64 `go test ./...`, `go test -race ./...`, and
  `go vet ./...`;
- **PASS:** Linux/amd64 and Windows/amd64 `go build ./...`;
- **PASS compatibility oracle:** all 13 test-bearing Windows/amd64 packages
  under Wine on host APFS;
- **PASS compatibility oracle:** all 13 test-bearing Linux/arm64 packages under
  Lima with test temporary files on guest `/var/tmp` ext4.

Wine and Lima do not establish native NTFS/ext4 durability, filesystem
identity, installer behavior, or physical power-loss safety.

Implementation/measurement locators:
`internal/generation/delta_exact_index_candidate.go`,
`internal/generation/tiered_index_candidate.go`, and
`benchmarks/m2_tiered_indexed_test.go`.
