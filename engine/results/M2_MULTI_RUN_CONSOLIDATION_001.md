# M2 multi-run and consolidation measurement 001

Status: **MEASURED component evidence; eight runs are the strongest tested
candidate bound, sixteen runs are REJECTED for this heap-overlay
representation, and no live format or compaction policy is admitted**.

Date: 2026-08-05.

Source manifest (Go/C files, `go.mod`, and `Makefile`):
`1f7c6e70baf4bd6b29565a6c3b85cf871646d8257c89fc54679d2482e0a0552e`.

## Claim boundary

The experiment adds three candidate mechanisms without changing manifest v1 or
the service query path:

1. standalone delta format 1.1 binds both the exact base generation and base
   catalogue digest, so sequential runs form a checked generation/digest chain;
2. `NewMultiRunOverlayCandidate` collapses only the caller-bounded net changed
   path union over an off-heap immutable base and exposes the existing exact
   index interface;
3. `WriteConsolidatedDeltaCandidate` rewrites that net union as one run relative
   to the same base, eliminating changes that cancel across runs. It does not
   rewrite the base or publish a manifest.

This is one consolidation cycle. It excludes repeated consolidation cycles,
manifest/header-slot publication, replacement-base compaction, reclamation,
crash boundaries, and physical filesystem write amplification. Consequently
the complete less-than-3x update gate is not passed by this result.

## Environment and workload

- macOS 14.8.7 (23J520), Apple M3 arm64, 8 logical CPUs, 8 GiB RAM;
- Go 1.26.5, `darwin/arm64`, internal APFS SSD;
- generated `scale-v1`: 1,000,000 files, 100,000 directories, 1,100,000 base
  bindings;
- disjoint mixed runs: 25% metadata updates, 25% deletions, 12.5% renames
  represented as delete plus add, 12.5% creates, and 12.5% hard-link adds;
- checkpoints at 2, 4, 8, and 16 runs for both 4,096 and 10,000 changes/run;
- 20,000 paired/alternating base-versus-overlay exact-path samples, 2,000 paired
  repeated-name top-100 samples, and up to 50,000 exact-path samples while
  consolidation ran;
- paced consolidation candidate: cooperative yield and a 250 us pause after
  every 256 net operations. These pacing values are experiment inputs, not a
  selected production policy;
- warm process/filesystem state; no cache-clearing method.

## One-million-record result

| Batch | Runs | Cumulative/net paths | Open | Open allocation | Retained heap | Consolidate | Cycle WA | Path p99/base | Name p99/base | Overlap/quiescent |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 4,096 | 2 | 8,192 | 38.170 ms | 12,095,848 B | 2,882,480 B | 105.345 ms | 2.3124x | 1.016x | 0.809x | 1.330x |
| 4,096 | 4 | 16,384 | 62.029 ms | 24,395,256 B | 5,764,624 B | 190.159 ms | 2.3121x | 1.041x | 0.787x | 1.726x |
| 4,096 | 8 | 32,768 | 150.406 ms | 49,049,600 B | 11,527,368 B | 375.032 ms | 2.3120x | 0.996x | 0.952x | 1.348x |
| 4,096 | 16 | 65,536 | 312.665 ms | 98,164,248 B | 23,057,520 B | 724.095 ms | 2.3119x | 1.136x | 0.974x | **2.761x** |
| 10,000 | 2 | 20,000 | 77.513 ms | 28,342,768 B | 6,611,928 B | 243.184 ms | 2.3114x | 1.065x | 0.713x | 0.961x |
| 10,000 | 4 | 40,000 | 195.096 ms | 57,126,392 B | 13,193,696 B | 444.934 ms | 2.3113x | 1.010x | 0.907x | 0.774x |
| 10,000 | 8 | 80,000 | 373.227 ms | 114,469,544 B | 26,353,016 B | 839.299 ms | 2.3113x | 0.934x | 0.772x | 1.904x |
| 10,000 | 16 | 160,000 | 829.194 ms | 229,123,336 B | **52,674,472 B** | 4.989 s | 2.3113x | 1.002x | 0.677x | 1.553x |

The ratios below one do not establish that the overlay is faster. They show
paired-run timing variation. The bounded claim is whether the candidate crossed
2x in the named run.

**MEASURED:** all quiescent exact-path and repeated-name ratios were below 2x.
At eight runs both batch sizes also held paced-consolidation overlap below 2x.

**REJECTED:** sixteen is not a viable general run bound for this representation.
The 10,000-change workload retains 52,674,472 bytes for the overlay alone,
already above the complete service's 48 MiB one-million-record idle ceiling.
The 4,096-change workload also measured 2.761x overlapping path p99 at sixteen
runs. Reconsideration requires a representation that does not retain one heap
entry per net changed path or stronger repeated latency evidence under an
explicit resource scheduler.

**CANDIDATE, not DECIDED:** eight runs and at most 80,000 net changed paths are
the strongest tested point. Its largest overlay retained 26,353,016 bytes, but
the complete service memory—including Go runtime, readers, caches, transport,
and supervision—was not isolated, so this is not proof of the 48 MiB service
gate. A run count without cumulative change and byte budgets is insufficient.

## Write economy and foreground pressure

At eight 10,000-change runs, input runs wrote at least 13,685,016 application
bytes, consolidation wrote at least 13,680,627 bytes, and the canonical logical
stream was 11,840,000 bytes: 2.3113x for the first run-plus-consolidation cycle.
The corresponding 4,096 workload was 2.3120x. Each lower bound includes the
fixed final header rewrite but excludes filesystem metadata and a future
manifest commit.

An initial consolidation implementation repeated a binary base-path lookup for
every net change and repeatedly breached the overlap target. Retaining the
already-validated base ordinal removes that lookup; updates and additions no
longer touch the base reader during consolidation, while tombstones read the
exact base row they must encode. Cooperative pacing then trades background
completion time for foreground headroom without adding writes or busy waiting.

This result does not model a second consolidation cycle. Rewriting an already
consolidated run can make amplification cumulative, and replacement-base cost
is still absent. No trigger may be selected until a multi-cycle/24-hour model
includes both.

## Correctness and failure evidence

- mixed sequential generations match the exact reference after every run;
- ordered composite iteration matches every final reference row;
- digest and generation discontinuities, wrong-base same-generation inputs,
  run-count overflow, cumulative-change overflow, and cancellation fail closed;
- delete/add, add/delete, and update/revert sequences are removed from the net
  changed union;
- consolidated one-run state matches the multi-run state and target catalogue
  digest;
- consolidation cancellation before its first operation leaves no output;
- standalone payload corruption and truncation remain rejected by the checked
  delta reader.

Test locators: `internal/generation/multi_run_candidate_test.go`,
`internal/generation/delta_candidate_test.go`, and the opt-in measurement in
`benchmarks/m2_multi_run_test.go`.

## Verification matrix

- **PASS:** `go test ./...` on macOS;
- **PASS:** `go test -race ./...` on macOS;
- **PASS:** the 200,000-record query/consolidation overlap campaign under the
  race detector (421.301 s);
- **PASS:** `go vet ./...`;
- **PASS:** Linux/amd64 and Windows/amd64 `go build ./...`;
- **PASS compatibility oracle:** all 13 Windows/amd64 package test executables
  under Wine 11.10 on host APFS;
- **PASS compatibility oracle:** all 13 Linux/arm64 package test executables in
  Lima with each executable copied onto guest ext4.

Wine and Lima do not establish native NTFS/ext4 durability or power-loss
behavior.

## Remaining promotion gates

1. repeat paired distributions under controlled power/thermal state and add a
   service-level query scheduler rather than selecting the measured pacing;
2. measure overlapping name queries, query throughput, CPU, RSS/private bytes,
   backlog age, and actual complete-service idle memory;
3. run repeated consolidation cycles and replacement-base compaction against
   the less-than-3x 24-hour write-amplification gate;
4. add manifest publication, reader pins, reclamation, and fault injection at
   every consolidation/base-compaction durable boundary;
5. add directory light-threshold churn and hard-link removal/intrinsic-change
   scale cases;
6. compare the identical transcript with the copy-on-write and SQLite controls;
7. admit no run/change/pacing bound except through a numbered decision record.
