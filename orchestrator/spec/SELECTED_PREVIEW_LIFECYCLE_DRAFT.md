# Selected preview lifecycle: executable draft fragment

2026-10-04 UTC. **CANDIDATE; lifecycle fixture draft only.** This is part of the
bounded selected-preview intake, not a frozen protocol, runtime capability or
decision to open plugin execution. Source-read authority, native process limits,
transport, provider placement and pixel-transfer ownership still require their
own reconciliation. No new public contract ID is assigned to this fragment.

## Authority and scope

**GIVEN:** the active File Manager goal calls for useful previews and responsive
selection. ADR-003 and Orchestrator B0 permit provider-independent lifecycle,
cancellation, quotas and fake-peer conformance work. The contract ladder permits
a disposable executable laboratory for fixture drafts. This fragment uses that
permission. ORC-PLG-002/003 and the admitted UTF-8/PNG product profile remain as
recorded; a test model cannot promote their availability.

The exact lifecycle is independent of whether a later admitted isolated decoder
uses portable codecs or a platform adapter. The laboratory does not launch a
process, open files, authenticate clients, issue capabilities or display pixels.
Its source/window/session numbers are inert fixture identifiers. They cannot be
serialized as a substitute for the negotiated native identity or authority.

## Proposed ownership and ordering

One Orchestrator session owns a fixed eight-client table, at most one pending
selection per client and one global process/result slot. Session plus increasing
nonce identifies a job; no nonce reuse or wrap is permitted within a session.
A production session identity must change on restart. Reusing a window-table
position does not reuse a request ticket. Authenticated window registration and
cross-session message rejection remain adapter requirements.

The slot has four states:

| State | Owned responsibility | Allowed transition |
|---|---|---|
| Idle | No process or offered result | Dispatch the next pending client |
| Running | One request and decode deadline | Matching process reaped; or request stop on cancellation/deadline |
| Stopping | The same process is still owed termination/reaping | Only its matching reap event frees the slot; completed pixels are discarded |
| Offered | Process already reaped; one validated result awaiting client acknowledgement | Matching acknowledgement, revocation, replacement or acknowledgement expiry |

A stop request, a decoder success message, stream EOF and process reaping are
different events. `reaped` means the future host has observed process exit and
completed the required OS cleanup. It must not be sent merely because a stop
call returned. The draft also requires descendant handling before that event;
the laboratory has no descendant/process implementation and proves none.

Before each external-event batch the host advances a monotonic clock and executes
the returned effects. At exact deadline/expiry equality, expiration wins over
completion or acknowledgement. The model rejects a backward clock. Real timer
scheduling, sleep/resume behavior and timely host execution remain unmeasured.

Effects have named responsibilities: stop a ticket, retire an offered raster,
retire a displayed grant, and emit a terminal failure. An adapter must execute
retirement before admitting replacement work. The model changing to Idle is
not evidence that a real allocation, handle, client copy or process was released.

## Selection, fairness and failure

Submitting a new selection retires that client's old display/offer and replaces
its pending request. A replaced queued request gets a cancellation terminal.
If its previous process is running, issue stop once and hold Stopping until
reaped. Further rapid selections replace only the pending slot. An already
stopping process is not stopped repeatedly, and its original stop reason stays
intact. A cancelled/timed-out process can never offer successful pixels.

Idle dispatch is round-robin across the fixed client table; continuously changing
one window does not take another pending window's turn. There is no thumbnail
queue in this slice. A helper that never dies/reaps still holds the process slot:
the model refuses to hide that failure by launching an unbounded replacement.
Host escalation and unavailable-provider reporting are unresolved adapter work.

Explicit cancel/disconnect retires the client's pending, offered and displayed
state and stops its active work. Hiding an inspector cancels pending work. A
completed display may retain its existing grant across temporary hiding, as the
current frontend already does; the caller need not cancel completed content.
Changing selection or destroying the window must retire that grant.

After reaping, compare the trusted host's post-read source observation with the
request snapshot, then validate the result descriptor. Wrong revision/object,
invalid raster and decode failure discard output and free the slot. A wrong
ticket or an event in the wrong phase cannot release another request's slot.
No timestamp/size tuple, supplied profile tag or opaque model number is claimed
to establish filesystem identity or byte stability.

Only the originating live client with the current ticket may acknowledge an
offered result. A late/wrong acknowledgement cannot retire new work. An offer
expires even when its client remains connected but never acknowledges. A
post-reap clock-range failure discards the result rather than stranding a slot
whose process has already exited.

## Candidate limits and accounting

The laboratory instantiates these deliberately revisable limits:

- Eight clients; eight pending request records maximum; one process/result slot.
- Three-second decode deadline, as a failure ceiling; one-second offer
  acknowledgement timeout. Neither is a measured acceptable UX latency.
- Positive raster axes at most 1024, tight stride `width * 4`, exact payload
  extent `stride * height`, at most 4 MiB, orientation already applied, opaque
  sRGB BGRA8. The opaque JPEG profile is a subset of the earlier premultiplied
  output candidate; PDF/transparency/color policies are not selected here.
- One retained display grant per registered client, each at most 4 MiB: 32 MiB
  aggregate grant accounting. The model holds descriptors, not these pixels.

Whole-buffer geometry is checked before multiplication or payload admission.
Width/height limits make the products representable. The eventual receiver must
independently validate actual wire extent and pixel bytes; descriptor checks do
not prove alpha, provenance, shared-memory permissions or payload integrity.

Process memory, encoded input, decoder scratch, IPC buffers, GUI.Forms image
registry copies and client residency are **not** measured or enforced here.
The earlier candidate's transfer/result staging budgets remain unresolved.
Revoking a grant is not proof that an unresponsive client released physical
memory. Future adoption must account for that distinction explicitly.

## Executable checks and remaining gate

The independent crate `../experiments/preview_lifecycle/` tests replacement during
Running/Stopping, stale completion/acknowledgement, deadline and expiry equality,
disconnect/reconnect, source changes, malformed raster extents, eight-client
admission/display accounting, restart sessions, clock limits and a 10,000-update
selection storm. It allocates no storage while performing model transitions.
These are deterministic state checks, not an OS scheduler or performance test.

Before a real preview adapter freezes, complete source-read/revision semantics,
provider build/profile binding, authenticated wire/version/critical-field rules,
bulk-data ownership, close/restart/reaping escalation, OS resource enforcement
and independent provider/consumer conformance. The complete preview intake is
still open; this fragment does not turn an incomplete family into `frozen-v0`.
