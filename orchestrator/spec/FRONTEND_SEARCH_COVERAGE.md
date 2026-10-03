# ORC-FE-001 search coverage projection repair

2026-10-03 UTC. Scope: existing development C++ source client and File Manager
consumer. No wire revision, operation, route, capability or Engine meaning changes.

**GIVEN:** the owner directs practical search and truthful incomplete/stale
states. `contracts/FRONTEND_AND_GUI_FORMS.md` already requires their display;
`contracts/ENGINE_AND_KOLMOGROV.md` requires retained provenance and distinguishes
cached observations from current filesystem authority.

**OBSERVED:** `kernel.rs::search_response` forwards catalogue `stale_roots`,
`unavailable_roots`, `warnings`, generation and continuation, or live
`unavailable_paths`, `warnings`, `scan_id`, work and continuation. The C++ client
previously discarded the coverage lists and scan identity. This repair preserves
those existing fields as owned source-client records. It does not add them to
the wire or change provider behavior.

## Reconciled projection behavior

- `SearchPageInfo.coverage` owns optional string vectors for `stale_roots`,
  `unavailable_roots`, `unavailable_paths`, and `warnings`, plus optional string
  `scan_id`. Missing or null is unreported. An explicit empty array is a reported
  empty list. Order and string values are preserved. No synthetic freshness,
  incarnation, path identity or catalogue generation is inferred.
- Present non-null fields must have the declared type; every list member must
  be a string. A malformed field fails the entire projection with `ClientError`.
  A partially prepared page is never returned. Existing parser depth/value
  bounds and transport frame bounds still apply. Projection reserves list/row
  storage before its loops and retains no response-buffer or connection borrows.
- The C++ record addition requires rebuilding consumers of this source library.
  It is not a frozen binary ABI. Older peers omitting optional fields remain
  consumable with explicit unreported coverage.
- The frontend accumulates stale/unavailable/warning/unreported/omitted flags
  across pages of the displayed query. A replacement resets them. A later clean
  page cannot erase earlier gaps. Latest completeness/continuation governs the
  More results control; incomplete without a cursor never promises another page.
- Primary status says how many matches are **shown** and includes the most
  consequential coverage state. Secondary status retains the remaining states.
  Catalogue matches are disclosed as potentially changed. Preparing current
  path observations does not verify the original match's identity or predicates.
  No cached criteria result is described as guaranteed current.

The UI currently summarizes warnings rather than displaying their full provider
strings. The C++ page retains the strings, but the frontend's accumulated display
state retains only flags. A browsable per-query evidence view remains open.

## Scope still open

The subsequent [source-record repair](FRONTEND_SEARCH_SOURCE_RECORDS.md)
reconciles the identity spellings and signed metadata projection listed below.
The paragraph retains this earlier repair's scope; ranking/evidence, platform
identity comparison and criteria revalidation remain open.

Per-row identity, signed stored metadata, rank/certainty/evidence, live work
counters, platform identity comparison and criteria revalidation are not fixed
by this repair. Runtime `root`/`id` and fixture `root_id`/`file_object_id` spellings,
signed size compatibility and nested evidence-number preservation require their
own explicit reconciliation. `ENGINE_CATALOGUE_SUBSTRING_001` remains CANDIDATE;
this projection does not admit its predicate, budgets, evidence or cursor changes.

## Conformance and reversal

The private `search_projection.hpp` seam is shared by the real client and focused
projection checks. Cases cover owned values after response retirement,
catalogue/live shapes, absent/null/empty distinctions and malformed lists/scans.
Application interaction tests cover empty partial results, continuation,
append retention, replacement reset, criteria and omitted paths using actual
retained controls. These are deterministic fixtures, not provider performance
or native pixel evidence. Execution receipts live with the frontend audit.

Reversal removes the additive source projection and its consumer display;
provider behavior, storage and transport remain unchanged. Such a reversal
reopens the known lost-provenance defect and must not claim equivalent coverage.
