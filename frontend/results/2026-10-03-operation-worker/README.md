# Operation worker and independent navigation

**OBSERVED baseline:** at tested PR25 source `8da1f66`, one application worker
serialized navigation, previews, service reads and all filesystem mutations.
A held mutation therefore prevented a later navigation or preview from running.
PR25 subsequently merged with an identical tree as `b328529`.

## Changed behavior and ownership

Application now owns a separate serialized operation worker when an operation
service exists. Create, both rename routes, Copy/Move, internal drop, quarantine
and Undo all enter that queue. The existing read worker retains navigation,
preview, search, checksum and service work. The mutable FileOperationService
and its undo record belong to the operation worker; UI code reads its immutable
policy and root fields and the terminal result's undo-availability projection.

Queued named jobs retain their application and input values. Both workers invoke
jobs outside their queue locks. Stop revokes admission and UI delivery, wakes
both workers, drains admitted operations in order, and joins both before owner
teardown. Copy's existing cancellation token observes stopping. Noncancellable
admitted mutations still finish. If starting the second thread throws, the first
is stopped and joined before constructor unwinding.

Independent execution exposed an ordering defect: a completed operation could
refresh the displayed old location while a new destination was pending, revoking
the user's navigation. A UI-owned PendingNavigation now retains the latest path
and history intent. Completion refreshes that destination, and only a matching
directory result retires the pending record. Search/criteria surfaces are not
replaced by an automatic directory refresh. Selection is not adopted across
a pending navigation.

An operation completed while Settings is open defers the directory refresh
until Back to files. This includes New Folder whose naming context was abandoned
when Settings opened. The completed operation neither closes Settings nor
reopens the old naming editor. Returning to a search/criteria surface still
preserves that surface rather than replacing it with a directory.

## Correctness evidence

**REJECTED baseline:** `WindowsRejectedLastTest.log` records navigation failing
to complete with the operation queue held before the queues were separated.
`WindowsRejectedNavigationLastTest.log` records the old-location takeover after
separating the queues but before retaining pending navigation.
`WindowsRejectedSettingsLastTest.log` records the stale directory on Back to
files before deferred refresh was added. Fixtures own generated temporary
directories; these checks do not modify personal files.

**MEASURED Windows correctness:** integrated Release build with the shared
MinGW toolchain, GCC 16.2.0 and at most two compiler jobs. Final application
interaction and transfer suites passed in 3.47 s and 2.35 s, respectively,
5.87 s total CTest wall time. `WindowsLastTest.log` retains the output. Cases
cover navigation and a real text preview with the operation queue held, retained
copy cancellation/retry/late-cancellation rules, Create/Undo/Create FIFO at
shutdown, rejected post-stop admission, pending-destination and Back/Forward
history preservation, naming/focus contexts, and completion while Settings is
open. The ordinary queued-parent identity fixture now holds the actual operation
queue. Existing UI-callback revocation checks remain in the interaction suite.

These are deterministic ordering/correctness checks, not performance benchmarks.
Native macOS/Linux verification and package delivery of this source are pending.

## House style and limits

Root reviewed the authored hunks in the five C++ files recorded in
`reviewed-source-sha256.json` against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md`. Review covered explicit initialized types,
named predicates/jobs, owned paths and callback state, shared-owner lifetimes,
borrowed fixture lifetimes through thread joins, lock boundaries, shutdown and
construction failure, visible operation order, checked history-index conversion
in the existing navigation path, and storage at job rather than byte boundaries.
The sibling's read-only review identified the pending-navigation defect before
commit. A five-file spelling scan reported zero candidates; that scan is not
the semantic review. No remaining violation was identified in the authored
scope; unchanged legacy code and dependencies are not certified.

The read queue is still serialized: a slow checksum, native launcher or service
request can delay another read. Shared-disk contention, heavy-copy frame timing,
OS-call deadlines, general job-exception/busy-state recovery and search-result
reconciliation after mutation are not resolved here. No extra worker is started
for explicit read-only applications. No ordinary Copy/Move/Delete admission,
metadata-fidelity policy, public API/ABI or durable recovery contract changes.
