# Frontend house-style correction — ongoing

Status: **OBSERVED incomplete correction; no whole-frontend compliance claim**.

The owner rejected the delegated implementation and required the complete
`C:/Users/Shadow/Downloads/programming-house-style (2).md` to govern source,
tests, and authored tooling. The frontend application chat owns all first-party
frontend application code except `document_picker*` and `tests/picker_consumer`,
which remain with the picker chat. No worker agents, commits, publication,
running-app changes, or frozen-SDK changes are part of this work.

## Baseline

**OBSERVED:** the root `tools/check_house_style.py` lexical checker, applied to
33 non-picker C++ source/header/test files at the starting HEAD, reported 3,022
findings: 1,973 arrow/trailing-return forms, 757 inferred-type forms, 291 lambda
review candidates, and one ranges/views candidate. Unlike the initial rough
regular-expression inventory, this checker excludes comments and literals.
These counts do not establish semantic compliance or noncompliance by themselves.

## Checkpoints

- **OBSERVED:** leaf model and test changes replace inferred types, anonymous
  predicates and fault callbacks with named records/functions, make result
  construction and return explicit, and initialize owner state. No-follow
  identity, read bounds, staging, cancellation, and collision policy remain.
- **OBSERVED:** directory sorting uses a private named comparator without
  allocating lowercase strings per comparison. It retains directories-first,
  byte-wise `tolower`, and exact-name tie-breaking. In-memory regression cases
  cover mixed case, equal-folded names, prefixes, UTF-8 bytes, and directories,
  independently of case-sensitive filesystem support.
- **OBSERVED:** the About listener and application command/event listeners have
  named targets. Application destruction explicitly revokes subscriptions and
  accelerators after stopping its worker. The application and interaction tests
  still contain outstanding inferred types and anonymous deferred behavior.
- **OBSERVED:** Windows argument storage now has a local resource owner so a
  conversion/allocation exception cannot bypass `LocalFree`.
- **OBSERVED:** icon names use named fields; raster callbacks are named methods
  or explicit loops. Resource/density tests pass, but exact before/after raster
  byte comparison remains outstanding at this checkpoint.

## Measured verification and negative evidence

**MEASURED:** Windows leaf suites passed at incremental checkpoints: checksum,
preview, drag and Windows platform 4/4 in 0.40 seconds; filesystem model 1/1 in
0.15 seconds; file operations/platform commands 2/2 in 0.43 seconds; subsequently
Windows fixtures/House art 2/2 in 0.34 seconds. These are scoped observations,
not a final full rebuild result.

**MEASURED negative result:** the application interaction test built using the
current development SDK crashed after 0.19 seconds when run beside the older
latency-build DLL. The preserved `mismatched-runtime-negative.log` records that
failure. The latency DLL SHA-256 was
`e89cf95f7c5599c0f3718589f456847f5bdfdaae13ba2586a7e6d42a5ff11b9b`;
the development SDK DLL was
`3c66e1c188a183eff2c93833c7c9a3781de57936a14721f00fa9ddbc865c664f`.
The same test executable passed after being copied into a separate verification
directory with the matching SDK DLL. The latency directory's user-running DLL
was not replaced. Existing Windows error-1314 symlink fixture skips remain;
the junction tests execute.

**OBSERVED stronger verification in progress:** a fresh Ninja build in
`frontend/.build/frontend-style-audit` uses only the complete frozen SDK at
`.build/sdk-checkpoints/dbe3766/windows-x64/gui-forms-sdk`, selected through
`CMAKE_PREFIX_PATH`, with GCC 16.2, Release, C++20, and two build jobs. Picker
targets compile current sibling-owned source; no old picker object is reused.

## Outstanding review

Complete the application and interaction-test correction, then review all
changed units beyond spelling: borrowed callbacks, revocation, shutdown,
execution order, conversions, failure cleanup, reusable storage, and remaining
compound expressions. Finish the authored shell-tool review; macOS-only
launcher execution is not established by a Windows build. Run a final coherent
build/test checkpoint and record the exact changed-file list. The root chat
will independently review before any commit or publication.


## Final handoff before owner shutdown deadline

**MEASURED:** fresh Release/Ninja/GCC 16.2 C++20 build in
`frontend/.build/frontend-style-final` completed against the complete
`.build/sdk-checkpoints/house-style-review/windows-x64/gui-forms-sdk`.
Final `cmake --build ... --target all file_manager_application_latency_benchmark
--parallel 2` passed, including the excluded-by-default benchmark target.
All **12/12 CTest suites passed in 2.47 seconds**, including application
interaction in 1.88 seconds and the new object ordering regression.
`final-ctest.txt` preserves the final test output. This final section supersedes
the earlier in-progress build/checkpoint descriptions.

**MEASURED:** all 45 first-party frontend C++ files under include/src/tests/examples
had **zero spelling findings/review candidates** in the root checker. The initial
33-file non-picker baseline had 3,022. These scopes differ because the final scan
also includes sibling-owned picker and interaction files and new private files.
Zero lexical findings is not proof of every semantic house-style rule.

**OBSERVED corrections:** named worker/completion/cancellation records expose
captured state; retained UI listeners use named methods and explicit bindings.
Three directory workers receive show-hidden snapshots on UI submission rather
than reading the UI-owned mutable boolean. Undo availability now reaches the UI
through OperationResult instead of reading the worker's mutable undo record.
`drain_ui` checks stop before every pending callback, including a batch already
removed from the queue; the sibling's reentrant-stop regression passes.
Worker jobs deliberately drain during join, preserving admitted mutation
semantics; cancellable reads still observe the atomic stop/generation flags.
Runtime explicitly stops Application before releasing its ownership. Generic
worker-error completion borrows the owner under that stop/join guarantee.

**OBSERVED:** object sorting selects the mode before stable_sort and compares
folded bytes without allocating lowercase strings during comparisons. Named
mode regressions cover folders first, kind/size/modified order, stable equal-fold
names, prefix names, UTF-8 bytes and missing-observation name fallback. Directory
ordering keeps its distinct exact-name tie-break. Scalar results are calculated
before return; owner records have explicit initial state; Windows argument
storage, benchmark fixture storage and ordinary preview/platform fixtures have
cleanup owners. Assertion failures now unwind the checksum/preview/platform/drag
test main boundaries. The benchmark compiles; it was not run as a new performance
measurement or used to launch a native window.

**MEASURED raster equivalence:** starting-HEAD versus revised house_art.hpp
produced identical concatenated BGRA for all 17 icons at extents
17, 22, 34, 42, 44, 72, 84 and 144 (136 cases). Both outputs have SHA-256
`8902e68889608bf3b7d5f8342f790724e08e0145c2cb0b6bafb3a958505d1b08`.
The temporary comparison programs and byte outputs remain ignored under
`frontend/.build/house-art-equivalence`. Icon extent validation now rejects
nonfinite/out-of-range logical dimensions before image-list allocation and
bounds private raster extents before allocating the pixel buffer.

**MEASURED/OBSERVED tooling:** seven authored .sh/.command files passed MSYS
`sh -n`. Their quoted-path, failure and manifest-validation flow was reviewed.
macOS launcher execution and native macOS compilation were not available in this
Windows checkpoint. Existing Windows symlink privilege skips remain; native
junction checks execute. No running application, frozen SDK, Plan Paint source,
commit, release or publication was changed by this frontend chat.

**Remaining semantic review, explicitly not certified:** the owner shutdown
instruction froze source immediately after passing validation. The full remaining
compound-expression review was not exhaustively completed. In particular,
file_operations_tests.cpp still contains effectful undo_last() calls embedded
inside some assertions; filesystem observation/conversion chains remain in some
leaf tests. These are outstanding style refinements, not newly reported test
failures. Visual prototype fidelity, rich breadcrumb material, native visual
verification, live installed-service probes and performance measurements remain
open and are not established by this handoff. The integration chat owns commit
and push and has been told these limits.

### Application-chat changed source inventory

- `frontend/src/application_jobs.hpp`
- `frontend/src/directory_name_order.hpp`
- `frontend/src/object_order.hpp`
- `frontend/tests/object_order_tests.cpp`
