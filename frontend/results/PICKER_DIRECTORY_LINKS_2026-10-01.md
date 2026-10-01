# Trusted local picker directory navigation

The owner reported unusable symbolic-link navigation while dogfooding SwiftEdit
on macOS. Source review found three refusal layers: the shared contained reader,
controller selection, and disabled view rows. The bounded contract amendment is
recorded in Orchestrator's `DOCUMENT_PICKER_LOCAL.md`.

The fix is confined to `document_picker.cpp` and `document_picker_view.cpp`.
Current trusted-local navigation canonicalizes an existing directory and checks
it against the explicitly admitted roots before invoking the unchanged no-link
reader. Browser location and subsequent observations use that canonical target.
Directory-link rows retain their alias path/identity and are classified only as
navigable folders; direct acceptance of a link leaf still fails. File-operation,
reader, Orchestrator-session, write-target and overwrite identity policies are
unchanged. Canonicalization is navigation, not an atomic authorization for later
host I/O; the host still revalidates before accessing a selected path.

Windows Shadow/GCC 16.2: rebuilt the two picker targets in
`.build/native-windows-x64/picker-sdk-build`; controller and view suites passed
2/2 in 0.37 seconds. Added generated-fixture tests cover canonical directory
navigation, initial link routes, folder observation, save into a canonical
directory, save-link refusal, broken/outside/file-link refusal, revoked grant,
unchanged session behavior, and selecting a filtered folder-link row followed
by the actual Open button. Fixture cleanup owns only newly created temporary
directories. Symlink tests explicitly skip if the platform cannot create links;
native platform CI is needed to establish its executed coverage.

The first view regression draft incorrectly treated a void selection setter as
Boolean and called a private accept method. Compilation refused both; corrected
test uses the public ObjectView setter, controller observation and Button action.

Reviewed every authored helper, controller/view hunk and added test against
`planning/PROGRAMMING_HOUSE_STYLE.md`: explicit types, borrowed row lifetime,
canonical versus alias identity, initialization, failure preservation, mutation
order and named stateful test calls. Four-file spelling scan found zero findings;
this is supplementary to source review. No remaining violation was found in
that scope. Unchanged source is not certified. Directory enumeration and link
resolution remain synchronous; no latency or native macOS fix claim is made
before the rebuilt native consumer is tested.
