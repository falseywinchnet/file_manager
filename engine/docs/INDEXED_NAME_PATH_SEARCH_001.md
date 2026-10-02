# Next search slice: catalogue-backed filename and relative-path substrings

**GIVEN:** owner direction on 2026-10-01 prioritizes search speed and completeness
for real-world File Manager use. **CANDIDATE:** the following implementation
sequence; no new API or storage format is accepted by this note.

## Existing boundary

**OBSERVED:** `exact.QueryIndex` rejects residual text. Orchestrator's
`EngineJsonlSearchAdapter::query_catalogue` reports unsupported before dispatch
for a new ordinary-text query, allowing its existing live fallback policy.
Exact metadata/no-match queries remain authoritative catalogue queries.

The first index must preserve today's ordinary case-lowered substring predicate
over names and root-relative slash-separated paths. Filename-only postings
cannot silently remove matches through ancestor directory names. Go lowercase
is not Unicode full case folding or canonical normalization. Any later change
to those meanings requires separate fixtures and versioning.

## Concrete sequence and rejection gates

1. Reconcile a separately advertised catalogue substring capability through
   Orchestrator and the Engine negotiation record. Specify predicate version,
   scope/descendants, generation-bound pagination, exact evidence, staleness,
   cancellation, response/candidate budgets, and unavailable-index behavior.
   Do not reinterpret existing exact `filters.name` or a successful no-match.
2. Build a cancellable exhaustive catalogue control with exact differential
   fixtures: ASCII/Unicode, repeated names, hard links, ancestor-only and
   separator-spanning matches, nested roots, rename/delete/replacement, short
   queries, no-match, and continuation across generation publication.
3. Compare a disposable generation-bound character-gram candidate index with
   that control on identical generated 10k/100k/1m workloads. Exact stored paths
   verify every candidate. Compare full-path postings with component postings
   plus ancestor resolution; common ancestors and one/two-character queries
   must not create unbounded posting materialization or missed matches.
4. Keep durable records off the Go heap. Measure build/read scratch, retained
   memory, bytes per binding, update amplification, warm/cold p50/p95/p99/max,
   cancellation and corruption recovery. A component benchmark does not close
   service readiness or GUI-to-first-result targets.
5. Connect only a checked matching generation through the negotiated capability;
   preserve source-bound cursors and the existing fallback allowlist. Repeat the
   real frontend search probe and publish coherent native dogfood archives.

An index is not current merely because it exists. Preserve stale/coverage-gap
and unavailable states, and keep filesystem observations authoritative for file
operations. No foreground query builds or compacts a persistent index.

Read `../results/LIVE_PATH_MATCHING_2026-10-01.md` for the initial live-query
optimization and retained failed batching experiment. Neither selects the
catalogue candidate structure nor establishes indexed substring availability.
