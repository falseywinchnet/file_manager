# New Folder naming and deferred focus

**GIVEN:** The owner requests complete everyday operations and practical,
responsive interaction. The everyday-operation audit records that New Folder
creates a deterministic directory but does not offer its name for editing.
ADR-017's collision-safe creation and protected mutation scope remain unchanged.

**OBSERVED implementation:** Successful creation in the current context refreshes
the directory, selects the returned filesystem identity, and opens the existing
rename editor with the full directory name selected. Enter uses the ordinary
identity-checked rename. Escape keeps the created default-named directory and
its existing one-step creation undo; cancellation of naming does not delete it.
An existing New folder is preserved and the next deterministic name is offered.

Creation jobs carry a named context with navigation, search and request
generations. Only the latest creation in the unchanged context can request
naming. A completed older operation still updates the UI's undo snapshot and
reports its committed path; it cannot replace a newer navigation or edit.
The deferred naming record owns the created identity and expected refresh/search
generations. A new navigation retires both that record and its matching pending
selection. A superseding search retires the creation refresh. An unavailable
listing retires naming. Current text-editor focus prevents automatic naming.
If the user begins editing Location after refresh was queued, that refresh
preserves the editor's text/focus while updating the folder's object list.

No new dialog, filesystem mutation policy, public API, synthetic object identity,
worker thread or retained path borrow was added. Naming is an offer after actual
creation, not a pre-creation dialog or provisional directory. A subsequent real
rename replaces creation undo with the existing rename undo semantics.

## Verification

**MEASURED:** Shadow Windows, MinGW GCC 16.2 Release, one compiler process,
installed development GUI.Forms SDK. Final build and all 15 CTest suites pass in
4.91 seconds; `WindowsLastTest.log` retains output. Four selected C++ files have
zero house-style spelling candidates.

Five generated-fixture application cases use the real New Folder command:

1. Preserve a preexisting New folder, focus/select New folder 2, type a new name,
   commit Enter, and verify the created filesystem identity at the new name.
2. Escape preserves the default-named directory, restores object focus, and
   leaves creation undo available.
3. A named worker barrier delays creation while navigation to another folder is
   queued. Creation must neither replace that navigation nor open naming there.
4. Another barrier allows creation to commit but delays its refresh. Newer
   navigation must retire the deferred naming request.
5. After creation, while refresh is delayed, the user focuses Location and types
   draft text. The refreshed list must show the new folder without changing the
   Location draft/focus or opening the rename editor.

The fixture owns its exclusively created temporary root, validates canonical
scope and identity before cleanup, and outlives application shutdown. Named
barrier release owners prevent failure-path worker-join deadlocks. Existing
copy cancellation tests remain in the same asynchronous-operation suite.

`InitialFocusFixtureFailure.log` preserves a failed extension of case 5 that
attempted to focus Search in an application without a configured search provider.
That field is correctly unavailable in this fixture. The final case uses the
real available Location command; no product availability check was weakened.
The added Location preservation follows source review of the queued refresh's
path-text/breadcrumb update; the old behavior was not separately reproduced as
a baseline test failure.

Native three-platform CI and physical-keyboard/visual dogfood of this new naming
flow remain pending. These are retained-application input and real filesystem
tests, not a whole-product usability acceptance claim. Exhaustive interruption,
settings-transition and multiple-creation schedules are not claimed measured.

## House-style review

Reviewed against `planning/PROGRAMMING_HOUSE_STYLE.md`: the context and pending
record definitions, CreateFolderWork/Ready, request/application result paths,
navigation retirement, guarded directory projection, pending selection/focus
handoff, new test probe accessor and complete five-case test implementation.
Explicit initialized fields, named callbacks/predicates, synchronous read-only
borrows, shared application lifetime, UI-owned counters/state, atomic generation
reads, checked object identity, operation order and terminal cleanup were
reviewed. Context and results are owned across both queues; no borrowed editor,
path, entry or snapshot survives a callback. No per-object loop allocation was
added to directory publication by this change.

No known house-style violation remains in this authored scope. Legacy
Application, GUI.Forms, unrelated tests and generated forms are not certified.
