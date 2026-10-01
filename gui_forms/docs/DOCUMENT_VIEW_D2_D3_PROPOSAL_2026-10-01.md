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

Initial D3a work unit: at most 16 KiB display input including context, at most
16,384 caret boundaries and 65,536 glyphs, at most 512 visual rows. These are
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
