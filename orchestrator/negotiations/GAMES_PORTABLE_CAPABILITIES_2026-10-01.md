# External Games portable capabilities — intake 001

Status: **concrete consumer requirements; dedicated provider scope being
coordinated; no new runtime capability or frozen interface**.

## Provenance and boundaries

**OBSERVED** — Coordinator chat `01a0f009-7508-78a2-9fd4-cbf544e9193d` routed
Games chat `01a0f6b6-22cd-7b33-9b8f-293c735cf84f` here. The latter supplies the
owner's direction for broad portable C++ cleanup, repository builds/releases
and Windows/macOS/Linux support. Its quoted direction permits minimal necessary
Apple adapters inside GUI.Forms, with each raised to the owner. The exact turn
locator remains requested; this records the chat's supplied provenance.

The active consumer is `C:/Users/Shadow/games`, reported initial checkpoint
`12740c4`. This does not silently open this repository's planning-only `games/`
implementation gates. No GPU work or File Manager scope expansion is implied.
Orchestrator owns shared semantics; Games owns consumer/assets/build tooling.
Frozen SDKs remain intact; shared additions need a new matched validated SDK.

## Bounded needs and source locators

| Topic | Consumer requirement | Existing consumer locator / disposition |
|---|---|---|
| Image | Owned top-left CPU pixels, explicit dimensions/stride, premultiplied BGRA8, at most 64 MiB decoded | `src/puzzle_image.mm` currently resizes PNG to 1024x512 RGBA. Games can pre-bake the single environment asset and validate exact dimensions/bytes at runtime; no runtime decoder need is implied by that route. |
| Text mask | Owned top-left one-byte grayscale coverage from UTF-8 plus explicitly bundled licensed font/weight/size/wrap; metrics and typed missing-coverage/error | `vendor/eggy/src/text.mm` uses Apple text/fonts, with non-antialiased pixel and antialiased speech modes. No Apple fallback; actual wrap/profile/limits still need reconciliation. |
| Audio | Owned stereo 48 kHz PCM, exact frames <=600 seconds per clip, bounded voices, loop bounds, gain/rate/pause/stop, device lifecycle and inspectable failure | `src/audio.mm`, `src/audio_pcm.hpp`, `vendor/eggy/src/audio.mm`; Games prefers packaged lossless PCM over runtime AAC decode. Callback must not allocate; device loss may degrade UI silently while exposing status. Choose exact float/PCM16 representation and device policy before freezing. |
| Presentation | Existing CPU LiveSurface instead of direct Apple presenter | `vendor/eggy/src/eggy_view.cpp:468` onward. Games owns consumer migration and backend/SDK validation; no new GPU surface. |

Source paths in the table are relative to the external Games checkout. Games
requests synchronous typed failures for bounded asset loading, with owned
results and explicit caller lifetime; no background decode/shape cancellation
is requested for that operation. Playback/voice/device stop and shutdown remain
separate lifecycle requirements. These topic names are not registry IDs.

## Concrete ownership proposal and remaining reconciliation

Games chat has offered to implement separately owned new GUI.Forms service files
without diverting the active File Manager provider. The coordinator confirmed
use of the current `C:/Users/Shadow/file_manager` checkout on `codex/native-dogfood`
with disjoint ownership: no separate checkout, worktree or branch switch required.
Root integrates File Manager commits/push; Games commits its own repository.

- Games owns asset preparation, portable consumer migration and acceptance
  fixtures now. Build-time PNG/PCM preparation can remove runtime codec needs.
- The coordinator assigns Games chat the dedicated new audio-files/test scope,
  contingent on its exact new path list and minimal PCM/API/device-failure/
  shutdown reply. Those details are being requested before files become active.
  Implementation may follow that agreement without another general user approval.
  Existing CMake/common-host hunks remain reserved for coordination. No native
  adapter or audio architecture is selected solely by this file assignment.
- GUI.Forms owns reusable typography/raster semantics. A new text-mask adapter
  must reconcile with the prepared-text work to avoid duplicate font/raster
  ownership. The private FT paint proof is not a public text-mask service and
  provides neither wrapping nor general Unicode coverage. Dedicated mask files
  can be assigned after the common boundary and backend are agreed.
- Orchestrator reconciles exact sample/frame units, typed results, failure
  preservation, limits, leases/voice stop and device shutdown, then records
  development contracts before adapters freeze. The coordinator integrates
  shared changes and validates a new SDK against Games independently.

The Stillwater C++ AudioQueue example is offered as source research, not an
accepted adapter. Any necessary Apple adapter follows the reported owner route.
All first-party implementation, tests and tooling require exact-scope review
against `planning/PROGRAMMING_HOUSE_STYLE.md` in addition to functional checks.

Games reports its full build blocked on toolkit services while independent
consumer portability work proceeds. Availability on each backend remains
unverified. This intake assigns semantic/consumer ownership and a concrete
provider-scope proposal; it does not manufacture an existing service or replace
the required shared-file coordination.

## Scope and semantic reply 002

**OBSERVED** — Coordinator confirmed Games ownership; the following new paths
were checked absent and reserved to Games chat in the current shared checkout:

- `gui_forms/include/gui_forms/audio/audio.hpp`
- `gui_forms/src/audio/audio.cpp`
- `gui_forms/tests/audio/audio_service_tests.cpp`
- `gui_forms/cmake/Audio.cmake`

The nested test path replaces the earlier proposed flat test filename. Root
retains existing root-CMake/common-host hunks and commit/push integration.
Independent `GUIForms::Audio` and an optional `GUI_FORMS_BUILD_AUDIO` CMake
switch are the proposed build boundary, pending exact integration hunks.

Development semantics agreed in the exchange, not a frozen ABI:

| Object/operation | Minimum meaning |
|---|---|
| Clip | Immutable owned interleaved stereo 48 kHz finite float32, 1 through 28,800,000 frames; maximum 230,400,000 payload bytes before overhead |
| File input | Only same-rate/channel PCM16 or float32 WAV; unsupported/compressed formats refuse; exact input-length and decoded-size checks before allocation |
| Voice | At most 64 occupied slots, shared immutable clip lifetime; generation prevents stale slot reuse; whole-clip looping, with explicit consumer trim before creation |
| Controls | Finite gain 0..1, finite rate ratio 0.25..4; play resumes, pause preserves cursor, stop halts and rewinds; destruction releases voice |
| Offline | Explicit offline engine writes caller-owned interleaved span; checked frames-times-two extent; never concurrent with physical-device render |
| Device failure | Typed initialization/runtime status, inspectable by consumer; no implicit offline/null-device success or silent automatic reopen |
| Shutdown | Stop admission and playback, quiesce/join native callback before releasing owners; no audible drain required; surviving handles return closed and retain no device authority |

Public controls have one owning thread. Native callback work must be bounded,
allocation-free and nonblocking, with no I/O or allocated error message. The
consumer requests no public streaming queue. Internal command synchronization,
maximum callback work, mix clipping/interpolation/EOF behavior and aggregate
clip/workspace bounds still need concrete review. Shared clips count once;
64 voices neither implies 64 full clip copies nor proves a process hard quota.
Synchronous file-load input bound and cancellation behavior remain explicit
open items; absence of a public streaming queue does not bound file reads.

**CANDIDATE dependency only:** Games proposes miniaudio with WAV-only decode and
native device adapters hidden privately. Path reservation does not admit or
authorize fetching that dependency. Exact immutable version/commit, SHA,
license, new dependency paths and restricted build macros are required, along
with source-backed enabled-backend/thread behavior and callback guarantees.
The coordinator owns dependency admission. No codec other than the agreed WAV
input or necessary Apple adapter is inferred from this proposal.

The coordinator's independent upstream license check identifies dual Unlicense
or MIT No Attribution (MIT-0), not ordinary MIT, at the reported `9634bed...`
revision. Full pin/header hash and restricted backend/build evidence remain
required before dependency admission. Root owns root-CMake option/include and
optional Audio package-component wiring. Validation must include an independent
installed Audio consumer and explicit refusal when an absent Audio component is
requested with REQUIRED, using a new SDK prefix.

Prepared text-mask integration waits for common types and a proved production
raster route; Games must not duplicate the active FT font/cache/lifecycle service.
Pre-baked PNG and PCM consumer assets remain useful independent progress.

### Dependency/profile refinement and coordinator admission

Games replaced the earlier master-revision proposal with reported miniaudio
0.11.23 commit `f40cf03f80cdb7e741d43e53b7e706e8c1394bcf`, using private
FetchContent build storage `_deps/gui_forms_miniaudio`, not vendored source.
License and hash must be checked at this exact revision; the preceding license
check of a different revision cannot establish it. Dependency fetch/admission
was subsequently granted by the coordinator as recorded below.

The revised proposal disables miniaudio decoding, encoding, resource manager and
generation; only platform-selected WASAPI/CoreAudio/ALSA device backends remain.
First-party C++ loads bounded little-endian RIFF PCM16/float32 itself. Proposed
input cap is 256 MiB, decoded payload <=230,400,000 bytes and shared aggregate
clip capacity <=512 MiB charged once. Sixty-four occupied voice slots include
paused/stopped voices; no queued commands or streaming frames are proposed.
Named callback mixing divides work into <=4,096-frame chunks, linearly
interpolates bounded pitch and clamps mixed output to [-1,1]. Controls are
synchronous on the non-callback owning thread; loading claims no cancellation.

These finite choices advance the profile but do not prove callback safety:
private synchronization, backend allocation/waits and quiescence before owner
release still require source review. Chunking does not alone bound the total
frames supplied by a native callback. No format/device availability is promoted.

**OBSERVED coordinator admission:** root independently verified miniaudio
0.11.23 at the full commit above, header SHA-256
`7e4f3f13c8fe66df2080ac3dd12a89193e3c2463cb7f067c798abd7331cd8ee6`,
and dual Unlicense or MIT-0 (not ordinary MIT). Immutable build-local dependency
fetch and the scoped new audio files may proceed. Disabling engine/node-graph
facilities is conditional on their being unused. Games subsequently proposed
an offline `ma_engine` graph feeding the device callback, so those facilities
may remain enabled if that implementation requires them; its synchronization,
allocation and retirement behavior still needs source review.
This admits the private device dependency for development, not a public decoder
or installed service. Root retains shared build/package integration.

The remaining concurrency question is explicit race-free handoff between
single-threaded controls and the native mixer without callback allocation or
blocking. Last clip references must retire on a non-callback executor; a shared
owner alone does not guarantee that destructor placement. Games must specify
the handoff/retirement design before claiming callback safety. Bounded synchronous
WAV loading belongs off UI/audio execution; shared-clip quota is separate from
read/conversion peak storage and process RSS. Development may progress within
the reserved scope while these guarantees receive source review.

## Additional Switchbox interaction intake

Games reports Switchbox remake integration at `de23ab8` and shared text-mask
needs matching Eggy: six bundled roles, left word wrapping, explicit newlines,
approximately 0.05em additional line spacing, and monochrome versus grayscale
coverage by role. Exact metrics/rounding remain to reconcile with the shared
text provider; this report does not establish mask availability.

The coordinator separately requested the minimal
[window cursor proposal](GAMES_WINDOW_CURSOR_2026-10-01.md) for scoped native
hide/restore and finite client-DIP warp. No implementation assignment is made
while Windows host A1 hunks remain owned by the active provider. Transparent
cursor appearance, native hiding, capture and warping remain distinct.

## Shutdown checkpoint — codec extension remains candidate

**OBSERVED** — Games reports a later owner direction permitting a small shared
portable decoder for compact releases. Its initial FLAC proposal was superseded
by a Vorbis comparison candidate after reported asset-size comparisons. The
proposed source is the pinned miniaudio tree's independent `extras/stb_vorbis.c`;
the engine's `MA_NO_DECODING` configuration and existing WAV foundation remain
separate. Vorbis is lossy; smaller output alone does not establish acceptance.

No decoder admission or edit permission was granted in this shutdown turn.
Outstanding: exact new paths, immutable source/hash/license, persisted format/
size comparison, actual coverage of the proposed 16 MiB arena, bounded owned
encoded input <=64 MiB, actual decoded frames <=28,800,000 independent of header
claims, finite stereo 48 kHz output, shared clip-budget reservations, typed
malformed/truncated/failure results, background-only loading, bounded-chunk
cancellation, prepublication cancellation and shutdown/drain cleanup. A partial
clip must never publish on failure. Opaque/stack/temporary memory cannot be
assumed covered by the arena without source evidence.

Coordinator owns dependency/codec admission and shared repository checkpoint.
Owner-directed imminent shutdown stops further record edits after handoff;
this checkpoint does not open decoder implementation or widen availability.

## Resume — conditional compact-decoder development authorization

**OBSERVED** — After Games received direct owner resume and compact-decoder
direction, the coordinator authorized development in its existing four reserved
audio files: `include/gui_forms/audio/audio.hpp`, `src/audio/audio.cpp`,
`tests/audio/audio_service_tests.cpp` and `cmake/Audio.cmake`, all relative to
`gui_forms/`. This supersedes the shutdown no-edit state for that bounded work;
it does not grant unrelated host edits or runtime availability.

Dependency edits are conditional on independent verification of the pinned
`extras/stb_vorbis.c` bytes and license. Games reports stb_vorbis v1.22 at the
existing miniaudio commit `f40cf03f80cdb7e741d43e53b7e706e8c1394bcf`, LF-byte SHA-256
`4c7cb2ff1f7011e9d67950446b7eb9ca044f2e464d76bfbb0b84dd2e23e65636`,
and MIT alternative A in Sean Barrett's 2017 license trailer. These remain
provider reports pending review of the independent verification receipt; the
previously verified miniaudio header/license does not verify this separate file.

Games also reports 243 lossy Vorbis q6 candidates totaling 54,735,409 bytes,
compared with 1,169,418,011 bytes of prepared PCM. Those figures are attributed
reports, not independently reviewed measurements in this record. They motivate
the compact-release work without establishing decoding correctness, audio
acceptance, release publication or installed availability.

Implementation must preserve the existing WAV foundation and satisfy bounded
memory, cancellation and failure tests: admitted encoded/decoded capacities,
actual frame-count checks, explicit arena coverage and excluded storage,
finite sample/channel/rate validation, no partial clip publication, cleanup on
malformed/truncated/cancelled/resource failure, and background-owner shutdown.
No audio/UI-thread decoding or hidden asynchronous owner is admitted. Report
exact authored source review against `planning/PROGRAMMING_HOUSE_STYLE.md`
separately from scanner/test results before integration.

Games retains audio implementation and consumer ownership; Orchestrator edits
only the relevant negotiation records. Coordinator owns shared integration.
Cursor remains a proposal with no implementation assignment. A2's public
prepared-text/mask service remains unavailable. SwiftEdit owns the separate SDK
workflow/export-packaging slice; that assignment introduces no runtime changes.

### Subsequent cursor development assignment

The coordinator subsequently assigned Games the first Windows cursor slice
after stable audio handoff; see
[cursor scope and pending API reply](GAMES_WINDOW_CURSOR_2026-10-01.md).
This supersedes the preceding unassigned cursor status only. Scope is the new
lease/policy and cursor-only host/Window integration with focused tests, excluding
committed A1 raster/text and retaining root CMake ownership. Other backends stay
unsupported; exact public API/status reconciliation precedes adapter freeze.
Development assignment does not establish native or installed availability.

## Vorbis development evidence receipt — review 001

**OBSERVED** — Orchestrator read the provider's initial local receipt and then
the durable [Audio Vorbis receipt](../../gui_forms/experiments/AUDIO_VORBIS_2026-10-01.md).
It independently matched all four audio source/CMake SHA-256 values against the
receipt. Implementation commit is `00dca7c6d057cd9a0919ea33ae7c6abef53631cb`;
the implementation source hash is
`a6771e87a44a34d5f4253a2a314474ad3adcfab5b963f88f9279729d2b474728`.
The durable receipt supersedes the Git-ignored `astra` reference and records
reproduction instructions and captured source 1/1 and installed-consumer 4/4
results. It was added after the implementation commit under the coordinator's
separate evidence scope; no evidence commit identity is asserted here.
This review did not rerun tests, repeat the vendor audit or certify implementation
house style. The provider's receipt supplies its exact authored review scope.

**MEASURED, provider receipt** — An Audio-enabled, tests-disabled library was
installed into Games' separate development Audio SDK. Its independent consumer
CTest passed 4/4: public API, 243 WAV assets, 243 Ogg assets with all 17 exact
loop frame/seam checks, and Games mixer-policy cases. Six adapter translation
units compiled against imported `GUIForms::Audio`. Corpus output is 54,735,409
Vorbis q6 bytes versus 1,169,418,011 prepared PCM bytes. Q6 is lossy; zero playback
seam error does not establish perceptual equivalence to original source audio.

The receipt records pinned-source hash/license checks, fixed 16 MiB arena,
<=64 MiB owned encoded input, actual-frame/finite-sample validation and shared
512 MiB clip reservations before output allocation. Container CRC/sequence/
continuation/EOS validation does not replace packet validation; malformed and
truncated inputs refuse without partial publication. Overshoot is explicitly
saturated to [-1,1]. Source tests cover cancellation, arena/quota exhaustion,
reservation release, old-owner preservation/retry and concurrent decode checks;
private failure seams are absent from the installed library.

Important bounds remain explicit: codec scalar/local arrays and the fixed output
block are stack storage outside the arena. The audited include-boundary sort
replacement avoids a possible libc-qsort allocation; vendor bytes are unchanged.
Opening/initial pumping is serialized to handle the selected decoder's global
CRC initialization. Codec open/decode calls and waiting for that mutex are
noninterruptible. Cancellation checks occur between bounded reads, pages, decode
requests and publication; legacy WAV cancellation surrounds the whole WAV call.
These are not instantaneous cancellation or whole-process memory guarantees.

The Games loader is separately consumer-owned: reported queue 32 plus one active
job, one worker, per-slot cancellation and drain/join on destruction. That queue
does not alter the shared audio foundation's absence of streaming commands.
No physical-device listening, complete Application SDK, macOS/Linux validation
or release publication is claimed. Cursor implementation still follows stable
audio handoff; A2/text-mask readiness is unaffected by these decoder results.

The durable receipt also makes explicit that shared clip payload accounting
excludes encoded inputs, per-load arenas, stream/control allocations, stacks,
backend state and allocator overhead. Applications must independently bound
concurrent loads; Games uses one loader thread. This is neither a total-process
memory limit nor a hard real-time guarantee.

### Independent coordinator follow-up — corrected source

Review 001 above preserves the original `00dca7c` source/hash snapshot. The
coordinator subsequently requested explicit `std::barrier<>` spelling and a
named sample count outside the conversion loop, plus receipt encoding cleanup.
The corrected source hashes are:

- `src/audio/audio.cpp`:
  `23091b14e7c6b889f375964cb2f5839e598ccf96c71897d90f20e23fccbd6cd2`.
- `tests/audio/audio_service_tests.cpp`:
  `658dc474539655c09f7aab9ee150b712486328a81479791bdc98f03cf319a7de`.

Orchestrator independently matched these two current file hashes against the
updated receipt. This does not rewrite the earlier snapshot or imply its test
run used the corrected bytes.

**MEASURED coordinator follow-up:** before correction, independent source CTest
passed 1/1 in 0.45 seconds and installed consumer 4/4 in 12.72 seconds. After
correction, root rebuilt and passed source 1/1 in 0.34 seconds, rebuilt the
tests-disabled library, installed the Audio SDK, rebuilt the independent consumer
and passed 4/4 in 10.91 seconds. These are coordinator reruns, not Orchestrator
reruns. The durable receipt is being updated with this distinct provenance.

Root reviewed the complete authored implementation/header/CMake/test delta and
the two corrections and reported no remaining blocking finding in that exact
scope. This is not vendor, legacy, whole-platform or physical-device listening
certification. Games owns the follow-up code/evidence commit; root integrates
the negotiation batch. Cursor reply 003 now reconciles precise status, unique
window metrics and terminal lease behavior for its assigned development scope;
native/installed cursor evidence remains pending.

### Subsequent source evidence and scheduling boundary

Cursor source development is integrated at `50ea223`, with receipt follow-up
`87ce23e`; see [cursor receipt 004](GAMES_WINDOW_CURSOR_2026-10-01.md).
Focused tests and authored source review do not establish visible native or
matched-SDK availability. Other cursor backends remain unsupported.

Games' subsequent audio-scheduling request is research/proposal only. No
implementation scope is assigned until sample-frame time units, loop boundaries,
late-command policy, cancellation, rate changes and bounded command/storage
ownership are reconciled. Existing playback/decoder authorization does not
silently admit scheduled-start or sample-accurate timeline APIs.

The concrete next-bar request now has a dedicated
[proposal-only intake](GAMES_AUDIO_BAR_TRANSITION_2026-10-01.md), owned by
Orchestrator. Games supplies exact API/profile details; no scheduling source
or SDK mutation is assigned by reserving that record.

Subsequent coordinator admission opens only the dedicated private Stage 1
scheduler/model/tests/receipt scope recorded there. Clarification 004 resolves
its bar-grid, timeline, fade, cancellation and retirement model laws. Public
Audio APIs, miniaudio-node integration and concurrent callback guarantees remain
unadmitted Stage 2 work; model success cannot establish those properties.

The coordinator subsequently reviewed/integrated Stage 1 at `8e5368f` and opened
the exact Stage 2 development scope recorded in the dedicated negotiation.
This supersedes the prior unadmitted implementation state, not the requirement
for actual PCM, concurrency, ownership and shutdown evidence. API freeze, device
playback and SDK publication are not authorized by that development assignment.

The later Stage 2 coordinator review accepts the corrected transport for
continued source-build development integration only; the dedicated negotiation
links its review and ON 2/2 / OFF 1/1 independent evidence. Pending polls now
reflect parent engine backend failure while preserving receipt phase/timing and
completed outcomes. Development-ON installation still refuses. No installed SDK,
native listening or macOS/Linux runtime availability follows.

## Candidate mask extension — existing text negotiation

**OBSERVED consumer request:** Games needs pixel-width-limited word wrapping,
hard newlines and monochrome pixel-font masks in addition to the current
grayscale paragraph profile. It identifies encoded serif text for Four Pegs and
condensed sans text for Atom Probe. Masks would be consumed through existing
LiveSurface, without depending on typed Painter readiness. This does not prove
that the requested mask generation is available.

These are **CANDIDATE** extensions of the existing shared text-mask negotiation,
not a selected layout/raster algorithm or a new architecture. The current A2
single-paragraph/no-wrap/no-newline/grayscale scope is unchanged. The active
GUI.Forms text provider retains shared source ownership; no parallel Games text
implementation assignment is granted. Games owns consumer requirements and
fixtures and may continue its separate LiveSurface integration.

The requested concrete reply must state width units and pixel/DIP/scale rounding;
hard-newline sequences, CRLF, empty/trailing-line behavior; word-break whitespace
retention and long-word overflow; cache keys, invalidation, capacity and ownership;
and exact mono coverage/raster semantics. Thresholding a gray mask is not silently
equivalent to a monochrome font-rendering request. Bundled font roles, exact
encoded-font fixtures, licenses and expected mask/advance/bearing results are
needed for the serif and condensed-sans cases. The earlier approximate extra
line-spacing request also needs an exact conversion/rounding rule.

Provider feasibility, bounded context/work/storage and explicit failure behavior
must be reconciled before an implementation choice. No feature, SDK or runtime
availability is promoted by this candidate intake.

### Consumer mask fixtures — concrete candidate reply

**OBSERVED** — Orchestrator read external Games files
`C:/Users/Shadow/games/docs/PORTABLE_TEXT_REQUIREMENTS.md`,
`tests/fixtures/portable_text.json` and `assets/fonts/manifest.json`. The latter
two paths are relative to that checkout. Local font/license file hashes match
their manifest; this checks supplied bytes, not independent upstream license
interpretation or accepted renderer coverage. The manifest records upstream pins
and OFL notices; numeric mask goldens remain explicitly pending the shared raster.

The consumer proposes pre-DPI logical mask-pixel units, double layout arithmetic
and one-time nearest 1/64 size/width quantization with ties away from zero. Zero
width means no soft wrap. It requests identical logical line breaks at scales
1, 1.25, 1.5 and 2. This invariant needs provider feasibility: current device-size
quantization/hinting must not be assumed to produce it automatically.

Requested line semantics preserve LF, treat CRLF as one break and normalize
isolated CR at the adapter boundary; explicit empty/trailing lines contribute
height. Leading/repeated spaces retain advance. Soft-wrap spaces may be consumed
without ink at the next line start. No automatic hyphenation/ellipsis; long-word
fallback must respect extended grapheme and shaping boundaries. An indivisible
overwide cluster remains intact with explicit overflow, leaving clipping to the
compositor. Tabs may refuse. Additional line gap is proposed as 0.05 times size,
with baseline, logical extent and signed ink bounds reported separately.

Mono means matching monochrome glyph loading/hinting and 0/255 coverage expanded
to gray8 storage, not thresholding grayscale output. Requested masks retain
overhangs/accents, signed origin and stride; an optional transparent border cannot
clip to the logical advance. Pixel roles use integer-scale mono, others grayscale.

Consumer candidate limits are 16 KiB input, 4,096 logical pixels per axis,
256 lines and 16 MiB mask storage; cache <=700 entries/32 MiB per active game,
keyed by text, face revision, size/width/spacing, raster profile and scale.
Eviction must not invalidate a composed frame; detach drops cache authority and
stale work cannot revive it. These requests do not override A2's current 4 MiB
mask/two-mask 8 MiB limits. Physical raster dimensions at scale, retained output
owners and cache/working peaks need an explicitly reconciled profile.

Encoded-face candidates are Libre Baskerville regular/bold for Four Pegs,
Barlow Condensed regular/bold for Atom Probe, Comic Neue for Switchbox, Cousine
for pixel roles and Carlito for neutral UI. They remain independently registered
consumer faces, not silent replacements for toolkit-wide roles. Fixtures include
actual help text/widths, score spacing, CRLF/empty/trailing lines, long words and
combining text. Goldens and ordinary/fractional-DPI help-panel inspection remain
future evidence. No shared-source assignment or feature availability changes.

### Provider wrapped-mask profile — reconciliation pending

**OBSERVED provider report:** the provider read the Games requirements and
fixtures and proposed `logical_wrapped_mask_v1` as a separate profile from A2's
device-sized paragraph path. Standalone wrapping and mono implementation remain
untouched. The provider reports A2 integration frozen for coordinator review;
its reported 9/9 test result in 0.91 seconds is not an independently reviewed
wrapped-mask result or an availability change.

**CANDIDATE:** scale-1 logical, unhinted layout with contextual per-line bidi and
shaping, followed by raster placement scaled exactly once; true FreeType mono
at integer scale is a separate raster profile. Hard/soft-break metadata and
overflow retain source correspondence. This proposes a mechanism for invariant
logical line breaks, not measured proof. Consumer 1/64 quantization, line-gap
rounding, raster bearings and fractional-scale coverage still require explicit
agreement and fixtures.

The provider proposes retaining 16 KiB input and existing font, prepared-payload
and workspace limits while evaluating the proposed 256-line bound. It recommends
an initial 4 MiB per-mask and 4,096-device-pixel axis limit. Supplied widths up to
704 logical pixels do not establish a need for 16 MiB masks. The consumer's
4,096-logical-pixel axis at scale 2 would permit 8,192 device pixels and therefore
conflicts with that initial provider limit; neither axis nor storage limit is
implicitly increased.

A 700-entry/32 MiB cache requires a proposed shared immutable mask lease and
accounting for every distinct live allocation, including the candidate and
evicted masks retained by frames. Consumer LRU eviction alone cannot establish
the memory bound. Key/metadata capacity must have a separate finite bound;
cache bytes, retained output bytes, active/candidate masks and workspace peaks
must be reconciled without bypassing A2's current two-mask/8 MiB scope.

Coordinate units, scaled-axis refusal, individual-mask capacity, all-live mask
ownership/accounting and cache metadata remain open for coordinator, provider
and consumer reconciliation before implementation assignment. This record does
not select the profile or modify A2's accepted development limits.

### Consumer acceptance of initial candidate limits

**OBSERVED consumer reply:** Games accepts an initial 4 MiB mask and 4,096
device-pixel axis limit with explicit refusal. Its supplied fixtures do not
require 16 MiB or a 4,096-logical-pixel axis at scale 2. It accepts the proposed
scale-1 unhinted logical layout, contextual per-line bidi/shaping, one-time
position scaling and true FreeType mono at integer scale as candidate behavior.

Games defines 32 MiB as the total distinct live coverage allocations per session,
including candidates and evicted frame-held leases. Shared reuse counts one
allocation; reclamation occurs only after the last owner. It proposes separate
key limits of 700 records and 2 MiB aggregate retained UTF-8 storage, charging
owning-string capacity, with fixed metadata bounded by record count and bounded
pending requests. No unbounded auxiliary map is implied. The provider may tighten
these limits with explicit refusals; exact pending-request and metadata bounds
still require provider reconciliation.

The requested rounding is size and wrap width quantized once to nearest 1/64
with ties away from zero; additional line gap is quantized once from size times
0.05 and reused for every line. The final profile must specify whether that
formula uses already-quantized size. Per-device rerounding must not change
logical line breaks. This is consumer acceptance of candidate constraints,
not implementation proof, numeric goldens, coordinator selection or a change to
A2's separate two-mask/8 MiB scope.

**OBSERVED consumer clarification:** line gap is
`quantize_1/64(quantized_size * 0.05)`, using the already quantized size. Both
quantizations use nearest with ties away from zero. This closes the consumer's
rounding-input question, pending provider reconciliation and fixture evidence.

Games proposes at most one executing request plus eight queued requests per
session. Additional admission refuses without replacing retained visible masks.
Cache metadata is limited to 700 records and pending metadata to nine records;
all retained UTF-8 keys across cache and pending requests share the 2 MiB
capacity bound. A transient candidate counts toward the total 32 MiB live mask
allocation budget. Completed results retain their bounded request slots until
consumed or cancelled; there is no additional unbounded completion queue.
Provider reconciliation must define slot retirement and actual byte accounting
for fixed metadata and cancellation-held work. These are consumer-acceptable
upper bounds that the provider may tighten explicitly, not assigned source work
or changes to A2's independent replacement-slot and mask limits.

### Concrete provider API reconciliation — development implementation assigned

**OBSERVED:** Orchestrator read the complete provider reply
[`LOGICAL_WRAPPED_MASK_V1_PROPOSAL_2026-10-01.md`](../../gui_forms/docs/LOGICAL_WRAPPED_MASK_V1_PROPOSAL_2026-10-01.md).
The coordinator finds the bounded profile coherent and reports Games accepted
the complete proposal, tighter bounds and lifecycles without a fixture mismatch.
The coordinator now assigns bounded development implementation of this reconciled
profile. Earlier intake proposals above remain history. The assignment is
source-only and default OFF; no global architecture decision, installed SDK or
availability is claimed, and no additional approval or research gate is added.

The four proposed source-only headers are `gui_forms/text_mask.hpp`,
`gui_forms/text_mask/types/text_mask_types.hpp`,
`gui_forms/text_mask/lease/text_mask_lease.hpp` and
`gui_forms/text_mask/service/text_mask_service.hpp`, under the provider's
`include/` directory. They are a concrete ownership/API sketch, not frozen ABI.
`TextMaskService` owns its lifetime ledger, registered fonts and one current
`TextMaskSession`. Session operations are lookup, submit, snapshot, take, discard,
cancel, clear-cache, begin-close and join/release. `TextMaskLease` is a copyable
immutable owner exposing borrowed coverage, exact admitted source, line records
and metrics; a valid zero-ink result differs from an empty handle. Borrowed views
require a surviving lease. Submitted string views are borrowed only during the
call; successful admission owns their bytes. Reusing existing opaque font/wake
types does not authorize cross-ledger font borrowing.

All following bounds apply to the service lifetime ledger, including old-session
and frame-held owners after eviction or close; reopening cannot reset them:

| Resource | Candidate bound and accounting |
|---|---|
| Input and lines | 16,384 UTF-8 bytes/request; 256 lines including empty/trailing lines |
| Requests | Nine occupied slots total across assigned, queued, completed and retiring; one native worker lane and at most eight queued |
| Cache | 700 exact-key records; no unbounded auxiliary index |
| Live masks | 709 distinct objects, including candidates, completed, cached, evicted/frame-held and zero-ink masks |
| Live exact-source keys | 718 owners: up to 709 mask-associated keys plus nine pending inputs; shared allocation counted once |
| Retained UTF-8 | 2 MiB allocated capacity across requests, cache, candidates and lease-retained source until last-owner release |
| Individual mask | 4,096 device pixels/axis and 4 MiB coverage, charging stride times height before allocation |
| All live coverage | 32 MiB, including candidate and evicted/frame-held allocations |
| First-party metadata | 8 MiB actual requested allocation bytes/capacity, including records, retained line arrays and shared-owner control blocks |
| Transient shaping payload | 8 MiB for the single executing job; not retained by completed masks |
| First-party workspace | 16 MiB for the worker, including bounded registration metadata |
| Fonts | Eight faces/bank, 4 MiB/face, 8 MiB aggregate, at most two live banks; retired owners remain charged |
| Native shaping work | At most 2,048 native shaping calls and 4 MiB aggregate submitted UTF-8 context/request; exceeding either returns `shape_work` |

Check sizes, multiplication and alignment and reserve under the ledger lock
before allocation; release after destruction/deallocation. Reservations convert
to live usage without double charging. Metadata includes private shared-owner
allocation requests, not just public handle sizes. Host allocator overhead and
opaque vendor allocations/caches are outside these requested-byte bounds; this
is neither an RSS quota nor a hard latency guarantee. The finite native-work
limit bounds repeated contextual trials without claiming linear runtime.

Admission reserves its slot and input/key/metadata before publishing an ID.
Failure preserves the caller's output ID and prior masks. Coverage admission
occurs later, after measuring ink, before candidate allocation against the live
coverage/object limits. Queue acceptance therefore does not promise a successful
mask: late budget failure is a typed completion preserving prior output. Eviction
can release cache ownership, never a still-retained frame allocation. Exact
cache equality uses normalized options, profile, font bank identity/generation,
primary face, raster/scale and UTF-8 length/bytes; hashes only propose matches.

IDs use a process-wide nonreused session identity and checked request serial,
never a reusable slot index. Completed results/errors occupy their original slot
until take/discard/cancel; no additional completion queue exists. Queued and
completed cancellation retire synchronously. Running cancellation revokes
publication and reports `pending_retirement`: slot, input, candidate/native work
and reservations stay charged until worker acknowledgement and destruction.
Cancellation and completion publication linearize under the slot mutex; cancelled
results cannot enter cache or revive a view. Successful take returns an owning
lease and retires the slot; taking a failed completion preserves prior output
and consumes the error slot. Pending, stale and wrong-executor calls preserve
output and slot.

Close stops admission/delivery, clears cache ownership, retires queued/completed
slots and marks executing work retiring. Join waits for worker retirement and
wake-target quiescence; the target must live until join returns. Replacement
sessions wait for the preceding join, while old leases remain charged/readable
without new-view publication authority. Controls use the opening executor; the
worker posts only a coalesced payload-free wake. Lookup hits use no request slot;
submit hits still occupy a completed slot. Retiring work still excludes another
native job. Nine fixed snapshot entries suffice; no unbounded subscriber list.

The literal-source profile is independent of D1 document identity/proofs.
Offsets reference the exact admitted string retained by the lease. The adapter
normalizes isolated CR; the provider admits LF/CRLF and initially refuses isolated
CR, tabs and other Unicode hard separators. Size is 4–128 logical units, width
0–8,192 (zero disables wrap; positive rounding to zero refuses). Size/width/gap
use the agreed 1/64 quantization; gap derives from quantized size with no final
trailing gap. Grayscale scale is 0.5–4 and mono integer scale 1–4. Validation
ranges do not promise masks fit the device-axis/coverage limits.

Complete-paragraph bidi levels feed line reordering and contextual final-line
shaping; no joining/ligatures cross the chosen break. Greedy legal breaks and
grapheme-plus-shaping-cluster fallback preserve indivisible overflow without
truncation. Consumed whitespace/hard breaks remain represented by source offsets.
Empty input is one empty line with primary-face logical height and no ink.
The logical first-line box top anchors baselines; signed device ink origin and
native bearings remain separate from logical advance. Raster positions scale
once; true mono expands native packed mono to 0/255, never thresholds gray.
Primary face and registration-order fallback are explicit encoded-bank choices.

Games' reported acceptance covers the tighter lifetime, metadata, work and
retirement bounds. The assigned implementation must receive exact
source review against `planning/PROGRAMMING_HOUSE_STYLE.md`, plus focused
cross-scale, Unicode context, mono, budget-boundary, zero-ink/frame-held eviction,
cancellation and close/reopen evidence. Initial fixtures use approved
Carlito/Cousine; new Games font intake requires coordinator source/license
reconciliation. Numeric goldens and actual help-panel inspection remain distinct.
`PreparedTextService`/A2 and its three-generation/two-mask limits are unchanged;
this independent service neither borrows nor enlarges those budgets.

**OBSERVED coordinator report of consumer evidence:** ten fixtures contain
0–129 UTF-8 bytes each, sizes 11.5–16, widths 0–704 and at most four hard lines.
Proposed per-game banks contain six faces: dialogue regular/bold, Cousine
regular/bold and Carlito regular/bold. Reported encoded totals are 2,208,324 bytes
for Four Pegs, 2,117,796 for Atom Probe and 2,018,368 for Switchbox, with a maximum
individual face of 682,468 bytes. These reported input/font sizes fit the stated
bounds; actual ink extents, coverage allocation and shaping-work cost remain
unmeasured. This recording chat has not independently reproduced those counts.

**OBSERVED coordinator assignment:** the GUI.Forms provider owns staged
implementation of the proposal's four development headers and service/session/
lease boundary; the lifetime ledger, bounded queue/retirement and exact-key cache;
logical wrapping/contextual shaping and grayscale/true-mono raster; and focused
tests and evidence. Reuse private mechanisms where their contracts match while
preserving A2's independent semantics and budgets. The coordinator owns CMake
wiring and install-boundary review. Source remains development-only/default OFF;
this assignment is not header installation, SDK publication or consumer readiness.
The complete house-style review and scoped test evidence above are implementation
acceptance work, not another permission gate. This chat owns only this negotiation
record; the coordinator owns commit integration.

### Stage 1 ownership/lifecycle — observed development checkpoint

**OBSERVED:** the provider receipt
[`TEXT_MASK_STAGE1_2026-10-01.md`](../../gui_forms/experiments/TEXT_MASK_STAGE1_2026-10-01.md)
records the four development headers, lifetime ledger, immutable lease/source
ownership, fixed exact-key cache, nine-slot worker queue, cancellation retirement
and structural font-byte registration. Its private backend fixtures exercise
bounded ownership and failure behavior; the default backend still returns
`unsupported_profile`. Native font validity, wrapping/bidi/shaping and gray/mono
raster remain unsupported Stage 2 work. A2 and its budgets are unchanged.

**OBSERVED accepted coordinator refinement:** service `begin_close`, like session
begin-close, is nonblocking. Explicit session join waits for worker retirement
and wake-target quiescence; destruction still waits. This supersedes the original
proposal's service begin-close join-on-return spelling. Cache-hit submit
completions request the same coalesced payload-free worker-delivered wake as other
completions, never call the target on the opening executor or under the session
mutex. A previously selected wake may still post after close, but cannot confer
completion authority; the target must survive through join. Replacement remains
refused until the previous worker joins, and retained leases remain charged.

**OBSERVED verification reports:** the provider's final strict compile and eleven
private lifecycle groups passed in 13.864 seconds. The coordinator independently
reports all eleven manifest hashes match, both lifecycle findings fixed and
reviewed, and a fresh CMake build with prepared text OFF/text masks ON compiled;
its focused CTest covering eleven groups passed in 13.82 seconds (13.83 total).
The provider receipt identifies exact eleven-file source review against
`planning/PROGRAMMING_HOUSE_STYLE.md`; no unrelated legacy/vendor compliance is
inferred. This recording chat read the receipt, not reran tests or hash checks.

The coordinator reports ON installation refused before copying files and an OFF
SDK installed without text-mask headers. The coordinator owns CMake, CI, install
review and integration receipt. These are development isolation checks, not
installed text-mask availability, native renderer evidence or a new approval gate.

### Stage 2 line-break dependency intake — candidate pending reproduction

**OBSERVED coordinator report:** Stage 1 was pushed at `4d24f43`. Stage 2 has no
admitted UAX #14 line-break provider. The coordinator is auditing libunibreak 8.0
in `.build/libunibreak-8.0-intake`, pinned to tag `libunibreak_8_0` and reported
immutable commit `28a2756b864c343f438cd22537d49d394d4666a5`.

**OBSERVED upstream claim:** the official
[libunibreak 8.0 release](https://github.com/adah1972/libunibreak/releases/tag/libunibreak_8_0)
states Unicode 17.0 line breaking under UAX #14 revision 55 and passage of all
Unicode conformance tests without skips. This chat read that release; upstream
claims are not locally reproduced conformance evidence. The coordinator reports
checking the vendor `LICENCE` zlib notice. Retain that notice and all applicable
Unicode data/test licenses with the pinned intake; this record does not claim
an independent license-file audit by this chat.

**CANDIDATE:** use the pinned dependency privately for bounded line-break
opportunities in `logical_wrapped_mask_v1`. The provider still owns wrapping,
source-offset mapping, paragraph/line shaping, cluster-safe fallback, typed
refusal and the existing input/workspace/work limits. A line-break opportunity
does not authorize splitting a grapheme/shaping cluster or truncating output.
No public API, A2 behavior or budget changes follow from dependency intake.
Source-only investigation does not admit installed availability or freeze an ADR.

Alternatives considered are a first-party UAX #14 implementation (not chosen
for this intake because of its maintenance cost), older library versions
(Unicode-version mismatch), and existing whitespace wrapping (not UAX #14
conformance). These are comparison reasons, not measured performance results or
blanket rejection of future alternatives.

Before accepted development integration, retain concrete evidence of the exact
source pin and relevant source/data hashes, vendor/Unicode notices, local build
and conformance reproduction with named data/version/environment and failures
or skips, and the private bounded adapter's source/offset/allocation behavior.
Review authored adapter/tooling changes against
`planning/PROGRAMMING_HOUSE_STYLE.md`; do not rewrite or certify vendor source
as first-party style. Integration must preserve the agreed finite accounting
and cancellation boundaries. The coordinator owns intake/reproduction and the
integration verdict; upstream claims alone do not close that evidence requirement.

**MEASURED — attributed coordinator reproduction:** GCC 16.2 upstream tests
passed 19,338/19,338 line-break cases, 1,944 word-break cases and 766 grapheme-break
cases with no skips. This chat did not rerun those tests. The coordinator accepts
the pinned library for bounded private development use, superseding the pending
reproduction status of the intake above without freezing an ADR or publishing
an SDK. The line-break adapter uses strict language mode (`lang = "-strict"`)
and caller-owned byte output. The coordinator reports no heap allocation found
in the production source reviewed; this is scoped source evidence, not a process
memory guarantee. Vendor/Unicode notices and the existing adapter accounting,
source-offset, cluster-boundary and failure obligations remain in force.

### Stage 2 native masks — final correction review pending

**OBSERVED provider receipt:**
[`TEXT_MASK_STAGE2_2026-10-01.md`](../../gui_forms/experiments/TEXT_MASK_STAGE2_2026-10-01.md)
records the native default backend, replacing Stage 1's unsupported default.
The development path now implements complete bounded-string logical wrapping,
paragraph-level bidi with line-boundary treatment, contextual line shaping,
grayscale raster and true FreeType mono expanded to binary coverage. Logical
scale-1 advances and agreed quantization keep line selection separate from device
scale; source consumption and cluster-safe overflow remain explicit. All native
work stays on the session worker under the reconciled bounds. This is observed
development implementation, not universal rendering correctness or visual parity.

**MEASURED — attributed coordinator verification:** eleven source-manifest hashes
matched, and four focused CTests passed in 13.79 seconds on Shadow Windows:
eleven lifecycle groups, six native-component groups, four public wrapping groups
and 19,338 UAX #14 conformance cases. The test total is correctness evidence,
not a throughput comparison. This recording chat read the provider receipt and
coordinator report; it did not independently rerun tests or verify those hashes.

**OBSERVED outstanding review correction:** the coordinator found invariant raster
mode/FreeType load flags reselected inside the glyph loop. The provider is making
the bounded house-style section 3 correction to select them outside that loop.
Final acceptance remains pending that correction and coordinator review of the
updated source hashes and applicable validation. The provider receipt's earlier
no-known-violation statement must be read with this later finding; passing tests
does not close source-style review.

Positive Arabic fixtures use the coordinator-approved, licensed Amiri font under
`gui_forms/tests/fonts/amiri/PROVENANCE.md`. It is test-only, not Games font intake,
an application fallback or an SDK asset. The receipt retains missing-coverage
failures and the narrowly scoped tiny-label allocation-control measurements,
including noisy/worse cold or tail observations. Those measurements support only
the named workload and storage correction; no general performance claim follows.

The source-only development option remains default OFF with no installed SDK.
A2, its Painter/host path and budgets remain unchanged. macOS/Linux execution,
visual mask goldens, application adoption and actual Games help-panel inspection
remain separate evidence. No new workstream, architecture freeze or availability
promotion is introduced by this checkpoint.

**OBSERVED final coordinator acceptance:** the provider completed the bounded
raster-configuration hoist. The coordinator reviewed the corrected file, verified
all eleven refreshed source hashes, and independently reran both affected CTests:
2/2 passed in 0.43 seconds. The exact authored source and coordinator build scope
are recorded in the integration-review section of the Stage 2 receipt. This
closes the source-review correction above and accepts source-only development
integration. It does not promote installed availability or native visual claims;
macOS/Linux CI and consumer evidence remain outstanding.

### Native public-API consumer checkpoint — provider `7b260cf`

**MEASURED — attributed Games consumer report:** against immutable provider
revision `7b260cf`, both native public-API text-consumer CTest targets passed on
Windows x64, macOS arm64, Linux x64 and Linux arm64 in
[Games CI run 36948428853](https://github.com/falseywinchnet/games/actions/runs/36948428853).
**OBSERVED coordinator verification:** the coordinator independently checked the
run with `gh run view`; the run and all four native-text matrix jobs reached
terminal success: Windows 2022 x64, macOS 15 arm64, Ubuntu 24.04 x64 and Ubuntu
24.04 arm64. The Mac consumer evidence is macOS 15, not the owner's macOS 26
environment. This recording chat did not rerun the tests or independently inspect
the CI jobs.

The consumer also reports fixture and mask-preview artifacts from that run.
Artifact production is attributed to the consumer; neither this record nor the
coordinator report establishes root visual verification, mask-golden agreement
or inspection of an actual Games help panel. The reported passes close the
native consumer-execution gap for this bounded text-mask profile at the named
provider revision, superseding that specific outstanding item above. They do not
establish correctness for other profiles, inputs or revisions.

Current-frame integration remains consumer work. These results do not establish
an independently installed SDK, an installed game, full GUI integration or
platform-wide product availability. Existing capacity, ownership, cancellation,
source-review and installation boundaries remain unchanged. This is an evidence
checkpoint only and assigns no source work.
