# Standalone engine delivery plan

Status: **implementation sequence; each milestone has a rejection gate**.

## M0 — reproducible shell

- Go module builds without network access after dependencies are vendored or
  eliminated.
- sandbox-required service, protocol smoke test, CI commands, fixture generator,
  result-manifest schema.
- API v0 golden messages.

Lifecycle/configuration continuation: **OBSERVED development slice**. One
process instance has random restart identity, ready/draining/stopped states,
cancel-and-drain shutdown, effective-configuration digest, stale-safe canonical
root apply, and an honest capability/backlog projection. Native service
installation, authenticated query/admin endpoints, observable startup,
crash-loop/update policy, and bounded status subscription remain release gates.

## M1 — exact reference catalogue

- canonical record and platform identity adapters;
- approved-root manifest and longest-root ownership;
- exhaustive in-memory/sorted reference store;
- live scan into a disposable sandbox;
- exact query, metadata ordering, inspect, and status;
- identity/mutation oracle on macOS first.

### M1L — required catalogue-independent live query

- implement `ORC-ENG-004` bounded name/path traversal without creating or
  requiring a catalogue;
- stream progressive pages with expiring query-bound cursors and exact live
  source evidence;
- preserve authorized-root containment, do not traverse directory links, and
  report permission/mutation gaps as partial rather than empty success;
- cap visited entries, metadata calls, wall time, open directories, result
  count, response bytes, and retained continuation state;
- prove prompt cancellation and zero durable Engine writes;
- measure first-result/page latency, CPU, RSS, descriptors, and bytes read on
  the guard workloads and native APFS/NTFS/ext4 fixtures.

Status: **REQUIRED under ADR-008; not implemented; Engine reply 006 pending**.

## M2 — custom durable generation

- implement bounded immutable root generations with atomic manifest
  publication;
- compare copy-on-write tree and SQLite controls under the identical
  crash/corruption campaign; retain evidence that could reverse the target;
- stable readers, checksums, integrity, quarantine, rebuild;
- one-root persistent engine with schema manifest;
- SQLite exact-store control for validity and performance comparison.

Current status: **IN PROGRESS**. Immutable publication, dual-slot recovery,
pinned readers, degraded-start projection rebuild, one-root persistent service,
schema fixture, abrupt-exit tests, and SQLite correctness/performance controls
are observed. Evidence-preserving quarantine, deterministic partial-write
campaigns, damaged-both-manifests rebuild, authenticated schema rejection, and
a bounded streaming generation diff are also observed. A standalone
checksummed delta writer/reader has component measurements for one, 4,096, and
10,000 metadata updates without changing the live manifest. A separate
base-plus-one-run exact overlay now has mixed-mutation differential tests and
one-million-record component measurements; it is not wired into the service or
manifest. A digest-chained multi-run overlay and one net-run consolidation
cycle are measured at 2/4/8/16 runs. Eight is the strongest tested point;
sixteen is rejected for this heap-overlay representation. Three actual
eight-run enforcement cycles also reject indefinite net-run rewriting after
cumulative amplification reaches 3.52x; full-base replacement at the same
250,000-record triggers is likewise rejected. No run/change or pacing policy
is selected. A cohort-only tiered schedule now has a checked disk-indexed exact
view and a checkpoint-free streaming compactor. At 22 epochs it measures 2.42x
final and at most 2.80x cohort-boundary indexed writes with 12.1/16.7 MB
retained growth, replacing the rejected 79.2 MB heap overlay. Its warm relative
exact-query p99 gate and isolated atomic-manifest crash/recovery campaign now
pass, but it remains outside the live service manifest. Physical disk-full and
power-loss campaigns, live delta publication,
long-horizon tier/base policy, migration execution, service-level overlap
trials, and the copy-on-write-tree control remain open promotion gates.

## M3 — lexical engine

- normalized field model and term dictionary;
- positional postings, phrase offsets, negative predicates;
- scoped query planner and pagination;
- judged lexical corpus and linear/FTS/Xapian controls;
- deterministic explanations.

## M4 — incremental ingestion

- native-adapter event model;
- committed native observation watermarks without ordinary per-event engine
  journaling;
- bounded coalescing, gap detection, event storms, scan reconciliation;
- batched durable publication and reader generations;
- polite scheduling, backlog status, battery/resource policy seams.

Current status: **OBSERVED/MEASURED experimental macOS slice and OBSERVED
Windows compatibility slice; not exact or durable currentness admission**. A
platform-neutral chained-cursor model, bounded
double-buffered coalescer, replay/gap/overflow/root-invalidation handling,
scan-overlap state machine, backoff, currentness status, opt-in macOS FSEvents,
and opt-in Windows `ReadDirectoryChangesW` adapters now drive full authoritative
reconciliation. Native macOS sandbox dogfood and a 4,096-operation storm pass;
the retained 50 ms FSEvents / 100 ms maximum-age candidate uses three
full-generation publications. macOS final-hard-link removal and Windows
root-replacement/journal coverage remain incomplete, so both adapters fail
closed rather than reporting current. The live manifest also does not commit
the source watermark, so restart forces a baseline. Native NTFS validation, a
Linux adapter, live delta publication, battery/device-power evidence, and
atomic watermark/root-policy manifest admission remain gates.

## M5 — fuzzy and structural candidates

- character-gram and deletion-dictionary controls;
- bounded Damerau-Levenshtein verification;
- exact hard-negative protection;
- versioned core similarity-channel interface;
- Kolmogrov reference-vector and conformance adapter when its transfer bundle
  passes the gates in `KOLMOGROV_CHANNEL_SEAM.md`;
- controls remain runnable when that adapter is absent or rejected.

Current status: **IN PROGRESS, disabled experimental slice observed**. The
literal-only coupled 40-bit history-tuple adapter, typed three-length plans,
bounded one-edit/transposition verifier, explicit overflow/budget states,
immutable liveness, cross-language fixture, checked off-heap component reader,
and bounded direct builder are implemented and measured through one million
generated names. A private pinned-generation service lease and disposable
10,000-file sandbox integration are also measured; neither changes the live
manifest nor public query features. Judged filename relevance,
character-gram/deletion controls on the same split, real authorized length
histograms, atomic background publication, update/compaction write economy,
idle CPU trials, production parameter lifecycle, and Orchestrator
reconciliation remain promotion gates.

## M6 — rank fusion and result motion

- channel-preserving baseline fusion;
- reciprocal-rank/calibrated/PRV-derived comparisons;
- certainty transitions and stable result revision protocol;
- judged quality, ambiguity, explanation, and visible-churn measurements.

## M7 — multi-root/offline/federated readiness

- nested-root lifecycle and fan-out;
- explicit offline catalogues with unavailable results;
- machine-qualified shard endpoints and partial failure;
- removable local volumes offer on-volume full indexing or no persistent
  catalogue; network/other-machine sources use remote-full plus local-coarse
  catalogue semantics;
- no remote mutation in search protocol.

## M8 — standalone release candidate

- API v1 and independent client conformance;
- APFS/NTFS/ext4 oracle runs;
- reproducible packages, no network install, clean shutdown/update;
- performance and resource dossier;
- parent integration example outside this directory;
- Kolmogrov production-readiness disposition recorded: admitted with evidence,
  or explicitly blocked/rejected with the failing gate and architect review.

The worker should deliver vertical increments. Do not spend months optimizing a
page codec before M1 can answer exact sandbox queries correctly.
