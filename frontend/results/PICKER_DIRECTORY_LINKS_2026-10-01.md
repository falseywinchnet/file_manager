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
native platform CI is needed to establish its executed coverage. Inspection of
this run's `LastTest.log` confirms Windows error 1314 skipped the link fixtures;
the 2/2 result establishes ordinary regression coverage, not execution of the
new symbolic-link assertions. macOS/Linux CI must execute those assertions.

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

## Windows native target correction

Native macOS and Linux CI executed both picker link suites successfully. Windows
CI could create links and exposed a real refusal that the local privilege skip
had hidden. Diagnostic commit `10fa664`, run `36869160214`, job `110392608199`,
reported `entered=0`: `std::filesystem::canonical(alias)` still returned the
alias, and the contained reader refused its symlink component. This was not a
path-comparison assertion mismatch.

Trusted navigation now uses `resolve_native_directory` in the private native
file adapter. Windows follows the directory with `CreateFileW`, obtains its
normalized final name from the handle, then closes the handle before allocating
or transforming the result. The bounded 32,768-wide-character buffer is allocated
before handle acquisition. DOS/UNC namespace conversion must preserve the exact
observed filesystem identity; unsupported or changed targets refuse. The picker
then rechecks admitted roots and invokes the unchanged no-link directory reader.
This observation is not an atomic grant for later file I/O. POSIX retains its
canonical-directory resolution. File aliases and write leaves are not admitted.

The first local draft preserved the extended namespace directly; both ordinary
picker suites refused root matching. Converting to the existing DOS/UNC path
representation with identity verification restored both suites: **2/2 passed in
0.33 seconds**. Local symlink assertions still skip with Windows error 1314;
the corrected native link path requires CI execution. All three authored source
files passed the spelling scan. Semantic review covered explicit types, buffer
bounds, handle cleanup before throwing operations, namespace/identity checks,
failure preservation, and the unchanged admitted-root policy. No known violation
remains in these hunks; unrelated legacy source is not certified.
