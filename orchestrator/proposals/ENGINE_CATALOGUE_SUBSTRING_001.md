# Catalogue substring predicate — negotiation proposal 001

2026-10-02. **CANDIDATE; semantic/fixture checkpoint only.** Parent review
accepted preparing this negotiation, not an index or source implementation.
Canonical registry, contract IDs, capability spelling and final version choice
remain Orchestrator/root-owned. This document is paired with Engine ledger
round 009. It supplies no runtime capability and changes no accepted exact
predicate. All new field/value spellings below are explicitly proposed.

## Operation and authority

**GIVEN:** practical ordinary text must preserve the predicate in
`../spec/FRONTEND_TEXT_PREDICATE.md`. Engine owns retrieval and exact catalogue
records; Orchestrator owns routing and meaning; File Manager and the independent
C++ client consume `orchestrator.search`. The first reference uses one approved
root and one checked catalogue generation. It performs no source-file reads or
writes, content extraction, root admission, automatic indexing or compaction.
It must remain optional: absent support leaves bounded live search available
under ADR-008's existing fallback allowlist. The filesystem remains authoritative.

**CANDIDATE outcome:** a cancellable exhaustive scan of existing generation
rows establishes a correctness/control baseline. A later candidate index must
produce the same eligible bindings and evidence. Nothing here selects grams,
postings, a durable format, or an update/currentness mechanism.

## Existing types versus required additions

**OBSERVED:** inspected `engine/api/types.go`, `api/errors.go`,
`internal/transport/jsonl.go`, `internal/exact/query.go`;
`orchestrator/src/engine_contract.rs`, `engine_jsonl.rs`, `engine_port.rs`,
`kernel.rs`; and `conformance/clients/cpp/{include/fileman_orchestrator/client.hpp,src/client.cpp}`.

| Existing surface | What it actually provides |
|---|---|
| Engine `api.Query` / `engine.query` | `text`, `scope:{root,path?,descendants}`, `limit`, opaque string `cursor?`, `exact_literals?`, `filters?`, `channels?`, `order?`. No required-generation, freshness, text-predicate or scan-budget field. `QueryIndex` rejects residual text. |
| Engine reply | `generation`, `results`, `next_cursor?`, `partial`, `warnings?`, `plan:{scopes,channels,unavailable_roots?,stale_roots?,elapsed_micros}`. `Result` already owns object/metadata/generation/rank/certainty/evidence. |
| Orchestrator request | `EngineSearchRequest`: `query_id`, `root_id`, `relative_path?`, `descendants`, `text`, exact `filters?`, `cursor?:{source,value}`, `budget`. The budget has result/visited-entry/stat/wall/open-directory/response-byte limits, with live meanings. |
| Catalogue projection | `catalogue_params` sends empty `text`, exact filters and `channels:["exact"]`; only old text-as-name continuations retain that mapping. New ordinary text is planned unsupported before dispatch. |
| Unified reply | `kernel.rs` derives catalogue `complete` from absence of `next_cursor`, and emits `{source:"catalogue",value:...}`. Catalogue wire plans currently deserialize only stale/unavailable roots. |

**CANDIDATE answer:** the current Engine request is insufficient. Add a
capability-gated predicate selector and catalogue scan budget to `api.Query`;
add optional work/predicate disclosure in the plan for verification. Reuse the
existing method and result records. No new operation or contract ID is proposed
for allocation here. A compatible additive semantic revision requires review
of unknown-field/unknown-value behavior and both peer versions; a field in a
Go struct alone is not negotiation. **OBSERVED:** current `decodeParams` uses
Go's JSON decoder without `DisallowUnknownFields`, so old providers can ignore
unknown additions. Capability/revision gating is mandatory; never probe support
by sending the new budget and assuming it was enforced. Old `QueryIndex` still
rejects residual text, but that rejection is not a substitute for negotiation.

Provisional spellings, not accepted API:

- `text_predicate:"root_relative_lower_substring_v1"`. Absence preserves exact
  query behavior, including rejection of residual text. Unknown selector returns
  unsupported, never an exact-name substitution. Advertise an independently
  named capability/revision to be assigned by root; do not overload
  `engine.exact.query` or `engine.lexical.index` to imply it.
- `scan_budget:{max_rows,max_read_bytes,max_wall_time_ms,max_response_bytes}`.
  `limit` remains the result-count limit; no duplicate `max_results` here.
  Omitted/zero budget fields select server defaults, positive values clamp to
  hard ceilings, malformed/negative/overflow values are invalid.
- `plan.text_predicate` echoes the selected revision;
  `plan.scan_work:{rows_examined,read_bytes,elapsed_micros}` supplies per-page
  work. These names are additions. No live `stat_calls` or catalogue `scan_id`
  is fabricated. `next_cursor` remains the completeness discriminator.

The current frontend request can remain unchanged. Orchestrator translates its
existing result/response/wall limits and clamps visited work to the new row
ceiling; the new read-byte ceiling is server policy. Catalogue does not consume
the live stat/open-directory fields. Document that mapping rather than changing
their live semantics. Work telemetry must be retained in the Engine/Rust
conformance path; adding it to the frontend public reply is not required for
this reference checkpoint.

## Predicate, scope, ordering and evidence

**CANDIDATE:** preserve exactly the reconciled live predicate: lower query text
with Go `strings.ToLower`; do not trim it or normalize its separators. Reject
empty/whitespace-only text and text over 4096 UTF-8 bytes. Match literal
substring against `strings.ToLower(filepath.ToSlash(rootRelativeBindingPath))`.
Basename OR path is equivalent because the full basename occurs in that path.
This is not NFC, full case folding, tokenization, prefix-only search or fuzzy
matching. Stored invalid-UTF-8 path behavior needs an explicit differential
fixture for Go's existing lowering/replacement behavior; do not silently change
it. An ASCII backslash in query text is not converted to slash on Windows.

Scope selects eligible rows; it does not change the matched root-relative
string. A query scoped at `work/2026` can match the `work` ancestor. Apply
existing exact scope/descendant and most-specific-root eligibility semantics;
never include a child shard as though the parent owned it. First slice supports
text alone: combined text/metadata filters, exact literals, requested fuzzy or
lexical channels, and alternate sorting remain unsupported rather than dropping
constraints. Existing empty-text exact filters remain unchanged.

Use the generation's canonical path order, with stable generation-relative
result ranks continued across pages; do not globally materialize/sort matches.
Resolve every returned binding to its stored exact record. Proposal: reuse
`exact_path` evidence with `channel:"exact"`, `exact:true`, `inferred:false`,
`score:1`, an explicit predicate calibration and root-relative anchor. This
means exact evaluation of a path-substring predicate, not whole-path equality;
root must approve that evidence meaning or select a new evidence kind. Do not
copy live's basename-only anchor for ancestor-only matches. No highlight byte
offsets are promised across Unicode length changes.

## Bounded reference and proposed numeric ceilings

**CANDIDATE laboratory defaults/hard ceilings; unmeasured, not latency claims:**

| Resource per request | Default | Hard ceiling / rule |
|---|---:|---|
| Returned results (`limit`) | 128 | 1000, preserving current query bound |
| Rows examined | 4096 | 65,536; count every inspected row, not just hits |
| Logical reader bytes charged | 4 MiB | 16 MiB; charge requested bytes including cache hits, separately measure physical I/O |
| Cooperative page time | 25 ms | 250 ms, additionally bounded by transport/context deadline |
| Complete encoded Engine response | 128 KiB | 512 KiB including envelope, cursor, evidence, warnings; below existing 1 MiB frame |
| Cursor size | — | 4096 encoded bytes; reject oversize before decoding |
| Scratch/page storage | — | proposed 8 MiB incremental scratch ceiling, plus existing bounded reader cache and encoded-response storage, separately accounted |

Scan sequentially through the checked reader, keeping a next-row position and
one bounded page. No `CandidateAll` million-row vector, whole-tree copy or
all-match accumulator. Bound a record's decode before allocation; if a single
valid record cannot fit the hard work/output/scratch ceiling, return
`RESOURCE_BUDGET_EXCEEDED`, not a repeating zero-progress cursor or skipped hit.
Requested read bytes are a controllable logical-work bound, not a physical-I/O
claim. Reader primitives must be audited for bounded decode and cancellation;
their existence alone is insufficient.

When a cooperative row/read/page-time/result/output limit is reached after
progress, return the completed page and next cursor, even if no rows matched.
`partial:false` with a cursor means incomplete pagination without a coverage
defect; absence of cursor means the checked scope was exhausted. Stale/coverage
warnings retain their existing semantics and may set partial. An exhausted
no-match is authoritative for that checked generation/policy and must not fall
back. Context cancellation, hard deadline, integrity failure and an oversized
single result are terminal failures with no successful page or continuation.
Check context at bounded read/row boundaries; blocked native I/O cancellation
remains a measurement gate, not an asserted guarantee.

## Cursor, source and cancellation ownership

**CANDIDATE preferred design:** stateless Engine continuation. It binds provider
instance, generation, root/configuration identity, predicate revision,
normalized query/scope/order, next row and next rank; authenticate it with an
instance-owned key. Validate bounds and identity before use. Pin the checked
reader only during a request and release it on every exit. Abandoning a cursor
retains no per-query object, timer, open file or old-generation lease. Restart
or generation/configuration change invalidates it with `GENERATION_EXPIRED`
(existing Orchestrator Stale mapping); wrong-query/tampered tokens are invalid.
Never restart automatically or switch source on continuation. A fresh cursorless
request may re-run current routing and legitimately choose live.

There is a real compatibility edge: catalogue cursors currently carry only
source/value, while `catalogue_params` treats continuing ordinary text as the
old exact-name predicate. **CANDIDATE minimal solution:** Orchestrator wraps
new substring provider tokens in its own versioned opaque `value`, binding the
predicate revision and provider token. It does not inspect/decode the Engine
token. Legacy bare values retain legacy exact-name dispatch; wrapped values
dispatch the explicit new predicate. Reserve and validate an unambiguous wrapper
namespace (legacy Engine cursors are raw base64url); bound/authenticate the
wrapper or validate its complete binding through the provider token. Malformed
or unknown wrappers fail, never fall through to legacy. No frontend field is
added; it already echoes opaque source/value. Root must reconcile wrapper
ownership/version handling and cross-version rejection fixtures before use.
Alternative: explicit additive predicate field in the outer cursor, with matching
Rust/C++ codecs. Do not silently implement neither.

**OBSERVED cancellation gap:** Go `Engine.Query` accepts `context.Context`, but
`transport.toFault` maps both context cancellation and deadline expiry to
`RESOURCE_BUDGET_EXCEEDED`. `project_engine_error` maps that to BudgetExceeded.
The C++ `search_subtree` call exposes no caller cancellation token, and current
frontend supersession only discards completed output.

**CANDIDATE:** the private reference accepts and tests real context cancellation
immediately. Before public admission, root must choose additive distinct Engine
error codes for cancelled/deadline (spellings not assigned here) with explicit
Rust terminal mappings, or explicitly retain the old wire limitation as a
blocked conformance gate. A new JSON `query_id` alone does not implement cancel.
No cancel method is invented by this proposal. Abandoning an in-flight client
request must end work through verified transport context/disconnect handling
or its bounded deadline; test this per transport. No result push stream or
unbounded prefetch: clients pull one bounded page. General scheduler/global
query concurrency remains server policy; budget-clamped pages cannot override it.

## Freshness, capability absence and version policy

**CANDIDATE:** initial reference is checked cached-generation search with
existing stale/unavailable-root provenance. Do not add a silent strict-current
promise: `api.Query` has no freshness selector and current Windows indexing is
manual-reconcile. If strict-current selection is required, negotiate an explicit
field and rejection before route activation. Keep the broker allowlist intact:
Partial stale results do not automatically trigger fallback. Catalogue-only
ordinary text without capability stays Unsupported; prefer-catalogue may use
live. Denied/invalid/time/budget/cancelled and authoritative zero-match never
fall back. Combined filters remain catalogue-only and unsupported for this
text slice. Capability disappearance during continuation is a failure on the
same lane; only an explicit new query may select another source.

Advertise the predicate only for a checked supported provider/configuration;
separate implementation availability from per-root catalogue availability and
currentness. Unknown predicate/revision must not be ignored. Existing
`METHOD_UNAVAILABLE` maps to Unavailable for `engine.query`; the proposal needs
an explicit Unsupported mapping for unknown predicate or an agreed existing
error projection. Root chooses the semantic revision and error spellings, not
this draft. Frontend Core 1.0 framing need not change if opaque cursors suffice.

## Generation match versus fresh pathname observation

**OBSERVED acceptance gap, added at parent direction:** C++ `SearchResultInfo`
currently contains only name/path/kind/unsigned size/unavailable.
`Client::search_subtree` drops `object.root`, `object.id`, optional platform key
and incarnation, record generation/rank/evidence and signed modified time.
`SearchPageInfo` also drops stale/unavailable roots and warnings that
`kernel.rs` already sends. `Application::apply_engine_search` and
`apply_engine_criteria` subsequently observe the current object at the pathname.
The criteria worker carries filters to the UI callback, but that callback's
unnamed filter parameter discards them. Current native facts are appropriate
for current display and operation checks; they do not prove that the observed
object is the binding that matched the catalogue generation, or that changed
size/date/kind still satisfies the query. Details source acceptance is not
indexed-search acceptance.

**CANDIDATE smallest typed projection:** preserve existing wire data in named,
owned C++ records before adding another Engine method. A result needs the source
object reference (root/id/path, optional platform namespace/key and incarnation),
its stored metadata including signed `modified_unix_nano`, generation and exact
match evidence (kind/channel/calibration/anchor/exact/inferred). Preserve rank
if its order is claimed. A page needs source/generation or live scan identity,
terminal/completeness and stale/unavailable-root/path/warning data. Maintain a
separate owned request context containing the actual text/predicate or exact
filters and scope through application. These are proposed C++ projections of
existing fields, not new frozen type names or a new binding-ID wire field:
current `ObjectRef` has an address but no distinct `binding_id`.

Retain generation-match facts separately from the fresh native observation.
Until identity compatibility is proven, label the match as a cached generation
observation and fresh pathname verification as unverified/changed, or reject
it from a current-verified view. Never replace the matched object ID with the
new pathname occupant and retain an unqualified exact/current match claim.
Unknown/missing platform evidence is not equality. Even confirmed same-object
identity does not establish that mutable metadata predicates still hold;
re-evaluate retained criteria against comparable fresh facts before making that
claim, otherwise preserve a stale/unverified match or exclude it according to
explicit consumer policy. Operation-time identity checks remain independent.

Identity comparison needs a negotiated platform adapter, not string parsing
guesses. For example, `engine/internal/identity/identity_windows.go` currently
uses `BY_HANDLE_FILE_INFORMATION` 32-bit volume/64-bit file index and creation
time incarnation; frontend `ObjectIdentity` retains `FILE_ID_128` low/high words
and has no matching incarnation field. Equality or truncation between these
representations is not established by this proposal. The minimal comparison
receipt must cover each platform's key encoding/incarnation availability and
fail closed when not comparable. No storage reopening is needed merely to
retain existing wire identity; stronger platform evidence remains its own gate.

Required acceptance cases: replace a matched pathname with a different object
before application; same-object size/time/kind no longer satisfying criteria;
rename and hard-link address changes; absent incarnation; incompatible platform
key forms; stale page provenance retained; unchanged verified identity and
predicate control. Old stored match and new observation must remain separately
inspectable. Typed projection and policy are required before a frontend
indexed-search acceptance claim, but can remain outside the private exhaustive
reference implementation checkpoint. Root owns that stage separation.

## Concrete fixture shapes

All envelopes/field names in the first fixture are current. Engine request
scope uses `root`/`path`, unlike live's `root_id`/`relative_path`. These examples
are design fixtures, not executed responses or ready-to-run cursor tokens.

Existing exact-name control, unchanged:

```json
{"id":"exact-1","method":"engine.query","params":{"text":"","scope":{"root":"docs","path":"work","descendants":true},"limit":128,"filters":{"name":"needle-report.txt"},"channels":["exact"]}}
```

Proposed substring request; only `text_predicate` and `scan_budget` are new:

```json
{"id":"substring-1","method":"engine.query","params":{"text":"needle","text_predicate":"root_relative_lower_substring_v1","scope":{"root":"docs","path":"work","descendants":true},"limit":128,"channels":["exact"],"scan_budget":{"max_rows":4096,"max_read_bytes":4194304,"max_wall_time_ms":25,"max_response_bytes":131072}}}
```

Exhausted no-match response (existing reply shape plus proposed plan disclosure):

```json
{"id":"substring-1","result":{"generation":7,"results":[],"partial":false,"plan":{"scopes":[{"root":"docs","path":"work","descendants":true}],"channels":["exact"],"elapsed_micros":1200,"text_predicate":"root_relative_lower_substring_v1","scan_work":{"rows_examined":40,"read_bytes":8192,"elapsed_micros":1200}}}}
```

For a page that inspected 4096 rows with no hit and more scope remaining, use
the same response with a nonempty `next_cursor` and corresponding work values.
It is not exhausted no-match. Continuation repeats the identical predicate,
text/scope and adds that opaque token as `params.cursor`. Page budgets may change
within policy; neither query meaning nor source changes. Kernel emits
`complete:false` and wraps the provider token in its opaque catalogue cursor.

Matched-result fixture body using existing Result/Evidence fields; the
calibration string below is proposed and not a new evidence enum:

```json
{"object":{"root":"docs","id":"fixture-object-1","path":"/fixture/docs/work/needle/report.txt"},"metadata":{"name":"report.txt","kind":"file","size":0,"mode":420,"modified_unix_nano":0},"generation":7,"rank":1,"certainty":1,"unavailable":false,"evidence":[{"kind":"exact_path","channel":"exact","score":1,"calibration":"root_relative_lower_substring_v1","exact":true,"inferred":false,"anchor":"work/needle/report.txt"}]}
```

Existing stale-error wire shape, unchanged code:

```json
{"id":"substring-2","error":{"code":"GENERATION_EXPIRED","message":"cursor belongs to a different committed generation"}}
```

Existing Orchestrator cursor field shape remains
`{"source":"catalogue","value":"<opaque Orchestrator wrapper>"}`. The angle
bracket text is a placeholder, not an encoded cursor fixture. Executable golden
fixtures must substitute deterministic test-key tokens and complete the actual
ORC1 envelope from existing fixture builders; this proposal does not invent
that envelope or a new contract major/minor.

## Required fixture matrix and reconciliation choices

**CANDIDATE fixtures:** exact-name hit/no-match unchanged; partial basename;
ancestor-only; match spanning slash; scoped descendant whose ancestor before
scope matches; descendants false; 1/2-character common text; exhaustive
no-match; empty/whitespace invalid; leading/trailing space literal; uppercase,
Unicode simple-lower versus full-fold, combining marks, invalid UTF-8 lowering;
query backslash unchanged; repeated names/hard-link bindings; nested-root
eligibility; row/read/output/time page stops; zero-hit progressing page;
oversized one-row no-progress failure; cancellation and cleanup; abandoned
cursor retaining no resources; mutation/generation publication between pages;
restart/configuration/authority change; corruption; wrong-source/query token;
legacy bare exact-name continuation; unknown wrapper/predicate/capability;
stale cached Partial without fallback; exact/substring authoritative zero-match
without fallback; combined filters never dropped; independent Go/Rust/C++
round-trip. Compare all pages to an independent exhaustive oracle, not live
timings over a mutating tree. Generated 10k/100k/1m workloads follow
`engine/docs/INDEXED_NAME_PATH_SEARCH_001.md` and the frontend audit.

**Choices requiring root reconciliation before bounded implementation:**

1. Approve additive predicate/scan-budget/plan-work shape, capability spelling
   and version gate, including explicit Unsupported projection.
2. Accept path-order/cached-generation scope and text-only first slice; approve
   evidence calibration/kind and treatment of strict-current requests.
3. Choose opaque Orchestrator wrapper versus explicit cursor predicate field;
   accept no inter-request generation pin and stale/restart behavior.
4. Accept or revise the unmeasured numerical laboratory ceilings and precise
   logical-read/encoded-byte accounting; audit bounded reader primitives.
5. Decide cancellation/deadline error additions and required cross-process
   cancellation scope. Private context conformance does not close public-client
   supersession cancellation.
6. Approve typed preservation of existing match identity/evidence/currentness
   and a platform comparison/changed-predicate policy before consumer admission.
   Decide whether unverified cached matches remain visibly cached or are excluded;
   they may not masquerade as fresh verified matches.

Reversal: leave the capability unavailable and ordinary text on its current
live route; retain the exhaustive reference/negative fixtures. No new persistent
index is created, so no catalogue migration is implied. Whole-name substitution,
full candidate materialization and per-query old-generation leases are not
recommended alternatives for this checkpoint. No performance claim is made.

Prerequisites: Orchestrator AGENTS/readme, ADR-003/006, master specification,
registry, authority/process, hive/settings, conformance/versioning, delivery
sequence and negotiation protocol were read in this investigation's conversation;
current types and ordinary-text reconciliation were inspected for this draft.
The complete house style governs any later implementation/tests/tooling. This
turn authored prose only and certifies no existing source as style-compliant.
