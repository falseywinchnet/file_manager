# Postorder directory-copy metadata

**OBSERVED baseline:** `5d86e94` preserves regular-file modification dates but
creates each copied directory with the source's attributes before adding its
children. It does not restore directory modification dates after traversal.
On POSIX a source mode without owner-write can therefore prevent populating its
stage. On Windows the baseline fixture instead reaches publication but fails
the final ordinary-permission comparison. Neither behavior is accepted as a
complete ordinary Copy implementation.

## Implementation and ownership

Directory staging now uses exclusive native creation. POSIX requests `0700`,
subject to the user's umask; Windows inherits the destination parent's security.
Children are copied before the source directory's ordinary permissions and
modification date are applied. This preserves the writable traversal stage and
puts the directory date after the writes that would otherwise change it.

The private finalizer opens source and stage with no-follow behavior, verifies
directory types, the source's captured revision and the stage's exact identity,
and applies metadata through the stage handle. Stage revision is deliberately
not compared: adding children necessarily changes it. The finalizer creates,
publishes and deletes nothing. Both handles close before return, with separate
close errors retained. A failed close prevents successful finalization.
No additional directory handle is retained across recursive child traversal.

The traversal checks cancellation before finalization and reports a completed
directory only after metadata and closes succeed. Metadata failure can leave
some attributes applied; the service retains its stage-cleanup responsibility
and reports a retained recoverable object when cleanup cannot complete.
This is not an atomic snapshot or a claim that every concurrent path substitution
is excluded. Existing no-replace publication remains unchanged.

## Verification

**REJECTED baseline:** the new read-only nested-tree regression was built against
`5d86e94`'s production source and failed on Windows at final permission
preservation. `WindowsRejectedLastTest.log` retains that failure. The test also
requires source/child directory dates to survive final publication. Native Unix
execution is still required to verify the no-owner-write traversal behavior.

**MEASURED Windows:** integrated Release build, shared MinGW GCC 16.2.0 toolchain,
maximum two compiler jobs. The final file-operation and native-copy suites pass
in 0.39 s and 0.26 s (0.65 s CTest total); see `WindowsLastTest.log`. The tests
cover nested read-only directory copying, ordinary final modes, restored dates,
unchanged source revision, occupied-stage preservation, unavailable source
identity, mismatched available stage identity, refusal to create an absent stage,
successful empty-directory finalization, and refusal after a source directory
revision change. Existing regular-file, progress, cancellation and failure
tests remain passing. Windows link cases report missing privilege and skip.

The POSIX fixture additionally checks that the initial stage grants no group or
other permission bits. That branch and the production Mac/Linux code are
source-reviewed, not locally executed on Windows. Native matrices and package
promotion are pending. These correctness durations are not performance results.

## House style and outstanding work

Root reviewed all authored hunks in the five C++ files identified by
`reviewed-source-sha256.json` against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md`. The independent sibling reviewed the
production creation/finalization/traversal path. Review covered explicit types,
initialized result alternatives, named execution, synchronous path borrows,
native handle ownership, separate close errors, cancellation order, per-directory
metadata work outside byte loops, and fixed bounded fixture restoration storage.
Fixture restoration binds its root and identity, accepts only absolute contained
paths, changes only matching directory identities, and catches teardown failures.
No new callbacks or retained source borrows are introduced. The five-file
spelling scan reports zero candidates; no remaining violation was identified in
the authored scope. Unchanged legacy code is not certified.

Permissions here mean the existing regular-writer subset: POSIX ordinary `0777`
bits and Windows read-only state. ACL/ownership, special mode bits, broader
Windows attributes, link timestamps, extended attributes, named streams,
sparse/clone behavior and complete ordinary Copy admission remain unresolved.
If a later sibling copy or callback fails after a nested stage directory became
read-only, cleanup can still retain the tree; unconditional cleanup is not
claimed. Injected metadata-system-call and close failures are not exercised by
these fixtures. Directory cancellation, native trash, durable recovery and
cross-platform daily-use acceptance are not closed by this receipt.
