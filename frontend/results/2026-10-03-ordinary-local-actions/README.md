# Ordinary local action checkpoint

Status: **Windows correctness passed; native macOS/Linux and package acceptance pending**.
Contract: `../../../orchestrator/spec/FRONTEND_ORDINARY_LOCAL_ACTIONS.md`.
Audit: `../../planning/ORDINARY_LAUNCH_AUDIT_2026-10-03.md`.

Normal startup now supplies an ordinary local operation controller. New Folder,
same-parent Rename (inline and Name property), and their one-step Undo require
no quarantine, Engine ID, or service. Each queued action retains its requested
parent identity and the selected source record where applicable. It refuses
observed parent/source replacement, link/reparse parent routes, filesystem-root
renaming and occupied destinations. Folder Undo removes only the same empty
directory. Rename Undo remains retryable when the original name is occupied.

Explicit `--read-only` remains available. The historical protected read-only
Mac launcher now passes that option explicitly; ordinary daily launch uses
the default. `--allow-mutations --quarantine` still selects the distinct
protected profile. Ordinary mode does not enable Copy/Move/Delete, clipboard
transfer state or internal drag. The package README and Windows search-launch
notes describe that distinction.

## Windows verification

Release built with the existing Shadow GCC toolchain and the installed
`.build/details-sdk`. `file_manager`, operation-model, interaction and transfer
targets built with one compiler job. Three focused CTest suites passed:

- `file_manager_file_operations_tests`: 0.37 s.
- `file_manager_application_interaction_tests`: 3.33 s.
- `file_manager_application_transfer_tests`: 1.39 s.

`WindowsLastTest.log` preserves the output. These are correctness-test durations,
not performance measurements. New checks cover generated repository-category
contents, navigation outside the initial directory, command availability,
F2 basename editing, Name property editing, create/rename/undo, selected identity
after refresh, stale queued parent, occupied destinations, nonempty folder Undo,
and adapter refusal of protected operations. Actual Home construction is checked
without modifying Home; mutations use generated fixtures.

Windows symlink fixture checks explicitly skipped with error 1314. Their pass
cannot be claimed here; Unix native runs must exercise the link cases.
The final executable rejects both `--read-only --allow-mutations` and
`--read-only --quarantine C:\unused-fixture` with exit 2 before the native loop.
Root used `Start-Process -Wait -PassThru` to obtain GUI-subsystem exit codes.
An initial direct PowerShell invocation reported stale `$LASTEXITCODE=0` while
the child correctly printed errors; that invocation is not exit-code evidence.

Retained development failures: the original local build cache named an older
SDK without the pre-existing Details sort interface; linking against the current
Details SDK corrected compilation. The staged application DLL was then stale,
causing loader error `0xc0000139`. Copying the exact linked SDK's DLL into the
build output corrected both application suites. No shared GUI source or SDK
was changed to conceal the mismatch.

## Native window acceptance and remaining scope

The existing macOS native-window test now selects ordinary policy. After its
text/PNG/unsupported/Details stages, it invokes menu New Folder, cancels initial
naming, invokes Undo, performs F2 basename Rename while preserving `.txt`, and
invokes Undo again. It observes filesystem identity and displayed rows at each
stage, verifies transfer/deletion commands remain disabled, and saves
`native-local-actions.png` for CI review. Delivery is synthetic semantic/key
input through the real AppKit window model, not physical input. The application
worker outlives neither the owned fixture nor its retained callback state.
Fixture cleanup checks acquired identity and its canonical temporary boundary.

The native test has not run on this Windows host. Launcher runtime checks require
macOS; Python package-tool syntax compilation passes locally. Normal packaged
entry-point interaction, close/reopen, macOS/Linux correctness, physical-input
dogfood, and ordinary transfer/Trash behavior remain open. Identity observation
and path publication are separate operations; parent rechecks are not an atomic
snapshot or complete concurrent-substitution guarantee. Undo remains one step,
in memory, and ends when the app exits.

## Native rejection and repair follow-up

**REJECTED checkpoint `922e0fc`:** both macOS native runs failed while Linux
passed. The retained `MacRejected922e0fcLastTest.log` comes from push run
`37110019348`, job `111165956703`; the PR run `37110027712` reproduced the same
two failures. Frontend passed 14 of 16 tests. The failures are retained rather
than treating a successful build as native acceptance.

- The ordinary interaction fixture tried to navigate from its generated
  `Documents` launch directory to its temporary parent. macOS admits Home,
  `/Volumes` and the launch directory, so that parent was not admitted. The
  test probe now explicitly admits the generated second location before window
  creation and verifies that it is outside the unchanged launch root. Product
  navigation and file-operation authority are unchanged. The sibling rebuilt
  and ran the Windows interaction suite successfully (3.18 s).
- The real macOS window completed text/PNG/unsupported previews, Details sort,
  New Folder and its Undo, then failed F2 basename Rename. Source tracing found
  that explicit semantic item actions retained Details' internal header focus
  after F6/sort. The header consumed F2 despite the object control being focused.
  The focused regression exercises select, focus, press and show-menu actions
  after header focus; each must publish focused item semantics and leave F2
  available to the application. The original native test remains unchanged in
  its acceptance assertions, with separate focus/key/editor diagnostics added.

**MEASURED local regression:** before the control correction, the new collection
test failed with `semantic item action must retire internal header focus`.
`WindowsRejectedSemanticFocusLastTest.log` preserves that result. After the
correction, collection controls and the Details allocation-failure suite passed
(0.31 s and 0.12 s respectively) in `.build/prepared-window-input`, Windows
MinGW GCC 16.2.0 Release, one compiler job. `WindowsSemanticFocusLastTest.log`
preserves the rerun. These are correctness durations, not latency measurements.

**House-style review:** root reviewed the complete added navigation probe,
setup and outside-root assertion; native failure diagnostics; internal focus
reset; and named four-action regression against the complete programming house
style. Types and state are explicit, the focus state is committed before focus
notification, no new callback or retained borrow is introduced, and the four
semantic snapshots have local owners. The control correction adds no item loop,
allocation or model replacement. No remaining violation was identified in this
authored scope. The separate spelling scan reports four files, zero candidates;
it does not certify unchanged code. `focus-repair-source-sha256.json` identifies
the reviewed source with LF-normalized UTF-8 hashes; the original receipt remains
historical evidence.

Native rerun remains pending. No packaged release is promoted by these changes.

## Original checkpoint source review

Root independently reviewed all authored hunks in the eight sibling-owned files:
`file_operations.hpp`, `file_operations.cpp`, `application.hpp`,
`application.cpp`, `application_jobs.hpp`, `main.cpp`,
`file_operations_tests.cpp`, and `application_interaction_tests.cpp`.
The sibling separately reviewed the same scope. Root also reviewed the new
macOS harness helpers/stages and cleanup, the explicit read-only launcher
argument and assertion, package text, and artifact registration.

Review applies the complete `planning/PROGRAMMING_HOUSE_STYLE.md`: explicit
initialized types, named callbacks with owned state, serial operation state,
queue-owned snapshots, borrow lifetimes, execution order, conversions,
failure/postcondition reporting and repeated work. No remaining violation was
identified in that authored scope. The C++ spelling scanner is a separate
regression aid; neither it nor these tests certifies unchanged legacy code,
generated source, dependencies, or full repository conformance. The SHA receipt
uses UTF-8 text with LF line endings to identify reviewed source across checkout
newline conversion.
