# ORC-FE-001 ordinary text predicate reconciliation

Date: 2026-09-29. **GIVEN:** after the real indexed/live mismatch was measured,
the owner directed a bounded correction for practical ordinary filename search.
This is an explicit compatibility change in the development unified frontend
projection, not a change to frozen Engine exact filters or identity semantics.

New cursorless ordinary `text` means the current bounded Engine live predicate:
Unicode simple lowercase followed by literal substring matching against basename
OR slash-normalized root-relative path. This is not full case folding, NFC,
tokenization, globbing, content search or AI interpretation. Query text itself is
not slash-normalized or trimmed; whitespace-only text is rejected by Engine.
Scope, containment, work/result limits, source and partial state remain enforced.

The current catalogue adapter cannot implement that predicate. It returns
`unsupported` with `ENGINE_CATALOGUE_TEXT_UNSUPPORTED` during planning, before
issuing `engine.query`. The existing ADR-008 allowed fallback routes text-only
requests to bounded live search. This remains true when a persistent catalogue
exists: ordinary text currently receives no catalogue acceleration. No executed
catalogue no-match is reinterpreted. Denied/invalid/budget/timeout/cancelled results
still never cause fallback.

Exact callers use `filters.name` with empty ordinary text. Exact name/path and
metadata meanings, catalogue evidence and authoritative no-match remain frozen.
New text plus metadata filters returns unsupported with no Engine call, because
live cannot enforce those combined predicates. It never silently drops filters.
Catalogue-only policy returns unsupported for ordinary text. Existing catalogue
cursors retain their original exact-name predicate and source, including old
text-as-name continuation; live cursors remain live. A continuation failure does
not restart on another source. Cursor fingerprint/generation validation remains
provider-owned.

**Compatibility impact:** pre-correction cursorless `text` callers expecting
whole-name exact matching must move to explicit `filters.name`. There is no wire
field change, new Engine method, changed exact filter, or provider capability
claim. A catalogue substring projection remains a separately negotiated and
measured additive slice; it must not be inferred from `engine.exact.catalogue`.

Engine owner reviewed invalid-request precedence, exact filter preservation,
combined predicates, catalogue-only policy and legacy source-bound cursors.
Focused adapter/broker tests and an actual frontend query against an indexed
Windows fixture are required for this correction's evidence.
