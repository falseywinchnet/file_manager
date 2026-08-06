# M11h renderer-free UI dispatcher

Status: **MEASURED PARTIAL M11h** on 2026-08-06.

## Question

Can GUI.Forms accept asynchronous and synchronous invocation from worker
threads while preserving a deterministic retained UI turn, owner lifetime,
fault isolation, bounded resource use, and platform-neutral shutdown?

## Decision boundary

- **GIVEN:** unsafe worker mutation of retained controls remains rejected.
- **GIVEN:** nested `DoEvents` is not admitted by this slice.
- **DECIDED:** work posted during a dispatch callback belongs to the next host
  turn. The dispatcher does not recursively drain until empty.
- **DECIDED:** dropping an observation handle does not cancel fire-and-forget
  work; cancellation is explicit while pending.
- **DECIDED:** control-scoped work requires a live attached owner when it runs.
- **DECIDED:** callback exceptions are retained on the operation and isolated
  from later callbacks rather than escaping through a native window procedure.
- **DECIDED:** synchronous `Invoke` runs inline on the UI thread. From a worker
  it posts once, wakes the running host, and blocks only that worker; the UI
  thread never enters a nested pump.
- **DECIDED:** synchronous callback faults are rethrown to the producer as the
  original exception. Owner detach and shutdown complete the wait with typed
  cancellation. A worker invocation with no host wake handler is rejected.
- **DECIDED:** input/timer callbacks finish before work they post; posted
  mutations finish before layout; layout finishes before paint; an active-
  surface callback advances state before its paint.

Synchronization-context projection, a BackgroundWorker component, generated
DML/C ABI public projection, and automatic diagnosis of an application that
synchronously waits on a producer which is itself blocked in `Invoke` remain
open. Reentrant pumping is explicitly not the remedy for that application
deadlock.

## Implemented center

- public `DispatchOperation`, state, sequence, cancellation, and retained fault;
- public Window- and Control-scoped `begin_invoke` plus affinity inspection;
- public Window- and Control-scoped `invoke`, inline and marshalled paths,
  typed cancellation, and original exception propagation;
- thread-safe FIFO queue, 4096-operation pending bound, and 1024-operation turn
  bound;
- one-snapshot dispatch turns with coalesced wake publication;
- automatic owner-detach/disposal cancellation and synchronous Window shutdown;
- explicit deterministic headless pumping;
- AppKit main-queue and Win32 private-message draining of the renderer-free core;
- separate generated C ABI queue corrected to the same nested next-turn rule;
- synchronous-wait completion notification for execution, fault, explicit
  cancellation, owner detach, shutdown, and host-handler removal;
- dispatcher telemetry in the complete showcase and Win32 automation snapshot,
  including synchronous/inline/marshalled counts.

## Measurements

- Dedicated renderer-free tests pass FIFO `A/B/C`, nested `D` deferral,
  coalesced wakes, explicit cancellation, detached-owner cancellation, fault
  isolation with a surviving callback, worker affinity, the 4096-operation
  bound, shutdown revocation, inline/marshalled synchronous invocation,
  original-fault propagation, typed owner cancellation, and unhosted/shutdown
  wait guards.
- Four concurrent producers publish 1,280 callbacks in assigned global FIFO;
  the first turn stops at 1,024 and the second drains the exact remainder while
  preserving each producer's program order.
- Headless phase proofs record input → next-turn dispatch → measure → arrange →
  paint, timer → next-turn dispatch, and active-surface callback → paint.
- The generated C11 ABI test records nested work in two dispatch turns rather
  than one reentrant pump.
- The AppKit host test posts from a worker, requires async, nested, and
  synchronous callbacks to observe UI affinity, returns the synchronous
  callback's original fault to its worker, and also completes eight active-
  surface disarm/quiescent/rearm cycles with real paints.
- Live AppKit dogfood completes full motion → reduced → paused → resumed with
  one action per transition. Reduced motion visibly moved from 45% to 62% over
  650 ms, pause held 37% across 650 ms, and resume advanced to 58%.
- Wine complete-showcase telemetry records
  `posted=4, invoked=4, cancelled=0, faulted=0, pending=0` after the nested batch,
  then `posted=5, invoked=4, cancelled=1, faulted=0, pending=0` after explicit
  cancellation.
- A following Wine snapshot records `posted=6`, `invoked=5`, `cancelled=1`,
  `synchronous_invocations=1`, `inline_invocations=0`, and
  `marshalled_invocations=1` after the showcase worker `Invoke` completes.
- The native suite passes 47/47 tests; a fresh renderer-, HarfBuzz-, and
  host-free build passes 36/36; strict x64 MinGW builds the core, generated ABI,
  gallery, complete showcase, and Win32 host with `-Wall -Wextra -Wpedantic
  -Werror`.

## Honest next edge

Synchronization-context and BackgroundWorker projection, generated-facade/C
ABI exposure, sustained producer benchmarks, and explicit application-deadlock
diagnostics remain open. Nested `DoEvents` remains a separate compatibility
decision and is not implied by the completed `Invoke` core.
