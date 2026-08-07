# M12-P3 retained input pressure

Status: **MEASURED PARTIAL**

## Question

Can portable retained input arrive while a `Window` owns an exclusive paint
lease without recursively entering application code, building an obsolete
motion backlog, or surviving retirement of its owner?

## Implemented contract

`Window` owns one renderer-neutral deferred-input queue per retained surface.
Pointer, key, text, drag, and stable-identity semantic ingress observed while
`in_paint` is true is accepted into that queue instead of entering
preview/target/bubble or semantic callbacks.

- Capacity is fixed at 1,024 retained events.
- Consecutive pointer moves for the same pointer replace the previous move.
  Consecutive drag-over events for the same session follow the same latest-
  position policy. Critical down/up/wheel, drop/leave, key, text, and semantic
  ordering is never compacted.
- Deferred drag negotiation returns only the last effect already accepted for
  that same active session when it remains in the current allowed-effect set.
  A new session returns none until application code negotiates after release.
- Semantic actions retain copied stable ID/action/value data and resolve against
  the live semantic tree only after release.
- One ownerless dispatcher callback is posted only after the outermost paint
  lease releases. Draining never starts a nested message loop.
- A callback fault is counted and isolated per event; remaining queued input is
  still attempted and the first fault is rethrown through the dispatcher.
- Root retirement, dispatcher shutdown, or failure to post the bounded drain
  abandons retained events explicitly and increments diagnostics.
- Input-caused invalidation returns to the ordinary damage/paint-wake path. No
  historical paint job is created for an intermediate input.

`DeferredInputSnapshot` exposes pending/capacity, accepted, delivered,
pointer-move- and drag-over-coalesced, capacity-rejected, abandoned, fault,
drain, queued, and draining state without renderer or platform types.
`DragDispatchResult` and `HostDispatchResult` distinguish deferred retention
and capacity rejection from an ordinary immediately handled or unhandled route.

## Measured gates

`gui_forms_invalidation_damage_tests`, in both the normal and renderer-free
builds, proves:

- 100 adjacent pointer moves plus pointer-down, key, and text are accepted while
  paint is active without one application input callback;
- the moves compact to one latest coordinate, leaving four retained events;
- one posted drain delivers enter/move/down/key/text outside paint in causal
  order;
- input mutation produces one ordinary later paint and zero residual
  dispatcher or input backlog;
- a throwing key callback is reported as one dispatcher/input fault while the
  later text event still delivers and the queue returns to zero;
- 32 drag-over events during paint reuse only the prior valid Copy negotiation,
  compact to one latest over, and deliver on one later host turn;
- a semantic Press issued during paint retains its stable identity, invokes
  once after release, and produces one ordinary later paint;
- 1,025 key ingress attempts retain exactly the 1,024-event bound, report one
  capacity rejection, and root disposal abandons the retained set without
  posting a drain; and
- the abandoned paint candidate and deferred-input queue retire independently.

The existing `gui_forms_host_protocol_tests` also passes in normal and
renderer-free builds, proving the normalized host protocol still routes through
the same portable `Window` boundary. A normalized pointer injected by
`HeadlessHost` from inside application paint reports accepted/handled,
`input_deferred=true`, enters no callback, and delivers on one later host
dispatcher turn. The strict local target builds without warnings, and the Win64
cross-build compiles the updated renderer-free core.

## Honest boundary

This does not close all M12-P3 work. Still open:

- physical Win32, macOS, and later Linux host probes must deliberately force
  input at a native paint callback boundary;
- managed key-preview return-value parity remains partial; and
- cross-window and prolonged wall-clock native mutation/cadence storms still
  require soak. Deterministic slow replay/presenter, occlusion, replay-fault,
  resize, and retirement pressure are now measured separately in
  `M12P3_PRESENTATION_PRESSURE_AND_FRAME_FAULTS.md`.

No claim is made that a 103-event deterministic fixture is a native latency or
throughput benchmark.
