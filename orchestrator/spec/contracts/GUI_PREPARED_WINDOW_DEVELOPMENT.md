# ORC-GUI-001 — prepared visible-window development candidate

Date: 2026-10-01. Revision: `prepared-window-candidate-1`.

Status: **CANDIDATE for provider/consumer reconciliation. Records only; no
executable assignment, ADR, SDK export or runtime availability promotion.**

This is the proposed multiparagraph prerequisite for a usable paged read-only
view. It extends the development meaning of a prepared generation; it does not
turn the existing single-paragraph API into a complete DocumentView by exporting
it. The existing [D1 contract](GUI_DOCUMENT_VIEW_DEVELOPMENT.md) and
[A2 contract](GUI_PREPARED_TEXT_DEVELOPMENT.md) remain authoritative for their
implemented stages. Dialogue and subsequent dispositions belong in the
[SwiftEdit negotiation](../../negotiations/SWIFTEDIT_DOCUMENT_VIEW_2026-10-01.md).

## Evidence and the proposed change

**OBSERVED source**, reviewed at checkout `c682bb3f980d2917a2dafa14002f0cf9a395c7ba`:

| Named source | Current behavior relevant to this proposal |
|---|---|
| `gui_forms/include/gui_forms/prepared_text/service/prepared_text_service.hpp` | Explicit caller-owned service/session; one open session; worker wake may only enqueue a signal and must not reenter session control |
| `gui_forms/src/render/text/prepared_service.cpp`, `open_session` | Rejects a font bank from another ledger and returns busy while the previous session is unjoined; creates the worker in session state |
| Same file, `desire`, `submit`, `adopt_ready` | One current session authority; valid desire advances its epoch even for an equal key; one occupied queued/running/ready replacement slot |
| Same file, `PreparedSessionState::run` | Calls the wake target under the session mutex after publishing readiness; shaping is outside that mutex |
| Same file, `close`, `join` | Revokes authority and wake access; joins noninterruptible work before releasing job/ready ownership |
| `gui_forms/src/core/text/prepared/prepared_storage.cpp` | Checks combined mapping/endpoint counts, capacities, full key and proof; ledger charges survive retained owners |
| `gui_forms/include/gui_forms/prepared_text/types/prepared_text_types.hpp` | Declares A2 limits and a single paragraph proof; the key has context generation but no explicit projection-generation field |
| `gui_forms/src/core/dispatcher/state/post_dispatch.cpp` and dispatcher types | Ordinary posted callback queue has a 4096-callback limit and can reject a post |

The provider's D2 source-review reply independently identifies the
single-paragraph limitation. Repeated `desire` calls for separate rows revoke
earlier rows; retaining their bytes does not preserve authority. Sharing a
service pointer does not grant independent sessions, font-bank portability or
independent viewport authorities. No multiparagraph implementation or reliable
prepared-readiness host channel is established by the source observations above.

**CANDIDATE:** prepare one bounded visible-window batch containing several
complete paragraphs under one session authority and one layout generation.
All paragraph layouts in that batch share admission, cancellation, publication
and retention accounting. The worker may shape paragraphs sequentially inside
one job, reusing bounded workspace. It must not publish independently current
row generations or multiply service/session limits by paragraph count.

## Ownership and identity

**CANDIDATE:** an explicitly application-owned projection controller owns one
D1 model, one service, its admitted font bank and one exclusive session. It
coordinates one active batch, one replacement slot and one latest desired
metadata value. GUI.Forms supplies reusable retained view/controller mechanics;
SwiftEdit owns bytes, decoding, projection, source-grapheme certification,
selection, source-copy policy and producer work. No daemon hop is introduced.

Several retained controls may mirror this same projection controller when they
share its exact document, viewport coverage, projection, font configuration and
device-scale/layout identity. They retain the same immutable batch, not copies
or extra generation reservations. Per-control placement and clipping may differ
only where they do not change the prepared identity. A different device scale,
font configuration or independently moving viewport is a different projection.
Independent projections require explicitly owned separate services/sessions
under the current model. Their workers and ledgers must be visible in application
ownership and resource reporting; no hidden worker is created by each control.
A shared multi-session service or cross-ledger font bank is a separate future
negotiation. Separate services do not establish an application-wide quota.

The proposed batch identity contains the model/controller instance, complete
D1 request token, actual covered source interval, projection generation,
boundary/context generation, layout serial, provider instance/generation,
font-bank identity/generation, effective font, actual device scale and raster
profile. Admitted wrap/tab policy is explicit even while restricted. Hashes and
serials alone cannot replace full equality. Controller and attachment identities
have nonreuse/exhaustion rules; native handles are not identity authorities.

A projection change revokes old layout authority and issues a new D1 request
token even at the same source revision and anchor. The controller carries the
projection generation alongside that token through the producer request,
paragraph metadata and completion. An old projection must not publish through a
new token. D1's existing token is not silently extended or reinterpreted; exact
source-level records for the associated generation require provider agreement.
Literal text resembling a visible-control label remains identity text. Atomic
labels use the consumer's explicit mapping and are never reverse-parsed.

## Paragraph descriptors and exact coverage

**CANDIDATE:** a batch input owns bounded display text, D1 mappings, certified
source endpoints and an ordered array of complete-paragraph descriptors.
Each descriptor names absolute source content extent, page-relative display
content extent, boundary certification and any following consumed separator's
source/display extents and explicit kind. Source offsets are uint64 byte
boundaries; display offsets remain page-relative byte boundaries. Neither is a
row index, glyph index, grapheme count or pixel position. Result rows carry
their descriptor identity, checked glyph/run spans and finite baseline/extent
geometry in declared units.

Descriptors and consumed separators must account for the complete declared
coverage, in order, without hidden gaps, overlap or invented source bytes.
The batch remains inside one D1 page and its permitted interval, including all
context. A separator is covered source even when it emits no glyph. The D1
mapping continues to cover its display projection; consumed separator metadata
does not erase or weaken D1's nonempty mapping-span law. A CRLF pair is one
indivisible source unit, with its full two-byte extent. Separator recognition
comes from explicit producer metadata and certified source semantics, never
from looking for newline-like text inside an atomic label.

The first proposed separator profile is explicitly certified LF, CR and CRLF;
other paragraph separators require a named supported profile or a typed refusal.
Separators are consumed through exact source/display metadata and row breaks,
not injected as glyphs. The provider and consumer must reconcile the exact
record encoding. A future visible separator-marker profile needs explicit
agreement; no implementation guesses that choice. A visible-control projection
changes generation; it does not change source coverage or turn a separator into
ordinary unrelated label text.

Empty paragraphs between consecutive separators have descriptors and explicit
line-box metrics, even though they have zero content bytes/glyphs. A trailing
separator may yield a final empty EOF row. Empty document/EOF has its certified
EOF endpoint and empty-row descriptor; it does not invent a zero-length D1
mapping span. Empty rows consume descriptor/metadata capacity and vertical
extent, so arbitrarily many empty rows are not free. Shared boundaries refer to
the same certified source/display pair; duplicate proof records must not evade
the existing strict endpoint ordering.

The zero-length EOF source anchor is extent evidence, not an implemented visual
caret, hit-test or editable endpoint API. Row baseline/height must be present for
empty rows as well as nonempty rows. **OBSERVED consumer reply relayed by the
coordinator:** SwiftEdit recommends a first development ceiling of 512 visible
plus overscan row descriptors together, under all the same aggregate budgets.
**CANDIDATE:** adopt that ceiling for comparison. The controller derives demand
from viewport geometry; excess returns an explicit unavailable/budget outcome
without silently dropping rows. This is not a permanent product row limit.

All paragraphs needed for the admitted window must be complete within the
permission. Unknown or partial boundary context returns `context_required`;
known source/display/record/work excess returns `budget_exceeded`. Unsupported
separator/tab/wrap policy returns an explicit unsupported profile. None clips a
paragraph into success, fabricates EOF, misclassifies split UTF-8 as an illegal
byte, advances the source anchor, or starts an unbounded backward scan/retry.
An explicitly smaller request may succeed only with its actual smaller coverage
and extent reported. It cannot claim the originally requested visible window.

The first slice may require the requested top anchor to be a certified paragraph
start or empty EOF; an interior anchor then needs a typed context/geometry
refusal until exact source-to-visual positioning exists. This restriction is a
candidate to reconcile, not a rewrite of D1's broader anchor admission. Vertical
navigation uses exact certified source anchors; scrollbar floating-point values
cannot manufacture exact byte offsets or an unmeasured global scroll extent.

## Combined admission and retained storage

**CANDIDATE extension of the existing A2 profile:** the following ceilings apply
to the entire batch, not each paragraph or each mirror. Existing A2 limits are
observed; charging new descriptors against them is the proposed extension.

| Quantity | Combined batch/service bound |
|---|---|
| Permitted source including context | 65,536 bytes, inherited from D1; not one allowance per paragraph |
| Prepared display input | 16,384 bytes total, including owned separator projection bytes |
| Visible plus overscan rows | 512 descriptors combined, consumer-recommended development candidate; also charged to metadata count/bytes |
| Metadata | 16,385 combined mapping, endpoint and new descriptor records, and 1 MiB total input metadata capacity; counts do not override bytes |
| Runs/glyphs | 16,384 runs and 65,536 glyphs across the batch, subject to the payload byte cap |
| One prepared generation | Full 8 MiB reservation, including input owners, descriptor/row tables, runs/glyphs and control capacities |
| Retained generations | Three distinct batch generations / 24 MiB reserved total, including active, replacement and retired command/frame owners |
| First-party workspace | 16 MiB combined; sequential shaping reuses scratch and charges retained per-paragraph output separately |
| Font ownership | Eight faces, 4 MiB per face, 8 MiB distinct retained encoded bytes; up to two banks only inside that same aggregate limit |
| Gray masks | Dimension <=4096 and <=4,194,304 pixels / 4 MiB each; two owners / 8 MiB combined, never per row |

D1's two producer slots and one published page remain independently charged at
their D1 bounds. The prepared input owner is separately bounded to 16 KiB plus
1 MiB metadata while it exists. Moving it into a reserved generation transfers
the charge; any simultaneous copy must be accounted separately. A D1 page may
be valid at D1's larger display limit while A2 batch admission refuses it.
Successful page publication therefore does not promise layout readiness.

Reserve the replacement generation before dispatch/allocation. Exactly one
queued/running/ready/cancelled-but-unreleased replacement occupies the session
slot. A newer desire keeps metadata only; no extra `PrepareInput` or row-job
backlog is hidden behind coalescing. Cancellation and failed completion do not
release the slot or storage charge. Actual retirement, including the last
retained display command/frame/mirror, releases it. A fourth retained batch
refuses; sharing references to one batch does not duplicate its charge.

All row results adopt as one batch after full validation. Failure in a later
paragraph cannot leave a half-published current window. Previous retained bytes
may survive replacement refusal, but any revoked generation remains unusable
for new painting. Controlled capacities are not RSS: allocator/control-block
overhead, thread stacks and opaque FT/HB/native caches remain separately measured
or unknown. Existing post-allocation native bitmap checks are not pre-allocation
quotas. No new hard bound on native shaping or shutdown latency is claimed.

## Completion, host wake and teardown

**CANDIDATE requirement:** the application supplies a named lifetime-owned host
wake connection before session work is admitted. Its worker-side target only
sets/coalesces a pending signal and requests a host UI turn. It carries no page,
layout payload, raw control/Window borrow or source callback. It must be safe
under the observed session mutex: no `inspect_ready`, adoption, cancellation,
UI invocation, waiting for UI, or synchronous dispatch from that target.

The host must supply reliable UI draining outside the ordinary full callback
queue: a reserved readiness connection/host signal with a pending latch and
an explicit failure/closed outcome. Simply calling ordinary `BeginInvoke` and
ignoring queue rejection is insufficient. The exact platform implementation
remains to be negotiated; a live connection must not silently lose a wake.
Pending readiness is drained on the owning UI executor outside paint, with a
bounded pass and a lost-wake-safe clear/recheck handshake. New work racing the
drain must leave a pending signal or schedule the next turn. No timer/paint
polling or hidden perpetual frame loop substitutes for this completion path.

UI drain inspects completion after the worker lock is released, compares the
entire desired identity and authority, adopts one complete batch or releases a
failed/stale result, then retries only the latest valid desired metadata when
capacity is actually available. It updates state and invalidates source-content
display chunks before notifying mirrors. Notifications occur after coherent
state publication; reentrant mutation receives an explicit refusal rather than
partially changing D1/A2 state. Page producer refusal/exception follows D1's
release-payload/source/scratch, then exact-token `finish` law; success follows
D1's move-publication law. Every accepted producer request has one terminal
ownership disposition. No slot release implies successful source progress.

Detach/reparent revokes that mirror's attachment epoch and future notifications.
The controller and wake connection can outlive one mirror; closing the last
owner explicitly cancels D1 requests, revokes layout authority, calls session
`begin_close`, disables the host connection and drains/rejects queued UI work
before releasing callback targets. The target survives until `begin_close`
returns, as the current API requires. Source producers finish ownership cleanup;
session work is joined before service/native engine destruction. Existing
executor requirements still apply: this proposal does not move `join_and_release`
to an arbitrary thread or claim nonblocking join. Any retirement mechanism
needed to meet UI responsiveness must be separately specified and measured.

## Readiness and presentation

**CANDIDATE:** inspection returns desired identity, actual coverage and an
explicit readiness state plus D1/prepared refusal reasons:

| State | Meaning |
|---|---|
| Current | Complete batch matches current authority and is ready for retained painting |
| Pending | Retrieval, layout or occupied-slot retirement remains outstanding |
| Unavailable | Named refusal prevents the desired coverage from becoming ready |

Previous coverage is an optional, separate metadata field and may coexist with
pending or unavailable desired coverage. It is not a mutually exclusive readiness
state and never authorizes repainting revoked glyphs. Unbound and closing
lifecycle states also remain explicit in the eventual concrete record.

Invalid configuration preserves prior state. A valid source, projection, font,
scale or provider change revokes old authority and invalidates affected display
chunks before reporting replacement pending. Horizontal translation/clipping
does not reshape in the nowrap profile; width-only clipping cannot claim wrapping.
Capture actual attachment/scale outside paint and validate again before replay;
paint consumes only admitted immutable results and performs no source reads,
layout preparation or completion polling.

Ready/current is not native presentation. `recorded` and `staged` remain distinct
from committed/presented. A host may retain its previously committed pixels
under their real frame receipt while replacement is pending or fails. That does
not make revoked source geometry current. Default new recording hides revoked
text and produces a coherent empty/status surface. Per-control physically
presented identity requires an explicit association with actual host receipts;
that association is not established by batch adoption or this draft.

## Required evidence and remaining boundaries

**CANDIDATE conformance gates:** complete multiparagraph and blank-row fixtures
must prove exact source coverage for mixed CR/LF/CRLF, trailing separator and
empty EOF, explicit separator labels versus literal lookalikes, split UTF-8,
partial paragraphs and a paragraph exceeding permission. Boundary failures must
preserve source, selection, incoming ownership and truthful previous coverage.

Identity fixtures vary the full D1 token, controller instance, projection,
context, font, scale, provider and authority epoch. Resource fixtures combine
many short/empty paragraphs at aggregate limits, late-row failure, busy/cancelled
replacement, three retained batches, a refused fourth, mirrors retaining old
commands, and distinct-service accounting. A second unjoined session and foreign
bank remain refusals. Tests must not obtain apparent success by dropping rows.

Host fixtures fill the ordinary dispatch queue, race completion with drain,
detach/reparent/disposal and close, and prove eventual UI drain or explicit
connection failure without paint polling, session reentry under worker lock or
use of expired borrows. Mirror tests demonstrate one shared projection and
separate explicit services for independent viewports. Native receipts must
distinguish readiness from presentation and preserve failed-frame evidence.
Separate installed consumer and native platform receipts are required before
export/availability claims. Prepare, UI drain, page-to-frame and close latency
need separate baseline/workload measurements; none is measured by this record.

Wrapping, tab geometry, arbitrary interior-anchor navigation, very-long-line
continuation, exact whole-document scroll extent, source-certified visual
caret/bidi affinity, pointer selection, editing and accessibility integration
remain open. Source selection/copy stays consumer-owned and unchanged. P1
printing/preview is independent; a screen batch is not a print page or proof of
immutable print snapshot support. These gaps remain product requirements, not
accepted exclusions inferred from this finite first development profile.

**OBSERVED consumer clarification relayed by the coordinator:** plain-source
visual batches are a useful intermediate target. CSV cells, formula displays
and Markdown-derived synthetic/noneditable projections require explicit
source/projection identities and mapping semantics; display offsets cannot be
equated with source offsets. **CANDIDATE disposition:** keep them outside this
first plain-source batch rather than pretending D1 identity spans describe
synthetic content. Independent fields, dialogs and viewports must not revoke
the main projection's authority. The 16 KiB refusal for an overlong paragraph
remains an open implementation requirement, not an owner-selected line or
document size limit, and stale/unavailable geometry must never appear current
and editable.

## Records review and disposition

**OBSERVED records-only review:** this new contract and the appended negotiation
round were reviewed against `planning/PROGRAMMING_HOUSE_STYLE.md` for explicit
units and bounds, named behavior, owner/borrow lifetimes, lock/executor order,
initialization/admission-before-mutation requirements, conversion and failure
states, repeated-work storage, and retirement accounting. No implementation,
test, registry, ADR or availability entry is changed or certified. Implementation
house-style review remains mandatory over each exact authored scope, separately
from tests; observed legacy source is not certified by these excerpts.

Provider agreement is still required for batch/descriptor records, aggregate
allocation checks and reliable host drain ownership. Consumer agreement is still
required for separator projection/coverage, empty rows and the first exact-anchor
profile. The coordinator owns any later implementation assignment and integration
scope. This draft freezes neither public C++ layout nor an installed ABI.
