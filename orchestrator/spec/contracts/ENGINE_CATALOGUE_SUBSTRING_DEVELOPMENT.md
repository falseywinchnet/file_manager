# ORC-ENG-001 catalogue substring development profile

2026-10-03 UTC. **Development reconciliation for implementation and conformance;
not a frozen release contract or an available capability.** Owner direction
requires practical indexed search preserving ordinary filename/path semantics.
This resolves the six choices in `../../proposals/ENGINE_CATALOGUE_SUBSTRING_001.md`
without selecting a new persistent index, codec, relevance model or store.
The existing checked generation remains the stored-record authority; filesystem
identity and bytes remain authoritative outside the catalogue.

## Predicate and compatibility

Use existing `engine.query`, gated by capability
`engine.catalogue.substring.v1` in state `available`. It may be advertised only
after this entire development profile passes conformance, including the bounded
reader and exact-lookup regression gates. A checked supported durable root is
also required; absence is not a successful empty catalogue. No negotiation by
sending unknown fields to an old provider. Existing ORC-ENG-001 semantic 0.1
and exact requests remain intact; the independently named v1 capability is the
additional selector. A changed predicate requires another capability revision.

`text_predicate:"root_relative_lower_substring_v1"` selects literal substring
of Go `strings.ToLower` on the root-relative slash-separated stored path. Lower
the query too; do not trim it, change its separators, normalize Unicode, tokenize,
or reinterpret it as a whole name. Scope filters eligibility without removing
the ancestor portion from the string being matched. Preserve the live path
predicate's invalid-UTF-8 lowering behavior in differential fixtures. Empty or
whitespace-only text, over 4096 UTF-8 bytes, malformed scope and invalid limits
are Invalid. Unknown nonempty predicate is Unsupported. Absence retains the
legacy exact path, including its refusal of residual text.

The first profile accepts text alone, default path order, and the exact channel.
Text with metadata filters, exact literals, alternate sorting or other channels
is Unsupported; no constraint is discarded. This is exact predicate evaluation
over a cached generation, not current-file verification or ranked relevance.
Return an additive evidence kind `exact_substring`, channel `exact`, calibration
`root_relative_lower_substring_v1`, root-relative path anchor, `exact:true` and
`inferred:false`. Do not use `exact_path` to imply equality. Stable global ranks
follow the checked path-order stream; certainty 1 refers only to the stored
predicate, not freshness. Preserve existing stale/unavailable-root information.

## Bounded pages

These are development ceilings to measure, not measured performance claims.
The Engine clamps positive caller requests, uses defaults for absent/zero
fields, and rejects negative/overflow/noninteger values before execution.

| Field | Default | Hard ceiling |
|---|---:|---:|
| Existing result `limit` | 128 | 1000 |
| `scan_budget.max_rows` | 4096 | 16384 |
| `scan_budget.max_read_bytes` | 4 MiB | 16 MiB |
| `scan_budget.max_wall_time_ms` | 25 ms | 250 ms |
| `scan_budget.max_response_bytes` | 128 KiB | 512 KiB |
| Encoded outer continuation value | — | 4096 bytes |
| Incremental query-owned scratch/retained page storage | — | 8 MiB |

Logical reads count admitted requested bytes, including cache hits and reads
that subsequently fail; refused reservations do not consume bytes. Validate
string/component dimensions before allocation. The existing reader cache is a
separate bounded owner, not hidden query scratch. Blocked native I/O and cache
lock acquisition are not interruptible merely because surrounding calls check
context; report their observed behavior and never promise a hard 25 ms deadline.

The response byte limit covers the complete encoded Engine response, including
envelope, request ID, cursor, warnings and evidence. **OBSERVED gap:** current
Service.Query cannot see the transport request ID. Implementation must reserve
actual encoded envelope space in the transport adapter and enforce the remaining
budget during page construction; a result-only JSON length is insufficient.
The service-library call must declare its result-only allowance explicitly.
Oversized single results produce BudgetExceeded, never omission or a repeating
zero-progress cursor. Include output accounting in scratch ownership review.

Scan sequentially without materializing all candidates or matches. A page may
contain zero hits and a continuation if it examined rows. Result/work/time/output
page stops after progress return the completed bounded page plus next position;
absence of continuation alone means exhausted scope. Cancellation, transport
deadline, integrity failure and a single row that cannot make progress produce
a terminal error with no usable page. A voluntary page timeslice is distinct
from an ended request deadline. Preserve whole-response failure semantics.

`plan.text_predicate` echoes the selector. `plan.scan_work` owns `rows_examined`,
`read_bytes` and `elapsed_micros`. These are catalogue work, not live stat calls.
Orchestrator maps existing max-results/visited/wall/response requests to their
respective catalogue fields, clamped to these ceilings. Read-byte policy is
server-owned. Live stat/open-directory limits retain their existing meaning.

## Continuation and routing

Engine cursors are opaque bounded authenticated tokens bound to provider instance,
generation, root/configuration identity, predicate revision, normalized query,
scope/descendants/order, next row and next rank. Exact token encoding is private;
full query strings need not be copied into tokens when a domain-separated digest
binds them. Pin the checked reader during a request only. No abandoned cursor
retains a reader, query object, timer or old generation. Generation/configuration
change or process restart is Stale. Wrong query or malformed token is Invalid.

Orchestrator's new catalogue cursor value is the literal prefix
`.fm-substring-v1.` followed by the opaque Engine token. The complete value must
fit 4096 UTF-8 bytes. Legacy Engine exact cursors are base64url and cannot start
with a dot. Reserve `.fm-substring-` for this versioned wrapper family: an unknown
version or empty token is Invalid and must never fall through to legacy dispatch.
Orchestrator does not decode or authenticate the Engine token. The Engine must
validate its complete query/predicate binding before using a continuation;
the wrapper alone is not authority. Bare legacy catalogue values retain the
old exact-name continuation meaning. Existing `{source,value}` wire shape stays.

New cursorless text may use the new capability under PreferCatalogue. Absent
capability is Unsupported and may take the existing allowed live fallback.
CatalogueOnly remains Unsupported without it. Continuations are always pinned
to their original lane/predicate: lost capability is a same-lane failure, never
a fresh query. Successful exhausted no-match, Partial/stale results, Invalid,
Denied, BudgetExceeded, Timeout and Cancelled never fall back. Exact filters
and live cursor dispatch remain unchanged.

For this profile, add Engine errors `UNSUPPORTED_PREDICATE`, `CANCELLED` and
`DEADLINE_EXCEEDED`, mapped to Orchestrator Unsupported, Cancelled and Timeout.
Wrap ended contexts on the new path so the legacy transport's resource-budget
mapping cannot erase those distinctions. This does not invent a cancellation
RPC or silently change old exact-query error behavior. Cross-process disconnect/
deadline tests and frontend supersession tests remain required; a private Go
context test alone is insufficient.

## Consumer policy and admission evidence

Unverified cached matches remain visible with cached-generation/stale disclosure.
They must not be relabelled current when the frontend observes the path again.
Stored identity/revision and current facts stay separate. Operations retain
their own current identity validation. The source-record repair preserves the
initial identity/metadata subset; full rank/evidence projection and retained
request context are still required before frontend indexed-search acceptance.
No Engine 64-bit Windows ID is equated to frontend FILE_ID_128 by truncation.
Current-verified criteria would additionally need identity compatibility and
fresh predicate evaluation; this profile makes no such claim.

Before route activation require: complete independent substring oracle on
ASCII/Unicode/invalid-UTF-8/ancestor/separator/short/no-match fixtures; scopes and
nested-root exclusion; hard links; bounded progressing empty pages; exact
read/output accounting; cancelled/deadline/oversized/failure cleanup; stale and
restart continuations; wrapper compatibility and capability absence; unchanged
exact controls; Go/Rust/C++ native integration; and truthful frontend provenance.
Measure 10k/100k/1m first-result/page distributions, allocations/read bytes and
retained memory against the live path and exhaustive reference. No speed or
million-row claim is established by this document.

The bounded-reader candidate remains under evaluation. Code existence or an
available private experiment cannot promote this capability. Reversal keeps it
unavailable and preserves live fallback and all rejected experiment evidence;
no catalogue migration is involved. Any later accelerator must match this
exhaustive predicate and complete evidence stream before selection.
