# Audio loop transport coordinator review

## Scope and status

Independent source review covered the draft `audio/loop_transport` public header,
private `loop_transport_state.hpp` and `loop_transport.cpp`, all transport tests,
Stage 2 changes to the serial scheduler, the guarded factory/quota/lifetime
bridge in `audio.cpp`, and the Audio CMake registration and installation guards.
The serial scheduler's earlier full review and independent boundary oracle are
recorded separately. This review does not certify unrelated legacy source.

## Findings and corrections

**OBSERVED:** unfinished receipt polling originally checked only transport-local
failure. Parent engine callback failures could leave queued/admitted receipts
reporting healthy pending work after the engine stopped advancing. Poll now
checks the parent engine when no transport-local failure is present. Mixer and
device interruption/unavailability map to `backend_error`; ordinary close stays
`closed`. Receipt phase and timing are retained; completed receipts retain their
historical outcome. The public draft documents that mapping.

A private regression uses the actual engine `callback_failure` path with
`backend_error` and `device_unavailable` separately. It checks queued and admitted
observations, completed outcome preservation, refused new commands, and explicit
shutdown. It does not interrupt a real audio device.

Four stateful `poll` calls were embedded in assertions, including short-circuit
expressions. They now execute into named results before checks. This matters
because polling also collects and destroys retired owners. The claimed
30-second CTest timeout was absent from module registration; root added it in
`2580014` and verified the generated test property.

## Ownership and publication review

- The producer owns clip handles. Queue release/acquire publishes immutable raw
  descriptors. The callback clears its payload mapping before publishing
  retirement; control-side acquire precedes metadata reuse and last-owner
  destruction. No callback clip-owner copy or destruction was found.
- Receipt fields are atomic; sequentially consistent sequence/field access
  supports the bounded coherent-read check. Terminal ownership is handed back
  after the callback's last write. Six sequence increments are reserved before
  reuse. Command, request and epoch counters refuse exhaustion rather than
  recycling authority.
- Callback scheduler/receipt work is bounded by the fixed capacities. It has no
  authored allocation, application callback, logging or blocking mutex path.
  Mixing uses immutable admitted PCM and at most two sources per sample.
- Both voice and transport creation check the shared 64-object quota. Repeated
  transport close compares registry identity before releasing its slot.
- Reviewed the pinned miniaudio source chain `ma_sound_uninit`,
  `ma_engine_node_uninit`, `ma_node_uninit`, `ma_node_detach_full`, and
  `ma_node_input_bus_detach__no_output_bus_lock`. It detaches output buses and
  waits for iteration/read references before returning. Transport close then
  destroys its data source and releases owners on control. Full engine shutdown
  quiesces the device before closing nodes. These conclusions assume the stated
  executor and offline-render preconditions.

## House style

Reviewed the authored scope against `planning/PROGRAMMING_HOUSE_STYLE.md` for
explicit types, named callbacks and retained context, ownership, borrows,
initialization, conversions, failure outcomes, operation order, repeated-loop
work and concurrency boundaries. The stateful assertion issue above was fixed.
No remaining blocking violation was identified in the reviewed scope. Vendored
miniaudio was read for teardown evidence, not rewritten or style-certified.

## Independent validation

Windows Shadow, GCC 16.2/Ninja through `tools/Enter-WindowsToolchain.ps1`.
Rebuilt `games/.build/audio-build` from the corrected source with two jobs;
CTest passed 2/2 in 0.86 seconds. The generated transport test has `TIMEOUT 30`.
Tests cover actual PCM, timing/fade behavior, bounded history, quota, concurrent
publication/collection and engine-failure observation.

Independently rebuilt `games/.build/audio-off-build` with transport disabled;
its unchanged Audio suite passed 1/1 in 0.37 seconds (0.39 seconds total).
The reviewed component is accepted for continued source-build development
integration; installation and release availability remain closed.
No native-device listening, sanitizer, macOS/Linux runtime, complete application
SDK or release readiness is established. The API remains behind the opt-in
development flag and ON installation remains refused.
