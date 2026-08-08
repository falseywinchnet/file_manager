# Benchmark harness contract

Benchmarks compare the purpose-built engine with controls on identical records
and semantics. A control may use SQLite/FTS5, Xapian, a sorted-vector scan, or a
flat similarity scan; it may not quietly answer a weaker query.

Every run records:

- engine revision and schema version;
- OS/build, filesystem/format, CPU, RAM, storage, free space, power and thermal
  state;
- fixture name, generator revision, seed, record count, and indexed bytes;
- cold/warm state and cache-clearing method;
- p50, p95, p99, maximum stall, throughput, CPU, resident/peak bytes;
- bytes read/written, write amplification, index bytes/record, rebuild time,
  backlog age, and result correctness digest.

## Ambitious candidate gates

These are **CANDIDATE** targets until the architect approves representative
hardware and workloads:

- common warm scoped name/metadata top-k queries complete in single-digit
  milliseconds at one million records;
- exact object/path lookup is sub-millisecond warm at the engine boundary;
- a 10,000-event burst exposes no partial reader generation and keeps query p99
  within twice its quiescent value when storage is not saturated;
- a one-record update writes at least ten times fewer bytes than a full snapshot
  rebuild;
- disabled service retains zero engine worker memory and causes no GUI failure;
- every quality optimization reports Recall@k/MRR/nDCG and may not trade away
  exact literals, negative filters, or identity correctness.

Do not tune the benchmark fixture into the implementation. Keep public workload
manifests and preserve rejected layouts in `results/rejected/`.

## M1 reference commands

The checked-in M1 measurement harness is opt-in so the ordinary correctness
suite does not create 10,000 filesystem fixtures:

```sh
FILEMAN_ENGINE_MEASURE=1 go test -v -count=1 ./benchmarks
go test -run '^$' -bench 'Benchmark(ReferenceShardBuild100k|ExactNameUnique100k|ExactNameRepeated10kOf100k|OwnerAmongNestedRoots)' -benchmem -count=5 ./internal/catalog ./internal/exact ./internal/ownership
```

`TestReferenceExactDistribution` uses generated objects only.
`TestSandboxScanAndServiceDistribution` creates and deletes its complete corpus
under `t.TempDir`; it never scans a user or repository tree. These M1 numbers
are reference/control evidence. They do not establish the accepted one-million
or ten-million durable-engine gates.

## M1L live-query control

The opt-in live-query campaign creates a 10,000-entry zero-byte flat corpus in
`t.TempDir`, leaves the catalogue unreconciled, consumes every progressive
page, and reports first-result time, full traversal time, Go allocation,
descriptor high-water, work counters, and catalogue-generation invariance:

```sh
FILEMAN_ENGINE_LIVE_MEASURE=1 go test -v -run '^TestLiveQueryDistribution$' -count=1 ./benchmarks
```

This scale point is a local implementation check. It does not satisfy the
required million-entry, deep-tree, RSS, CPU, bytes-read, NTFS, or ext4 gates.

## M2 durable-generation and SQLite control

The M2 measurement is separately opt-in. `FILEMAN_ENGINE_RECORDS` defaults to
one million and may be lowered only for an explicitly labeled scale point:

```sh
FILEMAN_ENGINE_M2_MEASURE=1 FILEMAN_ENGINE_RECORDS=1000000 \
  go test -v -run '^TestImmutableGenerationDistribution$' -count=1 ./benchmarks
FILEMAN_ENGINE_M2_MEASURE=1 FILEMAN_ENGINE_RECORDS=1000000 \
  go test -v -run '^TestDeltaCandidateBatchDistribution$' -count=1 ./benchmarks
FILEMAN_ENGINE_M2_MEASURE=1 FILEMAN_ENGINE_RECORDS=1000000 \
  go test -v -run '^TestOverlayCandidateMixedDistribution$' -count=1 ./benchmarks
FILEMAN_ENGINE_M2_MEASURE=1 FILEMAN_ENGINE_RECORDS=1000000 \
  go test -v -run '^TestMultiRunConsolidationDistribution$' -count=1 ./benchmarks
FILEMAN_ENGINE_SQLITE_CONTROL=1 \
  go test -v -run '^TestSQLiteCorrectnessV1Control$' -count=1 ./benchmarks
```

The external timed SQLite control is documented under `controls/`. It links
only the standalone benchmark program against the host's system SQLite. It is
not imported by Go packages or shipped as an engine dependency. Compare the
full-record query transcript, storage bytes, build mode, cache policy, and
process boundary; do not compare a weaker ID-only SQL projection with a complete
engine result.

The immutable-generation measurement also changes one object's timestamp,
streams the exact base-to-replacement diff, and then publishes the full snapshot
control. Its `write_lower_bound` includes segment bytes, the fixed header
rewrite, and manifest bytes but excludes filesystem metadata. The reported
amplification denominator is the 64-byte canonical changed object record. This
control demonstrates the cost that a candidate delta run must beat; it is not a
delta-format result.

The standalone delta batch measurement uses an encoding-independent logical
operation denominator and reports 4,096/10,000-update run amplification. It
excludes production manifest publication and amortized compaction, so it cannot
promote the format by itself.

The overlay measurement applies deterministic mixed update/delete/rename/create
and hard-link batches, checks the one-run view against the exact reference, and
compares warm base-only and base-plus-run p99. It measures one checked run only;
it does not establish a maximum run count or compaction result.

The multi-run measurement uses disjoint mixed batches through 2/4/8/16 checked
digest-chained runs. It alternates base and overlay timing sample order, measures
exact-path queries during explicitly paced consolidation, reports retained heap
and first-cycle write amplification, and checks the consolidated run against the
target digest. It excludes manifest bytes, repeated consolidation cycles, and
replacement-base compaction.

The repeated-cycle control enforces an eight-run active bound for three real
cycles and writes a full replacement base at every cycle as a counterfactual.
It reports cumulative application bytes against only fresh logical operations:

```sh
FILEMAN_ENGINE_M2_MEASURE=1 go test ./benchmarks \
  -run '^TestRepeatedConsolidationAndReplacementBaseDistribution$' \
  -count=1 -v
```

It rejects the measured trigger policies when their cumulative application
write amplification crosses 3x; it does not publish a manifest or select an
alternate compaction policy.

The tiered-cohort control compacts only each completed eight-run cohort and
leaves earlier cohorts immutable:

```sh
FILEMAN_ENGINE_M2_MEASURE=1 go test ./benchmarks \
  -run '^TestTieredCohortCompactionDistribution$' -count=1 -v
```

It measures output bytes and the final heap overlay separately. Full checkpoint
segments are exact diff oracles and excluded from candidate bytes; therefore a
streaming compactor remains required before admission.

The indexed continuation removes both limitations: checked sidecars serve
exact path/name/identity candidates, and cohort output streams directly from
the tiered view. It charges index bytes and reports cold plus warmed query
latency:

```sh
FILEMAN_ENGINE_M2_MEASURE=1 go test ./benchmarks \
  -run '^TestTieredIndexedCompactionDistribution$' -count=1 -v
```

Its write, retained-memory, exact-state, and warm relative-p99 query gates pass;
cold and background-overlap costs remain separate promotion evidence.
See `results/M2_TIERED_INDEXED_COMPACTION_004.md`.

## M5 Kolmogrov history-tuple candidate

The Kolmogrov measurement is opt-in and uses generated, non-sensitive filename
components. One million records are distributed across sixteen exact lengths so
each plan partition remains within the sealed 65,536-record dogfood envelope:

```sh
FILEMAN_ENGINE_KOLMOGROV_MEASURE=1 FILEMAN_ENGINE_RECORDS=1000000 \
  go test -v -run '^TestKolmogrovHistoryTupleDistribution$' -count=1 ./benchmarks
```

The flat control invokes the same matrix-free exact verifier over every record.
The adapter result includes hash candidate production plus exact verification;
it is not compared against a weaker hash-only kernel. The corpus is a resource
and correctness stressor with many near-duplicate hexadecimal names, not a
judged relevance set.

The exact-service integration campaign creates one bounded real filesystem
fixture under `t.TempDir` and measures the persistent exact reader and the
disposable projection on identical records:

```sh
FILEMAN_ENGINE_KOLMOGROV_SERVICE_MEASURE=1 FILEMAN_ENGINE_RECORDS=10000 \
  go test -v -run '^TestKolmogrovServiceGenerationDogfood$' -count=1 ./benchmarks
```

The accepted range is 1,000 through 65,000 records so the single-length
fixture cannot cross the sealed partition cap. This is an integration/resource
measurement, not a judged relevance or physical power-loss campaign.
