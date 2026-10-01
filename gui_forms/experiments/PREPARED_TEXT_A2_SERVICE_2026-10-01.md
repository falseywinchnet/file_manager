# Independent prepared-text service and grayscale raster checkpoint

**OBSERVED:** continuation of agreement 003 in
`orchestrator/spec/contracts/GUI_PREPARED_TEXT_DEVELOPMENT.md`, after the historical
shutdown pause was revoked. This supersedes the restart handoff's description
of service/raster files as absent. It does not announce an installed capability.

## Implemented development scope

- Public `prepared_text.hpp` hierarchy: service, worker session, immutable font
  lease, movable input/layout/mask owners, identity and status records.
- Private `prepared_storage.hpp/.cpp`, `prepared_service.cpp`, and
  `prepared_raster.cpp`: one service ledger across sessions and retained owners;
  owned mapping/proof/display text; checked full-key authority; bounded worker
  admission, completion, discard, cancellation, wake posting, close and join.
- Standalone `gui_forms_prepared_text` build target and two focused tests,
  controlled by the OFF-by-default `GUI_FORMS_BUILD_PREPARED_TEXT` option.
  Root owns the CMake changes. Draft headers are excluded from blanket install;
  no target export, new SDK or application stage is published by this checkpoint.

Each payload reserves one of three slots and a full 8 MiB before queuing.
Input ownership transfers without copying its arrays; its separate charge ends
only after the payload reservation owns them. Cancelled/failed/ready work keeps
its reservation until discard/join or actual retained-payload destruction.
Recorded-command retention is tested through the private typed storage owner;
actual display-command recording/replay is not implemented here.

The font ledger permits at most two banks with 8 MiB aggregate encoded bytes,
4 MiB per face and eight faces per bank. Copies of one lease extend one charge.
Admission validates scalable Unicode faces before publishing a bank; exact
requested primary role/weight/style is required by desire. Face indices 0..255
match the existing private engine. Default variation only, no synthetic styles.
The prepared worker uses a preallocated bounded registration table and omits
unused font-family strings. Its actual persistent table capacity is included
in the reported shaping workspace peak and deducted from the 16 MiB allowance.

Rasterization opens independent FT resources on its caller, never borrowing
mutable worker faces. It uses the exact retained font bytes, quantized 26.6
size at 72 DPI, and prepared device-pixel positions. Two passes determine ink
extent then fill a candidate gray8 mask. Axis/product checks precede controlled
pixel allocation. The service ledger charges at most two masks / 8 MiB,
including old output and candidate. Failure preserves old output. A zero-ink
success owns a 0x0 mask with zero bearing and separately retained logical metrics.

These are requested first-party storage counts, not a process hard quota.
Allocator/shared-pointer bookkeeping, thread stacks and opaque FT/HB/SheenBidi
allocation/cache peaks are not measured here. FT bitmap guards run after native
allocation and establish indexing safety only. No memory-exhaustion injection,
thread-sanitizer run, responsiveness benchmark or native visible-window test is
claimed. Native shaping is noninterruptible; service shutdown joins and may block.

## Validation

**MEASURED:** Windows Shadow, borrowed Plan Paint MinGW GCC 16.2, C++20 Release,
existing pinned FT/HB/SheenBidi and bundled Carlito fixture. Configure/build:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake -S gui_forms -B gui_forms/.build/house-style-text -DGUI_FORMS_BUILD_PREPARED_TEXT=ON
cmake --build gui_forms/.build/house-style-text --target gui_forms_prepared_text_service_tests gui_forms_prepared_text_raster_tests gui_forms_text_store_tests gui_forms_bounded_grapheme_tests gui_forms_bounded_shape_tests gui_forms_harfbuzz_font_engine_tests --parallel 2
ctest --test-dir gui_forms/.build/house-style-text --output-on-failure -R 'prepared_text|bounded|harfbuzz|text_store'
```

Final raw output is in `PREPARED_TEXT_A2_SERVICE_TESTS_2026-10-01.txt`.
The two new suites and four dependency suites pass. Durations are correctness
test durations, not application latency or throughput measurements.

Service tests cover retained third-generation accounting/fourth refusal,
command-held retirement, no new-session budget escape, input transfer and
failure preservation, invalid desire, same-key epoch revocation, failed
adoption preserving old output, cancellation versus retirement, shutdown wake
revocation/join, failed coverage retaining a slot, exact 8 MiB aggregate font
capacity, shared lease lifetime, paragraph proof, atomic source mapping,
invalid interior endpoints, and line-separator refusals before busy admission.

Raster tests cover positive ink and baseline bearing, combining composition,
fractional scale and all admitted scale endpoints, positive half-tie rounding,
minimum/maximum size-scale combinations, equivalent-device-size pixel equality,
logical advance conversion, deterministic repeated pixels, third mask refusal,
replacement after retirement, stale and oversized requests preserving old
pixels, service shutdown lifetime, move-empty ownership and empty/space masks.

Strict C++20 syntax checks pass with `-Wall -Wextra -Wconversion
-Wsign-conversion -Werror` for the three implementation files, the modified
HarfBuzz engine and both new test translation units. Existing public/vendor
headers are system includes for this check; this does not certify them.

## House-style source review and remaining boundary

Reviewed against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`: all five
new public headers, all four private prepared files, both test translation
units and their shared test helper; the newly authored bounded-registration
declaration/definition/registration branches and position-conversion/check
hunks in HarfBuzz. The earlier bounded Unicode/shaper scope is documented in
`PREPARED_TEXT_BOUNDED_REVIEW_2026-10-01.md`. Unchanged legacy/vendored code is
not certified. Root separately owns CMake review.

Review covered explicit types, named wake behavior and retained target lifetime,
control-executor confinement, mutex ordering and publication, worker join before
native destruction, source/display units, checked allocation arithmetic,
initialization, exact byte-owner transfer, failure-before-output-replacement,
borrow lifetimes and fixed repeated-loop storage. Fixed a partial-construction
cleanup dereference, removed expression returns, added explicit endpoint-span
guards, aligned face-index admission, distinguished allocation from native
initialization failure, and bounded registration metadata before promotion.
No remaining violation was identified in that authored scope. Functional tests
and a spelling scan were supplementary, not substitutes for this review.

Independent coordinator review remains pending. Window/service attachment,
typed Painter/display commands, replay and Windows DIB mask compositing remain
the next coordinated integration slice. There is no production backend readiness
or installed capability yet. Wrapping, tabs, monochrome, hit testing, selection,
caret/editing and SwiftEdit D4 remain outside this first visual-only profile.

### Coordinator review corrections

**OBSERVED:** independent review found that per-service session counters could
issue equal session/epoch pairs in different services. That allowed a foreign
expected authority to pass the local comparison. Session IDs now come from one
process-wide atomic counter with checked exhaustion and no reuse; the public
authority record retains its session/epoch shape. A two-service regression uses
equal epochs and otherwise matching keys, proves foreign raster/adoption refuse,
checks old output and ready-slot preservation, and then adopts with the correct
authority. Registry reconciliation was requested through the coordinator.

The review also found an unlocked interval between the final raster authority
check and output replacement. Both now occur under the same authority mutex.
The public contract allows a separate raster executor with exclusive output
ownership: concurrent desire/cancel linearizes before publication (stale) or
after successful publication. A success does not promise continuing authority
after return. This locking/order correction was source-reviewed; no deterministic
concurrent raster/cancel scheduling fixture or sanitizer evidence is claimed.

Strict compiler checks pass on the changed service/storage/raster and service
test scope; the final six-suite rerun passes 6/6 in 0.79 s. The adjacent source
hash CSV and raw test receipt have been refreshed. Coordinator acceptance is
still pending; this correction does not promote backend or SDK availability.
