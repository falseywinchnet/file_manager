# Test, fuzz, crash, and benchmark protocol

Status: **mandatory promotion gates**.

## Test layers

### Contract and pure invariants

- normalized path, case, Unicode, and collation fixtures;
- most-specific root ownership and duplicate suppression;
- stable serialization and schema decoding;
- exact predicate and ordering oracles;
- postings encode/decode and offset evidence;
- deterministic ranking and cursor order;
- API request/response golden corpus.

### Filesystem identity oracle

Replay at least:

- create, write, close, rename, same-volume move, cross-volume move;
- delete/recreate at the same path;
- hard links, symbolic links, clones where available;
- packages/bundles, sparse files, long paths, combining marks, mixed case;
- duplicate, delayed, out-of-order, and deliberately dropped events;
- root admission/removal and nested-root ownership changes;
- crashes after every declared commit boundary;
- scan reconciliation after event gaps.

Reject any implementation that authorizes or resolves an operation against the
wrong object, hides an identity collision, or retains a ghost after successful
reconciliation.

### Store campaign

- torn/truncated newest page or run;
- corrupted manifest and backup manifest;
- damaged old generation;
- disk full during commit and compaction;
- process kill at every fsync boundary;
- concurrent readers pinning old generations;
- migration interrupted before and after publication;
- integrity check, quarantine, bounded rebuild, and last-valid fallback.

### Query correctness

- exact names and paths;
- Unicode-normalized and case-sensitive platform variants;
- prefix/final-token behavior;
- phrases and offsets;
- negative predicates and ranges;
- typo/transposition cases and exact hard negatives;
- current-scope, descendant, root, and federated merges;
- unavailable and stale root behavior;
- no-result and continuation behavior;
- rank explanation reconstruction.

### Fuzzing

Fuzz parsers, page decoders, manifests, postings, cursors, query grammar,
normalization, event coalescing, and API frames. Seed the corpus with every fixed
regression. A fuzz crash becomes a permanent minimal fixture.

## Benchmark phases

1. Exhaustive reference correctness and asymptotic profile.
2. Single structure microbenchmarks with allocation profiles.
3. End-to-end warm/cold queries on 10k/1m/10m fixtures.
4. Event/update bursts with concurrent readers.
5. Crash-safe commit and recovery costs.
6. Control comparison under identical semantics.
7. Platform replication on APFS, NTFS, and ext4.

Report distributions, not averages alone. Preserve CPU and storage saturation
markers rather than attributing every stall to the engine.

## Sandbox validation

- process refuses an empty sandbox root;
- lexical `..` and prefix-confusion escapes fail;
- symlink/junction/reparse escapes receive platform tests before scanning code
  is enabled;
- no test accepts a home directory or repository parent;
- stores and temporary outputs remain outside indexed source roots;
- production approval manifests cannot be fabricated through the public query
  API.

## Promotion rule

Every optimization supplies:

1. correctness digest equal to the reference;
2. workload and baseline;
3. environment manifest;
4. quality metrics if ranking changes;
5. performance/resource distributions;
6. failure and reversal path.

An optimization is rejected if it improves median latency while violating exact
semantics, p99 interaction, write budget, memory budget, or recovery.

## Accepted initial resource gates

ADR-001 establishes the initial rejection constitution:

- cold manifest-ready service under 100 ms;
- idle private memory at or below 48 MiB for one million records and 96 MiB for
  ten million, without a complete heap-resident record mirror;
- idle CPU below 0.1% over ten minutes and zero writes without source changes;
- exact/prefix p95/p99 below 8/20 ms at one million and 25/50 ms at ten million;
- fuzzy filename p95 below 35 ms at one million;
- warm GUI-to-first-correct-result below 50 ms;
- representative name/basic-metadata storage below 384 bytes/object;
- mixed-workload update write amplification below 3×, including amortized
  compaction;
- HDD coverage and no foreground-query-triggered compaction.

Every report must state whether it tested a gate or merely a component proxy.
Missing the target is retained evidence; it is not permission to adjust the
denominator or silently weaken the threshold.

## Kolmogrov history-tuple dogfood

The disabled literal-only adapter has an opt-in native component campaign:

```sh
FILEMAN_ENGINE_KOLMOGROV_MEASURE=1 FILEMAN_ENGINE_RECORDS=1000000 \
  go test -v -run '^TestKolmogrovHistoryTupleDistribution$' -count=1 ./benchmarks
```

It differentially checks sampled verified results against a complete flat
one-edit scan and reports whole-memory and direct-to-file builds, bounded build
scratch, checked open, off-heap query distributions, probes, posting visits,
candidate tails, storage bytes, and retained heap. It is a filename-component
proxy on generated exact-length partitions, not a judged relevance corpus,
service admission, durability campaign, idle-service measurement, or native
filesystem result.

The private persistent-service integration campaign is separately opt-in:

```sh
FILEMAN_ENGINE_KOLMOGROV_SERVICE_MEASURE=1 FILEMAN_ENGINE_RECORDS=10000 \
  go test -v -run '^TestKolmogrovServiceGenerationDogfood$' -count=1 ./benchmarks
```

It uses disjoint temporary source, exact-store, and similarity-store roots and
reports exact reconcile/query/restart, generation-lease open, direct projection
build/open/query, retained heap, and identical-corpus bytes. The non-opt-in
service dogfood test owns mutation, stale-generation, cancellation, capacity,
corruption, read-only, and exact-fallback assertions.
