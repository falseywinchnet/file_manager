# Games next-bar audio transition — intake 001

Status: **Stage 1 reviewed; Stage 2 development assigned in exact scope;
API not frozen, runtime/installed availability not asserted**. Orchestrator owns
this record; Games supplies
its API/profile reply through the established coordination channel.

## Consumer request

**OBSERVED** — Games reports incoming Four Pegs portable core/save integration
and a need for a shared sample-accurate next-bar transition. The coordinator
authorized proposal work only. Existing Audio source and SDK remain frozen for
this request; playback/decoder permissions do not authorize a scheduler.

The consumer proposes already admitted immutable stereo 48 kHz clips, fixed-rate
loop transport, manifest-supplied bar intervals and scheduling on the audio/render
thread from its actual source cursor. Minimum lead is 3,840 frames (80 ms at
48 kHz); the chosen boundary must be strictly after insertion plus lead. No UI
or device-time conversion is proposed. Exact insertion meaning remains to be
specified by the reply, not inferred from the caller's command timestamp.

The actual loop boundary is a valid candidate independently of the nominal bar
grid. Reported tier-one values are 99,310 frames per interval and 3,177,931 frames
per loop: 32 intervals total 3,177,920 frames, leaving 11 frames. Do not silently
round, stretch or replace the actual loop length to make those quantities equal.
These values are consumer reports pending manifest/fixture evidence.

## Required concrete reply

- Public operations and typed outcomes; preadmitted clip identity and transport
  identity/epoch, with no decoding or file access during scheduling/rendering.
- Audio/render-thread command admission point and acknowledgement frame; precise
  strict-boundary selection across wrap using nonwrapping frame/cycle accounting
  and checked arithmetic. Define exhaustion rather than recycling identities.
- Which old sample is last and new sample first, the destination start offset,
  and whether the transition is a switch or a defined crossfade. No implicit
  interpolation, gain change or latency compensation.
- Pause, resume, stop, seek, EOF and rate changes. A fixed-rate proposal must
  explicitly reject or invalidate incompatible rate state; no silent conversion.
- Same-transport replace/cancel ordering, races at the render boundary and typed
  too-late/refused outcomes. Failed admission preserves the existing transition.
- Immutable command/clip ownership and finite command, pending and retirement
  slots. No allocation or blocking on the render callback; last clip-owner
  destruction occurs on an admitted non-callback executor. Specify backpressure.
- Exact offline sample fixtures covering boundary minus/equal/plus one, minimum
  lead crossing wrap, the residual 11-frame tail, repeated loops, cancellation/
  replacement timing, stale transport, exhaustion and cleanup/failure cases.

Orchestrator reconciles semantics before shared API freeze; the coordinator
assigns exact implementation files only after the concrete proposal is reviewed.
Games owns its manifest and thin consumer. Root keeps integration/build ownership.
This intake adds no device hook, wall-clock scheduling or native playback claim.

## Games provider proposal 002 — bounded loop transport

**CANDIDATE** — Games proposes additive `AudioLoopTransport`, a fixed-rate
48 kHz stereo data-source node occupying one of the engine's existing 64 voice
slots. Paused/stopped transports retain that slot until destruction. The
render callback alone owns source cursors and a monotonic frame timeline; no
seek, pitch change or nonloop playback is part of this transport.

Proposed source API: `engine.loop_transport(output)` and
`change(clip, {bar_frames, minimum_lead_frames=3840, fade_frames=2400})`, returning
typed status/request identity, plus gain, pause/resume, cancel-pending and stop.
An atomic receipt reports pending/applied/cancelled and actual transport boundary.
These names and defaults are not admitted API/ABI by recording the proposal.

Ownership proposal: 16 SPSC POD command entries and 32 control-owned immutable
payload slots, with callback indices only and atomic retirement acknowledgement.
The callback never allocates, frees the final clip owner or waits. Current,
retiring and pending clips retain shared clip-budget charges; at most two are
audibly active. Queue/payload exhaustion refuses without cancelling the existing
transition. Request identities never wrap. All control calls have one producer
executor; offline/device render must not become concurrent consumers.

The proposal replaces pending changes, defers boundary computation during an
active fade, freezes timeline/pending during pause, and cancels/rewinds on stop.
At transition, incoming starts at frame zero at full transport gain while the
outgoing clip linearly falls to zero over N frames; zero fade is a cut. This is
an incoming-full overlap proposal, not a constant-power crossfade. Exact weights,
clipping and boundary admission still require the corrections below.

## Required reconciliation before implementation scope

1. The original requirement says boundary strictly **after** insertion plus
   lead; the provider's subsequent greater-than-or-equal wording conflicts.
   Settle the exact inequality and test equality explicitly.
2. Define callback admission frame separately from caller enqueue acceptance.
   Specify bounded command-service points and ordering. Block-ingestion may make
   admission depend on callback boundaries; if chosen, report that explicitly
   and compare chunked rendering only at the same admitted frame. Caller enqueue
   timing alone cannot establish chunk-independent sample timing. A fixed quantum
   would be a separate explicit choice, not an inferred requirement.
3. During a fade, specify whether a waiting change uses original admission plus
   lead or restarts lead after fade completion. Name the exact eligibility frame,
   including which source loop's bar grid applies when the current clip changes.
4. Define N=0 and N=1 as well as the first/last outgoing weights for N>1; state
   incoming-full overlap, final clipping, transport gain changes and loop wrap
   during a fade. The recorded boundary is the first incoming sample index.
5. Define first-clip behavior with no current bar grid, and whether a request
   during stop starts playback or waits for explicit resume.
6. Cancel/stop queue exhaustion must have typed outcomes without pretending
   cancellation. Specify too-late/applied/fading requests and pause command
   processing. Stop may rewind source without recycling the monotonic timeline
   or stale request authority; define any separate transport epoch explicitly.
7. Receipt reads need coherent request/state/admission/boundary fields, bounded
   storage and explicit stale/retired-query behavior. No unbounded history or
   callback-side spin/wait is admitted.
8. Choose finite bar/lead/fade bounds and checked frame/cycle arithmetic. Free
   queue slots do not establish free payload/reservation slots; last-owner
   retirement must occur on the control executor, including shutdown.

The coordinator also requests an exact new/changed file list and a standalone
offline scheduler proof before integration with a miniaudio data-source node.
Neither that proof nor the node is authorized merely by requesting the plan.
Provider counterreply and exact offline sample fixtures precede coordinator
implementation assignment. Existing Audio source/SDK remains unchanged by this
negotiation; exact-scope `planning/PROGRAMMING_HOUSE_STYLE.md` review is required
for any later authorized source, tests and authored timing tools.

## Provider counterproposal 003 and Stage 1 development admission

**OBSERVED** — The coordinator authorizes only the private Stage 1 experiment in
`gui_forms/src/audio/loop_transport/scheduler.hpp`, `scheduler.cpp`,
`gui_forms/tests/audio/audio_loop_scheduler_tests.cpp` and
`gui_forms/experiments/AUDIO_LOOP_SCHEDULER_2026-10-01.md`. Root owns CMake and
Audio.cmake wiring. No public Audio header/factory, transport owner, data-source
node or native callback edit is included. Stage 2 requires separate admission
after model tests and exact-scope house-style evidence; it is not automatic.

The accepted experimental direction uses fixed 48 kHz/rate 1 loops and strict
boundary **greater than** callback admission frame plus lead. Actual loop end
is an independent boundary and the bar grid restarts after wrap, retaining the
11-frame tail. Arithmetic skips whole cycles by division and checks overflow;
no unbounded per-bar search. Bar bounds are 1..clip frames (<=28,800,000), lead
0..480,000 and fade 0..48,000 frames. IDs/epochs refuse exhaustion. Render-frame
overflow closes the transport, zeros remaining output and never wraps.

Callback ingestion is proposed at output-request boundaries, at most 16 queued
commands per visit. Enqueue success is not callback admission or application.
Pause emits zero and freezes transport/source frames while still ingesting
commands. The coordinator recommends unpaused empty/stopped rendering advances
the transport timeline; stop does not rewind it. Exact empty/paused behavior is
being confirmed before model finalization. Chunk equivalence tests compare the
same admitted frame and ordered commands, not caller enqueue timing alone.

A pending change retains its original admission-plus-lead cutoff during a fade.
Execution waits for a source boundary at/after fade completion and strictly
after that cutoff, without restarting lead. At most two clips play concurrently.
On an empty transport, the proposed explicit exception starts incoming frame
zero at the first unpaused output sample, ignoring lead because no current bar
grid exists. The applied boundary names that first incoming sample.

At switch B, incoming has full transport gain. N=0 removes outgoing at B; N=1
gives outgoing zero gain at B and retires it; for N>=2, outgoing at B+j has weight
`1 - j/(N-1)` for j=0..N-1 and is absent from B+N. Both clips loop independently.
Master gain applies to both; overlap can clip under the existing engine policy.
This is explicitly not equal-power mixing. Gain zero neither pauses nor cancels.

Every proposed control command has its own monotonic ID. Cancel affects a still-
pending target at callback ingestion; an already applied/fading target returns
too-late without changing playback. Replacement affects pending work only. Queue
exhaustion preserves prior state; stop has no bypass priority. Stop cancels
pending and retires active payloads; shutdown closes authority. Exact stop/pause
interaction and treatment of expired cancellation targets remain below.

Proposed storage is 16 SPSC POD command cells, 32 control-owned immutable payload
slots and 64 bounded receipt cells. The callback handles typed indices, never
last shared owners. Retirement acknowledges last callback use; control collection
reclaims payload owners. Capacity includes retired-but-uncollected slots, not
just two active plus one pending. One transport consumes one of the existing
64 voice slots until destruction, including paused/stopped state.

Receipts distinguish queued/admitted/applied/cancelled/replaced/too-late/closed,
with admission frame, application frame and transport epoch. Poll may separately
return unknown/expired/busy. The proposal uses atomic fields plus sequence and
at most two reader attempts; the callback never waits. Nonterminal receipts are
not evicted. Terminal eviction and writer ownership require an explicit release/
acquire handoff; a sequence counter alone cannot legalize races on plain fields.

## Remaining model-final clarifications

- Confirm `bar_frames` describes the incoming clip's stored grid, while the
  currently playing clip's grid selects the next switch. During fade, distinguish
  the active incoming timeline from the retiring outgoing clip.
- Confirm applied is terminal at first incoming sample B, with no later callback
  receipt writes after eligibility for eviction. Specify expired-target cancel.
- Specify stop's preserved/cleared pause state and control application receipts
  while paused; keep monotonic transport time distinct from clip source rewind.
- Specify sample arithmetic precision and exact clipping stage for the offline
  oracle, including N=1 and simultaneous gain commands.
- Prove slot acknowledgement follows its last callback/model read and producer
  reuse follows acknowledgement; state initialization/eviction writer ownership.

Stage 1 tests can establish model ordering, bounded storage and exact offline
sample behavior. They cannot establish real miniaudio callback quiescence,
native resource destruction, listening acceptance or installed availability.

## Provider clarification 004 — Stage 1 model laws reconciled

Games confirms that incoming `bar_frames` belongs to the incoming clip and is
stored with it. Scheduling uses the current clip's stored grid. During a fade,
current means the new incoming clip; the retiring outgoing grid is irrelevant.
Applied becomes terminal at the first incoming sample, with no later writes
after terminal receipt eviction. Cancel of an unknown/expired target returns
`expired_target`, not an inferred too-late result; a retained applied target
returns too-late.

Unpaused empty output advances monotonic transport frames. Pause alone freezes
them. Stop clears sources/pending but preserves the pause flag and timeline.
A change while paused can be accepted pending; an initial clip begins at the
first resumed sample. Stop followed by change does not implicitly resume.

Gain is finite in [0,1]; refusal preserves previous state. Outgoing fade weight
uses double arithmetic `1 - double(j)/double(N-1)` for N>=2, with the already
specified N=0/1 cases. Final mixed samples are float; no fast-math is admitted.
The offline oracle must retain this arithmetic/profile rather than silently
substituting equal-power mixing or a different fade endpoint convention.

Retirement occurs after last sample access and reuse only after explicit
collection acknowledgement. Stage 1 is a serial inert-index model of arithmetic,
state and fixed capacities; it does not implement or prove concurrent atomic
receipt access, clip destruction or callback shutdown. Those Stage 2 obligations
remain as proposed: every concurrently accessed receipt field atomic, bounded
reads and explicit writer/retirement handoff. No public Audio/node scope expands.

These clarifications complete the named Stage 1 model laws for the already
authorized private experiment. Functional evidence and exact authored review
must precede any separate Stage 2 admission.

## Stage 1 reviewed evidence and Stage 2 development assignment

**OBSERVED** — The coordinator integrated the private model, receipt and prior
negotiation batch at `8e5368f`. The durable
[Stage 1 receipt](../../gui_forms/experiments/AUDIO_LOOP_SCHEDULER_2026-10-01.md)
records the exact three source hashes, authored review and test scope.
Orchestrator read the receipt; it did not rerun the experiment or repeat the
implementation review.

**MEASURED coordinator evidence:** the focused test passed 1/1 in 0.07 seconds,
including 470,028 independent comparisons with a frame-by-frame boundary oracle.
All three source hashes matched; the coordinator reported no blocking finding
in the exact private model/test scope. Different admission frames deliberately
produce different boundaries in the retained negative fixture. No actual PCM,
atomic publication, native callback, shared-owner destruction or quiescence is
proved by the serial model.

The coordinator now authorizes Stage 2 development in the proposed exact scope:

- New `gui_forms/include/gui_forms/audio/loop_transport/loop_transport.hpp`.
- New `gui_forms/src/audio/loop_transport/loop_transport.cpp` and
  `loop_transport_state.hpp`.
- New `gui_forms/tests/audio/audio_loop_transport_tests.cpp`.
- Necessary private scheduler adaptations under `src/audio/loop_transport/`.
- Additive factory, voice-quota and lifetime bridge only in existing
  `include/gui_forms/audio/audio.hpp` and `src/audio/audio.cpp`.

Root retains Audio.cmake/CMake wiring. No new dependency, physical device
playback or SDK publication is authorized by this scope. Games owns the named
implementation/tests; Orchestrator owns semantic reconciliation. A development
header does not freeze the public contract or establish runtime availability.

Required Stage 2 evidence includes actual PCM sample output, concurrency and
clip-owner lifetime/retirement tests, callback-exclusive scheduler ownership,
and the 16-command/32-payload/64-receipt atomic publication and handoff protocol.
Quiescence must be demonstrated before owner destruction, including shutdown;
the serial Stage 1 acknowledgement model is insufficient. Wrong-thread, closed
and existing 64-voice quota outcomes remain explicit.

Rejected receipt phase must carry a reason such as arithmetic overflow rather
than collapse unlike failures into an unexplained terminal state. Queue admission,
render admission, application, rejection and release remain different events.
Exact status projection and ownership/concurrency implementation receive review
before API freeze. No native listening or cross-platform result is inferred.

## Stage 2 coordinator acceptance — source-build development only

**OBSERVED** — The independent
[Stage 2 coordinator review](../../gui_forms/experiments/AUDIO_LOOP_TRANSPORT_COORDINATOR_REVIEW_2026-10-01.md)
accepts the corrected component for continued source-build development consumer
integration. Orchestrator read this receipt; it did not repeat source review,
native dependency inspection or test execution. The API remains opt-in development
and ON installation refuses; no installed capability or frozen API is admitted.

The corrected pending-poll law checks parent engine failure when no transport-
local failure exists. Mixer failure and device interruption/unavailability report
typed `backend_error`; ordinary close remains `closed`. Queued/admitted receipt
phase and timing remain intact, and completed receipts retain their historical
outcome. The private regression injects the actual engine failure path separately
for backend error and device unavailability, checks pending and terminal behavior,
refused new commands and shutdown. It is not a real device-interruption test.

The coordinator reviewed producer-owned clips, release/acquire descriptor
publication, last callback use before retirement, control-side owner destruction,
atomic coherent receipts, terminal writer handoff and checked counter exhaustion.
The pinned miniaudio detach chain was inspected for quiescence before data-source
and owner release under the stated executor/offline-render preconditions. This
source-backed conclusion is not sanitizer or native listening evidence.

Four stateful poll calls were moved outside assertions because polling also
collects retired owners. The previously claimed CTest timeout was absent; root
added actual registration in `2580014` and verified generated `TIMEOUT 30`.
Historical tests do not retroactively gain that timeout.

**MEASURED coordinator evidence:** rebuilt development-ON tests passed 2/2 in
0.86 seconds, including actual PCM, fades/timing, bounded history/quota,
concurrent publication/collection and engine-failure observation. Independently
rebuilt OFF Audio tests passed 1/1 in 0.37 seconds, 0.39 seconds total. Exact
authored header/private implementation/tests/scheduler/bridge/CMake review under
`planning/PROGRAMMING_HOUSE_STYLE.md` found no remaining blocking violation.
Unrelated legacy and vendor code are not style-certified.

Continued source-build consumer integration is now open within the reviewed
development scope. Installation, full Application SDK, physical-device/listening,
macOS/Linux runtime and release availability remain closed or unverified. Root
owns the separate correction/receipt commit; this semantic receipt adds no code.
