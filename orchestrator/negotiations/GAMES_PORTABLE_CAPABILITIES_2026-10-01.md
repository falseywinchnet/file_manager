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
