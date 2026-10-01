# Retained document view and bounded cluster layout: D2/D3 proposal

Date: 2026-10-01. **CANDIDATE**, provider-originated after reviewed D1 source
checkpoint `bb61f68`. No implementation or capability promotion occurs here.
Canonical D1 remains
`../../orchestrator/spec/contracts/GUI_DOCUMENT_VIEW_DEVELOPMENT.md`.

## Evidence and problem

**OBSERVED:** D1 gives page ownership, full-token freshness, exact scalar/token
mapping and bounded request slots. It has no glyph geometry, grapheme selection,
paint, input routing or workers. The reviewed maximum-page publication samples
include a 9.5329 ms worst stall; this cannot be ignored when adding layout.

`TextMetricsProvider` currently exposes aggregate metrics/font runs. `Painter`
measures and draws UTF-8 strings but does not consume an immutable prepared text
layout. Private `HarfBuzzFontEngine::shape` exposes glyph clusters and bidi/font
runs internally; it creates a TextStore and whole-input grapheme metadata and
shapes the supplied string. Its `resolve` then retains aggregate/font metadata.
It supplies useful source evidence, not an existing bounded visible-layout API.
TextBox's prefix-measurement and all-row layout remain unsuitable for scaling
by lifting the old limits.

## Proposed sequence for agreement

1. **D3a bounded prepared text layout**: reusable renderer-neutral request/result
   and immutable geometry with the same prepared content used for paint/hit
   testing. Add adapter evidence without introducing a document storage engine.
2. **D2a retained read-only DocumentView**: compose D1 plus D3a, visible rows,
   bounded selection/hit navigation, page-request intents and generic scrollbar
   coordination. It owns no source file or mutation/history.
3. **D3b long-line/context indexing and wrapping continuation**: extend measured
   context support and checkpoints after guards compare alternatives. This is
   required for the full large-file product; D3a refusal is not completion.
4. **D4 editable selection/intents** remains separately reconciled before any
   source mutation. P1 printing and W1 dynamic windows remain independent.

Proposed D3a/D2a is a useful bounded proving slice, not a final refusal of the
owner's long-line or Unicode requirements. End-to-end capabilities stay partial
until the later slices work together against the consumer.

## D3a candidate public contract

Names are provisional. `TextLayoutRequest` borrows a page-local valid display
UTF-8 interval **only for a synchronous call**; includes exact page token,
layout generation, effective font, scale, finite wrap width/tab policy and
boundary-context classification. `TextLayoutResult` reports exact/estimated,
missing coverage, context-required, budget-exceeded, unsupported or invalid;
only complete matching results may replace current geometry.

`PreparedTextLayout` uniquely or immutably owns its display bytes, visual rows,
glyph/caret geometry and font-lifetime dependencies. Public fields contain no
Skia/HarfBuzz/FreeType/native handle. A matching draw operation consumes this
same prepared result, rather than reshaping a prefix or substring with changed
context. Selection rectangles, caret affinity and pointer hits derive from
that result. An estimated headless provider is visibly a test capability, never
substituted for native exact geometry without reporting it.

Candidate implementation routes requiring parent/registry review:

| Route | Benefit | Reversal/cost |
|---|---|---|
| Separate text-layout service and painter prepared-layout operation | Existing aggregate metric API remains intact; explicit new capability | Additional provider attachment, typed layout ownership and invalidation lifetime |
| Add bounded prepared-layout operation to existing typography provider | Uses existing Window provider invalidation | New virtual/return types require matching rebuild; must not silently change historical installed ABI |
| Geometry-only metrics plus draw UTF-8 again | Small initial surface | Duplicate shaping and possible context/font drift; inadequate unless identical geometry/paint is proved and work bounded |

Provider preference **CANDIDATE**: separate prepared-layout capability, attached
through the Window's existing owner/lifecycle rules, with a non-owning service
borrow and an owned result. No public header currently implements this choice.
Whether opaque private glyph payload or portable glyph records are retained
needs reconciliation; no generic type-erasure container is proposed.

### Proposed resource and cancellation profile

Initial D3a work unit: at most 16 KiB display input including context, with
separate endpoint/visual-stop caps refined below, 65,536 glyphs and at most
512 visual rows. These are
**CANDIDATE** comparison values, not accepted product limits. Output byte capacity
and private shaper workspace must be measured and explicitly capped before
publishing capability. A byte cap does not prove a time cap for shaping.

Retain at most one active geometry and one candidate replacement per view,
plus a separately bounded reusable workspace. Page ownership should be shared
immutably only where two live layouts truly need the same page; otherwise move
or copy only the bounded text interval and account for that copy explicitly.
D1's two producer slots plus one page accounting must not hide layout copies.

No shape/retrieval runs from `on_paint`, pointer motion or caret blink. UI work
is scheduled in bounded named slices; each slice validates cancellation and
the page/layout generations before publication. One pending layout intent
coalesces newer font/width/scale changes; no growing queue. Whether a single
shaping slice is UI-thread synchronous or uses an independently owned worker
font engine is unresolved until timings establish a usable stall bound.
The existing mutable engine cannot be assumed thread-safe or borrowed by a
background worker. Host/build ownership agreement precedes such changes.

### Unicode and long-line boundary law

Source grapheme legality is distinct from display geometry. An expanded control
label is one D1 atomic source unit even if it draws several glyphs; its only
admitted caret endpoints are its two source boundaries. Literal label-looking
source remains ordinary text. CRLF is indivisible. A ligature may cover several
graphemes; a shaping cluster is not automatically a legal source caret stop.

Each shaped unit must declare complete context: a complete bounded paragraph,
an independently rendered atomic label, or an established continuation state
from later D3b. Arbitrarily slicing 16 KiB through joining, bidi or grapheme
context is forbidden. Known excessive extent gives budget-exceeded; unknown
context gives context-required, preserving source and previous valid geometry.
An enormous grapheme must never be recoded as illegal bytes. Partial coverage
cannot impersonate an exact completed paragraph.

D3b must compare measured bounded checkpoints, persistent paragraph indexing
and segmented layout strategies for a single very long line and mixed bidi.
Full source-byte storage and encoding policy remain SwiftEdit's. A renderer
cannot independently choose source normalization or flatten control characters.
Space/tab wrapping and grapheme fallback require an explicit break policy;
global visual row count/width stays unknown or estimated until established.

## D2a candidate retained-control behavior

`DocumentView` is a retained control with a D1 state owner and explicit layout
provider. Named operations bind a revision, publish a page, finish/cancel a
request, request a viewport and inspect current presentation status. The view
emits bounded value intents through named targets; it does not retain a
consumer Session/source pointer or construct an editor privately in SwiftEdit.

Before exposing control callbacks, specify order: adopt a complete page/layout
on UI; publish coherent state; invalidate affected layout/paint; emit one
notification. A notification cannot reenter publication/edit mutation; later
intents are queued/coalesced at the owner boundary with bounded retained state.
Disconnect and destruction revoke targets before dropping page/layout owners.

Pointer/key events map using the current page **and layout generation**, with
explicit upstream/downstream soft-wrap/bidi affinity. D1 exact mapping still
rejects scalar/token interiors. Native source grapheme proof belongs to layout
metadata tied to that same revision; never infer legal edits from D1 scalar
mapping alone. D2a read-only selection/copy emits source ranges; consumer reads
exact source bytes under its own bounded copy command. D4 owns mutating intents.

Viewport consists of exact source anchor plus bounded local visual offsets.
Generic H/VScrollBar values represent an explicit projection with
exact/estimated/unknown extent. Integer endpoint conversion must preserve EOF
and positions beyond 2^53; doubles remain local DIPs. Drag/wheel/page motion
emits one coalesced desired source request, preserving selection unless the
interaction specifically changes it. Pending old geometry is either hidden or
displayed with its actual previous coverage; it is never relabelled as current.
Visible rows and overscan are bounded, with no full-file scan on scroll or resize.

## Required replies and gates

SwiftEdit: confirm read-only D2a as a proving stage, provide complete source/
display boundary context at adapter edge, and identify availability behavior
for a region that lacks context. The final product must still fulfill long-line
requirements; this intermediate outcome must be surfaced honestly.

Orchestrator: register layout generation, prepared-result ownership, capability
availability and errors separately from D1; reconcile one concrete minimum
layout/painter seam before headers freeze. Parent: coordinate typography,
Window/Painter, private renderer and build owners before source changes.

Tests: same page with changed font/scale/width; same revision different page;
stale completion/reentrant callback/disposal; label/scalar/grapheme/ligature and
bidi hit boundaries; missing fonts and unsupported painter; glyph/row/output
caps; cancellation and resource failure preserving coherent old state.

Measure cold first layout and warm no-op paint, resize/wrap, page navigation,
single long line, tabs/controls/invalid-byte expansion and combining/ZWJ/bidi.
Report total end-to-end page adoption (D1 validation plus D3 layout plus paint),
p50/p95/p99/worst stalls, bytes fetched, retained/peak capacity and UI frame
misses. Compare existing prefix metric baseline only within its admitted limits;
do not count refusal beyond those limits as a performance win.

All executable implementation/tests/tooling must receive full semantic review
against `../../planning/PROGRAMMING_HOUSE_STYLE.md`, with exact scope and remaining
violations recorded separately from scanners and test results. No source,
installed SDK, native-platform or finished-product claim follows from this draft.

## D3a refinement 002: concrete seam for reconciliation

**CANDIDATE**, following parent review. This section makes the preferred seam
concrete but does not authorize source mutation. Previous route alternatives
remain comparison evidence. The initial executable slice can prove the
renderer-neutral publication contract with an explicitly estimated fixture
provider; native prepared painting requires its own allocator/font evidence.

### Identity and immutable input

```text
TextProviderIdentity { uint64 instance; uint64 generation; }
TextFontSetIdentity { uint64 instance; uint64 generation; }
TextLayoutIdentity {
    DocumentPageRequest page;
    uint64 layout_serial;
    TextProviderIdentity provider;
    TextFontSetIdentity font_set;
    FontSpec effective_font;
    double device_scale;
    double wrap_width_dip;
    TextWrapMode wrap;
    TextTabPolicy tabs;
    uint64 boundary_context_generation;
}
TextBoundaryProof {
    DocumentPageRequest page;
    uint64 generation;
    TextContextKind context;
    owned ordered pairs<absolute SourceByteOffset, page-local DisplayByteOffset>;
}
TextLayoutInput {
    TextLayoutIdentity identity;
    page-local display interval;
    owned valid UTF-8 interval, mapping spans and TextBoundaryProof;
}
TextLayoutRequest { const TextLayoutInput& input; }
```

Each identity component is validated, nonreused within its lifetime, and
compared exactly. No hash alone substitutes for equality. Nonfinite scale,
font/width values and invalid tab settings are refused; zero wrap width is
admitted only under explicitly no-wrap mode. Font size, tracking, fallback
set and effective font role remain part of geometry identity. Page-local ranges
stay distinguished from absolute source ranges.

The adapter input **owns** its bounded bytes and proof once queued. A builder may borrow
a D1 page synchronously to validate and copy that interval, but the task never
retains a D1 `page()` borrow across dispatch. Account for this additional bounded
copy explicitly. No worker calls Window or uses the UI thread's mutable shaper.

The consumer supplies source-grapheme boundaries certified against exact source
bytes/context at the same revision. GUI.Forms validates ordered endpoint spans,
page identity, display mapping and counts; it cannot independently prove source
bytes it does not own. Identity text boundaries may additionally be checked
against grapheme segmentation of complete-context display text. Expanded tokens
allow endpoints only; source-grapheme proof may remove a token endpoint when
adjacent source scalars share a grapheme. Literal label text has no special rule.
An invalid-byte unit has both source endpoints admitted under the consumer's
explicit invalid-byte projection policy. Shaping clusters or glyph counts never
replace source-grapheme proof. Stale/absent proof returns context-required and
supplies no caret/edit authorization.

### Service, ownership and proposed operations

```text
TextLayoutService::capabilities() -> TextLayoutCapabilities
TextLayoutService::open_session(TextLayoutBudget, TextFontSetIdentity)
    -> TextLayoutSessionResult
TextLayoutSession::prepare(const TextLayoutRequest&) -> TextPrepareResult
TextPrepareResult { status; optional<PreparedTextLayout> prepared; }
PreparedTextLayout::identity() const -> const TextLayoutIdentity&
PreparedTextLayout::rows() const -> borrowed immutable rows
PreparedTextLayout::caret_stops() const -> borrowed immutable caret stops
PreparedTextLayout::storage_usage() const -> TextLayoutStorageUsage
Painter::draw_prepared_text(const PreparedTextLayout&, placement, clip, color)
    -> TextPaintStatus
PreparedLayoutState::adopt(PreparedTextLayout&&, expected_identity) -> status
PreparedLayoutState::hit(expected_identity, local_point, affinity) -> result
```

`TextLayoutService` is the separate typed capability. Window observes it only
while the host attachment is alive; replacement increments provider generation,
invalidates geometry and revokes queued publication. A session owns its mutable
workspace and is confined to one nominated executor. `prepare` is synchronous
**on that executor**, not specified as UI-thread work. Session destruction runs
after its job completes. No detached callback owns a Window/model reference.

`PreparedTextLayout` is a unique immutable value with private typed storage,
movable and noncopyable. It retains explicit immutable font dependencies through
a font-set lease so the result cannot reference destroyed face data. Its public
surface exposes no native handles or untyped payload. A renderer validates the
provider/font-set lease identity and device scale before drawing; mismatches
return unsupported/stale without drawing a partially compatible subset. Lease
revocation prevents new paint; retained bytes remain alive until the layout is
released. A lease therefore distinguishes memory lifetime from authority.

One concrete native storage candidate is CPU glyph/font-run data privately owned
by this result, paired with portable caret/row geometry. The painter draws those
positions directly without reshaping. Font-face lookup uses the retained lease,
not whichever unrelated current host font happens to share an integer ID.
Public glyph-array layout and backend type erasure are not selected here.

`prepare` borrows immutable input only for this synchronous executor call. Its
caller owns that input throughout; no pointer, span or reference is retained
after return. It first validates identity, input and budget, then constructs an
independently owned result using its admitted output capacity. Refusal preserves
input and yields no prepared result. Copying the bounded text needed by the
result is explicit and budgeted while the caller's input still exists; avoid
pretending this simultaneous ownership is a move. A future move-admission
optimization requires separate semantics and is not part of this minimum seam.

`adopt` validates the entire expected identity and complete geometry before any
swap. Failed adoption preserves both last valid geometry and incoming owner;
caller releases rejected payload and acknowledges job completion. Successful
adoption moves the candidate, releases the previous owner and invalidates affected
paint. It emits no callback until coherent state is installed. Hit testing
requires the current full layout identity, not merely D1 page identity.

On font/scale/provider replacement, old geometry may remain retained for orderly
release but is **not current paint or hit state**. Failure to produce replacement
cannot relabel old glyph positions as the new font/scale. Drawing a stale previous
page is allowed only as explicitly previous coverage under its still-valid exact
identity; consumer chooses whether to display it or an unavailable state.

### Resource guarantee and current native blocker

The refined candidate request ceilings are 16 KiB display/context, 16,385 caret
stops, 65,536 glyphs and 512 rows; N single-byte graphemes require N+1 stops.
Numbers remain subject to agreement before source. Proposed comparison profile:

| Category | Candidate capacity ceiling | Counted lifetime |
|---|---|---|
| Input display bytes | 16 KiB | Adapter input through prepare return |
| Input mapping/proof | 16,384 spans / 16,385 endpoints, at most 1 MiB / 128 KiB | Adapter input |
| Result copied text and mapping/proof | Same limits as input | Each active/replacement result |
| Glyphs | 65,536 records and at most 4 MiB | Each result |
| Caret geometry | 32,770 visual-affinity records and at most 1 MiB | Each result |
| Rows | 512 records and at most 64 KiB | Each result |
| Mutable provider workspace/cache | At most 16 MiB combined | One session, not multiplied invisibly by calls |
| Immutable font-set dependency storage | At most 128 MiB | Shared explicit font-set owner; overlapping old/new sets both charged |

These are requested allocation-capacity comparison values, not measured fit or
selected production constants. Both record count and byte capacity must pass;
actual record sizes and allocator behavior are part of the implementation
receipt. Session/lease metadata, string terminators and allocation overhead need
an explicit owner accounting category. Native hard-quota availability remains
unavailable while hidden allocations lack established bounds. A separately
labelled measured-capacity native experiment may proceed while reporting those
internals unknown; this does not claim the stronger guarantee.
Source context stays within D1's 64 KiB; a 9x display expansion can force a much
smaller paragraph. No source admission threshold is inferred from display caps.

`TextLayoutBudget` must declare independent capacity bytes for copied input,
mapping/proof arrays, glyph storage, row/caret storage, scratch, retained font
dependencies and provider cache. Every multiplication/addition is checked before
reserve/allocation; allocator rounding/headers and terminators are separately
accounted where they are not part of requested capacity. Provider capabilities
state whether these are hard enforced allocations or only measured capacities.
An adapter lacking a required hard guarantee returns unsupported before starting
work; it cannot advertise hard bounded native preparation based on byte count.

Provider-owned arrays use supplied/session-owned reusable storage sized within
the admitted budgets before processing. Shared immutable font data is charged
once to its explicit font-set owner budget; per-session face/size/cache objects
and every retained lease's contribution remain visible. Swapping a font set may
temporarily retain old and new font owners; both count until old layouts release.
No request can pull an arbitrary host font or unbounded fallback catalogue.

**OBSERVED unresolved native edge:** the current private HarfBuzz/FreeType path
does not demonstrate a pre-allocation quota over all internal shape/cache
allocations. Calling `shape` with a short input and checking output afterward
does not prove hard quotas. Inspect the pinned libraries' allocation controls,
face/cache ownership and worst admitted font behavior. Compare a measured-capacity
native experimental profile reporting opaque internals with a future proven
hard-quota profile; fixture-provider geometry may prove the protocol independently.
A generic allocator framework, process sandbox or new font/storage architecture
is not authorized by this proposal. None of these comparisons by itself grants
public D3a source go-ahead or native shape/paint parity.

Per view, one active result and one replacement job/result may coexist; cancelled
jobs retain that slot and all reservations until completion/release. One desired
metadata value coalesces changes. D1 payload slots are counted independently.
Missing job capacity gives busy, not an unbounded task queue. Cancellation is
checked between bounded work phases; a noninterruptible library call's worst
stall must be measured. No assumption puts that call on UI by default.

### Outcomes and proving matrix

Expected outcomes: success, invalid-input, stale-page,
stale-layout, revoked-provider, stale-font-set, context-required, budget-exceeded,
busy, cancelled, unsupported, missing-font-coverage and allocation-failure.
Resource exceptions are contained at the session boundary; failed output is
never published. Native exact availability requires shape/paint parity evidence.

Before the first source go-ahead, reconcile: (1) exact boundary-proof encoding;
(2) prepared value's private typed storage and font lease; (3) numerical capacity
profile including observable workspace/font categories and unknown internals;
(4) measured-capacity native, hard-quota native or fixture profile appropriate
to the admitted guarantee; (5) executor/lifetime ownership.
Required fixtures vary each identity field separately, fail each admission and
adoption step, revoke font/provider leases with a pending result, exercise N+1
caret limits, and prove old geometry preserved but never painted under a changed
identity. Native workloads measure preparation on its selected executor and UI
publication/paint independently, plus complete page-to-frame latency.

### Registry refinement 003: endpoint proof and capability distinction

**CANDIDATE clarification accepted by provider:** source-grapheme proof contains
explicit paired absolute uint64 source-byte endpoints and page-local uint32
display-byte endpoints. Both orders are strictly increasing; duplicates in
either coordinate are rejected. Every pair must round-trip under the exact D1
page token. No endpoint is synthesized merely because it is a page or mapping
edge. For an empty document or admitted empty EOF page, exactly one certified
pair maps source EOF to display zero. Nonempty text with no certified endpoints
is not hit-testable and reports context-required. Partial endpoint proof does
not authorize unseen endpoints; D3a complete-paragraph profile requires both
actual paragraph edges to be certified, or refuses that profile.

One certified endpoint may have multiple visual caret stops for bidi or soft
wrap affinity. Endpoint and visual-stop capacities are independent: the
comparison profile proposes 16,385 endpoint pairs and 32,770 visual stops,
still subject to the declared caret byte ceiling. There is no assumption that
one glyph, one shaping cluster and one source grapheme share an index. Unknown
or stale proof grants no current hit authorization, including when an old
layout remains retained for release or previous-coverage display.

Terminal status `success` means a complete result within its **reported**
capability class. Estimated fixture geometry is a separate capability metadata
value, not a success alternative that a caller could mistake for native exact
support. `estimated-fixture-only` in the earlier outcome list is superseded by
this distinction. Native controls must require native-exact metadata before
claiming native preparation/paint support. Numerical profiles, native quotas,
executor and exact private storage remain unresolved; only partial semantic
reconciliation is sought at this point, not source go-ahead.

### Parent clarification 004: native experimental capacity is a distinct candidate

The earlier hard-quota prerequisite language is **superseded** where it implies
that every native experiment must enforce a cap over every third-party
allocation. The parent requires preallocation admission for controlled arrays
and known output/workspace needs; it has not selected a new allocator
architecture. Keep three comparison profiles explicit: estimated fixture,
native measured-capacity experiment with opaque internals reported unknown,
and native hard-quota capability only after that stronger guarantee is proved.
Fixture-only versus hard quota is not an exhaustive choice.

Read-only pinned-library allocation/face/cache audit and a bounded diagnostic
shape benchmark of existing native code are authorized independently of any
public D3a implementation. They may measure noninterruptible call latency and
observable returned capacities; they must not claim whole-process allocation
coverage or hard quotas. No generic allocator, sandbox or production host API
change follows from this diagnostic authorization.

Numerical note: 16,385 absolute-source/display endpoint pairs can occupy
262,160 bytes at a 16-byte record layout, exceeding the earlier 128 KiB proof
comparison ceiling. Count and byte caps both apply; that table does not imply
every maximum fits simultaneously. Final record layout/profile remains open.
