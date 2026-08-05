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

## M2 durable-generation and SQLite control

The M2 measurement is separately opt-in. `FILEMAN_ENGINE_RECORDS` defaults to
one million and may be lowered only for an explicitly labeled scale point:

```sh
FILEMAN_ENGINE_M2_MEASURE=1 FILEMAN_ENGINE_RECORDS=1000000 \
  go test -v -run '^TestImmutableGenerationDistribution$' -count=1 ./benchmarks
FILEMAN_ENGINE_M2_MEASURE=1 FILEMAN_ENGINE_RECORDS=1000000 \
  go test -v -run '^TestDeltaCandidateBatchDistribution$' -count=1 ./benchmarks
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
