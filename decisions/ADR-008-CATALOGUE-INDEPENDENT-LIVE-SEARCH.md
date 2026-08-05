# ADR-008: Catalogue-independent live search is a required engine provision

Status: **accepted**.

Date: 2026-08-05.

Owner approval: the grand architect clarified that search must remain usable
without a catalogue and directed Orchestrator to require that capability from
the Engine while continuing Core work.

## Question

Is a zero-catalogue filesystem search lane optional, or must the Engine provide
it as a bounded fallback when a catalogue is absent, disabled, stale beyond
policy, rebuilding, quarantined, or otherwise unavailable?

## GIVEN constraints

- Search must work without a catalogue.
- The filesystem remains authoritative; a live traversal result is an observed
  filesystem fact, not an inferred catalogue generation.
- The lane belongs to the Go Engine provision. Orchestrator owns its contract,
  routing policy, availability truth, and cross-project conformance.
- The fallback may be reduced relative to indexed retrieval, but it may not be
  represented as an empty indexed result or require building a catalogue first.
- The implementation must be cancellable and bounded in memory, CPU, open file
  descriptors, visited entries, stat calls, wall time, result count, and output
  bytes.
- It must not persist an implicit catalogue, perform periodic whole-tree scans,
  follow links outside the authorized scope, open file contents by default, or
  write inside the searched tree.

## Workloads and failure modes

The required lane covers an approved current directory or descendant scope when
the persistent catalogue is disabled or unusable. It must produce early pages,
survive permission failures and concurrent tree mutation with explicit partial
state, stop promptly on cancellation, and remain bounded on deep trees and
directories with millions of entries.

Failure modes include treating manual reconcile as live search, sorting or
materializing the complete tree before returning, bypassing authorization when
falling back, silently following symlink/junction/reparse escapes, returning an
empty success for an unreadable root, or claiming snapshot completeness while
the filesystem changes during traversal.

## Candidates

### A. Require manual reconcile before every fallback query

This reuses the catalogue path but does not provide search without a catalogue.
It also delays the first result until a complete scan and durable publication.

### B. Let each frontend implement platform-specific live traversal

This preserves fallback but duplicates filesystem identity, containment,
normalization, cancellation, and error semantics outside the Engine.

### C. Add a bounded Engine live-query contract and route it independently

Keep catalogue queries under `ORC-ENG-001`. Add `ORC-ENG-004` for non-persistent
filesystem traversal, with explicit source semantics and a progressive bounded
cursor. Orchestrator prefers the catalogue when policy permits and falls back
to live traversal only for named catalogue-availability failures.

## Evidence and measurements

- **OBSERVED:** `orchestrator/planning/AUTHORITY_AND_PROCESS_MODEL.md` already
  requires live reduced search to survive Engine catalogue absence.
- **OBSERVED:** the current Engine implements manual reconcile followed by
  catalogue query, but no zero-catalogue query operation.
- **OBSERVED:** the Engine already owns capability-rooted metadata traversal,
  platform identity adapters, cancellation, containment, and bounded query
  primitives needed to implement this lane without duplicating them elsewhere.
- **UNMEASURED:** no implementation yet establishes first-result latency,
  traversal throughput, memory, CPU, descriptor, or cancellation bounds for
  live query. Those claims remain promotion gates.

## Decision

Choose C.

`ORC-ENG-004` is a required product/Engine contract family. Its first slice is
bounded live name/path search over an already-authorized scope. It does not
create or consult a required catalogue, persist scan results, or promise a
stable catalogue generation.

The provider returns progressive pages in traversal discovery order. A bounded,
expiring scan cursor may retain only the state necessary to resume traversal;
it is not a durable journal or hidden index. Complete global ranking is not
required before the first page. Every reply names the live source, scan
identity, completeness, visited-work counters, partial/unavailable subtrees,
and exact filesystem evidence available at observation time.

Orchestrator routing follows these rules:

1. A successful catalogue no-match remains authoritative for that requested
   catalogue policy and does not trigger live fallback.
2. `unsupported`, `unavailable`, `stale`, or `quarantined` catalogue outcomes
   may route to live search when the caller permits fallback.
3. `denied`, `invalid`, `budget_exceeded`, `timeout`, or `cancelled` never route
   around the failure into a more privileged or more expensive lane.
4. A live result never claims a catalogue generation or silently merges into an
   exact indexed result. The consumer sees its source and completeness.

ADR-007 remains accepted for catalogue query/status semantics and platform
gating. This record supersedes only its classification of zero-catalogue live
query as optional.

## Why the other candidates lost

- A does not meet the stated requirement and can turn a small query into a full
  tree scan plus publication.
- B creates three search implementations and fractures identity and security
  semantics across clients.
- An unbounded asynchronous walker is rejected because it can consume memory,
  descriptors, and CPU proportional to the entire tree while a client is slow.

## Consequences

- Orchestrator can implement and test routing before the Engine implementation
  arrives, using an independent fake provider.
- The Engine receives an explicit required capability rather than an inferred
  criticism of its catalogue work.
- Catalogue and background-indexing progress remain useful accelerators; they
  no longer determine whether basic search exists.
- Core 1.0 may continue with truthful provider availability under ADR-006, but
  File Manager search readiness cannot be declared while `ORC-ENG-004` is
  unavailable.

## Reversal and migration path

Traversal algorithms, platform enumeration APIs, cursor encoding, and direct
versus brokered hot-path routing may change behind `ORC-ENG-004`. If measurement
shows that a richer predicate cannot remain bounded, the provider may report it
unsupported while preserving the required basic name/path lane.

## Unresolved edges

- Exact first-slice filename normalization/case policy on each filesystem.
- Numeric default and hard budgets after APFS, NTFS, and ext4 measurement.
- Whether live content scanning is ever admitted; it is excluded from the first
  slice.
- Cursor recovery after process restart; the initial proposal permits expiry
  and restart from the beginning.
- Direct-engine fallback client conformance for `ORC-ENG-004`.
