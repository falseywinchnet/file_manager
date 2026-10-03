# Next search/index responsiveness stage audit

Date: 2026-10-02. Status: **CANDIDATE stage proposal for parent review**.
Read-only investigation of the shared checkout following the reported Details
integration at `17a748f`. This audit changed only this record; it ran no builds,
benchmarks, service operations or Git commands. Measurements below are attributed
to retained receipts, not reproduced here. No candidate structure, new public
capability or currentness claim is accepted by this document.

## Subsequent implementation evidence

The investigation below retains its original source observations. Private
checked-generation substring-reference correctness/paired evidence is now
recorded in `../../engine/results/SUBSTRING_REFERENCE_EXPERIMENT_2026-10-02.md`;
it enables no production search route. The next bounded-reader candidate is
retained as a rejected experiment in
`../../engine/results/BOUNDED_SCAN_READER_2026-10-03.md`: paired 10k controls
did not establish the required legacy exact-query non-regression. Active Engine
source is unchanged. Larger-corpus/native throughput acceptance remains open.

The source-client match/request repair is specified in
`../../orchestrator/spec/FRONTEND_SEARCH_MATCH_CONTEXT.md`. It preserves rank,
certainty and ordered evidence, shares submitted arguments once per returned
page, and prevents appended pages from relabeling earlier correspondence rows.
Validation is recorded separately in
`../results/2026-10-03-search-match-context/README.md`. It enables no indexed
substring route and supplies no latency improvement claim.

The frontend follow-up adds generation/shutdown checks to SearchWork and
CriteriaWork before connection and at synchronous service-call boundaries.
Already obsolete queued work retires before connection; obsolete replies retire
before UI enqueue. The existing UI generation guard remains. This corrects
the missing admission checks observed below but supplies no in-flight transport
cancellation. Local build/12-suite evidence, exact source-review scope and
remaining native verification are recorded in
`DOGFOOD_ACCEPTANCE_AUDIT_2026-10-02.md`. The source audit and public-contract
choices below otherwise remain open.

## Product boundary and next outcome

**GIVEN:** ordinary search starts below the navigated folder; indexing is opt-in
and begins with names/metadata; navigation works without an index. See
`planning/search/SEARCH_ARCHITECT_INTERVIEW.md`, initial GIVEN list, and
`planning/search/SEARCH_RECONCILIATION_001.md`, identity/root ownership and
freshness sections. ADR-001 accepts the immutable-generation/off-heap spine;
ADR-008 requires bounded catalogue-independent Engine search. Orchestrator owns
the public semantics and route. These accepted directions supersede older
interview alternatives. Frontend `planning/DOGFOOD_SEQUENCE.md`, F3, requires
current-subtree search, deterministic updates, live fallback and unavailable
states; fuzzy/semantic research is not a prerequisite for this work.

**CANDIDATE:** next implement a bounded, cancellable exhaustive substring
reference over one checked catalogue generation, with a negotiated predicate
and differential fixtures. Use it to evaluate a disposable substring candidate
index before changing the ordinary frontend route. In the same acceptance
workload, attribute provider time separately from UI result-application time.
Do not start by enabling every root, implementing continuous currentness, or
promoting the existing exact-name index as substring search.

This narrows the already recorded sequence in
`engine/docs/INDEXED_NAME_PATH_SEARCH_001.md`; it does not replace that proposal
or reopen the accepted storage spine.

## Actual path and existing interfaces

**OBSERVED:** source locators below name exact symbols so the audit remains
readable after nearby line changes.

| Layer | Existing path and significance |
|---|---|
| Frontend request | `frontend/src/application.cpp`, `apply_filter`, `engine_search_available`, `request_engine_search`: nonempty text requests current-subtree search only when an Engine root ID is configured and the location lies under `protected_root_`. Default page limit is 100, settings clamp it to 25–500. This local scope check does not itself prove a live provider is available. |
| Worker/client | `frontend/src/application_jobs.hpp`, `SearchWork`: connects a typed client and synchronously calls `Client::search_subtree` off the UI thread, then posts `SearchReady`. `orchestrator/conformance/clients/cpp/include/fileman_orchestrator/client.hpp` declares root ID, optional relative path, text, limit, optional source cursor and exact filters. This API has no caller cancellation argument. |
| Orchestrator route | `orchestrator/src/engine_port.rs`, `EngineSearchBroker::search`: exact-filter requests stay catalogue-only; continuations stay on their original source. New unfiltered requests prefer catalogue, with only Unsupported/Unavailable/Stale/Quarantined eligible for live fallback. Successful zero results, denial, invalid requests, budget/time failures and cancellation do not trigger fallback. |
| Ordinary-text planning | `orchestrator/src/engine_jsonl.rs`, `EngineJsonlSearchAdapter::query_catalogue`: new nonempty text returns `ENGINE_CATALOGUE_TEXT_UNSUPPORTED` before dispatch. It does not substitute `filters.name`. The contained adapter delegates this semantic projection. Existing catalogue continuations retain their predicate. |
| Catalogue | `engine/internal/exact/query.go`, `QueryIndex`: rejects residual text/exact literals, supports exact/metadata filters, bounded candidates and generation-bound cursors. `exact.Index` exposes `CandidateAll`, `CandidateName`, `CandidateNamePage`, `CandidateID`, `PathIndex`, `Row`, `Record`; none promises substring retrieval. `engine/internal/service/query.go`, `Service.Query`, uses either the reference snapshot or checked durable reader. |
| Live | `Service.QueryLive` and `engine/internal/live/manager.go`: progressive discovery-order traversal, work budgets, root ownership pruning and exact observations. `path_matcher.go` matches lowercased root-relative slash-separated paths; the filename is contained in that path. Non-ASCII behavior remains Go `strings.ToLower`, not full Unicode case folding or canonical normalization. Ancestor-only and separator-spanning matches are therefore required equivalence cases. |
| Publication | `Application::apply_engine_search` rejects obsolete generation/text, validates routes, observes native identity/metadata for each returned row, then rebuilds ObjectView and Correspondence collections from all accumulated search IDs. Native checks and accumulated-row construction occur in the UI callback. Search normally displays Correspondence; the recent Details optimization alone does not establish search-view latency. |

**OBSERVED:** `SearchWork` has no supersession check before connection/query.
Generation checks discard stale output after work finishes; that is not proof
that obsolete provider work is cancelled. The typed API limitation above means
an end-to-end cancellation change must be reconciled with Orchestrator. Do not
claim a transport has no deadline merely because this caller has no token.

## Coverage and currentness are distinct blockers

**OBSERVED:** `engine/internal/service/roots.go`, `planRoots`, rejects multiple
roots for the durable service. `engine/internal/deployment/manifest_windows.go`,
`WindowsManifest.Validate`, likewise requires exactly one persistent approved
root. The most-specific-root rule exists, but a multi-root persistent product
route is not supplied by that rule alone. The first experiment can use one
explicit generated root; it must not claim whole-machine coverage.

**OBSERVED:** the current Windows profile exists, with host/SID/root-bound
manifest and authenticated named pipes. `engine/docs/WINDOWS_LOCAL_DEPLOYMENT.md`
supersedes the earlier September 29 toolchain receipt's statement that Windows
installed IPC is absent. It remains a user-process profile, not SCM promotion.
Indexing requires explicit consent and a separate store; indexed startup
reconciles before discovery, and the profile does not automatically start
watchers. Root changes require a new manifest/restart. This is manual-reconcile
currentness, not continuous filesystem truth.

**OBSERVED:** `engine/internal/service/background.go`, `backgroundWorkStatus`,
and `status.go`, `decorateStatus`, preserve incomplete coverage/staleness.
FSEvents final-hard-link removal and Windows journal/root-replacement coverage
remain limitations; the live manifest does not commit the observation watermark.
`capabilities.go` advertises lexical indexing unavailable and background
observation experimental. A new candidate index cannot cure these limits.

**OBSERVED:** `orchestrator/spec/CONTRACT_REGISTRY.md` registers ORC-ENG-001,
ORC-ENG-003, ORC-ENG-004 and ORC-FE-001 separately.
`spec/contracts/ENGINE_AND_KOLMOGROV.md`, Currentness and staleness, permits
cached checked results with provenance when freshness policy allows, and
requires strict-current failure to remain explicit. The current adapter maps
a successful response with stale roots to Partial; Partial is not a fallback
trigger. Therefore the next substring negotiation must specify acceptable
freshness and unavailable-index behavior explicitly, rather than assuming
every stale warning automatically sends the request live.

## What the existing measurements establish

**MEASURED, retained receipts only:**

| Evidence/workload | Result and limit |
|---|---|
| `frontend/results/2026-09-29-shadow-windows/SEARCH_INTEGRATION.md` | Actual Application → typed C++ → Rust → Go small generated fixture passed ordinary `needle` after routing correction: 30.5365 ms. Independent source checks distinguished live `needle`, exact catalogue full-name hit, and exact catalogue `needle` zero-match. A prior indexed partial-name failure is retained. Single timings are not distributions or indexed substring evidence. |
| `engine/results/LIVE_PATH_MATCHING_2026-10-01.md` | Warm string-only component measurements reduced uppercase no-match from 270.5–282.9 to 59.28–61.44 ns/op and 176 B/4 allocations to zero. The final generated NTFS 10k all-match traversal took 3.1508658 s versus an unpaired 2.1226435 s baseline; first result was 4.9959 ms versus 9.0001 ms. The receipt explicitly refuses a whole-search speedup claim. |
| Same receipt, rejected directory batching | A 32-entry ReadDir candidate regressed its 1000-file control and was removed. Fewer allocations did not establish lower elapsed time. Retain this negative result. |
| `engine/results/M1L_LIVE_QUERY_001.md` | One warm APFS/M3/Go 1.26.5 10k flat-file run: first result 82.875 µs, full traversal 40.802 ms/80 pages. Different host/filesystem/toolchain/cache conditions preclude comparison with the Windows numbers. |

**OBSERVED:** `engine/benchmarks/live_query_test.go`,
`TestLiveQueryDistribution`, currently creates a fixed 10,000-file all-match
fixture and records one traversal. Its name does not imply p95/p99 measurement;
its `/dev/fd` helper is not Windows handle telemetry.
`frontend/tests/application_latency_benchmark.cpp`, `run_live_search`, verifies
an applied identity-checked filename and layout within 30 seconds; it does not
measure native input-to-present, provider source itself, or a latency
distribution. Exact-generation and SQLite controls do not establish a substring
win unless run with the same predicate and corpus.

## Smallest candidate implementation and acceptance sequence

1. **CANDIDATE — semantic/fixture checkpoint.** Orchestrator opens the Engine
   negotiation for a separately advertised catalogue substring predicate.
   Preserve root-relative lowercased slash-path semantics, exact record
   verification, scope/descendants, generation-bound/source-bound continuations,
   bounded candidate/result/output work, cancellation, freshness and failure
   behavior. Keep `filters.name` exact. Unknown capability leaves the current
   route unchanged. No new capability ID is assigned by this audit.
2. **CANDIDATE — reference checkpoint.** Engine implements a private cancellable
   bounded scan over one pinned checked generation, with resumable position or
   explicit budget failure as the negotiated contract specifies. Reuse exact
   records/ownership rules; do not use `CandidateAll` to materialize an entire
   million-row candidate set. Establish result/evidence equivalence before
   evaluating full-path character grams versus component postings plus ancestor
   resolution. Candidate projection is disposable and generation-bound;
   catalogue storage remains authoritative. No foreground index build/compaction.
3. **CANDIDATE — acceleration checkpoint.** Compare those structures against the
   exhaustive reference and a mature control with equivalent substring semantics.
   Keep short/common-query work bounded without false negatives. Measure build,
   retained memory, bytes/binding, query tails, update amplification and corruption
   recovery. Only a passing candidate proceeds to checked publication and the
   negotiated provider/independent-consumer route. This checkpoint chooses an
   implementation only after evidence; it does not admit a new storage spine.
4. **CANDIDATE — consumer acceptance.** Extend the generated search probe to
   record request/first-page/UI-apply/layout/present boundaries, source and
   completeness, and maximum UI drain time. Measure 25/100/500-result pages and
   repeated continuation. If native observation or accumulated-row rebuilding
   dominates, the narrow frontend follow-up is owned worker preparation with
   generation/supersession validation and bounded UI publication. Preserve
   identity revalidation and selection semantics. Do not remove checks to meet
   a timing target. Cancellation transport work remains a separate negotiated
   edge if the workload exposes obsolete-query contention.

**CANDIDATE acceptance workload:** generated 10k, 100k and 1m bindings, using a
named seed and digest, one approved root, engine state outside source. Include
flat-wide and deep trees; selective filename, ancestor-only, slash-spanning,
one/two-character, common, no-match, mixed case, Unicode/combining marks,
duplicate names/hard links and long paths. Exhaust every page and compare exact
eligible bindings/evidence with the reference. Exercise rename/delete/replacement,
new generation during continuation, missing/corrupt candidate projection,
unavailable/stale catalogue, denied scope, cancellation and abandoned cursor.
Nested-root ownership belongs in correctness fixtures even though persistent
multi-root integration is excluded from this first slice. A generation change
must follow the accepted stale/restart rule, never silently mix pages.

**GIVEN acceptance references:** ADR-001 and
`engine/docs/TEST_AND_BENCHMARK_PROTOCOL.md` require revision/environment/corpus,
cold/warm distributions, p50/p95/p99/max, CPU, RSS/private memory, I/O, retained
storage and negative results. Preserve warm GUI-to-first-correct-result <50 ms,
one-million idle private memory ≤48 MiB, no full heap record mirror, storage
<384 bytes/object and mixed-update amplification <3× as named existing targets.
The exact/prefix 8/20 ms p95/p99 target at one million is not automatically an
accepted substring target; negotiate the substring gate and report exact-query
regressions separately. Repeat service/consumer evidence on native NTFS/APFS/ext4
before cross-platform promotion. Start with paired repeated 10k controls, then
scale; a fixture test pass or component microbenchmark does not close these gates.

## Review disposition

**CANDIDATE parent decision requested:** authorize only the semantic/fixture
and exhaustive-reference checkpoint first, with a paired workload/profiling
receipt. That is a concrete prerequisite to accelerating ordinary text through
the catalogue. Multi-root durable routing, continuous currentness, fuzzy/content
search, semantic providers and installer work retain their own stages.

**OBSERVED review scope:** source inspection named above plus root/component
instructions, accepted search handoffs, contract registry/currentness/routing
records, frontend interview/dogfood direction and retained measurements.
The complete `planning/PROGRAMMING_HOUSE_STYLE.md` governs any later authored
implementation/tests/tooling; this audit authored none and certifies no legacy
source as compliant. A later assignment must explicitly review owned callback
state, cancellation/borrow lifetimes, precommit preparation, conversions and
repeated-loop storage in its exact changed scope.
