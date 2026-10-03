# Private prepared-window input specimen

Status: candidate implementation evidence only. The coordinator assigned the
private input owner, its tests, and development-only CMake integration. This
does not accept the whole prepared-window candidate or export an API.

## Reviewed scope and changes

- `src/core/text/prepared_window/prepared_window_input.hpp`
- `src/core/text/prepared_window/prepared_window_input.cpp`
- `tests/prepared_window_input_tests.cpp`
- `CMakeLists.txt`: optional core source and focused test declarations only.

The existing untracked owner/validator was reviewed without changing its source.
The test now verifies independent ownership of all four arrays and independent
ledger admission/retirement. CMake includes the specimen only when
GUI_FORMS_BUILD_PREPARED_TEXT is enabled. No public headers, host, renderer,
capability record, print implementation, SDK manifest or consumer pin changed.

## Measured local evidence

Windows, MinGW GCC 16.2.0, Release, Ninja, two compile jobs. No desktop launched.
Configure from the repository root after `./tools/Enter-WindowsToolchain.ps1`:

```text
cmake -S gui_forms -B .build/prepared-window-input -G Ninja -DCMAKE_BUILD_TYPE=Release -DGUI_FORMS_BUILD_GALLERY=OFF -DGUI_FORMS_ENABLE_SKIA=OFF -DGUI_FORMS_ENABLE_WINDOWS_HOST=OFF -DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=ON -DGUI_FORMS_BUILD_PREPARED_TEXT=ON -DGUI_FORMS_BUILD_TESTS=ON
cmake --build .build/prepared-window-input --target gui_forms_prepared_window_input_tests --parallel 2
ctest --test-dir .build/prepared-window-input -R ^gui_forms_prepared_window_input_tests$ --output-on-failure
```

The focused test passed, most recent test duration 0.01 seconds (CTest total
0.02 seconds). Raw output remains in that build's Testing/Temporary/LastTest.log.
A separate `.build/prepared-window-off` configure with both prepared-text and
HarfBuzz disabled succeeded; its build.ninja contains no prepared_window_input
source or target. This is an off-configuration check, not an installed-SDK audit.
The three C++ files passed tools/check_house_style.py with zero findings.

The fixtures cover mixed CR/LF/CRLF, blank rows and EOF, atomic separator labels
versus literal lookalikes, scalar-edge refusal, incomplete context, interior
anchor refusal, source/display/row/combined-record limits, exact key equality,
occupied output, closing ledger, independent ledgers and all five allocation
failure points in a populated owner. Every injected allocation failure leaves
output unpublished and restores the ledger including an unrelated reservation.

## Semantic source review

Input views are synchronous borrows. Admission checks ordered exact coverage,
certified endpoints, scalar edges, separator spelling/profile and aggregate
limits before allocation. Count bounds precede size products. Output ownership
is published only after all copies succeed. The reservation precedes allocation;
the owner's first-declared reservation is destroyed after its arrays. Immutable
array element types prevent mutation through the published const owner. Ledger
mutation is mutex-protected in the existing reservation implementation.

Key equality covers the full D1 token, controller, projection and authority plus
the existing text key. This specimen does not compare against a live desired
controller state; equality tests are not proof of stale-result adoption refusal.
It does not shape text, schedule work, certify consumer source semantics, own
selection or publish native frames. Producer paragraph/EOF certifications remain
trusted inputs under the development contract. No latency claim follows from
the test duration.

## Required next provider choices

1. Agree the controller's desired identity and immutable batch result records,
   including projection generation and source-certified empty-row metrics.
2. Extend A2 admission/publication to one aggregate multiparagraph job, with
   shared generation reservation and all-or-nothing adoption; per-row desire
   calls cannot preserve one viewport authority.
3. Assign the shaping service/result storage files and tests for that batch.
   This input owner alone does not prove the three-generation retirement law.
4. Reconcile an owned readiness connection that survives queue saturation and
   is revoked before close; the existing native handle is thread-affine.
5. Keep wrap/tab policy, long-line continuation, interior anchors, editing,
   bidi/caret affinity, accessibility and P1 printing explicit remaining work.

The next bounded implementation requested is private multiparagraph result
storage and aggregate admission tests, followed by one batch shaping job after
its records are agreed. Public export and SwiftEdit adoption require separate
assignment, installed-consumer evidence and native validation.

## Coordinator review follow-up

The coordinator identified missing const qualification on read-only by-value
parameters that the spelling script does not detect. The correction covers
window_scalar_edge and certified_pair's source/display values, test allocation
and deletion parameters, Scope, require, key_for and paragraph. The complete
before/after diff was inspected; no executable behavior changed. Mutable cursor
and byte-count output references remain mutable. The ledger value remains
non-const because own_prepared_window transfers it into the reservation.

The same two-job focused build passed after this correction; the test took
0.05 seconds (CTest total 0.06 seconds). The three-file spelling check again
reported zero findings. No Git mutation or broader provider change was performed.
