# Confirmed Copy progress and bounded UI delivery

**OBSERVED baseline:** `fc6312c` has cancellable protected Copy but no progress
observer. The ordinary Copy command remains unavailable. This change adds
progress from native writes through recursive service traversal to the existing
application status area, without enabling unfinished ordinary Copy behavior.
The development meaning is recorded in
`orchestrator/spec/FRONTEND_COPY_PROGRESS.md`.

## Implementation and acceptance

The native writer reports initial zero after validation/stage creation, then
confirmed writes. Windows/Linux retain their 256 KiB reusable workspace; macOS
uses the existing synchronous fcopyfile callback and copied-count checks.
Observers are borrowed for the call and never retained by the native writer.
Exceptions are contained, including at the C callback boundary, and produce
`callback_failed` while preserving stage identity and close reporting.

The service reports aggregate bytes, completed-object counts, current source
and copying/finalizing/publishing phases. File totals are known; directory
totals remain unknown without a preliminary scan. Counts use checked unsigned
arithmetic. Observer failure cleans the owned stage or reports it retained.
Cancellation is rechecked after the publishing-phase observation and before
publication. Stage progress is not a successful or durable copy result.

The application shares one mutex-protected mailbox per copy. Repeated writes
replace numeric fields and only replace the owned source path when it changes.
There is at most one pending UI notification, plus a delivery already executing.
The status shows file bytes/total or growing tree bytes/object counts. Obsolete,
cancelled and completed transfers refuse late progress. Existing Cancel and
terminal-result handling remain authoritative.

Independent review found two integration issues before commit. First, a changed
regular-file request could stage against a newly observed size while reporting
the old total. The root traversal now checks the requested revision before
progress/stage creation, and native open validates against that same observation.
The regression grows the selected file and requires conflict, no progress and
no stage/destination. Second, allocating the optional mailbox or owning observer
could throw before the service started and strand the UI's busy state. Both
allocations now occur inside a named setup boundary; allocation failure selects
the existing copy/cancellation path without progress. The service call is outside
that boundary and is never retried by it. A test-local, one-shot, thread-local
allocation fixture rejects each setup allocation independently and verifies
empty fallback plus later recovery. Existing whole-job exception gaps outside
this setup boundary remain separate work. Consequential mailbox results are
marked `[[nodiscard]]`.

**MEASURED Windows correctness:** the native-copy, file-operation, application
interaction and application transfer suites passed in the integrated Release
build using MinGW GCC 16.2.0 and at most two compiler jobs. Final durations were
0.21 s, 0.35 s, 3.25 s and 1.40 s respectively; `WindowsLastTest.log` retains
the results. The sibling separately compiled the native writer
and its test with C++20, `-O2 -Wall -Wextra -Wpedantic -Werror` against the
previous model archive before root's integrated build. These are correctness
checks, not latency measurements.

Generated fixtures check empty, short and multi-buffer files; monotone bounded
progress; partial cancellation; observer failure at zero, after bytes and at
the full extent; source/content/stage identity and closes; recursive byte and
object counts; known file versus unknown tree totals; service cleanup after
observer failures in copying/finalizing/publishing; cancellation at publication;
10,001 updates coalesced to one notification with the newest snapshot; mailbox
rearming; and obsolete/cancelled/completed UI delivery refusal. Windows link
fixtures may be skipped for missing privilege; native Unix evidence is separate.

## Full house-style review and limits

Root reviewed every authored hunk in the nine implementation/test files named
by `reviewed-source-sha256.json` against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md`. The sibling independently reviewed its
three native writer/test files. Types, initialized state, named executable
behavior, callback ownership, synchronous borrows, operation order, conversions,
failure cleanup, foreign exception containment and repeated storage were
reviewed. No new per-byte buffer allocation or unbounded UI queue is introduced.
The source path changes at node boundaries; UI snapshots own their path during
delivery. Shared mailboxes own no Application pointer; queued callbacks own
Application and mailbox until invocation/discard. No remaining violation was
identified in the authored scope. The spelling scan is only supplementary;
unchanged legacy code and vendor dependencies are not certified.

Native macOS/Linux builds, Mac progress observations, packaged interaction and
physical-input acceptance remain pending. This does not resolve Copy metadata
fidelity, ordinary endpoint authority, worker serialization that allows reads
to proceed during long operations, directory publication/recovery gaps or
ordinary Move/Delete. No new release is promoted by these local checks.
