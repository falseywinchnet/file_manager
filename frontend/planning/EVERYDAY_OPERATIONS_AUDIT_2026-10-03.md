# Everyday operations: current source and next bounded repairs

Status: **OBSERVED source audit; no new operation acceptance or measurements**.
The existing visible sibling "Audit File Manager Details against interviews"
performed this read-only pass. Root retains implementation ownership. No worker
agents or sibling edits were used.

## Present behavior

Native default Open, regular-file/property rename, deterministic New Folder,
staged copy, same-volume move, internal single-object drag/drop, protected-profile
quarantine and one-step in-memory undo exist. Current errors/collisions have
owned modal reporting. The original missing-feature tables must not erase these
implemented paths. Default launch remains read-only; mutation requires its
explicit protected profile. ADR-017, ADR-020 and later owner directions govern
admission; broad daily-root operations are not inferred from this audit.

## First repair: portable no-overwrite publication

**OBSERVED:** `frontend/src/file_operations.cpp` separately checks destination
vacancy, then uses ordinary `std::filesystem::rename` for rename, staged-copy
publication, move, quarantine and restoration. POSIX rename's replacement
semantics do not establish ADR-017's no-overwrite guarantee under concurrent
filesystem changes. No overwrite incident was reproduced in this audit.

The POSIX destination helper also conflates some observation errors with absence;
the Windows helper treats unobservable destinations as occupied. Observation
failure is not proof of vacancy.

**Required bounded repair:** one private platform publication operation must
either preserve any existing destination or return an explicit failure. There
must be no fallback to replacement when the required native primitive is absent.
Use it at every existing commit/restore site, with correct conflict/error
classification and recoverable stage/source state. Keep replacement, merge and
cross-volume move unavailable. Verify ordinary successful publication and
destination preservation using owned disposable fixtures on all three platforms.
Existing identity/path validation remains necessary and must not be described as
complete race resistance merely because publication becomes no-replace.

## Second repair: cancellable regular-file copy

**OBSERVED:** `copy_node_no_follow` checks cancellation per node, but a regular
file uses one blocking `std::filesystem::copy_file`. Cancellation can prevent
final publication while waiting for that call. The UI has no explicit transfer
Cancel action; the shared serialized worker is joined on close.

**CANDIDATE bounded implementation:** explicit file owners, reusable chunk buffer,
short-read/write handling, cancellation between chunks, source revision checks,
coalesced byte progress and a named Cancel action. Keep staged publication and
cleanup/recovery terminal states. Cancellation after commit is not rollback.
An individual blocked OS call still has no implied hard deadline. Measure large-
file cancellation and queued-navigation delay before claiming responsiveness.

## Other concrete gaps

- Command rename selects the entire filename; the interview asks for basename-
  only initial selection with the extension visible. Its current interaction
  test preserves the mismatch. Unchanged command rename reports a collision;
  property rename already treats unchanged text as a no-op.
- POSIX launcher `waitpid(..., 0)` can occupy the shared worker without a deadline.
  This is an observed dependency, not a measured freeze.
- Multi-selection does not supply batch mutation. Full external/tree/preview
  transfer behavior, OS trash, cross-volume move, merge, replacement and durable
  restart recovery remain separate unfinished capabilities.
- New Folder does not immediately begin naming. Deterministic naming is admitted
  by ADR-017; this is an ergonomic limitation, not an unapproved contract failure.
- Internal drag feedback says "Option" across platforms although the route uses
  Alt/Option. Quarantine's two-press behavior belongs to the accepted protected
  profile; older general interview language alone does not reverse that decision.

## Review scope and implementation discipline

Inspected: `file_operations.hpp`; operation helpers and create/rename/copy/move/
quarantine/undo bodies in `file_operations.cpp`; native launch planning/execution;
Application operation guards, rename/transfer/drop/quarantine/result/modal paths
and worker shutdown; corresponding named job records; selected operation and
interaction tests; ADR-017/020 and DDV-007 transfer/error verdicts.

Named job owners and moved results are useful existing patterns, not a whole-file
house-style certificate. Explicit member initialization is missing in
`CopyOutcome` strings; recursive copy hides intermediate path construction in
the call; bounded name-search loops repeatedly construct paths/streams. No
performance defect is inferred from those last allocations without measurement.

Fresh implementation review must apply `planning/PROGRAMMING_HOUSE_STYLE.md` to
the exact changed source/tests/tools: explicit types/initialization, named
execution, resource owners/borrow lifetimes, visible order, checked conversions,
complete failure states and reusable storage. Functional tests alone do not
establish compliance. No tests/builds were run by this read-only sibling audit.
