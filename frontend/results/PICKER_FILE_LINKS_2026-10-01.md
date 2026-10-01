# Trusted-local file alias selection

**GIVEN:** the owner's picker symlink complaint includes opening files. Existing
trusted-local selection permits an owned first-party presentation without a
daemon; it does not grant file I/O or infer additional roots.

**OBSERVED implementation:** Open File, Open Files and Import Files admit visible
file-alias rows. Acceptance checks the snapshot alias revision, resolves an
existing regular target, checks its explicit admitted root and unresolved route,
then observes the target and checks the alias again. The returned path and
identity describe the target. Consumers still revalidate through their no-follow
I/O. Resolution and these observations are not an atomic reservation. Filters
apply to the displayed alias. Save/Export and daemon-session leaves stay no-follow.

The private native resolver now shares directory/file resolution by required
object type. Windows follows an attribute handle, obtains the final DOS/UNC path,
closes the handle before string/path work, and verifies the normalized path names
the same object. POSIX uses canonical resolution and the required target type.
No public picker API or installed header changes.

**MEASURED local:** Shadow Windows GCC 16.2 Release, existing picker SDK build:
controller/view targets compile; focused CTest 2/2 pass in 0.37 seconds. Windows
error 1314 skips symlink fixture assertions on this desktop. This is compilation
and ordinary-regression evidence, **not execution evidence for the new alias
cases**. Native CI on Windows/macOS/Linux remains required. An initial view-test
compile used an incorrect event API name; corrected to the existing typed
Delegate/SubscriptionToken API before this passing build.

Authored cases cover three read profiles, same-root and explicitly admitted
cross-root targets, target identity, target basename differing from the filtered
alias, broken/cyclic/outside targets, alias replacement, revoked authority,
whole-result refusal for mixed selections, unchanged session/Save/Export refusal,
and actual retained Open-button activation. Existing directory-link tests remain.

**OBSERVED source review:** reviewed the changed native resolver/declarations,
controller selection and acceptance branches, private profile helper, view row
enablement and the new tests in `native_file.cpp/.hpp`, `document_picker.cpp`,
`document_picker_policy.hpp`, `document_picker_view.cpp`,
`document_picker_tests.cpp`, and `document_picker_view_tests.cpp` against
`planning/PROGRAMMING_HOUSE_STYLE.md`. Explicit types/read-only inputs, named
callback with retained subscription lifetime, non-throwing handle interval,
initialized observations, reserved result storage, failure without partial
publication, alias borrows and replacement order were checked. Seven-file
spelling check reports zero findings. This is review of the changed scope; it
does not certify all legacy code in these files.
