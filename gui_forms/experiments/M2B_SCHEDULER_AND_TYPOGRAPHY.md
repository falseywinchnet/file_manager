# M2b bounded frame scheduler and Gallery typography evidence

Date: 2026-08-04

Status: **OBSERVED implementation evidence for the bounded M2b proving slice**.
This record does not implement the complete dispatcher/timer system, select the
renderer, or settle production font shaping, fallback, licensing, or packaging.

## Scope and evidence labels

- **GIVEN**: the scheduler wakes only for damage, declared deadlines, or an
  explicitly active custom surface. It does not introduce an unconditional
  frame loop.
- **GIVEN**: Portsmouth Rapids is used for Gallery titles and controls, not
  field text. Gallery fields retain Lucida Grande.
- **OBSERVED**: FutureScope supplies Portsmouth Rapids 1.0 regular and bold TTF
  faces with 331 glyphs and embedded experimental personal/artistic evaluation
  metadata.
- **CANDIDATE**: the 256-request limit, 32-active-surface limit, and 8 ms minimum
  active interval are proving-slice safety bounds. They are not accepted public
  compatibility values.

## Scheduler substrate

- One-shot paint deadlines and recurring active-surface leases are renderer-
  free, UI-thread-affine, and targeted at attached retained controls.
- Tokens are move-only and owner-revoked through the M1 component revocation
  substrate. Control disposal, explicit disconnect, window cancellation, and
  host shutdown synchronously revoke future wakes.
- Same-control requests coalesce to one paint invalidation. A late recurring
  surface emits one tick, counts skipped intervals, and schedules from the
  current poll time instead of issuing a catch-up burst.
- `next_wake()` exposes the earliest valid deadline to a host. AppKit uses a
  one-shot `NSTimer`, invalidates it when the view leaves its window, and rearms
  only when another deadline exists.
- Metrics expose scheduled requests, scheduler wakes, fired deadlines, active
  ticks, coalesced requests, current active surfaces, and the maximum active
  population.

## Deterministic executable observations

- Polling one nanosecond before a deadline performs no work; polling at the
  exact deadline invalidates once and disconnects the one-shot token.
- Two same-target deadlines fire twice but coalesce into one retained paint
  invalidation.
- A recurring request polled nine intervals late emits one tick, records nine
  coalesced intervals, and schedules one future wake.
- Sub-8-ms cadence, a thirty-third active surface, foreign targets, and wrong-
  thread scheduling are rejected without adding work.
- Owner disposal removes the active lease and leaves no latent wake.
- A deterministic 30-tick, 33,333,333 ns isolated-band fixture rebuilds exactly
  30 chunks, consumes exactly 30 paint invalidations, records 30 partial and
  zero full-window paints, and paints 18,000 logical square pixels: 30 times the
  exact 60-by-10 band. This is a correctness workload, not a latency benchmark.

## Shutdown negative result retained

The first active-timer close test stopped AppKit without crashing, but the
active token remained connected when `run_macos` returned. That exposed a
shutdown contract gap: relying on eventual view/model destruction did not
provide synchronous revocation at the host boundary. The host now calls
`Window::cancel_frame_requests()` before detaching the view. The strengthened
test requires the external token to be disconnected after return.

## Portsmouth Rapids integration

- Gallery-bundled regular SHA-256:
  `b7a98b9dc091f7319a658b1e924ecd97948ec4a1eea0a772501e893107d04f19`.
- Gallery-bundled bold SHA-256:
  `e9dd60dcd8198235fe531149e0b1ab282e28655cc2159ead6123ee398deb1f82`.
- Both faces are copied into the app bundle under `Resources/fonts`; a CTest
  policy gate verifies their exact hashes.
- The private Skia adapter accepts encoded typeface bytes by GUI.Forms font role
  and chooses the nearest registered weight. It does not install fonts into the
  operating system or expose a Skia type publicly.
- Control/title text uses `FontRole::control`; editable values, retained
  collection rows, diagnostic values, and descriptive field content use
  `FontRole::content`; the technical instrument remains `FontRole::monospace`.
- A renderer smoke test loads the bundled regular face, and the Gallery
  interaction test verifies representative control and content role routing.

## Verification

Environment: macOS arm64, AppleClang 16.0.0.16000026.

- Full Gallery/Skia/AppKit Debug: 14/14 tests passed.
- Renderer-free Debug/Werror: 9/9 tests passed.
- Renderer-free ASan+UBSan/Werror: 9/9 tests passed with unsupported Apple leak
  detection disabled.
- Renderer-free TSan/Werror: 9/9 tests passed.
- Renderer-free Release/Werror: 9/9 tests passed.
- Active-timer AppKit close under ASan: 1/1 test passed.
- Active-timer native close repeated ten times: 10/10 passed.
- Canonical lifecycle trace remains 42 lines and 1,959 bytes, SHA-256
  `7fd93035ae50971f7f8fa04f8a0dc3e6164fc8a9bd23978d36ca78ad55d0f99a`.

## Unresolved edges

- M2b schedules paint invalidation only. Ordered UI work queues, general timers,
  callbacks, caret deadlines, dispatch, and nested-loop behavior remain rows 10
  and 25 of the completeness matrix and later milestones.
- The 30-tick fixture proves locality and accounting, not p50/p95/p99 latency,
  CPU utilization, frame misses, allocation behavior, or renderer fitness.
- Portsmouth Rapids has limited coverage. Per-cluster fallback, shaped runs,
  script coverage, malformed UTF-8 policy, and font-resource trust boundaries
  remain M4/M9 work.
- The evaluation metadata and FutureScope provenance are recorded; this slice
  makes no legal determination or production redistribution decision.
- PNG registry/fuzzing, renderer benchmarks, comparison lane, and renderer ADR
  remain later M2 gates.
