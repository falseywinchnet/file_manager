# M12-P14 frame-poll arbitration

Status: **MEASURED PARTIAL M12-P14** on 2026-08-06.

## Question

Can scheduled paint deadlines, active surfaces, and UI timers safely stop,
restart, dispose, create peers, or attempt a nested poll from inside their
callbacks without same-turn catch-up or callback-recursive delivery?

## Evidence and rule

- **OBSERVED:** `poll_frame_schedule` already copied the request vector before
  callback delivery. Existing timer evidence therefore covered self stop,
  restart, and later-peer disposal safely.
- **OBSERVED GAP:** a frame callback could call `poll_frame_schedule` again.
  Because current deadlines advance before callbacks, this did not duplicate
  the current request, but it could deliver peers or newly registered due work
  recursively inside the callback.
- **DECIDED FOR THIS 0.x SLICE:** one outer poll owns one fixed due set. A
  recursive poll delivers nothing, reports that it was deferred, and leaves
  all connected work for the next host turn. Requests created by a callback
  never enter the current due set, even when their deadline is already due.

## Implemented center

- `Window` owns an exception-safe frame-poll reentry guard;
- `FramePollResult::reentrant_poll_deferred` identifies a suppressed nested
  poll without throwing through application animation code;
- the deferred result still reports occlusion, pending damage, and the current
  next wake so the host can schedule the next turn normally;
- `MetricsSnapshot::reentrant_frame_polls_deferred` and deterministic JSON
  expose the condition to diagnostics and future availability projection;
- existing strong request snapshots continue to give registration order,
  peer cancellation, stop/restart/disposal safety, per-target callback
  coalescing, isolated faults, and no catch-up bursts.

## Verification

- a controller deadline cancels a later due peer, registers a new already-due
  request, and attempts a nested poll;
- the nested poll delivers nothing, the cancelled peer never runs, the new
  request remains connected, and the outer result advertises its due wake;
- the next poll delivers that new request exactly once and records one
  reentrant deferral;
- focused frame-scheduler, Timer, range-control, animation, dispatcher, and
  Complete Showcase interaction suites pass in the normal build;
- frame-scheduler, Timer, and animation gates pass renderer-free and under
  combined ASan/UBSan; affected core/control libraries cross-compile for
  Win64.

## Honest remainder

- physical host nested-message-loop probes;
- BackgroundWorker and synchronization-context breadth;
- ABI/managed projection of the new result and metric fields;
- randomized request mutation at the 256-request quota boundary.

This preserves ordinary UI timer time during occlusion while keeping visual
active surfaces suppressed, as established by the earlier scheduler contract.
