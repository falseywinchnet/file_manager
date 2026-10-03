# Frontend copy progress development projection

Status: **development implementation; native acceptance pending**.
Date: 2026-10-03.

**GIVEN:** ordinary file operations and responsiveness are required for daily-use
File Manager. **OBSERVED:** the existing protected Copy controller supports
cancellation but supplies no progress observation to the application. This
projection supplies progress for that controller and is a foundation for the
ordinary Copy completion described in the launch audit. It does not activate
ordinary Copy, change metadata policy, or widen filesystem authority.

## Meaning and lifecycle

The native regular-file writer synchronously reports confirmed stage bytes and
the expected extent established by source validation. Empty files report zero
of zero; duplicate observations are allowed. No progress observation means
successful publication, clean closes, complete revision validation or durable
storage. Observer exceptions produce an explicit failed callback result while
retaining stage identity and close outcomes.

The frontend service aggregates bytes and completed objects across its current
traversal. A regular-file request has a known total; a directory has an unknown
total and no preliminary full-tree scan. Completed objects include regular
files, symlink leaves and directories after their children. The current source
is an owned path changed at node boundaries. Copying, finalizing and publishing
are progress phases, never terminal outcomes. The existing OperationResult is
the terminal authority. Cancellation observed immediately before publication
still prevents publication; a later request cannot undo an already published
copy. Observer failure before publication follows the existing stage cleanup
and retained-object reporting law.

Callers keep observer targets alive for the synchronous service call. Observers
must not reenter the service or mutate the traversed objects. They borrow each
snapshot only until return. The application uses one shared per-job mailbox
protected by a mutex, retaining the latest snapshot and at most one pending UI
notification. A running delivery may coexist with the next queued notification.
The UI rejects progress for an obsolete generation, an ended transfer, or an
acknowledged cancellation request. Terminal status is not replaced by late
progress. Shutdown retains the existing worker-join and UI-discard laws.
If the optional mailbox or owning observer cannot be allocated before the
service starts, the application uses its existing Copy/cancellation path
without progress. This setup fallback never retries a service invocation.

## Boundary and unresolved work

This is a source-level frontend-model addition with an optional callback. All
consumers rebuild together; it is not a stable binary ABI, new local-wire
operation, plugin contract, GUI.Forms control or Engine capability. The native
writer interface remains private to the frontend. Neither metadata policy nor
ordinary Copy authority is selected here. Ordinary endpoint admission, metadata
fidelity, service operation serialization independent of reads, publication
postconditions and broader recovery remain separate work. No performance gain
or physical-input acceptance is claimed from progress correctness tests.

Evidence and exact authored-source review belong in
`frontend/results/2026-10-03-copy-progress/README.md`.
