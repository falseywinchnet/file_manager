# Private prepared-window session stage

**CANDIDATE implementation assignment:** extend the source-private batch shaper
with one explicitly owned worker/session and a typed completion slot. This is
not public D2 availability, an SDK export, a File Manager preview activation or
a Text Editor gate change. The existing visible sibling owns the three new
session source/test files. Root owns the shared ledger claim, old A2 integration,
build registration, native integration, review and evidence. No worker agents
are used. All authored implementation and tests require review against the
complete `planning/PROGRAMMING_HOUSE_STYLE.md`.

The session must transfer an already-admitted unique batch into one occupied
queued/running/ready slot, without adding another payload reservation. A newer
desire retains metadata only. Cancellation does not free a running slot.
Worker construction, use and destruction of the native shaper stay on one
executor. Whole-batch adoption and failed/stale discard stay on the owner.
Thread start, initialization, notification and shaping failures need explicit
dispositions; a refused submit preserves its incoming owner. Confirmed join,
rather than a cancellation or exit flag, releases the workspace/session claim.

**OBSERVED ownership gap found during integration:** the old service's weak
session registry only recognizes its A2 session type. A new private session
could otherwise create an additional workspace against the same ledger. The
new private `PreparedSessionClaim` therefore arbitrates both types through one
mutex-protected ledger claim. A2 takes it before worker setup and releases after
join. Its startup-failure cleanup also releases it. Retained payload/font
reservations remain independent. The old service's public close method does
not yet discover or join the new private session; explicit application ownership
and ordered private close/join remain necessary before public integration.

## Native readiness investigation

**OBSERVED:** the ordinary dispatcher queue is limited to 4096 callbacks
(`src/core/dispatcher/state/post_dispatch.cpp`). The existing application
`wake_ready` supplies `std::function<void()>`; it has no typed delivery failure.
`dispatch_pending` runs on the UI executor. Reusing that void callback does not
establish the stronger prepared-window readiness contract.

| Adapter | Observed existing behavior | Required investigation before activation |
|---|---|---|
| Windows | `WindowsHostState::post_managed_dispatch` uses `PostMessageW` without inspecting its result. Single/multiple-window loops use `MsgWaitForMultipleObjectsEx` for frame clocks and native messages. | Reserve a lifetime-owned completion signal independent of posted-message capacity; check native failure, integrate both host loops, and account for the wait-handle limit. |
| macOS | `DispatchAndCollectWake` submits an owning block to the main dispatch queue for each invocation, retaining the dispatch target and native view. | Establish a coalesced owner connection, close/queued-block revocation and failure/lifetime semantics; prove UI drain outside paint and under ordinary-queue saturation. |
| Linux | `Wake` writes to a nonblocking pipe and ignores its result; runtime drains the pipe then updates windows before waiting. | Distinguish full-pipe pending readiness from interruption/failure, prevent descriptor reuse after close, and prove the latch-clear/drain handshake. |

These are source observations, not reproduced user failures or acceptance of
particular replacements. A shared pending latch alone cannot repair a failed
native wake. A successful signal needs a bounded owner drain and a race-safe
clear/recheck; failed delivery needs a separately reliable failure path or a
terminated connection before further admission. A private fake-host seam is
useful for session correctness but cannot prove native delivery. No timer or
paint polling substitutes for readiness.

The existing prepared-window development contract continues to govern full
identity, generation and source coverage, three retained payload generations,
one replacement slot, all-row adoption, truthful revoked/current distinctions,
and owner-executor teardown. Wrapping, tab/long-paragraph policy, retained batch
rendering, physical-input dogfood and installed consumer evidence remain open.
