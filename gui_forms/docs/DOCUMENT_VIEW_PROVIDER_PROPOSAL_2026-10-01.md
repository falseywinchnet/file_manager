# SwiftEdit document-view provider proposal

Date: 2026-10-01. Status: **CANDIDATE**, awaiting consumer and Orchestrator
reconciliation. This record proposes additive development C++ semantics under
`ORC-GUI-001`; it does not freeze a C ABI or change an installed SDK.

## Direction, evidence, and ownership

**GIVEN:** the parent verified the owner's SwiftEdit direction to implement the
expanded feature set and finish with a lag scan (consumer chat turn
`01a0f659-c634-7ed1-8495-4c1b6050d803`). The provider assignment opens staged
negotiation and implementation after agreement on each bounded stage.

**OBSERVED:** `controls/panel/text_box/text_box.hpp` admits 1 MiB and 4096-byte
logical lines in multiline mode, including read-only mode. Its retained layout
contains every visual row; its undo entries contain whole strings. The
`TextMetricsProvider` typography seam returns aggregate widths and font runs,
not cluster caret geometry. Generic H/VScrollBar controls exist but TextBox has
no public viewport setter. `text_shaping` has lower-level glyph cluster data;
that is not yet a control/renderer-consistent caret-layout contract.

**GIVEN:** the consumer requires a threshold between editable and paged
read-only documents, byte-faithful invalid input,
visible inert controls, source-preserving wrapping, draggable scrollbars,
discontiguous selections, and native printing. Thresholds and file/save policy
remain consumer-owned. **OBSERVED:** SwiftEdit interprets the owner's 16 MB as
16 MiB; that precise unit is not independently approved architecture.
GUI.Forms owns reusable presentation, input, layout and
host mechanics. Orchestrator owns the canonical contract registry. No bytes,
GUI handles or hot layout calls pass through the Orchestrator process.

## Candidate stages and additive names

Names here are negotiable development C++ names, not promises of availability.
Existing TextBox behavior and symbols remain intact.

| Stage | Proposed surface | Completion evidence |
|---|---|---|
| D1 | `DocumentRevision`, `SourceByteOffset`, `DisplayByteOffset`, `DocumentPageRequest`, `DocumentPage`, `DocumentViewState` | Renderer-neutral bounded request/publication, mapping and viewport protocol with stale/revoked/malformed fixtures; selections are D4 |
| D2 | `DocumentView` retained control; explicit `request_page`, `publish_page`, `set_viewport`, `viewport_changed`, `edit_requested` | Visible page painting/input, generic scrollbar composition, consumer byte/edit adapter, headless and native interaction |
| D3 | `TextClusterLayout` provider capability and reusable bounded layout workspace | Renderer/caret agreement, long-line continuation, wrap, Unicode boundary guards, measured scaling |
| D4 | Selection set editing and accessible text-range projection | Atomic equal-length parallel edits, unequal-length copy-only consumer policy, ordered events and native evidence |
| P1 | Separate print capability request/session/page-source contract | Platform capability discovery, cancellation, bounded page production and spool completion; independently negotiated |

D1 is the requested first implementation slice. It is deliberately testable
without new host hooks, renderer changes, application package claims, or a
storage-engine decision. D2/D3 may need joint delivery before a useful GUI is
advertised. A model-only D1 is not large-document editing availability.

## D1 concrete proposed semantics

### Identity, ownership, and execution

* `DocumentRevision` consists of nonzero document identity and nonzero monotonic
  revision, both uint64. Reopening/replacing a document changes identity;
  successful mutation changes revision. Exhaustion requires new identity; no
  wrapping or recycling while requests may survive.
* Source offsets are uint64 byte boundaries. Display offsets are page-local
  uint32 UTF-8 byte boundaries. They are never implicitly convertible.
  Grapheme positions belong to a separately computed layout, not either byte
  type. All ranges are half-open; anchor/caret orientation is preserved.
* The consumer owns exact bytes, encoding interpretation, storage/indexing,
  history, successful-save boundary and edit transactions. GUI.Forms owns its
  copied/moved display page, mapping, selection and viewport state. It never
  treats displayed replacement glyphs as source bytes.
* Model/control operations occur on one owning UI thread; D1 provides no hidden
  synchronization. Workers receive value request tokens and produce owned page
  results, posted to the application's established UI dispatch. No background
  worker may call a control or retain a borrowed UI string.
* A request token includes identity/revision, monotonically increasing request
  serial, and requested source interval. Only the newest outstanding request
  may publish. A newer request, detach/cancel, or revision replacement revokes
  earlier tokens. Cancellation is cooperative in consumer I/O; publication
  rejects a late result even if I/O could not be interrupted.
* Expected statuses are explicit: success, stale, cancelled/no-request,
  invalid-range, invalid-page, budget-exceeded and unavailable. Failed page
  publication leaves the previous valid page and selection unchanged; it must
  not disguise an old revision as current. Binding a new revision clears the
  old page until a matching page arrives. Allocation exceptions stay at the
  owning C++ operation boundary with old state intact.

### Bounded pages and source/display mapping

* Proposed D1 hard transport budgets: 64 KiB source interval, 1 MiB display
  UTF-8 and at most 65,536 mapping entries. Later D4 proposes 1,024 selections. These are
  request/result budgets, **not document-size or line-length limits**. Their
  values remain negotiable until the stage is reconciled.
* The consumer supplies a page with exact revision/request identity, source
  interval, owned valid display UTF-8 and ordered mapping spans. Each span has
  a source range, display range, and kind: `identity_utf8` or `atomic_token`.
  Spans partition both page ranges without holes or overlaps. An identity span
  is byte-for-byte mapped valid UTF-8. An atomic token represents an indivisible
  source unit such as one illegal byte, a visible control or line-ending token.
  GUI.Forms can validate geometry/UTF-8 but cannot certify equality with source
  bytes it does not own; that equality is a consumer obligation.
* Identity mapping accepts only scalar boundaries. D1 strictly rejects
  atomic-token interiors; later hit testing may choose a token endpoint with
  explicit upstream/downstream affinity before requesting a source mapping.
  No caret or edit occurs inside the represented source unit. Grapheme
  snapping is a later layout responsibility and is never claimed by D1 scalar
  mapping. Source outside the page yields unavailable, not a guessed position.
* Zero-length annotations (line metadata/rulers) remain out of the editable
  mapping and source payload. D1 has no synthetic editable insertion spans.
* Page requests may be adjusted by the consumer to valid decoding/grapheme
  context boundaries within the budgets. The response declares its actual
  covered interval and must cover the requested anchor. A source unit or
  shaping context larger than budget reports an explicit continuation/unsupported
  case; arbitrary slicing must not impersonate correct Unicode shaping.
* EOF, empty documents, CRLF, NUL, malformed bytes, and partial multibyte input
  have fixtures. No sentinel offset can collide with a real uint64 offset.

### Viewport and scrollbars

* D1 viewport is an exact source anchor plus finite nonnegative local horizontal
  DIP offset; it does not convert a huge file byte offset to a double. A page
  request uses that anchor and bounded source context. D2 adds local visual-row
  affinity/offset once layout semantics are settled.
* Page motion, wheel, scrollbar drag and caret reveal use named viewport
  requests. An explicit scroll request does not move selection; edits may
  request a separate reveal. A page arriving does not reset the user's later
  viewport request.
* Scrollbar geometry reports extent as exact/estimated/unknown. Until a line
  index/layout exists, source-byte proportion may be exposed as an explicitly
  approximate navigation coordinate, never an exact row count or pixel extent.
  Integer scaling and endpoint fixtures must cover offsets above 2^53.
* No full-file layout or index scan occurs synchronously on drag/paint/input.
  Consumer asynchronous indexing can improve extents; stale results use the
  same revision/token refusal. A continuous long line requires horizontal
  continuation and cached shaping checkpoints, not all-prefix measurements.

### Selection and editing boundary

* Later D4 selection sets contain ordered disjoint source ranges with anchor/caret
  direction and a primary index. Reject reversed storage order, overlap,
  duplicate empty carets, out-of-document positions and over-budget sets
  atomically; do not silently merge user selections. Empty set has no primary.
  Adjacent nonempty ranges are allowed; a zero-length caret at either endpoint
  of a nonempty range is rejected (consumer's clarified rule).
* D4 may store off-page source selections but cannot certify off-page grapheme
  boundaries. Consumer revision validation and later D2/D3 boundary resolution
  are required before editing. Copy reads exact source ranges under a bounded,
  cancellable consumer command, not display glyphs.
* D2 edit intents carry revision and ordered replacement ranges. Consumer
  validates permissions, boundary legality, resulting size and all changes
  before committing one revision and one history transaction. Stale, read-only
  or any failed subedit leaves all source bytes unchanged. Reentrant callbacks
  are not allowed to publish a second mutation into the first transaction.
* SwiftEdit's equal-length parallel edit/unequal-length copy-only rule is
  consumer policy; the consumer confirmed source-grapheme counts as the equality
  unit. Toolkit selection capability alone does not
  impose a product edit policy.

## Long-line layout and unresolved high-reversal choices

**CANDIDATE:** bounded shaping windows plus reusable cluster/caret geometry and
revision-keyed checkpoints, retaining only visible rows plus bounded overscan.
The layout key includes source/display mapping version, font/fallback identity,
device scale, wrap width, tab policy and revision. Painting and hit testing must
consume the same geometry. Ligatures, joining and bidi cannot be approximated
as arbitrary independent chunks without reporting degraded capability.

Compare contiguous text, chunked/piece storage and consumer-provided immutable
pages using the same workloads before choosing persistent storage or undo
architecture. D1 does not select one. Consumer whole-string snapshots are a
separate cost to measure; GUI.Forms must not duplicate that history. Full IME,
bidi, Unicode line-breaking and native accessibility parity remain separate
capabilities with truthful coverage. No native OS editor substitution is
implied.

## Printing is an independent negotiation

**CANDIDATE P1:** query host print support; begin an owned print session with a
frozen revision/layout specification; request bounded immutable pages; cancel
or finish exactly once. Distinguish user cancellation, unsupported host,
provider failure and accepted spool submission (not physical print success).
Source/save policy, plain versus rendered content and Markdown pagination
remain consumer decisions. Platform handles stay inside host adapters.
Preview must use the same page layout and cannot mutate source. No print method
is added to existing HostServices merely to return empty success. Parent/host
owner agreement is required before host/build edits. P1 cannot block D1.

## Evidence gates and reversal

D1 fixtures: empty/EOF, >2^53 and uint64 edge offsets, budget checks before
allocation/copy, malformed mapping and display UTF-8, token endpoints/affinity,
stale/reordered completion, cancellation, revision reset, overlapping/over-budget
selections, failure preservation and repeated page reuse. Benchmark request,
publication and lookup against a simple reference mapping at small/maximum
budgets; record p50/p95/p99/worst and allocations or retained capacities.

D2/D3 workloads: 16 MiB minus one / exactly 16 MiB / plus one, 1 GiB sparse
read-only source, a single long line, tiny/empty source, mixed endings, malformed
bytes, combining/ZWJ, tabs and controls. Measure first page, page/drag/caret/edit,
wrap toggle, cancellation and close under pending work; separate cold/warm
layout, bytes fetched, retained/peak memory and UI stalls. Compare against the
old TextBox only inside its admitted limits; refusal outside those limits is
the baseline, not a zero-time performance victory.

All authored implementation/tests/tooling require semantic source review against
the complete `../../planning/PROGRAMMING_HOUSE_STYLE.md`: explicit types,
named executable behavior, retained state/lifetimes, operation order,
initialization/conversions, failure states and repeated-loop storage/work.
Spelling scans and functional tests do not certify that review. Record exact
scope and unresolved violations with the evidence.

Reversal: experimental names can change together with matched consumers before
a new package is advertised. Keep TextBox/frozen SDKs and their consumers
unchanged. A later package uses a new coherent GUI/picker prefix only after
matching installed-consumer validation. Native acceptance and other-platform
availability are separate from portable source and headless tests.

## D1 reconciliation refinement and exact API sketch

**CANDIDATE 2026-10-01, following consumer reply:** first implementation excludes
selection storage and editing; those belong to D4 after D2/D3 geometry. The
consumer confirmed equality means grapheme count. Mapping D1 resolves scalars
and atomic-token boundaries only; it is not sufficient by itself to authorize
an edit. Page context must already have verified complete boundary state from
consumer indexing. An unresolved or over-budget grapheme is `context_required`,
not illegal input. Each attempt is bounded; no unlimited backward scan or
retry loop may run on UI/input/paint. Consumers may index asynchronously and
retry after new context becomes available. Until then that region remains
explicitly unavailable and source bytes are preserved.

The proposed public header is `gui_forms/document_view.hpp`, with types in
`document_view/types/` and implementation of `DocumentViewState` in its own
core source boundary. C++20; enum/record names below are developmental:

```text
DocumentRevision { uint64 document; uint64 revision; }
SourceByteOffset { uint64 value; }
DisplayByteOffset { uint32 value; }
SourceByteRange { SourceByteOffset begin; SourceByteOffset end; }
DocumentViewport { SourceByteOffset anchor; double horizontal_dip; }
DocumentPageRequest { DocumentRevision revision; uint64 serial;
                      SourceByteRange permitted; DocumentViewport viewport; }
DocumentMapSpan { SourceByteRange source; DisplayByteOffset begin;
                  DisplayByteOffset end; DocumentMapKind kind; }
DocumentPage { DocumentPageRequest request; SourceByteRange covered;
               string display_utf8; vector<DocumentMapSpan> mapping; }
DocumentViewStatus { success, invalid_revision, invalid_range, invalid_page,
                     budget_exceeded, stale, cancelled, busy, unavailable,
                     context_required, serial_exhausted, invalid_boundary }
DocumentRequestResult { status; optional<DocumentPageRequest> request; }
SourceMappingResult { status; optional<SourceByteOffset> position; }
DisplayMappingResult { status; optional<DisplayByteOffset> position; }

DocumentViewState::bind(DocumentRevision, SourceByteOffset document_size) -> status
DocumentViewState::request_page(DocumentViewport, SourceByteRange permitted) -> result
DocumentViewState::cancel() -> void
DocumentViewState::finish(const DocumentPageRequest&) -> status
DocumentViewState::publish(DocumentPage&&) -> status
DocumentViewState::page() const -> const optional<DocumentPage>&
DocumentViewState::viewport() const -> DocumentViewport
DocumentViewState::pending_count() const -> size_t
DocumentViewState::source_position(const DocumentPageRequest&, DisplayByteOffset) const -> result
DocumentViewState::display_position(const DocumentPageRequest&, SourceByteOffset) const -> result
```

`bind` validates identity/revision; within one identity a changed revision must
increase. An identical binding with identical size is an idempotent success;
same identity/revision with changed size is invalid. A successful changed bind
clears page/viewport and cancels publication authority, but leaves outstanding
producer slots occupied until acknowledged. Serial never resets across binds.

`request_page` validates anchor within document and permitted range (including
end boundary for EOF), finite/nonnegative horizontal offset, ordered permitted
range within document, and at most 64 KiB permitted source. Validation precedes
slot lookup, so invalid requests preserve valid publication authority even when
both slots are occupied. Success installs
the new viewport and reserves one slot. Its serial supersedes older publication
authority. There are exactly two slots covering all queued, in-flight and ready
results collectively; no third payload job may start. `busy` changes nothing.
When a valid newer desired viewport receives `busy`, the caller calls
`cancel()` to revoke older publication authority, then retains at
most one desired viewport value, in one non-reentrant UI sequence, for retry after a
slot drains, not a backlog of page jobs. A permitted range includes all source
context; no implicit bytes beyond that range are authorized.

`cancel` revokes the current publication authority and preserves the current
page. It does not release producer slots prematurely. A producer posts an owned
result or terminal failure to UI, then the UI calls `publish` or `finish`.
`finish` consumes the exact matching slot and releases it for reuse; repeated
or forged acknowledgements fail. `publish` checks slot identity, newest serial
and revision, validates the complete payload, then moves it into the model and
consumes the slot on success. Failed publication leaves its matching slot
occupied and the incoming page owned by the caller. The caller must release
that payload and all per-slot source/scratch ownership before `finish` releases
the slot. This prevents a rejected-but-still-owned payload plus a replacement
job from violating the two-slot bound. Unknown or forged tokens consume no
other slot. Successful publication leaves input empty; per-slot source/scratch
must already be released before calling `publish` on a ready result.
Terminal producer errors (including `context_required`) use `finish`, plus an
application status display. D1 does not spawn, queue or join workers.

`finish` is the caller's explicit acknowledgement that worker completion and
all per-slot ownership releases have occurred. It may not be called merely
because cancellation was requested. A model cannot inspect consumer allocations;
the adapter and its tests must establish this ownership condition. On model
destruction the owner cancels/drains dispatch and joins its workers first; the
producer retains no model pointer, reference or callback after shutdown.

Covered range must lie inside permitted range and contain the anchor; nonempty
coverage requires nonempty display and mapping. Empty coverage is allowed only
at an empty document or EOF with no source/display payload. Every mapping span
is nonempty in both units, ordered and contiguous; identity spans have equal
byte lengths. UTF-8 and all display endpoints are validated before mutation.
Mapping lookup uses binary search over validated spans; offsets at a shared
endpoint have the same source/display result from either span.

Budget accounting includes capacity, not only live length. A payload may own
at most 1 MiB display capacity and 65,536 mapping slots; vector byte cost is
bounded by that count times `sizeof(DocumentMapSpan)` in the build receipt.
At most two producer payloads plus one published page may coexist. A producer
may own at most one 64 KiB source page per slot; any indexing/shaping workspace
has a separately reported consumer/D3 budget and cannot masquerade as page
storage. The model performs no full payload copy or allocation during
publication: validates then swaps ownership, releases the former page and
empties the incoming owner. Consumers check expansion/count before allocating.
Native rendering/layout memory is outside D1 and unavailable until later
measured stages. Borrow from `page()` only until the next successful publication,
changed bind, destruction or explicit ownership mutation; never across dispatch.

Source/display mapping requires exact equality with the current published
request token (all fields, including serial, document revision, permitted range
and viewport). Old page-A mapping calls after page-B publication at the same
document revision return stale. Results are synchronous values; callers retain
the supplied page token alongside any deferred result. Off-page mapping is
unavailable. A failed request
does not revoke an older page; a successful viewport request may leave the old
page observable until the new page arrives, but callers must use its actual
request/covered metadata and cannot treat it as the new viewport's page.

Allocation failure during producer acquisition is terminal for that attempt:
release partial buffers, acknowledge the exact token with `finish`, and report
failure through the application boundary. Context/budget failure publishes no
page, advances no anchor and never fabricates progress. It is retried only after
changed context/profile or user navigation. Every acknowledgement validates
the full original token before releasing a slot. Tokens are scoped to one
DocumentViewState instance; dispatch may not route tokens to another instance.
Destruction drains all producers/results first, so a new model's serial cannot
revive an old model's request.
