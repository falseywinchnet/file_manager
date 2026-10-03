# ORC-FE-001 retained search match and request context

2026-10-03. Development repair of fields already emitted through ORC-ENG-001/004;
no new wire operation, search capability or relevance model is introduced.
This extends FRONTEND_SEARCH_SOURCE_RECORDS.md before substring activation.

**OBSERVED:** Engine api.Result includes rank, certainty and ordered Evidence.
The C++ source client currently discards them. Frontend criteria completion also
discards its filters, and correspondence rebuilding describes every retained row
using the newest page's provenance. These lose the context of earlier matches.

## Source-client projection

Append optional unsigned 64-bit rank, optional finite double certainty and an
optional ordered vector of SearchEvidenceInfo to SearchResultInfo. Missing/null
values are unreported; explicit zero and an empty vector remain reported.
Do not synthesize rank from row position, clamp certainty, or interpret certainty
as current-file verification. Rank refuses negative, fractional, exponential or
overflowing integer values. Finite scores use locale-independent binary64 parsing;
invalid/nonfinite/overflowing values fail the complete page.

Each reported evidence record requires string kind/channel, finite double score,
Boolean exact/inferred. Calibration, anchor and observed_at are optional owned
strings. Preserve unknown kind/channel strings in order; do not promote them to
known semantics or compare scores from unrelated calibrations. Missing/null
optional strings remain unreported; reported empty strings remain empty.
The compact semantic fixtures omit score and other required local-wire record
fields, including metadata. They remain illustrative contract examples, not
complete source-client response fixtures. Current api.Evidence always emits
score/exact/inferred; no score is invented to make an incomplete example parse.

Optional details is an object retained as owned inert JSON text, exposed as
details_json. Preserve all nested JSON values and number spellings without
floating-point conversion. Object-key ordering and insignificant whitespace may
change; string values, array order and numbers may not. Null/missing is unreported,
an empty object is reported. Existing JSON depth/value/frame limits apply. No
dynamic public value system, executable metadata or provider-controlled UI is
introduced. Malformed evidence anywhere fails the whole response.

These fields change source-client layout only; rebuild consumers together. No
stable ABI or wire-version change is claimed. Tests must cover response retirement,
ordered/unknown evidence, optional/empty values, numeric boundaries, nested inert
details and malformed later entries without partial publication.

## Frontend request and page ownership

SearchRequestContext owns root ID, optional root-relative scope, text, exact
filters, descendant policy, result limit and incoming continuation. Construct it
from the actual submitted arguments, never from controls read after completion.
One immutable request owner is shared by the page's retained rows; no per-row copy
of query/filter/cursor strings. An absent owner means context was not supplied
(for older fixtures), not an empty query. The page owner also preserves its own
lane, scan ID and generation. Duplicate rows retain their original record/page/
request pair; replacement, cancellation and navigation retain existing retirement
rules. An empty returned page need not retain a request once the job ends.

Correspondence provenance comes from each retained row's page and record. Do not
relabel earlier rows with the newest generation/lane. Show unknown or unreported
match evidence honestly. Current native observations remain separate; no provider
rank/score is used to reorder rows and no cached predicate is freshly validated.
The summary describes the first evidence entry in provider order. Only recognized
exact-channel kinds and the existing live name/path claim receive specific text;
unknown kind/channel combinations remain generic, including an inference marker
when reported. Unknown nonempty source lanes are distinguished from absent lanes.

## Acceptance boundary

Owned projection and truthful display are required before indexed-search
acceptance; they do not themselves activate the capability, prove search speed,
grant file access or implement thumbnails. Local tests and native Windows/macOS/
Linux checks remain required. Full programming-house-style source review includes
all authored implementation, tests and tooling; spelling checks alone do not pass.
