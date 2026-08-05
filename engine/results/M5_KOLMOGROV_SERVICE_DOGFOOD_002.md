# M5 exact-generation service dogfood 002

Status: **MEASURED internal integration evidence; exact-generation lease gate
passes, public fuzzy query and similarity-manifest admission remain blocked**.

Date: 2026-08-05.

Source manifest (Go/C files, `go.mod`, and `Makefile`):
`962603849ea6950f205ae831b9d3b314b47df4eed32e329c122d9c0be2d64b24`.

Raw transcript: `results/raw/M5_KOLMOGROV_SERVICE_DOGFOOD_002.txt`, SHA-256
`81c5484c7f056b6d8a2bab9b4e1cd7b47a895d82b6264442280875f0573a1bc1`.

## Claim boundary

This round connects the disabled Kolmogrov component to a pinned, committed
exact catalogue generation through a private service lease. The direct builder
streams exact filenames by generation-local ordinal and candidate verification
resolves full anchors lazily from the exact reader. It does not copy a
catalogue-sized name/anchor input array, expose fuzzy search through
`engine.v0`, or add the disposable component to the durable manifest.

The durable store opens a second header-checked reader only when its manifest
generation and catalogue/segment digests match the already authenticated live
reader. Its existing pin count keeps that exact segment alive across a
concurrent later publication. A stale projection cannot open against a new
same-cardinality exact generation.

## Sandbox and attack result

`TestPersistentSimilaritySandboxDogfood` creates disjoint source, exact-store,
and similarity-store children beneath `t.TempDir`. It exercises:

- durable scan, projection build, exact anchor inspection, verified adjacent
  transposition, and a Unicode-scalar filename;
- rename, deletion, same-path replacement, insertion, and generation rollover;
- an old pinned reader remaining valid while its candidate generation is
  visibly stale, followed by fail-closed open against generation two;
- cancelled and deliberately over-capacity builds leaving no component;
- restart with exact recovery and projection reopen;
- header corruption returning `corrupt_projection` while exact query remains
  healthy;
- repeated exact and fuzzy reads leaving exact-store and projection file size,
  mode, and modification time unchanged; and
- absence of engine segments, manifests, or projection files in the indexed
  source tree.

A separately compiled `fileman-engine` binary also completed JSONL version,
root admission, scan, exact query, integrity, and shutdown against an explicit
temporary sandbox. Its observed engine writes were confined to the separate
store directory.

## 10,000-file preliminary measurement

Environment: macOS 14.8.7, Apple M3 arm64, Go 1.26.5, internal APFS SSD, warm
process/filesystem state, no cache clearing. The generated non-sensitive corpus
contains 10,000 zero-byte files in one exact length partition. This is one
integration run, not a statistically repeated release claim or a judged
relevance corpus.

| Measurement | Result |
|---|---:|
| exact reconcile | 72.255 ms |
| exact generation lease open | 31.625 us |
| exact store bytes / record | 1,540,933 / 154.09 B |
| exact-service retained heap delta | 35,216 B |
| direct similarity build | 50.488 ms |
| similarity build allocation / bounded scratch | 4,439,560 / 2,520,000 B |
| checked similarity open | 1.231 ms |
| similarity file bytes / record | 1,989,979 / 199.00 B |
| exact plus similarity bytes / record | 353.09 B |
| combined retained heap delta | 836,328 B |
| checked exact restart | 1.330 ms |

| Warm path | Samples | p50 | p95 | p99 | max |
|---|---:|---:|---:|---:|---:|
| exact name | 1,000 | 5.583 us | 7.917 us | 19.250 us | 112.667 us |
| off-heap verified fuzzy | 1,000 | 14.584 us | 150.708 us | 187.041 us | 418.667 us |

The first integration run decoded full rows for every build visit. Introducing
a narrow exact-reader filename path and a once-per-lease authenticated
generation reduced observed build allocation from 15,747,104 to 4,439,560
bytes, build time from 77.190 to 50.488 ms, and combined retained heap from
1,810,528 to 836,328 bytes on the same 10,000-record setup. These before/after
figures are optimization observations, not independent benchmark repetitions.

## Gate disposition

**PASS:** a disposable projection can be built and queried from the exact
service's immutable committed generation without a second catalogue mirror.

**PASS:** stale generation, cancellation, capacity, and corruption failures do
not disable exact query. No read-path writes were observed in the bounded
campaign.

**PASS component proxy:** 353.09 combined bytes/record is below the provisional
384-byte name/basic-metadata storage target on this deliberately compact
same-corpus fixture. A realistic filename/path distribution and filesystem
allocation accounting can reverse this result.

**UNRESOLVED:** every changed generation still requires a full roughly
199-byte/record similarity rewrite. This does not pass the mixed-update write
amplification, SSD-wear, dead-posting, or compaction gate. The live service must
not publish this component on every event or foreground query.

**UNRESOLVED:** similarity manifest atomicity, crash boundaries during
publication, background scheduling, idle CPU, physical power loss, native
NTFS/ext4 identity, judged quality, production parameters, lexical controls,
and public planner fallback remain required. Because the component is derived,
exact readiness continues independently and corruption means erase/rebuild,
not source or exact-store repair.

## Verification matrix

- **PASS:** `go test ./...`, `go test -race ./...`, and `go vet ./...` on
  macOS/arm64;
- **PASS:** Linux/amd64 and Windows/amd64 `go build ./...`;
- **PASS compatibility oracle:** all 13 test-bearing Windows/amd64 packages
  under Wine 11.10 on host APFS;
- **PASS compatibility oracle:** all 13 test-bearing Linux/arm64 packages under
  Lima kernel 7.0.0-28 with test temporary files on guest `/var/tmp` ext4.

Wine and Lima do not establish native NTFS/ext4 durability, filesystem
identity, installer behavior, or physical power-loss safety.
