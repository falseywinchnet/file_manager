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
