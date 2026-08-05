# M2 repeated consolidation and replacement-base control 002

Status: **MEASURED negative policy evidence; repeated eight-run consolidation
and full-base replacement at that trigger are REJECTED for this representation
and workload; no live format is admitted**.

Date: 2026-08-05. Source manifest (Go/C files, `go.mod`, and `Makefile`):
`3ba0198986b43be3a4fc6994d08487fbdbd9271d93ee10f4b448066413ca8fe7`.

## Question

The first consolidation result measured one eight-run cycle at about 2.31x
application write amplification. This experiment asks what happens when the
same immutable base remains in place and the engine repeatedly enforces the
eight-run query/memory bound, and whether writing a replacement base instead
of the current consolidation satisfies the less-than-3x gate.

It does not select a scheduler or change manifest v1. It exercises checked
component writers/readers and exact digest chains only.

## Environment and workload

- macOS 14.8.7/Darwin 23.6.0, Apple M3 arm64, 8 GiB RAM;
- Go 1.26.5 darwin/arm64; warm internal APFS state;
- generated `scale-v1`: 250,000 files plus 25,000 directories;
- disjoint mixed epochs with 4,096 or 10,000 exact operations per epoch using
  the existing metadata-update/delete/rename/create/hard-link distribution;
- an active chain is consolidated whenever it reaches eight runs: fresh epochs
  1–8, then the consolidated run plus seven fresh runs at epochs 15 and 22;
- three actual paced consolidation cycles, each reopened and checked against
  the exact reference digest;
- at every cycle, an actual full replacement segment is also written as a
  counterfactual “replace instead of this consolidation” control;
- 250 us pacing pause after every 256 consolidated net operations. Values are
  experiment inputs, not production policy.

Command:

```sh
FILEMAN_ENGINE_M2_MEASURE=1 go test ./benchmarks \
  -run '^TestRepeatedConsolidationAndReplacementBaseDistribution$' \
  -count=1 -v
```

## Results

Application bytes include every fresh run or prior consolidation written in
the schedule plus the fixed final header rewrite. The replacement schedule uses
all bytes written before the current consolidation plus the measured full base
replacement. Both exclude manifest slots, filesystem metadata, reclamation,
and device amplification, so crossing 3x already fails the complete gate.

| Batch | Cycle/epochs | Net paths | Logical bytes | Runs + consolidations | Consolidation WA | Replacement base | Replace-this-cycle WA |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 4,096 | 1 / 8 | 32,768 | 4,849,664 | 11,212,452 | 2.3120x | 40,926,584 | 9.5955x |
| 4,096 | 2 / 15 | 61,440 | 9,093,120 | 26,626,756 | 2.9282x | 40,817,616 | 6.2616x |
| 4,096 | 3 / 22 | 90,112 | 13,336,576 | 46,943,972 | **3.5199x** | 40,708,672 | 5.4169x |
| 10,000 | 1 / 8 | 80,000 | 11,840,000 | 27,365,796 | 2.3113x | 40,747,088 | 4.5973x |
| 10,000 | 2 / 15 | 150,000 | 22,200,000 | 64,990,948 | 2.9275x | 40,481,088 | **3.5956x** |
| 10,000 | 3 / 22 | 220,000 | 32,560,000 | 114,586,100 | **3.5192x** | 40,215,088 | **3.5989x** |

Consolidation elapsed from 0.363 s to 2.267 s as the net changed-path union
grew. Full replacement writes took 0.058–0.091 s in this warm component test;
that throughput does not rescue their byte cost and is not a service-overlap
latency result.

## Disposition

- **REJECTED policy:** repeatedly rewrite the growing net run every time a
  chain reaches eight. It crosses the application-byte gate on cycle three for
  both required batch sizes before manifest/filesystem costs are counted.
- **REJECTED policy at this trigger:** replace the roughly 40 MiB base whenever
  this 250,000-record chain reaches eight. It exceeds 3x at every 4,096 point
  and at cycles two/three for 10,000.
- **NOT REJECTED:** immutable delta encoding, the eight-run measured query
  bound, or background base compaction under a different measured trigger.
- **UNRESOLVED:** a passing policy needs a different amortization boundary,
  lower-retention overlay/leveled structure, smaller independently compacted
  shards/components, or a measured combination. It still must preserve one
  atomic root-policy/watermark/component manifest commit and bounded queries.

The result also closes an ambiguity: component algorithms, including lexical
and content/fragment similarity descriptors, share this write budget. No
Kolmogrov-specific compaction path is implied or warranted.

## Correctness and limitations

Every fresh run, multi-run overlay, consolidated run, and replacement base
matched the target exact catalogue digest/generation. Active run count never
exceeded eight. This is application write accounting on a generated component
workload, not a 24-hour service trace, crash campaign, manifest measurement,
physical NAND measurement, or native NTFS/ext4 evidence.

Verification after the measurement: **PASS** macOS `go test ./...`, race,
`go vet`, Linux/amd64 and Windows/amd64 cross-builds; **PASS compatibility
oracle** all 13 test-bearing Windows/amd64 packages under Wine 11.10 and all 13
Linux/arm64 packages under Lima with guest `/var/tmp`. Wine/Lima do not add
native durability evidence.

Implementation/measurement locator:
`benchmarks/m2_repeated_cycle_test.go`. Shared mechanics:
`internal/generation/overlay_candidate.go` and
`internal/generation/segment.go`.
