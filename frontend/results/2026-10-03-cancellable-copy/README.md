# Cancellable copy: application and service integration

**GIVEN:** Continue the everyday-operation and responsiveness audit. Root owns
this integration; the visible Details sibling supplied the private adapter
recorded in `PRIVATE_ADAPTER.md`. No public ABI or admission policy changes.

## Application behavior

**OBSERVED:** Active copies expose Cancel copy in the footer and Edit menu.
Requesting cancellation changes the footer to Stopping… and disables repeated
requests. The job retains its generation and busy state until the worker's
authoritative terminal result. A request after publication does not discard
success or leave the captured transfer available for accidental repetition.
Cancelled copies retain the capture for retry. Atomic moves do not advertise
cancellation. Both Paste and internal-drop copy predicates observe the request.
Cancellation with a retained stage reports cleanup incomplete and its path.

The control uses the existing command/semantic-action route. The footer's third
auto-sized column is hidden outside an active copy. No Escape binding is added:
existing edit/popup keyboard ownership remains unchanged. Byte progress is not
implemented by this change.

## Service behavior

**OBSERVED:** One 256 KiB workspace is allocated before creating the stage and
reused across regular files in a directory traversal. The private helper owns
its file handles; the service owns stage cleanup and no-replace publication.
Creation refusal does not claim an existing path as a stage. The service uses
the identity returned by its creator, rather than adopting a later arbitrary
path observation after failure. An unavailable created-stage identity causes
retention for inspection. Directory/link creation retains its existing path
observation limits; this is not pinned-parent or snapshot authority.

Native cancellation becomes a cancelled operation. Source revision mismatch
becomes conflict. Primary and close failures remain observable. Exceptions
during traversal and its last cancellation check take the same cleanup path.
Workspace allocation can fail before creation. Existing source/parent checks,
protected mutation admission, one-step undo policy and native no-replace
publication remain the service boundaries.

Read-only stage cleanup on Windows, hard native I/O deadlines, durable crash
recovery, arbitrary metadata preservation, sparse extents and throughput
comparison remain unresolved. There is no claim that cancellation makes an
individual filesystem call interruptible or that this implementation is faster
than the previous copy primitive. See the adapter receipt for platform limits.

## Verification

**MEASURED:** Shadow Windows 11 build 22621, adjacent read-only MinGW GCC 16.2,
Release consumer `.build/details-consumer`, one compiler process. Final build
and all 15 CTest suites pass (4.53 seconds). `WindowsLastTest.log` preserves
the complete test output. Nine selected C++ files produce zero spelling
findings; this is separate from semantic source review.

The new service test copies an exclusively owned generated 16 MiB file, requests
cancellation after observing actual stage bytes, verifies no published object
or retained stage, checks source revision and contents, then retries to a
complete equal-content copy. Windows/Linux require cancellation within the
first 256 KiB; macOS requires actual bytes without asserting that chunk bound.
The operation test fixture now creates its own unpredictable root exclusively
and validates its absolute canonical parent and identity before cleanup. It no
longer deletes a preexisting process-ID-derived temporary directory.

The assembled application test uses a named worker barrier to verify queued
cancellation, active/busy/requested/terminal states, both cancellation predicates,
semantic button activation and footer bounds at 800 x 600. It performs a real
retry, waits for destination publication without draining the UI completion,
requests cancellation, and verifies that the committed result wins. It also
verifies that a queued move does not offer cancellation and completes normally.
This is retained-control/layout verification, not a native screenshot or a
three-platform visual parity claim. The barrier has a bounded failure timeout
and a release owner destroyed before application shutdown joins its worker.

**PENDING:** Native macOS/Linux integrated compilation and tests, especially
the macOS ordinary-xattr/quarantine baseline comparison. The branch must not
merge or be released until the native checks establish the adoption gate.
Native screenshots of the active cancellation control remain pending.

## House-style review

Reviewed against `planning/PROGRAMMING_HOUSE_STYLE.md`: complete new adapter
header/source and two new tests; changed application command creation, binding,
state fields, cancellation predicates, terminal projection and failure dialog;
changed service stage ownership/outcome/traversal/cleanup paths; new service
test and revised test-root ownership; CMake registration and footer HTML/CSS.
Reviewed explicit initialized types, const inputs, named callbacks, synchronous
borrows, handle cleanup, the atomic request boundary, retained application
ownership, UI-thread-only controls, buffer reuse, conversion bounds and failure
order. Root made read-only native handle-owner inputs const and named atomic
load results during review. The handle's referenced OS file can still be
written; const applies to the C++ owner, not filesystem immutability.

No known house-style violation remains in the authored scope. This is not a
compliance certification of legacy application/service/test code, generated
forms, dependencies or all GUI.Forms. Existing broader allocation/error and
filesystem-race limitations remain outside this bounded change.

## Prior tested repairs

**MEASURED:** PR 12 merged as `6de7711ed576f703b2fab06f706ebd322e948211`
after both complete native runs `37098920849` and `37098923532` passed, including
Windows symlink tests. Tested and merged trees match
`2a98fb83218166dae5244ddc61956291a07894d0`.

PR 13 was rebased onto that main revision after its old ancestry conflicted.
The rebased head `21444bb734a44264a1a775e6df61c0ac8dc7e0c7` passed both complete
native runs `37100247471` and `37100250544`, and merged as
`2402893cc2bced3d03ff361d0e3346d83bffc029`. The complete tested/merged tree is
`277820c8b5073313902ea45fd4b5ccbb8fba2010`. Neither repair is claimed present in
the previously published `v0.001-alpha.8a13ba1` package.
