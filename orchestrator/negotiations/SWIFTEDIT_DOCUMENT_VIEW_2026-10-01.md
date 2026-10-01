# SwiftEdit document/view negotiation — round 001

Date: 2026-10-01.

Status: **D1 semantic draft reconciled for development implementation; later
stages remain unresolved. No new runtime operation or availability is
published. The original candidate intake is retained below; final disposition
is appended at the end.**

## Authority and reviewed scope

**GIVEN:** the owner directed the SwiftEdit chat to continue implementing its
expanded feature set and perform a final lag scan. Parent coordination verified
the actual owner message in chat `01a0f0bd-1423-7710-923f-c76ade7a8490`, turn
`01a0f659-c634-7ed1-8495-4c1b6050d803`. This permits work toward the requirements;
it does not select a high-reversal-cost document architecture or establish
measured performance.

Participants:

- GUI.Forms provider: chat `01a0f0bf-3d19-7fe1-8995-119065b2448b`.
- SwiftEdit consumer: chat `01a0f0bd-1423-7710-923f-c76ade7a8490`, repository
  `C:/Users/Shadow/notepad`.
- Orchestrator: canonical semantics and development registry reconciliation.
- Parent: integration/build coordination and owner escalation.

SwiftEdit is the active external consumer in this round. The planning-only
`text_editor/` project's interview and implementation gates are not silently
changed by that consumer's owner direction.

Reviewed records: `planning/PROGRAMMING_HOUSE_STYLE.md`, applicable root and
component instructions, ORC-GUI-001 and ORC-FE-001 in the registry and
`spec/contracts/FRONTEND_AND_GUI_FORMS.md`, GUI.Forms' project-local negotiation
ledger and future-application profile, Text Editor's intake, and SwiftEdit's
objective ledger and current `src/session.hpp` / `src/display.hpp`. This task
authors contract/negotiation records only. Provider and consumer source review
against the full house style remains mandatory for their implementations;
functional tests or a spelling scan alone cannot establish compliance.

## Existing admitted boundary

**OBSERVED:** the current development TextBox has provisional 1 MiB document
and 4096-byte logical-line limits, one selection, UTF-8 storage, and bounded
snapshot history. Its successful-save history-reset addition is separately
recorded. Neither read-only mode nor successful SDK installation supplies a
paged-document, multiple-selection, source/display mapping, or native-print
capability.

GUI.Forms owns reusable retained view/input/layout behavior. SwiftEdit owns
source bytes, encoding interpretation, mutation, save/conflict policy, and
file handles. Orchestrator records cross-project semantics; document bytes,
paint, pointer motion, and edit commands do not acquire an Orchestrator daemon
hop. Historical frozen FM0 artifacts remain independent.

## Consumer reply received

**OBSERVED source and consumer reply:** SwiftEdit's Session owns exact mutable
bytes below `16 * 1024 * 1024`; at or above that threshold a retained read handle
supplies owned page copies of at most 65536 source bytes. The exact binary
threshold is SwiftEdit's documented interpretation of the owner's “16 MB”, not
a separate quoted owner answer. Display expansion must not change source-byte
admission.

The consumer's current projection owns valid display UTF-8 plus source/display
range units. Controls and illegal bytes have inert labels; a CRLF pair forms
one mapping unit. Literal source text resembling a label is not decoded as a
control. Internal label/scalar/CRLF boundaries are rejected. Existing unit
offsets are page-relative; the cross-project adapter must additionally carry
absolute source base, document identity, and observed revision.

Its synchronous `replace_ranges` validates ordered disjoint ranges and observed
revision before a single undoable publication. Stale/range/size/payload failure
preserves source and history. Equal-grapheme parallel-edit policy belongs to
the selection adapter; the internal range primitive does not impose it.

These consumer-local C++ shapes are evidence and proposals, not frozen public
provider types.

## Candidate stage boundary

1. Bounded document/view projections, explicit source/display coordinates,
   immutable revision-bound ownership, viewport requests and scrollbar mechanics.
2. Selection sets and atomic edit intents over the same document identity and
   mapping law; expose unsupported until the provider and consumer conform.
3. Native printing and layout preview as a separately negotiated capability.

Document/view work need not wait for printing. No stage is advertised available
merely because the requirement or a C++ declaration exists.

## Points proposed for agreement

- Distinct absolute source-byte offsets and page-relative display-byte offsets;
  neither silently becomes a grapheme index, UTF-16 offset, row, or pixel.
- Immutable owned bounded page/projection values tied to document identity,
  revision and requested view generation. No source borrow survives deferred
  retrieval/layout work. Only the owner/UI thread publishes Session mutation.
- Source window at most 64 KiB, with separately bounded context and output
  expansion. Exact context/output/layout-work bounds await provider reply.
- At most two pending viewport requests; a newer viewport supersedes older
  work. Cancellation is checked between bounded slices. Stale identity/revision
  or view generation cannot publish paint, hit mapping, selection, or edits.
- Explicit typed invalid, stale, cancelled, unavailable/unsupported, and
  budget-exceeded results; failed replacement does not partially edit bytes or
  history. Exactly one terminal disposition for each accepted request.
- Inert display labels map by metadata, never reverse parsing. Expanded labels
  and edit graphemes are indivisible; CRLF retains its source extent.
- Selection sets identify a primary range, preserve direction deliberately, and
  normalize overlap/duplicates by an agreed rule before an atomic edit intent.
  Equal-length eligibility counts source graphemes, not label characters.
- SDK/manifest version, limits and supported capability stages must match the
  actual installed provider. Missing support keeps the current document intact
  and reports an explicit refusal or separately supported read-only route.

## Open questions before stage reconciliation

1. Provider public operation/result proposal and the smallest independently
   implementable stage; no native layout/allocator/ABI is selected here.
2. A source grapheme or shaping context may exceed 64 KiB. What is the bounded
   context rule and exact refusal/placeholder behavior? A clipped UTF-8 sequence
   must never be falsely classified as an illegal source byte.
3. Exact source-to-display endpoint/affinity rules at page, label, CRLF and soft
   wrap boundaries; how mapping version changes invalidate hit-test results.
4. Selection overlap/duplicate/direction normalization and capacity; edit intent
   lifetime and consumer acknowledgement before the view changes.
5. Numerical cache, expansion, shaping-work and scheduling bounds; bounded
   cancellation latency must be measured, not inferred from a byte cap.
6. Native-print ownership, cancellation, pagination and platform availability
   remain separate. No print support is implied by document/view agreement.

## Evidence gate draft

Preserve source-byte oracles for empty input, threshold boundaries, mixed
endings, split UTF-8/page boundaries, controls and invalid bytes, literal label
collisions, long lines and over-budget clusters, stale identity/revision/view,
cancel/supersede/close, resource refusal, and atomic multi-range failure. Test
mapping round trips only at admitted boundaries. Independently built consumers
must consume the matching public package. Native platform evidence and a
same-machine workload baseline are required before performance/availability
promotion; p50/p95/p99 and worst stall, retained bytes and work counts remain
unmeasured in this round.

Provider reply and subsequent reconciliation will be appended rather than
rewriting this intake as though it were already accepted.

## Provider reply 001 and consumer confirmation

**OBSERVED recorded replies, 2026-10-01:** GUI.Forms supplied and refined
`gui_forms/docs/DOCUMENT_VIEW_PROVIDER_PROPOSAL_2026-10-01.md`, then explicitly
accepted the parent/Orchestrator corrections. SwiftEdit explicitly accepted
the bounded D1 first stage and its ownership, identity and mapping requirements.
The parent reviewed both proposals and authorized bounded implementation after
those corrections were agreed and recorded. This is a reversible development
semantic reconciliation under the existing owner feature direction, not a new
high-reversal-cost architecture decision.

Final agreement differs from the initial candidate list in these ways:

- D1 is the renderer-neutral state/request/publication/mapping model only.
  Selection storage/editing moves to D4; D2/D3 must establish retained control
  and cluster geometry before a useful large-document GUI can be advertised.
- The 64 KiB source permission includes all page context. Display capacity is
  separately 1 MiB; mapping capacity is 65536 spans. Total bytes depend on
  native record size/overhead and separately reported workspace, not source size.
- Two producer slots include queued/in-flight/ready/cancelled-unreleased work,
  plus one published page. Rejected publication retains its slot and caller's
  payload until cleanup followed by full-token `finish` acknowledgement.
  Cancellation never implies ownership release. Successful publication moves
  the incoming owner only after producer source/scratch has been released.
- Every mapping call uses the exact published request token, including serial;
  same-revision page replacement invalidates previous page positions. Deferred
  consumers retain that token alongside synchronous mapping results.
- Full request validation precedes capacity checks. Invalid input never revokes
  valid authority. Valid `busy` changes nothing until the caller explicitly
  cancels and replaces one desired-viewport metadata value in a non-reentrant
  UI handling sequence; no payload backlog is admitted.
- Unknown boundary context is `context_required`; known hard capacity/work
  violation is `budget_exceeded`. Neither publishes empty success, advances an
  anchor or triggers automatic unbounded scans/retries. Exact mapping rejects
  token/scalar interiors; affinity resolution belongs to later layout.
- Allocation failure, late results, forged tokens and destruction have explicit
  ownership/slot-release laws. Tokens are model-instance scoped and dispatch
  drains before model destruction.

Consumer-confirmed D4 policy is retained without admitting a D1 edit API:
reject overlapping/duplicate selections, duplicate empty carets, and an empty
caret at either endpoint of a nonempty selection; adjacent nonempty ranges are
allowed. Preserve direction and primary selection. Parallel user edits require
equal source-grapheme lengths; unequal lengths are copy-only.

## Orchestrator reconciliation 001

Canonical semantics and required negative cases now live in
[`../spec/contracts/GUI_DOCUMENT_VIEW_DEVELOPMENT.md`](../spec/contracts/GUI_DOCUMENT_VIEW_DEVELOPMENT.md).
The provider's narrow D1 implementation/model tests may proceed. Model source,
headless fixtures, installed independent consumer, retained/native UI and
performance are separate evidence stages. No new executable availability is
published. Printing, storage/history selection, long-line shaping and other
high-reversal choices remain unselected; the planning-only Text Editor gate is
unchanged.

Records-only review checked coordinate units, complete token equality,
ownership on rejection/cancellation, validation-before-mutation order, resource
capacity accounting, terminal outcomes and current-vs-proposed availability.
No runtime code, test or authored tool was added in this negotiation task.
Implementation house-style source review and performance/native evidence remain
provider/consumer obligations, not facts inferred from this agreement.
