# Standalone engine delivery plan

Status: **implementation sequence; each milestone has a rejection gate**.

## M0 — reproducible shell

- Go module builds without network access after dependencies are vendored or
  eliminated.
- sandbox-required service, protocol smoke test, CI commands, fixture generator,
  result-manifest schema.
- API v0 golden messages.

## M1 — exact reference catalogue

- canonical record and platform identity adapters;
- approved-root manifest and longest-root ownership;
- exhaustive in-memory/sorted reference store;
- live scan into a disposable sandbox;
- exact query, metadata ordering, inspect, and status;
- identity/mutation oracle on macOS first.

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
are observed. Quarantine,
disk-full and physical power-loss campaigns, deltas/compaction, migration, and
the copy-on-write-tree control remain open promotion gates.

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

## M5 — fuzzy and structural candidates

- character-gram and deletion-dictionary controls;
- bounded Damerau-Levenshtein verification;
- exact hard-negative protection;
- versioned core similarity-channel interface;
- Kolmogrov reference-vector and conformance adapter when its transfer bundle
  passes the gates in `KOLMOGROV_CHANNEL_SEAM.md`;
- controls remain runnable when that adapter is absent or rejected.

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
