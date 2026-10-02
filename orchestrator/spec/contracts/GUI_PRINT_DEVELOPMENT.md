# ORC-GUI-001 P1 — owned print and layout-preview development draft

Date: 2026-10-01. Status: **CANDIDATE semantic draft for GUI.Forms/SwiftEdit
reconciliation**. P1 here means print, not the unrelated plugin-placeholder stage
in the Orchestrator delivery sequence. No implementation, architecture acceptance,
installed capability or platform readiness is claimed.

**OBSERVED authority:** the coordinator assigned this draft under the existing
SwiftEdit owner feature direction recorded in
[`SWIFTEDIT_DOCUMENT_VIEW_2026-10-01.md`](../../negotiations/SWIFTEDIT_DOCUMENT_VIEW_2026-10-01.md).
That direction includes native print of plain/rendered content and a separate
Markdown layout preview without source mutation. It does not select a print
backend or make P1 depend on completion of D1–D4. This is a direct in-process
development boundary; Orchestrator does not carry document or page bytes.

## Responsibility and first profile

SwiftEdit owns exact source bytes, decoding/error-label policy, selection and
range ordering, plain versus rendered mode, Markdown/CSV interpretation, document
title and any headers/footers. It supplies a frozen, owned print-content source.
The provider owns reusable page layout, shaping, page leases, retained preview
mechanics, owned native dialogs and platform spool submission. Native handles
remain private. Neither provider pagination nor preview changes source, history,
selection, save state or file contents.

**CANDIDATE default:** whole-document plain source, black text on white, explicit
registered monospace face at 12 points, wrapping at the content width, no automatic
headers/footers or page-number decoration, one copy. The consumer supplies its
versioned tab/control/invalid-byte projection; the provider does not reinterpret
literal text as markup or escape commands. Selection printing is an explicit
consumer-created frozen projection, never inferred from a live control. The
consumer specifies separators for discontiguous ranges and owns their source map.

Rendered mode is explicit: the consumer supplies bounded paragraphs and typed
style runs produced by its parser. Initial content admits text runs, paragraph
spacing, explicit line/page breaks and registered face/style choices. Unsupported
blocks such as an unimplemented table or image return `unsupported_content`;
there is no silent plain-text substitution. This proposes a useful bounded first
profile, not full Markdown/CSV rendering acceptance. No browser, remote resource
fetch, shell printing command or application-owned PDF export is introduced.

## Frozen identities and units

Each session carries a nonreused session ID, owner-window identity/lifetime,
document identity, frozen document revision, content-projection revision and
mode. The source lease guarantees repeatable content through disposal. A live
path, mtime check or retained read handle alone does not establish immutability.
If SwiftEdit cannot provide that guarantee for a paged document, begin refuses
`snapshot_unavailable`; this contract does not select its storage/copy strategy.

The layout key includes a checked monotonic layout revision, source/projection
identity, exact font bank revision, typography/shaper profile, page geometry,
orientation, margins, line/paragraph rules and resolved native ticket geometry.
A page key adds zero-based page index and production serial. Source-byte offsets,
projection-byte offsets, page indices and geometric coordinates remain distinct.
All source extents and identifiers retain integer precision beyond 2^53.

Geometry uses finite double **points, 1/72 inch**, top-left origin, positive x
right and y down. Preview zoom changes raster scale, not pagination. Conversion
to a raster axis is points times declared DPI divided by 72, with checked extent
rounding outward; signed glyph placement uses the selected raster profile's
declared rule. Device DPI never silently chooses line breaks. Negative margins,
nonpositive content boxes, nonfinite values and overflow refuse.

Default paper is the explicitly queried native default ticket, with portrait
orientation and 36-point user margins intersected with the reported imageable
area. Nothing assumes Letter/A4 from locale. Without a printer, preview-only
begin requires the caller to supply paper/imageable geometry. If the native
dialog changes effective geometry, create a new layout revision and repaginate;
old pages may remain visibly labelled old/pending, never presented as the new
revision. Print submission must reference the final ticket and matching layout.

Preview and print consume the same immutable logical page plan, fonts, breaks
and geometry for that key. Native raster output can differ by device; pixel
identity is not promised. A backend that cannot preserve the page plan refuses
`unsupported_layout` rather than silently reflowing through another text engine.

## Owned operations and state

These are semantic operation names, not C++ declarations or a frozen ABI:

| Operation | Candidate behavior |
|---|---|
| `query_print_support` | Return provider/profile version, limits and separate preview/dialog/spool/cancel states for the current platform |
| `begin_preview` | Validate owner, frozen source and layout; acquire the sole active session of this print service; reject busy without consuming caller owners |
| `next_content_request` / `supply_content` | Poll a bounded stamped source-block request and return owned projection data or typed source failure; no worker calls live editor code |
| `request_page` / `take_page` | Request a page at the current layout key; adopt an immutable owning page lease only on matching completion |
| `set_layout` | Validate a complete replacement ticket/spec before revoking old layout publication; increment revision, cancel/drain old work and repaginate |
| `show_print_dialog` | Present one native dialog owned by the live document window; return cancellation, chosen ticket or native error distinctly |
| `submit_print` | Require final pagination, explicit user Print intent, exact ticket/layout key and selected page range; create one submission attempt |
| `cancel_work` | Revoke pending layout/page publication; report requested versus resources-retired separately |
| `begin_close` / `poll_closed` | Stop admission, revoke delivery, dismiss owned presentation, retire work and report when native callbacks/worker borrows are quiescent |

Control calls and page adoption belong to the opening UI executor. Slow source
production belongs to a consumer-owned job using the frozen source; provider
layout/rendering belongs to its named worker. Ready notifications are coalesced,
payload-free wakes to a target retained until close completes. Native dialog and
spool calls obey their backend executor requirements; they never invoke arbitrary
consumer work from a native callback. Polling exposes bounded state, not a growing
event log. Wrong-executor and invalid requests preserve current output.

Session phases are `preparing`, `preview_ready`, `dialog_open`, `submitting`,
`closing`, `closed`, with explicit pending/error details. Pagination separately
reports discovered pages and `total_unknown` until end-of-source proves the final
count. Empty input produces one blank logical page. Preview supports previous,
next, validated page-number navigation and fit-page zoom; a distant undiscovered
page remains pending while bounded sequential pagination advances. No fabricated
final count or automatic last-page jump is returned. No persistent side panel is
added to SwiftEdit: preview and secondary controls use an owned popup/dialog.

Source edits do not mutate an existing frozen session. SwiftEdit labels an older
snapshot as such and explicitly starts a replacement to preview current edits;
the provider never silently switches revision. A source-lease failure invalidates
the affected job. Reopen creates a new session identity. Stale results from a
cancelled/closed session cannot revive the new presentation, even for equal text.

## Proposed bounded production profile

All numbers here are **CANDIDATE initial limits**, not measurements, product file
size promises or permission to enlarge A2/Games budgets. The provider must accept
or counter with exact accounting before implementation assignment.

| Resource | Initial proposed bound |
|---|---|
| Active work | One session and one executing page/layout job per print service; one latest desired preview-page intent |
| Content supply | One outstanding source request; at most 64 KiB source bytes including requested context, 256 KiB projected UTF-8 and 1 MiB metadata per owned reply |
| Paragraph context | At most 64 KiB projected UTF-8 per complete contextual paragraph; oversize context refuses rather than breaking shaping arbitrarily |
| Document preflight | At most 64 MiB cumulative admitted projected UTF-8 and 10,000 logical pages; these are print-profile limits, unrelated to the editor's 16 MiB admission policy |
| Page index/checkpoints | 8 MiB actual retained allocation capacity; no unbounded per-page map |
| Logical page plans | 8 MiB/page, 65,536 glyphs and 4,096 runs/page; at most three live distinct plans, 24 MiB aggregate including candidate and externally held leases |
| Preview rasters | At most 4,096 pixels/axis and 64 MiB stride-times-height per BGRA page; two distinct live allocations/128 MiB including previous and candidate |
| Worker storage | 32 MiB controlled first-party workspace including simultaneous buffers and reallocation peaks |
| Fonts | Eight faces/bank, 4 MiB/face, 8 MiB aggregate across live/retired banks; source ownership outlives all native uses |
| Work | 2,048 shaping calls and 4 MiB submitted context/page; check cancellation between source blocks, paragraph/line trials and pages |
| Submission | One attempt/session, one contiguous inclusive page range, one copy initially; 16 MiB/page and 256 MiB/job of provider-produced spool bytes if serialized output is used |

**OBSERVED reuse gap:** the existing A2 prepared-text and Games text-mask profiles
admit at most 16 KiB of complete contextual text. The proposed 64 KiB P1 paragraph
bound exceeds that capacity; it is a candidate requiring an explicit provider
reply, not capacity inherited from either provider. Reconciliation must select a
supported bound or separately justify and verify an extension. Splitting a
paragraph arbitrarily to evade the existing limit is not an admitted substitute.

Reserve before copying/allocation; charge container capacities and retained owner
metadata, not just visible payload lengths. Shared allocations count once and
remain charged to the service lifetime ledger until their last owner disappears.
Reopen/eviction cannot reset accounting. Frozen consumer source storage is owned
and reported separately; these figures do not claim a whole-process quota over
it, native allocators, printer drivers or OS spool storage. Unsupported native
allocation bounds are stated explicitly rather than inferred from page limits.

The executing slot includes queued/running/ready/cancelled-unreleased work.
Cancellation does not release it until native work returns and its borrows and
candidate allocations are destroyed. New navigation replaces only the one desired
intent. Complete page replacement is atomic; failed or stale work leaves the prior
page lease intact and explicitly old/pending when its layout key differs. There
is no cache of every rasterized page. Revisited pages regenerate from bounded
checkpoints and the same frozen content; missing capacity returns a typed refusal.

Pagination preflight must finish before spool submission, so unsupported content,
page ranges or known layout limits do not knowingly begin a partial job. It need
not retain every page plan. Native rendering/driver failure can still occur after
submission begins and is reported as such. Long native calls remain potentially
noninterruptible; cancellation or close cannot promise a hard deadline without
measurement. No first slice borrows A2's two-mask or payload reservations.

## Dialog, spool and cancellation outcomes

Dialog completion is `ticket_chosen`, `user_cancelled`, `unsupported`,
`unavailable`, `owner_lost` or `native_error`; closing/cancelling it submits
nothing. The native Print action may provide the one explicit print intent for
the returned ticket. The controller may paginate that ticket and submit without
an extra user prompt, but may not substitute changed settings or another source.
UI wording and state must distinguish pagination pending from job submitted.

Each submission ID reaches exactly one terminal receipt:

- `spool_accepted`: the backend observed OS acceptance of the complete requested
  job and reports the platform job identifier if available.
- `cancelled_before_submission`: no page/job crossed the native submission boundary.
- `submission_failed`: known failure before native acceptance, with exact stage.
- `partial_or_unknown`: a failure, cancellation or disconnect occurred after
  output may have been accepted; report known page progress and its uncertainty.

**Spool acceptance is not physical print success.** Driver/spooler continuation,
paper/ink state, delivery and physical completion remain unknown unless a later,
separately supported observation supplies them. Never retry an ambiguous or
accepted submission automatically. A retry is a new explicit user action and ID.
Native cancellation after submission reports `cancel_requested`, `unsupported`
or `too_late`, not an assurance that nothing printed; it does not rewrite the
original terminal submission receipt. A native job may outlive the UI session.

Closing stops new requests, invalidates completion authority and requests dialog/
work cancellation. The session retains source/font/native owners until quiescence.
Owner-window disposal must wait for owned native-dialog callbacks to finish under
the existing window lifecycle; do not pump an ad hoc nested loop or block the UI
waiting for a callback that requires that UI. Worker join and native teardown
ordering must be demonstrated by the provider. Source/selection/history remain
unchanged on every close/failure path. A bounded retained terminal snapshot is
readable until the session is disposed; no callback targets a disposed consumer.

## Availability, errors and reconciliation

| Platform | Preview/layout | Native dialog | Spool submission / cancellation |
|---|---|---|---|
| Windows | Proposed, unverified | Proposed, unverified | Proposed, unverified |
| macOS | Proposed, unverified | Proposed, unverified | Proposed, unverified |
| Linux | Proposed, unverified | Proposed, unverified | Proposed, unverified |

No specific Win32/AppKit/Linux print API is selected here. An installed manifest
must later identify the exact provider/profile/backend/version and independently
verified capability subset. Preview can be available while printing is unavailable;
a working native dialog alone does not establish page generation or spool support.
Missing/mismatched support returns typed `unsupported`/`unavailable`/
`version_mismatch`, never an empty success or a silently executed fallback.

Common refusals also include `invalid_request`, `stale`, `busy`,
`snapshot_unavailable`, `source_changed`, `unsupported_content`,
`unsupported_layout`, `missing_font_coverage`, `limit_exceeded` with named resource,
`cancelled`, `closing`, `wrong_executor`, `identifier_exhausted`, `resource_failure`
and `native_error` with platform diagnostic. Admission failure preserves caller
owners/output; accepted asynchronous work gets one terminal disposition, including
cancelled work whose slot remains occupied until retirement.

Reconciliation asks for concrete replies to four points: SwiftEdit's immutable
source/selection and plain/rendered projection defaults; provider acceptance or
bounded correction of the numerical profile; provider page-plan/native-ticket
fidelity and nonblocking close protocol; and the first platform's exact native
capability subset. These are required contract replies under existing feature
direction, not a new owner permission gate or a request to choose a storage
architecture. No executable API sketch is introduced. Any later C++ sketch and
authored implementation must follow the complete house style, with exact source
review recorded independently of functional tests.

## Fixture obligations before promotion

Use a deterministic page oracle with frozen font bytes, source/projection IDs,
ticket and layout revision. Cover empty/trailing lines, CRLF, invalid-byte labels
and literal-label collisions, tabs under the consumer policy, combining/RTL and
cluster boundaries, overwide units, explicit breaks and page-boundary paragraphs.
Verify preview/print logical plans match and zoom changes no pagination. Check
all independent limits at boundary/plus-one, failed reservations and retained
leases across eviction/reopen. Preserve unknown page count until end-of-source.

Exercise revision/layout/ticket replacement, stale completion, equal-key reuse
after cancellation, one-slot backpressure, selection snapshots, source loss,
close during source/native work, owner loss in dialog, queued native callback
teardown, repeated close and checked identifier exhaustion. A fake spooler must
distinguish cancel-before-submit, complete acceptance, failure after some pages,
ambiguous response and no automatic retry. Native tests on each claimed platform
must verify dialog ownership/focus, settings/range fidelity, cancellation and
observed spool receipt; they must not label that physical print success.

Independent consumer builds and matching install manifests precede SDK claims.
UI responsiveness, first-page latency, page throughput, peak retained/workspace
bytes and close/cancel tails need named workloads and baseline measurements;
bounded input alone proves none of them. Keep failures and unsupported content
visible. P1 adds no availability to D1–D4, A2 or planning-only `text_editor/`.
