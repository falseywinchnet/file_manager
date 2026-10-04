**Scoped source acceptance: no actionable findings in the corrected preview-demand implementation and regression test.** The earlier undefined-predicate finding is fixed, and the ancestry-only subscription has been replaced.

The current code subscribes to `Window::control_availability_changed` after publishing `window_` in [make_window](C:/Users/Shadow/file_manager/frontend/src/application.cpp:553). The named handler reconciles demand from current effective visibility rather than the event’s potentially deferred snapshot. [update_preview_demand](C:/Users/Shadow/file_manager/frontend/src/application.cpp:4236) returns immediately for stopped, empty, or ready state, avoiding unnecessary ancestry traversal and preventing completion/reset visibility changes from recursively restarting work.

The retained lifetime and reentrancy review remains sound: application-owned tokens revoke the non-owning callback; selection and request setup enter `empty` before changing controls; completion enters `ready` before publication; hidden pending work advances the generation before becoming deferred. Workers continue to observe atomic cancellation rather than ordinary UI state.

The added [layout-collapse fixture](C:/Users/Shadow/file_manager/frontend/tests/application_interaction_tests.cpp:4850) specifically verifies that authored visibility stays true while layout collapse hides the preview, cancels its pending generation, and schedules exactly one replacement when restored. The retained [WindowsRejectedLayoutLastTest.log](C:/Users/Shadow/file_manager/frontend/results/2026-10-03-preview-demand/WindowsRejectedLayoutLastTest.log) records the expected prior failure:

> layout-only collapse must cancel pending preview demand

That is useful negative evidence for the previously missed transition. It does not establish that the corrected revision passes; the final build and test run remain pending.

The reviewed scope covers the demand declarations, subscription, state transitions, selection ordering, test probes, and complete new demand test against the full [PROGRAMMING_HOUSE_STYLE.md](C:/Users/Shadow/file_manager/planning/PROGRAMMING_HOUSE_STYLE.md). **No remaining concrete house-style violation identified in that scope.** Unchanged legacy code is not certified.

| Reviewed file | SHA-256 |
|---|---|
| `frontend/src/application.hpp` | `5B53E1EDAAFDF6D5C7DC50F5281737523E568C041E6C866690970F0256579AEA` |
| `frontend/src/application.cpp` | `86A04621E4FDD6EC1C62B2541AB6C3765581BED7AE39D983E0C385378CC7DED3` |
| `frontend/tests/application_interaction_tests.cpp` | `ED8C9C07D7CADECB0A354F098F82F192837366EFD2171BCB486746FACAECF793` |

No edits, builds, or Git operations were performed.
