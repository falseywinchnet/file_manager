# ORC-GUI-001 — prepared paragraph development proposal D3a

Date: 2026-10-01. Revision: `prepared-paragraph-candidate-1`.

Status: **CANDIDATE semantic proposal for provider/consumer reconciliation**.
This is an actionable development boundary, not a frozen public API or an
available runtime capability. Existing owner direction authorizes practical
implementation; provider/coordinator reconciliation of the exact integration
scope is the next step, not another routine user approval gate. High-reversal
architecture choices still follow the ADR process. No registry availability changes.

Provenance: [negotiation ledger](../../negotiations/SWIFTEDIT_DOCUMENT_VIEW_2026-10-01.md),
[provider proposal](../../../gui_forms/docs/DOCUMENT_VIEW_D2_D3_PROPOSAL_2026-10-01.md),
[D1 contract](GUI_DOCUMENT_VIEW_DEVELOPMENT.md). All operation/type names below
are semantic names for a matched C++ source projection, not a wire/C ABI or
binary layout promise. No daemon transports document, layout or paint data.

## First consumable slice

D3a prepares one bounded complete display paragraph and paints its prepared
glyph positions through an explicitly compatible production backend. D2a-v0
is the retained read-only consumer: it shows current/pending/unavailable coverage,
requests source pages, and changes exact-source viewport/font configuration.
It does not mutate source or invent grapheme navigation from glyph clusters.

The first slice has no caret/hit-test/selection-geometry API, wrapping or tab
layout claim. Those require later, separately reported layout capabilities.
Consumer-owned source selection survives view changes, but this slice cannot
create a selection from a pointer or calculate its rectangles. D4 editing,
printing P1 and dynamic windows W1 remain separate. Full long-line product
requirements remain open; refusal is not product completion.

**OBSERVED consumer reply:** SwiftEdit explicitly accepts this visual-only stage
for intermediate validation. Its existing source-based copy may operate only
on a consumer-certified selection, independently of this view. Full interactive
selection/editing remains required before GUI migration is complete. It requires
proved production painter support before exposing `ready` on the Windows host;
otherwise the view reports unsupported.

## Ownership and records

| Semantic record | Contents and authority |
|---|---|
| `PreparedTextKey` | Full D1 page request; display interval; layout serial; provider instance/generation; font-set identity/generation; exact effective font and variation; scale; paragraph/context generation; wrap/tab mode; raster-profile identity/revision |
| `LayoutAuthority` | Session instance plus checked nonreused uint64 admission epoch; distinct from resource lifetime and from the externally supplied key |
| `EncodedFontLease` | Immutable encoded bytes and exact face index, registered role/style and identity for every admitted face; genuinely shared lifetime, no mutable alias or native FT/HB object |
| `PreparedFontTable` | Result-local nonzero unique face IDs mapped to lease entries; no reinterpretation as current host/provider IDs |
| `PrepareInput` | Owned bounded valid display UTF-8, exact interval and D1 mapping/context evidence; no retained consumer Session/file/UI borrow |
| `PreparedTextLayout` | Unique movable consumer handle over private typed shared immutable `PreparedTextStorage`; copied display text, complete glyph/run records and portable extent, key/authority, font table/lease and capacity report; no `void*`/`any` payload or exposed native handles |
| `RasterProfile` | Versioned font-variation/synthesis, size rounding/DPI/scale, load/hinting, supported glyph formats, raster and pixel/compositing semantics; implementation details may remain private but compatibility cannot be implicit |
| `TextCapacityProfile` | Count and capacity-byte bounds for controlled input/result/table/metadata/workspace and retained encoded-font owners, plus separately declared measured/unknown opaque allocations |

Exact structural identity is authoritative; a hash is not the sole equality
proof. Matching IDs do not permit different font bytes/configuration. Native
glyph identity must be compatible with the exact face and profile; unsupported
variation, color/bitmap format or missing glyph coverage is an explicit refusal.
The historical combining-cluster fixture alone did not establish the cause.
The provider's subsequent diagnosis identifies a nominal-cmap false negative
before HarfBuzz composition: Carlito shapes `a` plus U+0301 to glyph 1955 despite
lacking a nominal U+0301 cmap entry. The diagnostic locator/fix receipt is pending;
no font-pack expansion or general Unicode-coverage claim follows.

The worker exclusively owns mutable shaping faces/engines on its executor.
Painting creates/owns its own native resources on the painter executor using
the retained exact font data; no mutable worker face crosses to paint. Native
resources are destroyed on their owning executor before dependencies disappear.
The lease can survive worker destruction, but revoked authority cannot paint or
publish as current. Public immutable borrows end on owner release; retained-view
borrows also end on its next successful replacement or destruction.

Recorded display commands retain the typed immutable storage through replay;
they never borrow the consumer's unique wrapper. Releasing/replacing that wrapper
does not free bytes still held by a display chunk. Revocation prevents stale
replay even while command storage lives. Bound queued/in-flight display chunks
and all distinct retained payloads; charge each allocation once with explicit
owner/reference counts. A claimed one-active/one-replacement budget must include
old chunks or retire them before admitting storage that would exceed it.
The negotiated profile includes finite `max_retained_generations` and aggregate
prepared-storage bytes. The service tracks distinct payload generations retained
by view, replacement, recorded chunks and previous/candidate frames. New result
admission returns busy when those reservations cannot fit; only actual retirement
releases them. Shared references do not duplicate payload bytes, but they extend
the lifetime of distinct generations. Controller slot count alone is insufficient.

## Capabilities and admission

`capabilities()` returns source-projection revision, provider identity, supported
preparation/raster profiles, explicit production backend compatibility, and
capacity/guarantee classes. Three distinct facts must be reported: prepared
native glyph geometry, compatible production painting, and any interaction
geometry. Estimated fixture results cannot satisfy native requirements.
Resource guarantee is independently `measured_capacity` or proved `hard_quota`.
Measured-capacity does not imply a process or opaque-library peak bound.

`open_session(font_lease, profile, capacity_request)` validates compatibility,
known bounds and ownership before creating a worker. It returns the negotiated
finite numeric capacities and an owned session, or a typed refusal. No silent
downgrade, zero-as-unlimited limit, missing category or SDK-version substitution.
Exact numeric profiles are supplied and checked at this boundary; they are not
inferred from the 16 KiB display limit.

The initial comparison profile uses display input <=16,384 bytes, runs <=16,384,
glyphs <=65,536 and an explicit finite admitted font set. Count and capacity-byte
checks both apply; actual record sizes, metadata and simultaneous owners count.
These proof-derived counts are proposed development ceilings, not selected
universal limits. Source/context remains inside D1's 65,536-byte permission.
Complete paragraph/context is required; unknown context returns
`context_required`, known excess returns `budget_exceeded`. Tabs, wrapping or
other unsupported operations return `unsupported_profile` before native work.

Controlled input allocations occur after admission. Controlled output/storage
growth checks the admitted count and byte capacity before allocation; a provider
must identify which allocations it controls. Opaque FT/HB bitmap/cache/temporary
memory is separately unknown/measured, never hidden under these bounds. Bitmap
guards after FT allocation establish indexing safety, not pre-allocation quotas.
An adapter unable to enforce a requested controlled-storage bound refuses that
profile; opaque unknowns alone do not block an admitted measured-capacity profile.

**OBSERVED source qualification:** the current private engine reserves/grows
result vectors before a public count/byte admission policy exists. A production
adapter therefore needs scoped checks before glyph/run result allocations;
checking a finished `ShapedText` against proof counts is insufficient. This is
a concrete implementation requirement, not a claim that existing code enforces
the candidate controlled-storage guarantee.

## Named operations and asynchronous lifecycle

All control operations run on one owning UI executor. The service owns worker
dispatch; there is no foreign-thread callback into the consumer.

1. `desire(key)` validates the entire key, context metadata and capability, then
   advances the session's checked authority epoch and stores one desired value.
   Valid replacement revokes earlier authority even for an identical key;
   invalid input preserves prior desire/authority. Exhaustion refuses, no wrap.
2. `submit(authority, input)` validates before capacity checks. One replacement
   slot covers queued, running, ready, failed and cancelled-but-unreleased work.
   Busy or invalid admission preserves caller ownership and epoch. Accepted
   admission transfers the bounded input owner; the caller becomes empty.
   No UI/page borrow survives the transfer. No further payload is queued.
3. The worker performs synchronous `prepare` on its owned executor. Its request
   borrow lasts for that call; its complete result independently owns the needed
   data. Exceptions are contained as terminal failure with no partial success.
4. A coalesced readiness wake targets the owning UI dispatcher, carries no
   payload backlog, and checks a live connection before dispatch. `inspect_ready`
   observes slot status on UI. `adopt_ready(expected_authority)` validates full
   key/epoch and complete result, then moves ownership atomically into the view.
   Failed adoption preserves previous presentation and the ready owner/slot.
5. `discard_ready` releases completed input/result reservations before the slot
   becomes reusable. Successful adoption transfers its result reservations to
   the separately accounted active owner. No acknowledgement frees a running job.
6. `cancel` revokes authority and clears desired metadata; it never frees an
   occupied slot. Rapid changes retain only the latest desired key and request
   its payload once the slot is released. Same-key desire cannot revive old work.
7. `begin_close` stops admission and revokes notifications/authority. Completion
   drains owned work; `join_and_release` releases unpublished resources after
   worker termination. Model/service/font owners survive through join. Native
   shaping remains noninterruptible; synchronous join may block and has no
   promised latency. Host shutdown integration must state where it waits.

At most one active prepared result, one replacement job/result and one desired
metadata value occupy the controller. Distinct older payloads held by display
chunks/previous frames are additionally bounded and charged as specified above;
this is not a claim that only two payloads exist overall. D1 slots are independent.
Account also for bounded simultaneous input and result copies, old/new font
owners, candidate paint staging and previous visible content. No paint, pointer
motion or caret timer performs shaping or source retrieval.

## Transactional paint and retained-view behavior

`draw_prepared_text(layout, expected_authority, placement, clip, color)` requires an
explicitly compatible painter. It checks full current key/epoch, lease/table,
profile, glyph/face ranges and finite geometry before visible mutation. It uses
prepared glyph IDs/positions without reshaping. The backend stages any fallible
work needed so an error cannot leave partially replaced visible content; the
implementation may use a bounded surface or its existing transactional frame,
but that mechanism needs source feasibility and capacity evidence.

A recording painter returns `recorded` only after retaining a valid typed
command; this is not a successful raster commit. Replay revalidates authority
and backend compatibility and reports `painted` or a named failure at the actual
frame transaction. No recorder can convert future backend refusal into success.
If replay cannot preserve the prior coherent frame on failure, that backend is
not yet compatible with this transactional profile.

Success commits coherent pixels and painted identity. Refusal preserves prior
pixels with their actual identity; it cannot make old content current. A retained
view hides stale content or labels it as previous coverage while pending. Hit
tests are unavailable in this slice, including against stale content.

Consumer operations are `bind_document`, existing D1 page publication/finish/
cancel, `set_viewport`, `set_text_configuration`, `presentation_status` and a
named bounded page-request target. The consumer owns file bytes, decoding,
selection/copy commands and save/history. No retained raw Session pointer is
required. Page intents carry exact source anchor/range and generation, not an
unbounded byte payload or implicit scan. Source/display/grapheme units remain
distinct. Notification runs after coherent state installation; mutation during
notification returns `reentrant_operation` or is deferred as one named intent,
never recursive publication. Disconnection revokes targets before view release.

## Results and failure laws

Named statuses: `success`, `recorded`, `painted`, `pending`, `busy`, `invalid_input`, `stale`,
`cancelled`, `closing`, `closed`, `generation_exhausted`, `context_required`,
`budget_exceeded`, `unsupported_profile`, `incompatible_backend`,
`missing_font_coverage`, `incompatible_font`, `incompatible_glyph`,
`invalid_geometry`, `resource_failure`, `native_failure`, `version_mismatch`,
`reentrant_operation`. Operation-specific result types admit only applicable
statuses; a status alone never implies ownership transfer. Admission, adoption,
discard and paint obey the explicit ownership/transaction laws above. Readiness
is a notification, not success. Cancellation cannot be mistaken for cleanup.

## Source feasibility and freeze checklist

**OBSERVED provider reply:** a separate optional service observer can follow
`include/gui_forms/window/window.hpp` and
`src/core/window/typography/window_typography.cpp`'s metrics attachment pattern,
but must revoke generations/epochs before detach; the existing setter alone is
insufficient. The host owns service/worker lifetime through shutdown and join.
`include/gui_forms/types/painter/painter.hpp` can add a typed operation whose
default is explicit unsupported, with a new `prepared_text.hpp` declaration.
This is matched-source development and requires rebuilding implementations.

Retained integration also needs `src/core/display/command/display_command.hpp`,
`recording_painter/recording_painter.*` and `replay/replay_display_chunk.cpp` to
retain typed storage and separate record admission from frame commit. These
are proposed edit scopes to coordinate under existing implementation direction;
this candidate does not assign another chat's files without that coordination.

The current Windows `DibPainter` in
`src/host/windows/application/windows_host.cpp` shapes through GDI/Uniscribe
inside text drawing and cannot consume HB-prepared glyphs today. It must report
unsupported until a compatible adapter is proved. Skia has a glyph-draw source
path but its dependencies are absent in the reviewed Shadow checkout. The
private FT fixture selects neither backend. Resolve one usable Windows route
before implementing an unsupported-only public surface. The provider must compare
a bounded prepared-glyph raster adapter in the current CPU Windows host against
bringing the existing Skia route online, naming dependency/build scope, exact
files, font/glyph compatibility and frame transaction. Recommend a finite first
profile and account for retained generations without a generic quota framework.
Keep other backend absence explicit; a portable contract does not establish
their implementation.

### Provider comparison and selected Windows development route

**OBSERVED coordinator development selection — route A:** add a bounded private prepared-glyph
FT raster adapter to the existing Windows `DibPainter`, leaving ordinary GDI text
behavior intact. Reuse existing pinned FT/HB/SheenBidi sources in a new HB-enabled
build/SDK projection. Own production service/storage/raster files under
`src/render/text/prepared_*`; do not promote diagnostic probe files into a public
implementation. Reconcile Window/service, typed command/replay and Windows host
hunks as one bounded integration scope.

Source inspection finds the current host begins a frame, calls Window paint,
then presents by BitBlt, but begin-frame only resets state and drawing mutates
one backing DIB. Skipping presentation after failure does not restore that DIB
for later exposure/live presentation. The recommended transaction is explicitly
bounded front/candidate DIBs with correct GDI flush/ownership sequencing; failed
replay denies paint receipt/presentation and leaves the coherent front untouched.
Surface dimensions, combined pixel capacity, resize failure, display-chunk
retirement and live-present behavior need focused tests and finite first-profile
limits. These are concrete implementation work, not a generic allocator design.

**CANDIDATE B:** bring the existing Skia glyph-draw route online. This requires
pinned Skia/dependency fetching and patches, a Windows GN/Ninja build and four
archives (Skia/skcms/PNG/zlib), then a Skia-backed host surface/present integration;
turning on CMake alone does not replace the concrete DIB host. Its FT pin differs
from the shaping stack's, so compatibility still requires evidence. It also needs
the same public ownership, typed replay and transaction laws. The provider reports
no current Shadow source/archive artifacts for this route.

The coordinator selected route A for development integration, not as a universal
renderer ADR. A1 follows the separate combining-cluster correction and starts
with the provider's exact source/test scope for finite front/candidate DIBs,
rollback preservation, GDI flush order and paint-receipt propagation. A2 adds
service/storage/prepared raster and typed display retention after a finite first
numeric profile and generation accounting are reconciled. This is practical
implementation under existing authorization, with no additional routine owner
approval gate. New matched HB-enabled SDK artifacts use a distinct prefix;
other backends remain unsupported until independently implemented and verified.

Provider reply must name actual Window/provider attachment and Painter/backend
files, typed private result ownership, readiness-dispatch integration, chosen
finite capacity profile and transactional paint mechanism. Consumer reply must
accept visual-only first-slice behavior and name required missing interaction
operations. No unavailable backend may silently fall back to reshaping strings.

Before production-source integration: reconcile those replies and numerical
profiles; identify exact owned directories/headers and matched rebuild scope
under the existing owner direction. No extra routine user approval is required.
Before capability availability: test invalid/busy ownership, same-key cancel,
all identity fields, release acknowledgement, shutdown/reentrancy, missing and
incompatible fonts/profiles/glyphs, paint atomicity, and installed independent
consumer behavior on each advertised backend. Measure native page-to-frame and
close latency and report known capacities versus opaque unknowns separately.
Private fixture parity does not satisfy this installed/backend gate.

All later implementation/tests/tooling require exact-scope semantic review
against `planning/PROGRAMMING_HOUSE_STYLE.md`; scans and functional tests are
separate evidence. This candidate adds no implementation or new architecture ADR.

## A2 finite development profile — reconciliation 002

**OBSERVED** — Following A1 checkpoint `f26b143`, the coordinator accepted the
following finite A2 profile in principle for implementation. Exact record names,
units and ownership accounting are reconciled with the provider before headers
freeze; no further owner approval or shared-new-file assignment is required.
Provider owns the new `prepared_text` public hierarchy, private `prepared_*`
files and focused tests; root owns CMake. Portable mask/service work proceeds
first without waiting for Window integration. Audio files remain Games-owned.

| Controlled quantity | First development limit |
|---|---|
| Display input | 16,384 bytes; complete single paragraph |
| Source/context permission | D1 maximum 65,536 bytes |
| Mapping/endpoint records | At most 16,385 under a combined 1 MiB metadata budget; result metadata is included in its 8 MiB, not additional |
| Run/glyph records | At most 16,384 runs and 65,536 glyphs; counts do not override byte limits |
| One prepared payload | At most 8 MiB of requested allocation capacity, including text, table/control records and array capacity times actual record size |
| Distinct retained prepared generations | At most three and 24 MiB combined, including active, replacement and retired command/frame-held owners; a fourth refuses |
| First-party shape workspace | At most 16 MiB, including TextStore/segment/bidi and other first-party growth; every allocation/growth path guarded |
| Encoded font owners | At most eight faces, 4 MiB per face and 8 MiB aggregate retained bytes across current/old sets and leases |
| Gray mask | Each dimension <=4,096; product <=4,194,304 pixels; gray8 capacity <=4 MiB |
| Mask staging | Two masks / 8 MiB combined including previous and candidate; no hidden third retained output |

Font changes may retain up to two banks only when combined distinct encoded
owners fit the aggregate 8 MiB limit; shared identical owners count once. Old
leases remain charged until actual release. Admission refuses/defers when the
limit would be exceeded and never destroys valid active presentation merely to
make the new request fit. The earlier two-bank/16 MiB suggestion was not selected.

Reserve replacement-generation and payload budget before dispatch/allocation.
Cancelled/ready results retain reservations; command/frame references extend
them after view replacement. Only actual storage retirement releases the charge.
Transfer to active ownership is not deallocation. Input, output metadata and
copied text coexist where required and need explicit separate accounting, not
double use of a single reservation. The implementation must state allocation
overhead coverage separately; these limits are not process RSS or vendor quotas.

The 16 MiB workspace claim covers all first-party buffers, not just a named
scratch vector. FT/HB opaque allocations/cache peaks remain unknown. FT glyph
bitmap dimension/64 MiB guards are post-native-allocation checks and do not
establish pre-allocation quotas. Existing unbounded result growth must receive
scoped admission checks before the bounded overload is advertised.

The initial raster profile is static/default variation, no synthetic style,
FT outline-to-grayscale, font size 4..128 DIPs and scale 0.5..4. No tabs, newlines,
wrapping or monochrome-mask capability is claimed. Exact 26.6 size rounding,
glyph position units and shape/paint conversion must match, with noninteger and
limit-scale tests; scale 1 private proof alone does not establish this range.
Empty/space-only mask success, zero-ink extent/bearing and owner lifetime require
explicit result rules before implementation conformance can be assessed.

Games' six-role wrapped/newline/monochrome mask needs are a concrete follow-on,
not satisfied by this profile. SwiftEdit accepts this visual-only intermediate
stage; full interaction/editing remains open. No installed capability is inferred
from acceptance of these development limits.

### Provider agreement 003 — names, reservations and coordinate law

The provider accepts `PreparedTextService` as the owning per-view-lifetime
context, `PreparedTextSession` as the worker/session, `PreparedTextLayout` as the
unique wrapper over typed immutable retained storage, and `GrayTextMask` as a
unique movable output with const borrows only. Names identify the matched source
development projection; no ABI or installed capability is frozen by this reply.

One private ledger survives with every session/layout/command/mask that retains
its resources. Reopening a session through the service cannot evade charges for
retired payloads or encoded-font owners. No global process quota is implied.
Up to two font banks may coexist only under the same aggregate 8 MiB admission;
the same allocation shared by several owners is charged once. Initial use may
keep one immutable bank until session drain and all retained payload release.

Each replacement reserves a full 8 MiB and one of three generation slots before
queuing. That reservation includes owned text, mapping/proof, run/glyph arrays,
font-table/control capacities; encoded font bytes and opaque native storage are
separate categories. Smaller actual retained capacity is reported, but unused
reservation remains held until generation retirement for this first profile.
The input slot is independently bounded to 16 KiB text plus 1 MiB mapping/proof
while it owns those bytes. Transfer into the prepared result moves that owner
and its charge; it does not create an unaccounted simultaneous copy. If validation
or construction requires a copy, both live allocations must fit named budgets.

The exact scale law is: compute positive device size by rounding
`font_size_dip * device_scale * 64` to the nearest integer (positive half ties
up), then use that same 26.6 size at 72 DPI in FT and HB. Letter spacing is
multiplied by scale once. Prepared glyph coordinates and advances are device
pixels; public logical metrics divide by scale. Raster consumption never scales
prepared glyph positions again. Placement/clip conversion from DIPs is separate
and checked once. Identity includes scale and this versioned rounding/profile.

Default variation and no synthetic styling are required; requested primary
registered weight/style must exist, while actual fallback style is explicit.
All paragraph line separators, tabs and wrapping requests are unsupported in
this first profile. Source and display mapping/proof are owned and tied to the
full D1 token/interval with certified complete paragraph boundaries; missing
certification returns context-required, not fabricated endpoints.

Gray-mask dimension/product/capacity admission precedes growth. A zero-ink
success has width/height zero and bearing (0,0), retaining logical advance and
line metrics separately. Old successful and candidate masks share the 8 MiB
two-surface ledger; const borrows do not create external shared payload copies
and end on owner release/replacement. Refusal leaves old output intact.

These exact laws permit scoped implementation under the already assigned files.
Conformance still requires tests for fractional scale/rounding, font retirement,
fourth generation refusal, queued/failed/cancelled ownership and zero-ink masks.

## A2 source-component checkpoint — pending independent coordinator review

**OBSERVED** — The provider submitted
[A2 service/raster receipt](../../../gui_forms/experiments/PREPARED_TEXT_A2_SERVICE_2026-10-01.md),
[raw focused tests](../../../gui_forms/experiments/PREPARED_TEXT_A2_SERVICE_TESTS_2026-10-01.txt)
and [14-file hash manifest](../../../gui_forms/experiments/PREPARED_TEXT_A2_SOURCE_HASHES_2026-10-01.csv).
Orchestrator read the receipt/raw results and independently matched all 14
manifest hashes to current files, with zero mismatches. It did not rerun tests
or perform the independent coordinator's implementation review.

**MEASURED provider evidence:** two service/raster suites and four Unicode/
shaping dependencies passed 6/6 in 0.75 seconds. These are correctness-test
durations, not application latency. Coverage includes full-key authority and
same-key cancellation, input transfer, failed adoption, retained generations/
font leases across sessions, mask capacity, fractional/end-point scale and
rounding, stale/oversized rejection preserving pixels, join/wake revocation and
zero-ink masks. Command-held retention is modeled with the private typed owner;
actual display recording/replay is not implemented by this checkpoint.

The submitted source provides the public declaration hierarchy and private
storage/service/raster component, an OFF-by-default standalone target and focused
tests. Bounded HB registration uses admitted persistent table storage charged
within the shaping-workspace allowance. Font registration and position-conversion
corrections are within the provider's reported exact authored review scope.
Root owns CMake review. Independent root acceptance remains pending; this status
is source-component test-ready, not accepted production integration.

Requested first-party capacities do not measure allocator/control-block overhead,
thread stacks or opaque FT/HB/SheenBidi caches/peaks. FT bitmap checks follow
native allocation. No exhaustion injection, thread sanitizer, responsiveness
benchmark or visible native-window test is claimed. Native shape remains
noninterruptible and close/join may block. The receipt's semantic review is
provider-attributed and does not certify unchanged legacy or vendor code.

Draft headers are excluded from blanket installation; no export, new SDK,
Application stage or runtime availability is published. Window attachment/
revocation, typed Painter operation, display-command/record/replay retention and
Windows DIB grayscale compositing with failure/receipt propagation are proposed
next hunks only: no assignment is granted here. Existing cursor baseline
`50ea223` is preserved. Wrapping, tabs, monochrome, hit/caret/selection and D4
remain outside the current profile.

### Shutdown correction checkpoint — review remains pending

**OBSERVED coordinator correction:** session identity must be process-wide and
nonreused, with exhaustion refusal rather than recycling. Authority validation
and publication must occur under the authority lock as one protected operation;
a check followed by unlocked publication must not admit a concurrently revoked
result. These are recorded correctness requirements/corrections, not independent
acceptance or a claim that the earlier 14-file hashes cover later source edits.

The owner directed imminent machine shutdown. This record is saved for the
coordinator's shared checkpoint; source review, corrected hashes and any required
validation remain pending. No A2 acceptance, runtime availability or next-slice
assignment follows from this shutdown checkpoint.

### Coordinator acceptance — continued development integration only

**OBSERVED** — The owner withdrew the shutdown hold. The coordinator completed
the correction review of checkpoint `6992bcb` and accepted the A2 service/raster
component for continued development integration only. This supersedes the
pending-review status immediately above. The
[independent coordinator review](../../../gui_forms/experiments/PREPARED_TEXT_A2_COORDINATOR_REVIEW_2026-10-01.md)
records the exact source/CMake scope and resolved findings. Orchestrator read
that receipt; it did not repeat the coordinator's review or test execution.

Session IDs use a nonzero, nonreused atomic sequence shared across service
instances within one linked provider. Saturation refuses before wrap; failed
construction may consume an ID. This process-wide claim assumes one linked
provider: separately loaded duplicate provider images are neither established
nor admitted. ID uniqueness alone does not publish session state.

The corrected cross-service test rejects foreign authority despite equal epochs,
preserves previous output and ready ownership, then accepts the matching token.
Final raster authority validation and mask replacement now hold the authority
mutex together, giving a defined order against revocation. Reviewed retirement/
reservation/close paths revealed no reverse nested-lock path in this component.
Successful publication does not preserve authority against subsequent revocation.

**MEASURED coordinator validation:** all 14 manifest hashes matched, six focused
targets were current, and the six selected tests passed in 0.63 seconds. No
additional blocking finding was identified in the corrected authored scope
under `planning/PROGRAMMING_HOUSE_STYLE.md`. This is not untouched legacy/vendor
certification, sanitizer coverage or a deterministic raster/cancel scheduling test.

Window attachment/readiness, typed display retention, Painter replay, Windows
DIB compositing, production backend proof and SDK export remain separate,
unassigned next integration work pending exact ownership coordination. No host,
Painter, DIB, installed SDK or runtime availability follows from this component
acceptance. Noninterruptible shaping, join latency, opaque native allocations
and the visual-only feature limits remain unchanged.

### Assigned next slice — guarded retained painting and DIB staging

**OBSERVED coordinator assignment:** the provider now owns the neutral core
storage/geometry extraction; guarded public Painter typed result; typed shared
storage display command and recording/replay; guarded Windows DibPainter/
compositor changes; and focused retained-lifetime and failed-frame/receipt tests.
Root owns all CMake changes. This supersedes the unassigned status above only
for these named edits; it records assignment, not completed implementation.

The service remains caller-owned for this slice. Window lifecycle/service
attachment is subsequent, unassigned work and Window remains unchanged. Existing
cursor and other unrelated host behavior are not widened by this assignment.

Painter outcomes distinguish `recorded`, `staged` and typed refusal. Recorded
means a command retained its typed immutable payload; staged means backend
work reached the candidate frame. Neither means pixels were presented. Only
the frame transaction/receipt can establish the final commit/presentation
outcome, including failure preservation. Earlier proposed `painted` operation
wording does not grant per-command presentation success in this retained route.

Windows frame commit runs on the owning UI executor. The standalone raster
contract remains independently executor-owned and is not silently restricted
to UI execution by host integration. Storage retention, revocation and failure
tests must cover actual recording/replay and candidate-frame behavior without
claiming physical-screen atomicity after native presentation failure.

The prepared-development-ON SDK installation must refuse to avoid exporting
incomplete headers or a changed Painter vtable ABI. Ordinary OFF SDK behavior
remains unchanged. This is an assigned development slice, not SDK publication,
production backend readiness or runtime availability. Subsequent evidence must
identify exact source review, tests and any remaining native behavior limits.
