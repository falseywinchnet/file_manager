# Document Picker house-style correction receipt

Date: 2026-09-30. Status: **OBSERVED scoped source correction; MEASURED focused
Windows validation; pending independent owner review.** No commit or publication
was made by this workstream. No frozen SDK, published archive, Plan Paint source,
or running application was modified.

## Authority and scope

Read the complete owner-supplied `programming-house-style (2).md` from Downloads.
Its SHA-256 is `2B16CEAD4EA11983009B6B7433CE12AC48701EEEE63A00DF4620EA7CD928652C`.
The relevant frontend instructions and `ORC-PCK-001` local selection contract
remain authoritative. This change does not widen scope, authority, or profiles.

Exactly these tracked files were changed by the picker workstream:

- `frontend/include/file_manager/document_picker.hpp`
- `frontend/include/file_manager/document_picker_view.hpp`
- `frontend/src/document_picker.cpp`
- `frontend/src/document_picker_view.cpp`
- `frontend/tests/document_picker_tests.cpp`
- `frontend/tests/document_picker_view_tests.cpp`
- `frontend/examples/document_picker_consumer/main.cpp`
- `frontend/docs/document_picker_house_style_2026-09-30.md` (this receipt)

`frontend/examples/document_picker_consumer/CMakeLists.txt` was reviewed without
changes: explicit C++20 target, installed public dependency, and named CTest. No
production Python, PowerShell, or generator was authored for this package. Scratch
transformation scripts, baseline copies, build products and probes remain under
`frontend/.build/picker-style-audit/`; they are not package/tooling deliverables.
The parent owns `tools/check_house_style.py`; it was used, not edited here.

## Baseline and resulting findings

The parent's lexical checker inspected the same seven C++ files before and after:

| Review category | Baseline | Result |
|---|---:|---:|
| Inferred type spelling | 78 | 0 |
| Arrow access / trailing return candidates | 145 | 0 |
| Anonymous callable candidates | 29 | 0 |
| Other checker categories | 0 | 0 |

Zero spelling findings is not semantic certification. Source review additionally
found captured callback lifetime dependencies, implicit teardown order, mutations
inside test assertions, repeated filter discovery/normalization allocation,
positional result states, and update flags that could remain set on an exception.

Corrections:

- Fourteen view callbacks are named private methods bound by non-owning typed
  GUI.Forms delegates. Binding, event connection and token ownership are separate
  statements. Tokens reserve their known count once. The view destructor revokes
  them before members are destroyed; there are no callback ownership cycles.
- Terminal emission is the last access to the view. A named nested update scope
  restores the previous flag on normal and exceptional exits. Cancellation during
  control synchronization revokes authority immediately and preserves one pending
  result until the outermost update finishes. Acceptance is refused while control
  state is being synchronized. Scope destruction itself emits no terminal event.
- Controller traversal uses explicit types, stable counted compaction and named
  result construction. Accepted/unavailable/overwrite factories keep status and
  payload relationships together. Filesystem conversions and result publication
  are separated into visible steps.
- Name and type matching use borrowed basename views without per-row normalized
  string allocation. The active type filter is resolved before the refresh loop.
  Duplicate selection validation compares the already-bounded input prefix (at
  most 32 IDs), without allocating hash nodes per selection. Filesystem snapshot
  storage and GUI.Forms-owned item vectors retain their existing ownership.
- Immutable root/type display choices are constructed once and transferred to
  GUI.Forms. Directory rows still require a fresh GUI.Forms-owned vector because
  that public API takes ownership; no unsupported scratch-retention API was added.
- Tests and the external consumer use named recorders/listeners, explicit types,
  fixture ownership and mutation-before-assertion sequencing. Existing profile,
  identity, overwrite, hidden visibility and grant semantics are retained.

## Public compatibility

Public method signatures and request/result meanings are unchanged. The view
now has an out-of-line destructor and one private optional pending-completion
record. The parent approved the necessary layout change before it was made.

**MEASURED on Windows x64 / MinGW GCC 16.2.0 / current matching GUI.Forms headers:**
`sizeof(DocumentPickerView)` changed from 1120 to 1232 bytes;
`sizeof(FileSelectionController)` remained 800 bytes. These values are evidence
for this compiler/environment, not a portable ABI promise. A fresh matching
picker SDK and clean consumer rebuild are mandatory before promotion. Existing
frozen SDKs deliberately retain their previous layout.

## Validation

A new Debug build was configured under
`frontend/.build/picker-style-audit/build`, reading the existing GUI.Forms SDK.
Only the picker targets and their required model dependencies were built, using
`--parallel 2`. No shipping app executable was rebuilt or launched here.

**MEASURED:** both focused suites passed, including the original selection,
identity, overwrite, navigation, keyboard and profile cases and these additions:

- retained controls invoked after view destruction cannot call dead delegates;
- a completion listener actually resets the owning `unique_ptr` during emission;
- cancellation from a path-change listener waits until reload borrows are released,
  then emits once and may destroy the view;
- nested reload restores the outer update state and preserves the final location;
- injected subscriber exceptions restore update state, emit nothing while unwinding,
  and permit subsequent callbacks/cancellation;
- cancellation followed by an injected exception is retained rather than dropped;
- extensionless/leading-dot names, uppercase extensions, final-dot selection and
  wildcard backtracking preserve filter behavior.

The final focused run passed **2/2**. Its log is
`frontend/.build/picker-style-audit/build/Testing/Temporary/LastTest.log`.
An earlier compile exposed an explicit `string_view`-to-`string` construction
requirement; that was corrected and subsequent fresh builds passed. Tests run
against an old executable after that failed compile were not counted as evidence.

A private scratch install used the `DocumentPicker` component and prefix
`frontend/.build/picker-style-audit/sdk`. The standalone consumer configured from
scratch against that prefix and the unchanged GUI.Forms SDK, compiled/linked,
and passed **1/1**. Its log is
`frontend/.build/picker-style-audit/consumer/Testing/Temporary/LastTest.log`.
This scratch prefix is validation output, not a published SDK replacement.

The explicit seven-file `tools/check_house_style.py` run reports **zero** spelling
findings/review candidates. `git diff --check` passes for the changed C++ scope.

## Remaining limits and review obligations

No known forbidden C++ spelling remains in this scope. The parent still owns
independent semantic review; the checker cannot prove lifetime or failure policy.
The host must keep the view alive during nonterminal control dispatch unless it
uses the tested cancel/completion route. Arbitrarily deleting it from an unrelated
control callback, bypassing that protocol, is outside the borrowing contract.

The existing directory snapshot remains synchronous and potentially large. This
work removes avoidable filtering allocation and fixes callback sequencing, but
makes no large-directory latency or allocation-failure recovery claim. A thrown
subscriber can leave presentation controls partially updated; it does not leave
the update flag stuck, and the host can retry or cancel. Cross-platform and native
visual/accessibility campaigns were not repeated for this style correction.
No claims are made about other workstreams' files or the full repository audit.

## Exact runtime environment for reproducing the focused tests

The focused test targets do not invoke the application's post-build DLL staging.
`Enter-WindowsToolchain.ps1` supplies the MinGW runtime directory, but does not
add the GUI.Forms SDK DLL directory. A view-test exit `0xc0000135` with only the
toolchain PATH therefore indicates a missing runtime DLL, not a failed assertion.
The successful test invocation used this process-local environment:

```powershell
Set-Location C:/Users/Shadow/file_manager
. ./tools/Enter-WindowsToolchain.ps1
$env:PATH = 'C:/Users/Shadow/file_manager/gui_forms/.build/shadow-sdk/bin;' + $env:PATH
ctest --test-dir frontend/.build/picker-style-audit/build -R document_picker --output-on-failure
ctest --test-dir frontend/.build/picker-style-audit/consumer --output-on-failure
```

The relevant DLL search directories are the unchanged
`C:/Users/Shadow/file_manager/gui_forms/.build/shadow-sdk/bin` and the toolchain's
`C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin`. No DLL copy, frozen SDK
mutation, machine PATH change, or running-application change is required.

## Parent integration and failure cleanup

**MEASURED:** the parent installed a new GUI.Forms review SDK and rebuilt the
picker from scratch against it under
`.build/sdk-checkpoints/house-style-review/windows-x64/`. Both picker suites and
the independently installed consumer passed. The frozen historical SDK was not
changed.

**OBSERVED:** a final test-source review found that assertion helpers called
`std::exit`, bypassing fixture and subscription cleanup. Assertions now throw to
a top-level reporting boundary, allowing local owners to unwind. Test fixtures
exclusively create their generated directories, refuse an existing directory,
and clean up on setup failure and test failure as well as normal return. The
view fixture guard borrows its stable path for the containing test call. This
changes test cleanup, not the installed picker interface or implementation.

**MEASURED:** after that correction, the parent rebuilt both picker tests against
the new review SDK and passed 2/2 in 0.34 seconds on Shadow. The failure-cleanup
property is established by the scoped ownership and reporting boundary; this
run did not inject filesystem allocation failures.
