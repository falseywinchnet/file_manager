# Application interaction test house-style review — 2026-09-30

Scope: frontend/tests/application_interaction_tests.cpp and this tracked audit. Application implementation remains owned by the frontend chat. No application/API/header, accepted picker, SDK, install, archive, or running-app changes. No commit or publication.

Source SHA256: 197FC9091A00A901E930F317F8D8701E66269CD2DF3689F3E8FB123543C73559
Baseline snapshot SHA256: 1E210F7F697F972980ADA8340F7A03F70FF292F100AFC62D1671B0379119802A

OBSERVED — Explicit types replace inferred local/iterator/parameter types. Member access uses dot spelling. Named scenario-local polling functors expose borrowed dependencies; common object-name polling has one named predicate. Four command subscriptions use GUI.Forms non-owning Delegate bindings to a named trace listener. Host callbacks have named targets. Search predicates and formatting have named helpers. Computed returns use named results. Owned fixture objects initialize explicitly.

OBSERVED — Polling remains synchronous, evaluates before draining, retains the original three-second deadline and one-millisecond wait, and drains once after success. Short-circuit guards and action order remain intact. Menu-category search remains shallow; recursive semantic lookup remains separate. No generic captured-wrapper framework was introduced.

OBSERVED — All nine original scenario functions remain, plus one stop-during-drain regression (ten total). The 55 polling calls remain (56 textual occurrences including the helper definition). Counts unchanged: 98 semantic actions, 46 key dispatches, seven pointer dispatches, four text dispatches. All original assertion/diagnostic strings remain after concatenating formatter-split literals. Only repeated formatting commas, module identities, and Folder predicate constants were deduplicated into helpers. Two added fixture assertions validate cleanup path containment and successful file writes.

OBSERVED — The trace listener precedes its four scoped SubscriptionTokens, so tokens disconnect before borrowed state destruction on normal or exceptional exits. Recording is synchronous, does not reenter, and propagates allocation failure to the existing test exception boundary. HostCloseObservation stops the application before its borrowed callback state dies; Application::stop clears callbacks/queued UI work and joins the worker. bind_host is not used for cleanup because it also schedules work. ApplicationStopGuard and TemporaryTree reject copying. Fixture construction reclaims partial data on exceptions; cleanup uses filesystem error-code overloads so ordinary cleanup errors do not mask assertions. Generated fixture targets are verified absolute immediate children of the canonical temporary directory before recursive deletion. Cleanup and callback lifetime findings are source review, not fault-injection measurements.

MEASURED — Windows x64, GNU C++ 16.2.0, C++20 Release, dedicated frontend/.build/application-test-style-audit/build, two build jobs, frozen .build/sdk-checkpoints/dbe3766/windows-x64/gui-forms-sdk headers/libraries/runtime. The interaction target with the stop-during-drain regression built; CTest passed 1/1 in 1.92 seconds (total 1.93). Eight ordinary scenarios executed; two environment-gated installed probes returned early because FILE_MANAGER_INSTALLED_* variables were unset. Their source scenarios remain present but are not claimed verified here. Other-platform conditional branches were retained and reviewed, not run on Windows.

MEASURED — tools/check_house_style.py on the source: one file, zero spelling findings/review candidates. git diff --check on the source: no errors. The checker is only a lexical aid; lifetime/order/scenario observations above are the semantic review.

Reproduction from repository root in PowerShell:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake --build frontend/.build/application-test-style-audit/build --target file_manager_application_interaction_tests --parallel 2
$env:PATH = 'C:/Users/Shadow/file_manager/.build/sdk-checkpoints/dbe3766/windows-x64/gui-forms-sdk/bin;' + $env:PATH
ctest --test-dir frontend/.build/application-test-style-audit/build -R '^file_manager_application_interaction_tests$' --output-on-failure
python tools/check_house_style.py frontend/tests/application_interaction_tests.cpp
```

Detailed ignored artifacts remain under frontend/.build/application-test-style-audit/: baseline/application_interaction_tests.cpp, ctest-final.log (initial style pass), ctest-stop-regression.log (regression pass), style-final.log, and scenario-evidence.txt. An earlier test regex matched no tests; the exact final test name above was then used and passed. No no-tests result is counted as validation.

## Stop during local UI drain regression

**OBSERVED:** `test_stop_during_ui_drain_revokes_remaining_callbacks` uses the existing private friend probe to call production `post_ui` and inspect the protected queue under its mutex. The named `StopDuringDrainObservation` owns three flags and borrows the application. Its named first callback calls `stop()` and marks completion; its named second callback would set a separate flag. The test asserts both are queued before `drain_ui`, first completion succeeds, and the second does not run. A third named callback submitted after stop must leave the queue empty, and a subsequent drain must execute neither remaining nor late work.

**OBSERVED:** The test does not bind a host or schedule navigation. Context is declared before the stop guard: on assertion failure the guard clears queued callbacks and joins the worker before the borrowed target is destroyed. The application owner outlives both. No anonymous callback, generic capture wrapper, sleeps, or timing-based success criterion was added.

**OBSERVED:** The frontend-owner change checks `stopping_` before each locally pending callback in `drain_ui`; `stop()` alone clears the shared queue but cannot revoke callbacks already moved into the local queue without this check. Production source was read, not edited by this assignment. The owner-side guard arrived before validation, so no measured red run against the former implementation is claimed.

**MEASURED:** The new regression passed together with the existing ordinary interaction scenarios against frozen dbe3766 GUI.Forms. Parent-wide final integration, including the newer house-style-review SDK, remains with the parent chat. No claim is made here for that newer SDK.

## Execution-order correction after parent review

**OBSERVED:** The first source review was incomplete despite zero lexical findings: effectful dispatch remained embedded in assertions. This follow-up separates 165 semantic/key/pointer/text dispatch, focus, module-enable, and command-execution calls from 127 assertion sites into typed named Boolean results. Two intentionally ignored dispatch results also have explicit names. Previously named dispatch results remain named. Each compound action sequence asserts preceding preconditions and action results before attempting the next action, preserving fail-before-next-action behavior. Rejected operations retain negated result assertions. Each sequence retains its original diagnostic in scoped string storage; diagnostic construction precedes that sequence. Optional/pointer preconditions use explicit presence checks where conjunctions formerly supplied contextual Boolean conversion.

**OBSERVED:** A named platform-specific terminal-state predicate preserves the original Linux/other-platform condition at its original evaluation point after selection. Semantic identity insertion now has a separate typed result before its duplicate check, and empty identities still skip insertion. Remaining member calls in assertion expressions were reviewed as queries, not dispatch or state mutation.

**MEASURED:** Token comparison against the pre-execution-order source confirms all 171 dispatch/focus/module-enable/command-execution calls have identical method names, argument tokens, and source sequence. No call in those categories remains inside `require`. All ten scenario functions remain. Detailed comparison output is retained locally at `frontend/.build/application-test-style-audit/execution-order-evidence.txt`. The two installed probes remain opt-in and were not enabled for this correction.

**MEASURED:** After the execution-order correction, the focused Release target rebuilt successfully and `file_manager_application_interaction_tests` passed 1/1 in 1.82 seconds (total 1.83), using the same frozen dbe3766 SDK and two-job build. Eight ordinary scenarios executed; two installed probes remained environment-skipped. The final spelling check reports zero findings, and the two-file diff check is clean. Final runtime output: `frontend/.build/application-test-style-audit/ctest-execution-order.log`.
