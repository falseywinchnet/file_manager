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

## D3a/D2a follow-up 001 — provider proposal and consumer reply

Status: **partial semantic convergence; concrete seam remains CANDIDATE;
source go-ahead and runtime availability not granted**.

**OBSERVED** — D1 bounded model source and review evidence were integrated at
`bb61f68`. That advances the earlier implementation-pending status for the model
only; installed SDK, native UI and complete product evidence remain separate.

The provider supplied
[D2/D3 proposal and refinement 002](../../gui_forms/docs/DOCUMENT_VIEW_D2_D3_PROPOSAL_2026-10-01.md).
It proposes D3a prepared layout followed by D2a retained read-only view, with
D3b long-line/context continuation and D4 mutating intents still separate.
The coordinator requested concrete seam reconciliation, without source changes.

**OBSERVED** — SwiftEdit's consumer chat accepted the read-only stage only as
an intermediate proving slice. Its adapter will supply complete bounded
paragraph/aligned display context, atomic expanded-label mappings and separate
source-grapheme proof. Excessive or unknown context produces explicit
region-not-available status, preserves source and selection, and never relabels
old geometry as the requested region. D3b remains necessary for the full
long-line requirement. These replies do not establish executable behavior.

### Minimum seam under reconciliation

The provider's concrete candidate has a separate typed layout service and
executor-confined session. Synchronous `prepare(const TextLayoutRequest&)`
borrows adapter-owned immutable input only until return. Queued work owns its
input; it never retains a D1 page borrow across dispatch. A successful result
independently owns bounded copied text/geometry and typed immutable font
dependencies. Simultaneous input and output copies count separately.

Prepared paint and hit testing consume the same immutable result. Full identity
includes the exact D1 page request, layout serial, provider and font-set
instance/generation, effective font, device scale, wrap width/policy, tabs and
boundary-context generation. Hash equality alone is insufficient. Adoption
validates identity and geometry before swapping; rejection preserves old
geometry and the incoming owner. A retained font lease keeps memory alive but
does not preserve paint authority after revocation. Old font/scale/provider
geometry cannot be painted or hit-tested as current. Previous-page coverage may
be displayed only under its actual still-authorized identity.

Resource accounting includes adapter input, each result, mapping/proof,
glyph/row/caret arrays, session scratch/cache, font owners, leases and overhead.
One active result and one replacement job/result may coexist; cancellation
retains the replacement slot until completion and release. One desired metadata
value coalesces requests. D1 slots remain independently counted. Count and byte
limits both apply before allocation; the numerical comparison profile is not
selected or measured. Font-set replacement charges both owners until release.

**OBSERVED** — The provider reports no demonstrated pre-allocation quota over
the current private HarfBuzz/FreeType shaping/cache path. Short input and a
post-allocation output check do not establish the requested hard bound. A
fixture provider may prove semantics as a candidate first slice; native exact
geometry needs lifetime and paint-parity evidence, while hard-bounded resource
availability additionally needs quota proof. These are distinct guarantees. This
record authorizes neither an allocator framework nor a worker/host redesign.

### Open corrections and admission gates

Provider refinement 003 accepts absolute uint64 source-byte endpoints paired
with page-local uint32 display-byte endpoints, strictly increasing in both
coordinates without duplicates. Every pair must round-trip under the exact D1
token and certify source-grapheme legality. No endpoint is implicit. Empty
document/admitted empty EOF coverage has exactly one certified EOF/zero pair.
Missing proof for nonempty text gives context-required; the complete-paragraph
profile requires certified actual paragraph edges.
Display clusters cannot prove source boundaries, and atomic label interiors
never become legal source carets. Source-grapheme proof may exclude a token
endpoint where adjacent source scalars form one grapheme.

Semantic endpoint counts and visual caret-stop counts must be separate because
bidi and soft-wrap affinity can supply multiple visual stops per endpoint.
The proposed 128 KiB proof capacity does not fit 16,385 ordinary 16-byte endpoint
pairs (262,160 bytes); count ceilings do not promise that maximum count fits the
byte ceiling. The provider has been asked to correct the comparison profile.
Refinement 003 proposes 32,770 visual stops separately from 16,385 endpoint
pairs; both remain comparison ceilings constrained by independent byte caps.
It also accepts estimated fixture geometry as capability metadata separate from
native exact geometry. Terminal success means complete within the reported
class; fixture results cannot silently become native exact support.

Remaining gates are private typed result/font-lease storage, complete numerical
capacities including metadata/overhead, the declared resource-guarantee profile,
and measured executor/scheduling ownership. Public seam and host changes require
parent coordination and go-ahead after reconciliation. Required evidence must
vary every identity field, test transactional failures and cancelled-slot
retention, verify source/label/grapheme/affinity boundaries, and measure prepare,
UI publication/paint and end-to-end page-to-frame latency separately.

No canonical availability registry changes follow from this partial agreement.
Full source review against `planning/PROGRAMMING_HOUSE_STYLE.md` remains a later
implementation obligation. This update reviews records only.

### Coordinator clarification — native diagnostic profile remains open

**OBSERVED** — The coordinator clarified that hard quotas for all opaque
third-party allocations are a candidate guarantee, not an owner-selected
prerequisite for native experimentation. Controlled arrays require checked
preallocation/admission. Opaque HarfBuzz/FreeType internal storage must be
reported as unknown or measured until evidence establishes stronger bounds.

The provider is authorized to audit the pinned libraries and run a bounded
diagnostic benchmark of the existing native shaping path, without public seam
or host changes. A measured-capacity experimental profile may be compared with
a quota-capable candidate. This supersedes any quota-or-fixture-only reading
of the provider's earlier proposal. It grants no hard-bounded native availability
and selects no generic allocator, sandbox architecture or UI executor. Profile
reconciliation must state which limits are enforced, measured or still unknown.

## Native diagnostic receipt 001 — negative scheduling evidence

**MEASURED** — The provider's
[native shaping receipt](../../gui_forms/experiments/TEXT_LAYOUT_NATIVE_PROBE_2026-10-01.md),
[initial CSV](../../gui_forms/experiments/TEXT_LAYOUT_NATIVE_PROBE_2026-10-01.csv)
and [reviewed CSV](../../gui_forms/experiments/TEXT_LAYOUT_NATIVE_PROBE_REVIEWED_2026-10-01.csv)
record two completed runs of the existing private shaping path. Orchestrator
checked the receipt against both CSV summaries; it did not rerun the probe.

Scope: Windows 11, GCC 16.2 Release, four fixed bundled fonts, HarfBuzz/FreeType
enabled, Skia/native hosts disabled, with concurrent development not isolated.
The receipt pins libraries, fonts and source hashes. Each case has a first shape
after font registration and 31 warm-engine samples; first is not OS-cache cold.
Timing covers `engine.shape`, including internal temporary destruction, but
excludes font setup, returned-result destruction, paint and the external oracle.

| Case | First-run warm p50 / worst (ms) | Reviewed-run warm p50 / worst (ms) |
|---|---:|---:|
| ASCII, 16,379 bytes | 4.1303 / 7.6094 | 4.1047 / 7.957 |
| Mixed bidi, 16,384 bytes | 328.826 / 894.725 | 312.668 / 331.370 |
| Emoji/fallback, 16,380 bytes | 155.825 / 170.971 | 152.886 / 197.012 |

The mixed-bidi and emoji cases produced 3,073 and 2,341 shaped runs respectively;
these counts are not timing sample counts. Both executions completed within the
external 60-second process deadline. That deadline supplies no within-call
cancellation. The second execution does not erase the first execution's tail.

This is negative evidence for admitting UI-thread execution from the 16 KiB
input cap alone. Worker placement alone would not establish bounded completion
or cancellation. Scheduling and work subdivision remain unselected; no
end-to-end latency, geometry correctness or speedup conclusion follows.

Returned-vector and encoded-font capacities were observed, but internal peak
allocations, library caches and temporary capacities remain unknown. The
16,383-byte enormous-grapheme case has one missing cluster in both executions,
so it does not establish complete coverage. Repeated glyph/cluster hashes and
aggregate checks provide limited repeatability, not full geometry or independent
shaping parity. Any optimization comparison needs complete baseline geometry
validation outside timing. Allocation-failure atomicity was not tested.

The receipt's house-style review covers the diagnostic and its narrow CMake
target only. It does not certify the production engine or vendor sources.
Public D3a/D2a, frozen SDKs, runtime availability and the separate input bugfix
are not changed by this evidence receipt.

## Worker development direction and consumer reply 002

**OBSERVED** — After baseline checkpoint `28ecc70`, the coordinator authorized
a concrete private worker proving direction: a worker-owned font engine, one
replacement job/result slot, one coalesced desired intent, generation-checked
publication, and close/drain/join. Private proof may follow narrow diagnostic
attribution. Public prepared-layout APIs are not frozen by that direction.

SwiftEdit confirmed that page, font or width changes immediately revoke stale
page/layout authority while preserving source and selection. An occupied slot
retains its job, result and reservations through cancellation until completion
and release acknowledgement. New input replaces only the latest desired
metadata; the latest job dispatches after the old slot is released. Explicit
pending-region status must not relabel old geometry as the target region.

Close disables new requests and revokes callbacks before drain/join. The native
shape call remains noninterruptible: cancellation suppresses publication but
does not promise bounded call completion or join latency. Model/service and
worker-owned resources must survive until join completes. No detached worker
or early slot release follows from requesting cancellation.

Prepared-result storage and font leases remain under concrete provider review.
The worker owns mutable HarfBuzz/FreeType engine state. The proposed paint path
must identify which immutable font bytes and glyph/run data survive in a result,
whether paint creates independent font objects, and which thread releases each
resource. An immutable wrapper does not establish that a mutable native face is
safe to share across threads. Revoked publication/paint authority is distinct
from the memory lifetime needed for safe release.

The measured-capacity profile must separately account for checked input/result
arrays and known font bytes, simultaneous active/replacement/input storage and
overlapping old/new font owners. Opaque library peak/cache allocations remain
unknown pending evidence. Lack of complete opaque quotas does not prohibit this
authorized private native experiment; it prohibits claiming a proved hard bound.

### Separate input-normalization correction

**OBSERVED** — The coordinator separately authorized the provider to add missing
`PhysicalKey::slash` (HID `0x38`) and the Windows `VK_OEM_2` mapping, with focused
US-layout Ctrl+Shift+slash tests. This is an additive development correction
within existing VK-derived normalization, not a new keyboard architecture gate.
It does not establish character-based Ctrl+? across keyboard layouts. Frozen
SDKs remain unchanged; implementation/test evidence is a separate provider
receipt. This authorization does not widen the document-layout API work.

Provider completion receipt:
[Windows slash-key audit](../../gui_forms/docs/WINDOWS_SLASH_KEY_AUDIT_2026-10-01.md).
**OBSERVED** — The provider reports the public slash constant and `VK_OEM_2`
case are implemented in a private translator called by the actual host.
**MEASURED** — The coordinator reran the Windows Release host build and focused
mapper test after removing construction-only synthetic-event assertions; 1/1
passed in 0.05 seconds. Evidence covers the actual mapper's behavior, not
modifier/down-up dispatch. Orchestrator read the receipt and coordinator report;
it did not rerun the test. Native keyboard delivery, active layout, WM_CHAR
behavior and SwiftEdit command dispatch remain untested by this fixture.
Installed consumer/native US-layout acceptance remain pending. The provider's
house-style review covers the named correction scope, not surrounding legacy
host code. No runtime-availability promotion follows from this source receipt.

## Provider reply 003 — concrete private prepared-result ownership

**CANDIDATE** — The provider nominated a private prototype route that makes the
worker/painter boundary concrete without freezing the public Painter API:

- The worker exclusively owns mutable HarfBuzz/FreeType engines and faces.
- The prepared result owns copied text and `ShapedText` glyph/run arrays, plus
  an immutable encoded-font lease and table mapping result-local face IDs to
  exact encoded bytes and face index. Those IDs are not current-provider globals.
- The painter creates its own mutable raster faces from the retained encoded
  bytes on its own executor. It draws the prepared glyph positions without
  reshaping and never shares worker `FT_Face` or `hb_font` objects.
- The encoded-byte lease may outlive the worker. Each executor destroys its own
  native faces. Revoked generation authority still forbids stale publication or
  current paint even while bytes remain alive for safe release.

This route resolves the proposed ownership boundary for the private experiment;
it does not establish implemented thread safety or glyph/paint parity. Exact
font bytes, face index and effective font configuration must remain consistent
with the prepared identity. The prototype must verify result-local lookup and
glyph-ID/position compatibility on the paint path, reject missing dependencies,
and preserve previous coherent presentation on failure. The existing shaping
receipt does not establish complete-vs-resource-failed atomicity.

The provider confirms active geometry plus one replacement slot, including
cancelled results until release, one coalesced desired intent and full-generation
publication. On close, admission stops; the worker completes any noninterruptible
shape call, drains/releases and joins before engine/service destruction. Worst
close latency must be measured; no instantaneous cancellation is claimed.

The concrete measured-capacity experiment accounts for controlled input/output
and immutable font owners separately from opaque worker and painter caches.
The independent paint faces may add memory beyond the shaping probe's known
font bytes and returned arrays; those additions must be measured or marked
unknown. Numerical byte/count profiles and simultaneous lifetime totals remain
to be reconciled from prototype evidence. Opaque quotas are not a prerequisite
for this authorized experiment and are not claimed by it.

Provider attribution and private prototype evidence are the next review input.
No public layout seam, installed capability or final native editor availability
is admitted by this records-only reconciliation.

## Native phase receipt 002 — historical attribution before reuse

**MEASURED** — The provider's
[phase-attribution receipt](../../gui_forms/experiments/TEXT_LAYOUT_PHASE_ATTRIBUTION_2026-10-01.md)
and [raw phase data](../../gui_forms/experiments/TEXT_LAYOUT_PHASE_ATTRIBUTION_2026-10-01.csv)
use the baseline environment, fixed fonts and generated inputs in a separate
Release diagnostic build. Orchestrator checked the receipt and relevant CSV
phase rows without rerunning the experiment. Arithmetic phase means cover 31
warm calls; they are not the main-row medians.

For mixed bidi, append total averaged 368.534 ms, including HB shape 245.441 ms
and glyph output 96.632 ms; intersections averaged 3.572 ms despite 3,149,313
intersection pairs. For emoji/fallback, append total averaged 205.633 ms,
including HB shape 148.270 ms and glyph output 37.811 ms; intersections averaged
0.424 ms. Append includes its subphases and native owner destruction, so these
numbers must not be summed as disjoint costs. Glyph output includes fallback
extent calls and result updates, not just array copying.

This narrowed the tested bottleneck evidence toward per-run work. At this phase,
reusing native font/buffer objects was a **HYPOTHESIS**, not yet a measured
speedup or selected implementation; receipt 003 below records its later outcome.
Instrumentation overhead and background
load are not isolated; these samples do not replace the uninstrumented baseline.
At that stage, full outside-timing geometry comparison was still required before
accepting an optimization; the initial repeatability checks did not supply it.

The diagnostic is OFF by default. Its receipt records focused ON/OFF tests and
source-equivalence review after removing diagnostic blocks, not binary identity
or global production house-style compliance. Opaque allocations, independent
paint caches, cancellation latency, failure atomicity and public layout
availability gain no new guarantee from phase attribution.

## Combined receipts 003/004 — reviewed reuse and private worker proof

**OBSERVED** — The coordinator integrated reviewed per-call reuse at `aa1f8d4`
and the private worker lifecycle slice at `dcadfe2`. Orchestrator read the
[reuse receipt](../../gui_forms/experiments/TEXT_LAYOUT_CALL_REUSE_2026-10-01.md)
and [worker receipt](../../gui_forms/experiments/TEXT_WORKER_LIFECYCLE_2026-10-01.md),
including the confirmed worker CSV; it did not rerun the experiments or perform
the implementation source reviews attributed to the provider/coordinator.

**MEASURED** — Before optimization, the provider captured full serialized
geometry for 27 cases across nine inputs and three font configurations. The
baseline and final failure-checked reuse output matched all 12,043,216 bytes;
the receipt supplies the durable compressed golden and hashes. This covers
ordered run/glyph fields, positions, advances, metrics, exact fonts and missing
coverage on the pinned fixture set. It is not native paint parity or universal
shaper correctness. Per-call reuse is now implemented and reviewed in this
private path, superseding its merely hypothetical status in receipt 002.

The same-profile comparison's mixed-bidi median fell from 249.801 to 79.2671 ms;
emoji/fallback fell from 130.591 to 26.4464 ms. Final worst values were 93.4156
and 65.9451 ms respectively. Negative results remain: ASCII 16 KiB median rose
from 3.5487 to 3.8719 ms and enormous-grapheme worst rose from 2.4172 to 13.1278 ms.
The earlier 894.725 ms bidi tail is preserved in its original baseline, not
substituted for the paired comparison. Background load was not isolated.

Reuse retains up to 64 lazily acquired HB fonts and one fully reset buffer for
one shape call only; no persistent cache is introduced. All call owners unwind
before return. Allocation acquisition timing and failure behavior deliberately
changed. Six diagnostic cases test empty/unbound HB objects, buffer rejection
and real shaping failure, old-result preservation and same-engine recovery.
They do not prove real allocator-exhaustion coverage or leak counts. The receipt
records final full-geometry equality and ON/OFF tests after these corrections.
Opaque native memory remains unknown and more font caches may coexist per call.

**MEASURED** — The standalone worker proof exercised one occupied queued/running/
ready slot, cancellation without release, stale and failed completion preserving
old display, coalesced intent, recovery, and close before start/idle/ready/running
plus repeated close. A checked private authority epoch fixes cancel followed by
an identical desired identity reviving an old result. Full identity and epoch
must both match. Epoch exhaustion was source-reviewed, not directly executed.

Both raw worker runs remain linked from the receipt. The confirmed run serviced
seven foreground ticks, six observing running work, with maximum gap 18.9779 ms,
pending elapsed 107.543 ms and synchronous close/drain/join 90.269 ms. This is a
foreground counter/sleep harness, not a native window/input/paint test. Close can
block the foreground and native shaping remains noninterruptible. Final typed
borrow cleanup followed the two measurements; pre/post source hashes and rebuild/
syntax results are recorded without pretending those runs sampled the final hash.

The post-close result retained 16,384 text-capacity bytes, 4,096 run-capacity
elements, 10,752 glyph-capacity elements and 1,314,192 shared encoded-font bytes.
These mixed units are not a total byte budget, peak/RSS measurement or hard quota;
owner overhead, native temporaries/caches and peak simultaneous output are excluded.
No painter is implemented by the worker proof. It admits scale 1/no wrap and
tab metadata only, and does not establish D1 mappings or source-grapheme proof.

The inspected Shadow application profile and `tools/Build-Windows.ps1` disable
HarfBuzz. These receipts therefore establish private-engine/prospective-view
progress, not a measured improvement in deployed File Manager or SwiftEdit.
Their exact-scope house-style reviews do not certify legacy or vendor code.

The next authorized private scope is paint-side font/glyph/configuration parity
using independently owned faces and prepared positions without reshaping.
Compatibility refusal, public contract reconciliation, native host behavior,
complete resource accounting and matched SDK/consumer adoption remain open.
No installed, runtime or native-UI availability is promoted by these receipts.

## Paint receipt 005 — private ownership and position transfer

**MEASURED** — The provider's
[private paint receipt](../../gui_forms/experiments/TEXT_PAINT_PROBE_2026-10-01.md)
and [final output](../../gui_forms/experiments/TEXT_PAINT_PROBE_2026-10-01.txt)
record byte-equal 102,400-byte grayscale surfaces, with 1,369 nonzero pixels and
seven runs, from worker-prepared geometry and an independently synchronous
shaper. Both shapers were destroyed before independently owned paint-side FT
faces were created from retained immutable font bytes. Both paths use the same
private raster implementation. Orchestrator checked these records without
rerunning the fixture; this is ownership/position-transfer evidence, not an
independent raster oracle or Skia/GDI/native-window parity.

The fixed fixture uses static/default variation, scale 1, 72 DPI and matching
26.6 sizing, `FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP`, default hinting and normal
outline-to-256-level-grayscale rendering with positive pitch. It consumes
prepared positions without reshaping. Font bytes, local face/index/style,
effective identity and authority epoch are checked; complete staged pixels
replace the previous surface only after success. These settings and the private
status numbers are not a selected public raster profile or protocol.

Fourteen executed refusal cases preserve old pixels and identity, covering
missing coverage/lease, revoked authority with live bytes, provider/font/config
mismatches, face/font/glyph incompatibility and invalid geometry. The real
mixed-text input containing `a` followed by U+0301 has one missing cluster and
is retained as a refusal fixture, with its
[initial failed expectation](../../gui_forms/experiments/TEXT_PAINT_PROBE_initial_refusal_2026-10-01.txt).
The fully covered positive case did not erase that reported limitation. The
subsequent [canonical-pair correction](../../gui_forms/experiments/COMBINING_ACUTE_2026-10-01.md),
integrated at `8528f27`, established a nominal-cmap false negative before
HarfBuzz composition and corrected the bounded two-scalar case. The historical
failure remains evidence; the updated paint fixture accepts that original input
and retains isolated U+0301 as a genuine unsupported case. All saved geometry
fields except three declared missing-coverage counters remained identical.

Negative-pitch/color/bitmap refusal and native/allocation-failure cleanup were
source-reviewed, not exercised by the final fixtures. No paint latency or leak
measurement is supplied. FT bitmap allocation precedes the bitmap-size guards;
those guards provide indexing safety, not pre-allocation quota proof. Controlled
staging pixels do not bound opaque FT caches, temporaries or peak memory.

Concrete remaining public semantics include font variation/load/hinting/raster
profile identity, lease lifetime versus revocation authority, face/glyph
compatibility and refusal outcomes, and separately declared native resource
guarantees. Production integration, public API reconciliation, native host/UI
evidence and matched SDK adoption remain open. No production renderer choice or
runtime availability follows from this private proof or its exact-scope source
review against `planning/PROGRAMMING_HOUSE_STYLE.md`.

## Concrete D3a public-consumer proposal — candidate 001

The next semantic proposal is
[GUI_PREPARED_TEXT_DEVELOPMENT.md](../spec/contracts/GUI_PREPARED_TEXT_DEVELOPMENT.md).
It replaces further general experiment planning with named admission, ownership,
revocation, asynchronous completion, record/replay and failure laws for one
bounded complete paragraph. It is a candidate for matched-source development,
not a frozen API or runtime availability entry.

Provider source feasibility identifies Window service attachment with explicit
revocation, host-owned service lifetime, Painter unsupported default, and typed
retained display commands/replay. A unique public handle needs private shared
immutable storage because recorded commands outlive the wrapper; their retained
bytes and revocation checks are part of the contract. Recorded acceptance and
successful pixel/frame commit are separate outcomes.

SwiftEdit explicitly accepts visual-only read-only intermediate validation:
source selection is preserved and source-based copy remains separate. Pointer
selection, caret geometry, wrapping/tab layout and editing are not claimed by
this first slice. The current Windows DIB painter cannot consume HB-prepared
glyphs; the consumer requires unsupported until a production adapter is proved.
The private FT fixture neither selects that renderer nor substitutes for it.

Exact controlled byte capacities, bounded display-chunk retention, production
transactional paint route and readiness/close dispatch integration need provider
reconciliation before source freeze. The candidate names these implementation
scopes. The later route-A development selection in that candidate records
coordinated A1 host transaction work and A2 prepared-text integration under
existing implementation authorization; it does not imply installed availability.
