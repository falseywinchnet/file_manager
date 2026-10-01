# ORC-GUI-001 — bounded document/view development stage D1

Date: 2026-10-01. Revision: `document-view-d1-draft-1`.

Status: **reconciled semantic draft for bounded D1 development implementation;
provider and consumer replies recorded in round 001. No
runtime capability, installed SDK, stable ABI, or large-document GUI availability
is asserted.**

Negotiation provenance:
[`../../negotiations/SWIFTEDIT_DOCUMENT_VIEW_2026-10-01.md`](../../negotiations/SWIFTEDIT_DOCUMENT_VIEW_2026-10-01.md).
Provider proposal:
[`../../../gui_forms/docs/DOCUMENT_VIEW_PROVIDER_PROPOSAL_2026-10-01.md`](../../../gui_forms/docs/DOCUMENT_VIEW_PROVIDER_PROPOSAL_2026-10-01.md).

## Authority and stage

**GIVEN:** SwiftEdit's owner directed implementation of its expanded feature set
and final lag review. **OBSERVED:** its current source interprets the requested
16 MB boundary as 16 MiB: mutable below, paged read-only at or above. That exact
unit choice is a documented consumer interpretation, not a new owner answer or
a GUI.Forms storage decision.

GUI.Forms owns reusable retained presentation mechanics. SwiftEdit owns exact
bytes, decoding/invalid-byte interpretation, file handles, mutation, undo,
save/conflict policy and document identity. Orchestrator owns this semantic
record. Document data, paint, scrolling and editing remain in-process and do
not traverse the Orchestrator daemon. No new wire method or C ABI is added.

D1 admits only a renderer-neutral bounded page/request/publication/mapping
model and exact-anchor viewport state. D2 retained control and D3 bounded
cluster layout require subsequent reconciliation and evidence before a usable
large-document GUI is advertised. Selection storage/editing/accessibility is
D4. Native printing is a separate P1 negotiation and cannot block D1.

Contiguous, piece/chunk storage, history representation, cluster checkpoints,
renderer implementation, font/shaping parity and scheduling strategy remain
provider/consumer candidates or later-stage questions. D1 does not lift the
existing TextBox's provisional limits or replace historical frozen artifacts.

## Semantic records and projection

The proposed development C++ entry is `gui_forms/document_view.hpp`; C++20
source projection, matched provider/consumer rebuilds, no binary layout promise.
Names below identify meanings rather than freeze padding or allocator choice.

| Record | Meaning |
|---|---|
| `DocumentRevision` | Nonzero uint64 document identity and nonzero monotonic revision; identity changes on replacement/reopen |
| `SourceByteOffset` | Exact uint64 absolute source-byte boundary; document extent is its one-past-end offset |
| `DisplayByteOffset` | uint32 UTF-8 byte boundary relative to one identified published page |
| `SourceByteRange` | Ordered half-open begin/end source boundaries; no implicit conversion from display, grapheme, row or pixel |
| `DocumentViewport` | Exact source anchor and finite nonnegative horizontal offset in DIPs |
| `DocumentPageRequest` | Document/revision, monotonic request serial, permitted source interval, viewport; token scoped to its model instance |
| `DocumentMapSpan` | Contiguous source and display intervals and `identity_utf8` or `atomic_token` kind |
| `DocumentPage` | Full request token, actual covered source interval, owned display UTF-8 and owned mapping spans |
| Mapping result | Typed status and optional position; the caller retains its exact validated published request token alongside any deferred use |

Publication and mapping compare the **complete token**, not a caller-supplied
serial alone. Same-revision page replacement invalidates the former page token.
Tokens cannot be rerouted to another model instance. Owner dispatch binding and
shutdown must preserve that scope; serial values are not global identifiers.
Within an instance serials never reset across bind/cancel, wrap or recycle.
Exhaustion explicitly refuses a new request.

## Bounds and memory ownership

| Quantity | D1 hard bound |
|---|---|
| Permitted source interval, including all decoding/context bytes | 65536 bytes |
| Owned display string capacity per payload | 1048576 bytes |
| Mapping vector capacity per payload | 65536 spans |
| Producer slots | Two total across queued, in-flight, ready and cancelled-but-unreleased work |
| Published payload | One current page |
| Coalesced desired viewport | One metadata value, no payload/job backlog |
| Per-slot source copy | One owned source page, at most 65536 bytes |

Source bytes, display expansion, mapping entries and total memory are separate
quantities. Receipts must state mapping bytes as capacity times the actual
`sizeof(DocumentMapSpan)`, object/control overhead, and allocator overhead where
measured; the 64 KiB source bound is not a 64 KiB total-allocation promise.
At most two producer display/mapping payloads plus one published payload coexist.
Consumer document/history storage and bounded indexing/shaping workspace are
separately accounted; they cannot be hidden as page storage. D1 does not admit
an unbounded scratch allocation or implicit extra context beyond the permitted
interval. Later layout/native rendering storage remains unavailable here.

Producers check budgets before allocation/copy. Publication validates then
transfers ownership without copying a full payload, releases the former page,
and empties the successful incoming owner. Capacity, not merely active length,
is checked. Move/allocation failure preserves the prior page and keeps the
incoming ownership/release responsibility explicit.

## Operations, ordering and lifetime

All model methods execute on one owning UI thread; D1 adds no hidden workers,
locks, subscriptions or callbacks. Workers receive value tokens and return
owned results through the owner's existing bounded UI dispatch. No UI/source
borrow crosses deferred work.

1. `bind(revision, document_end)` validates nonzero identity/revision. Identical
   revision and size is idempotent; changed size at the same revision is invalid.
   Revisions must increase within an identity. Successful changed binding clears
   page/viewport and revokes old publication authority, but occupied producer
   slots remain until ownership release is acknowledged.
2. `request_page(viewport, permitted)` validates anchor, finite/nonnegative DIP
   offset, interval ordering, extent and source budget **before** slot capacity.
   Invalid input changes nothing. Success reserves one slot, installs the
   viewport and supersedes older request publication authority.
3. `busy` changes nothing. For a valid newer desired viewport that receives
   `busy`, the owner explicitly calls `cancel()` and replaces its one coalesced
   desired metadata value in the same non-reentrant UI handling sequence.
   Invalid input must not cancel a valid request. Retry occurs after a slot
   drains; no third job or payload starts in the meantime.
4. `cancel()` revokes pending publication authority while preserving the current
   page and occupied slots. A cancel request is not proof of worker completion.
5. `publish(page)` requires an exact occupied token, current document/revision
   and newest publication authority, then validates the complete page before
   mutation. Success consumes the incoming owner and slot. Per-slot producer
   source and scratch must already be released before successful publication.
6. Every rejected publication preserves both the prior valid page and caller's
   incoming payload, and keeps its producer slot occupied. The caller releases
   rejected payload/source/scratch, waits for producer completion, then calls
   `finish(exact_token)`. Unknown/forged tokens release no slot.
7. `finish` acknowledges completed work and released per-slot ownership; it
   releases only the complete matching token. Repeated/forged acknowledgement
   fails. Its success means capacity released, **not successful page coverage or
   document progress**. Producer allocation exceptions, cancellation and other
   terminal failures follow this same cleanup-then-acknowledgement path.
8. `page()` borrows last published state only until replacement, changed bind,
   destruction or explicit ownership mutation. It cannot be retained across
   dispatch. The page carries its actual request/coverage; an old page cannot be
   relabelled as the newly requested viewport or document revision.
9. Before destruction, the owner revokes requests, drains dispatch/results,
   joins or otherwise completes owned work, and releases all associated payloads.
   No producer retains a model pointer, reference or callback after teardown.

Publication, cancel/bind, mapping and later edit commit must not reenter one
another through callbacks. An eventual edit commit atomically validates both
document identity and revision; D1 itself supplies no edit API.

## Mapping and boundary law

Covered interval is inside permitted interval and contains the requested anchor.
Nonempty coverage has nonempty valid display UTF-8 and a complete mapping.
Empty coverage is allowed only for empty document/EOF and has empty payload.
Spans partition both covered source and display ranges without gaps/overlap;
each span is nonempty in both units. Identity spans have equal byte lengths and
scalar-aligned display endpoints. Consumer certifies byte equality with source;
GUI.Forms cannot certify bytes it does not own.

Atomic tokens represent complete source units such as one illegal byte, a
visible control or a CRLF pair. Exact mapping rejects interiors of tokens and
UTF-8 scalars with `invalid_boundary`. It never reverse-parses labels. Identical
literal source labels remain ordinary identity text. Shared span endpoints
have the same source/display position from either adjoining span.

Both exact mapping directions require the **current published request token**.
Results are synchronous; the caller pairs the supplied token with any deferred
result, never deferring a bare position. Old-page input returns `stale`
even when document revision is unchanged. Off-page positions return
`unavailable`; invalid boundaries are not silently snapped. Explicit hit-test
affinity, soft-wrap affinity and grapheme geometry are later D2/D3 operations,
not an implicit conversion in D1 or an authorization to edit at scalar boundaries.
Zero-length annotations/rulers/line metadata remain outside editable payload.

The consumer must establish complete decoding and grapheme boundary context
before producing a page for later editing/layout. A clipped multibyte sequence
does not become an illegal source byte. If that context cannot be established
inside permitted work, producer reports `context_required`, releases ownership
and acknowledges completion. A known capacity/work bound violation reports
`budget_exceeded`. Neither manufactures an empty successful page or advances a
cursor. Retry at the same anchor requires newly established context or a changed
request/profile; no automatic unbounded backward scan or busy retry loop.

## Outcomes and availability

Expected named outcomes: success, invalid revision/range/page/boundary, stale,
cancelled, busy, unavailable, context required, budget exceeded and serial
exhausted. C++ allocation/operation exceptions are contained by the owner and
follow the explicit release/finish rule. Validation precedes state mutation.
Known optional absence never becomes empty success; incompatible profile/SDK
is refused before using the new source interface.

Development record: D1 semantics are **reconciled for implementation**,
source/SDK conformance unmeasured;
D2/D3/D4 and P1 are separately unresolved. This record adds no executable
availability advertisement. Provider source implementation, model fixtures,
independent installed consumer, native platform behavior and performance
evidence are distinct receipts. A D1 model test pass cannot promote the larger
GUI feature set.

## Deferred selection policy and independent print

Consumer-confirmed D4 policy: preserve anchor/caret orientation and explicit
primary selection; reject overlap, duplicate empty carets, and an empty caret
at either endpoint of a nonempty selection. Adjacent nonempty ranges are allowed.
User parallel replacement requires equal **source-grapheme** lengths; unequal
lengths permit copy only. Consumer internal structured range edits remain a
separate primitive. D4 capacity, grapheme proof and atomic edit-intent protocol
still require reconciliation; these rules add no D1 edit capability.

P1 printing requires a separate versioned capability, frozen revision/layout,
owned session, bounded page production, cancellation and explicit host outcomes.
Spool acceptance is not physical-print success. No printing or preview support
is admitted by D1.

## Required development fixtures and review

- Empty/EOF, >2^53 and uint64 boundary offsets; nonfinite/negative viewport;
  invalid request while all slots occupied preserves valid authority.
- Each independent capacity exceeded before copying; malformed UTF-8,
  mapping gaps/overlap and unequal identity extents; failure preserves old state.
- Literal label collision, controls/illegal bytes, CRLF, token/scalar interiors,
  both map directions, shared endpoints, split decoding and missing context.
- Same-revision page A replaced by B: A mapping token stale, B succeeds;
  forged same-serial token with changed interval/viewport releases no slot.
- Newest request, reorder, busy/cancel/coalescing, bind with occupied slots,
  allocation failure, rejected payload retained until cleanup+finish, duplicate
  finish, teardown with pending work, serial exhaustion and repeated reuse.

Source review must cover the full `planning/PROGRAMMING_HOUSE_STYLE.md` rules:
types, named behavior/state, ownership/borrows, operation order, initialization,
conversions, failure states and repeated-loop storage. Record exact reviewed
files and remaining violations independently of test/spelling results. Measure
bounded mapping/publication work and retained capacities against a simple
reference; performance claims require same-host workload evidence with tail
latency and worst stalls. No speedup or native acceptance is inferred here.
