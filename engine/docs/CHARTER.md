# Purpose-built engine charter

Status: **GIVEN mission; architecture spine accepted in
`ARCHITECT_HANDOFF_001.md`; low-level mechanisms remain measured**.

## Mission

Build a standalone Go engine that turns approved filesystem roots into an exact,
economical, inspectable search projection and returns evidence-bearing results
on the order of milliseconds. It must be purpose-driven enough to outperform
general stores on File Manager workloads and valid enough that crash recovery,
integrity, and migration can be compared seriously with SQLite.

The engine is not a database product, shell, GUI, semantic assistant, cloud
service, or file-operation authority. It is a catalogue and retrieval machine.

## Product laws

1. Filesystem objects and bytes remain authoritative.
2. Exact identity precedes approximate retrieval.
3. Each object belongs to its most-specific approved root.
4. Indexed parent roots prune independently indexed child roots.
5. Exact, lexical, fuzzy, structural, provider, and federation evidence remain
   distinguishable through ranking and presentation.
6. Later evidence may reorder a result only when calibrated certainty improves;
   mere arrival time is insufficient.
7. Offline catalogues are explicit setup choices and expose absent records as
   unavailable.
8. Navigation and live folder enumeration work without the service.
9. No public engine call mutates source files.
10. Every durable projection is integrity-checkable, quarantinable, and
    rebuildable from its declared authority.

## Quality bar

The project aims beyond “good enough embedded search.” It should pursue:

- exact platform identity under rename, move, hard-link, clone, replacement,
  event loss, and scan reconciliation;
- deterministic millisecond-scale retrieval without opaque adaptive behavior;
- bounded allocations, reads, writes, wakeups, and background pressure;
- stable reader generations while ingestion and compaction proceed;
- a query planner that pushes scope and predicates before expensive similarity;
- explanations precise enough for an AI client to audit every rank;
- file-format ownership disciplined enough to survive corruption and upgrades;
- a clean library/service boundary usable by GUI, CLI, tests, and future local
  providers without rebuilding the engine in each client.

## Non-goals for the first standalone release

- semantic interpretation or personal-assistant behavior;
- content extraction inside the core service;
- web or remote discovery;
- plugin execution inside the engine process;
- distributed consensus or transparent cross-machine mutation;
- arbitrary SQL, general transactions, or a general-purpose KV API;
- production admission of ConeDAG or PRV without their own transfer gates.

## Completion evidence

“Fast,” “valid,” and “economical” are not accepted adjectives. Completion needs:

- exact oracle transcripts;
- store/control comparison manifests;
- query judgments and ranking ablations;
- crash/corruption campaigns;
- race and fuzz results;
- platform fixture results;
- resource and latency distributions;
- a documented rebuild/migration path;
- an independently exercised API conformance client.
