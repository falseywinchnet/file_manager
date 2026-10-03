# ORC-FE-001 source object and revision preservation

2026-10-03 UTC. Development source-client repair of existing ORC-ENG-001/004
observations; no new wire operation, authority, decoder or thumbnail provider.

**OBSERVED:** Engine `api/types.go` emits object `root`, `id`, optional
`incarnation` and string-map `platform_key`; metadata has signed 64-bit `size`
and `modified_unix_nano`, unsigned 32-bit `mode`; each result has unsigned
64-bit `generation`. Orchestrator forwards result records. Semantic fixtures
spell the identity fields `root_id` and `file_object_id` and omit some runtime
metadata. They are semantic examples, not complete local-wire envelopes.

## Reconciled development projection

- The source client owns optional root ID, file-object ID, incarnation,
  platform-key map, record generation, stored mode and stored modification time.
  Missing/null optional fields remain unreported. Explicit empty strings/maps
  and generation zero remain reported values, without an authority claim.
- Runtime `root`/`id` and semantic `root_id`/`file_object_id` map to the same
  respective fields. If both spellings have non-null values, they must agree;
  malformed or conflicting values fail the whole page. No identity synthesis
  from paths, integer conversion, case folding or platform-key reinterpretation.
- Required `size` is signed 64-bit, matching the producer. Negative observations
  are preserved, never cast to a usable byte extent. Mode and timestamps must
  fit their exact integer domains; fractional/exponential numbers are refused.
  Record generation is not filled from page generation or a current observation.
- Existing parser depth/value and transport frame bounds apply. Typed values
  own their storage after decoding; failure publishes no partial page. This
  changes C++ source layout and signedness: rebuild all consumers together.
  It makes no stable binary ABI or wire-version change.
- Frontend worker preparation pairs each accepted current path observation with
  its separate owned provider record. Source lane, optional live scan ID and
  page generation have one immutable shared owner per page, avoiding repeated
  string copies per row; the last retained row releases that owner's storage.
  Rejected rows retain nothing. Cancellation discards the entire prepared page.
  UI publication retains source records only alongside displayed search rows;
  duplicate rows retain their original paired observation and source. Replacement
  or directory publication retires old records; obsolete replies publish nothing.

## Authority and limits

**GIVEN:** cached observations cannot overwrite filesystem authority. Provider
identity is not compared to frontend native identity in this repair. In
particular Windows Engine's 64-bit file index and frontend's 128-bit ID are not
interchangeable. No stored size/time is used for display facts or file I/O.
This does not revalidate cached predicates, grant content reads, prove thumbnail
eligibility, or accelerate ordinary text search. Rank/certainty/ordered evidence
and retained request context are extended by
[FRONTEND_SEARCH_MATCH_CONTEXT.md](FRONTEND_SEARCH_MATCH_CONTEXT.md).
Live work counters still need their own complete projection.

Conformance must cover ownership after response retirement, runtime and semantic
spellings, missing/null/empty fields, integer limits and invalid types; frontend
tests must separate deliberately different source/current facts and exercise
append, duplicate, replacement, cancellation and navigation retirement.
Source review uses `planning/PROGRAMMING_HOUSE_STYLE.md`; scanner success alone
is insufficient. Execution receipts are recorded separately from this contract.

Reversal removes these source fields and paired retention, reopening the known
loss of source evidence; provider/storage/transport behavior remains unchanged.
