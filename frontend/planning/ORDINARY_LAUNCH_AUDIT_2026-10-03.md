# Ordinary launch versus daily use

Status: **OBSERVED source audit; implementation work opened in named slices**.
The visible sibling `Audit File Manager Details against interviews` traced the
current application, launchers, packaging and deployment contracts without
editing source, running builds, or scanning personal files. Root reviewed the
findings and owns this record. The table records the source at audit baseline
`6e224d0`, before the ordinary-action implementation; it is not a description of
the subsequently changed files.

## Product gaps

| Area | Audit baseline behavior | Required daily-use behavior |
|---|---|---|
| Local actions | `main.cpp` defaults to no operation service. `Application` requires an explicit mutation flag and quarantine. `FileOperationService` rejects Home/repository roots and confines both endpoints to the launch root. | Ordinary explicit actions under native permissions, independently of service/index setup. Each command validates its actual objects and directories. |
| Deletion | Protected same-volume quarantine and in-memory one-step Undo. | Native recoverable Trash/Recycle with truthful unsupported-location and restore behavior. No permanent-delete fallback. |
| Transfers | Staged no-replace copy and same-volume move exist inside the protected profile. Metadata, independently selected endpoints, cross-volume move and durable recovery remain incomplete. | Useful ordinary copy/move with explicit fidelity, cancellation/progress and collision behavior. Cross-volume drag must not silently become unsupported Move. |
| Search scope | `engine_search_available()` depends on a launch-supplied Engine ID and containment in the original root. Service discovery alone does not populate an approved path-to-root mapping. | Select the appropriate admitted root for the displayed subtree; never silently widen search. Do not ask users to supply opaque IDs at a terminal. |
| Windows activation | `launch_windows_search.ps1` creates new per-run state, starts owned processes, and supplies root arguments. It remains read-only and stops those services at window exit. | Persist enacted root policy/index state across launches, with an ordinary application setup/activation path and an independent background lifecycle. |
| macOS activation | Installed service discovery exists, but ordinary `.app` launch still has no Engine root mapping. Historical daily launcher intentionally passes no arguments. | Connect available services and select enacted root policy from the application. |
| Linux activation | Native application packaging exists; the general installed Engine host-identification route still rejects non-macOS, with Windows implemented separately. | A supported Linux deployment/root-admission route and ordinary activation, not a development-sandbox wrapper. |
| Index consent/storage | Root administration applies previously authorized roots; it is not an authorization UI. Manifest checks reject roots containing Engine state, even when exclusions are supplied. | Explicit opt-in setup, durable enacted policy and strict Engine-state exclusions. Reconcile Home-prefilled selection with state stored beneath Home rather than simply removing the check. |
| Package qualification | Generated empty-root launch plus five-second survival and component/native checks. | Exercise normal entry point, actual commands, close/reopen, service absence and persistent setup through each packaged application. |

Source locators: `src/main.cpp`, `src/application.cpp`,
`include/file_manager/file_operations.hpp`, `src/file_operations.cpp`,
`src/native_publication.cpp`, `src/native_copy.hpp`,
`tools/launch_m4_dogfood.sh`, root `tools/package_native.py`,
`tools/launch_windows_search.ps1`, Engine `internal/deployment/manifest*.go`
and `host_other.go`, and Orchestrator's C++ client and
`spec/contracts/ENGINE_AND_KOLMOGROV.md`.

## Current bounded implementation

The owner's real-world development direction supplies authority to address these
gaps. It does not make them implemented or negate collision/identity safeguards.
The first source assignment is ordinary New Folder, same-parent Rename and their
Undo under
`../../orchestrator/spec/FRONTEND_ORDINARY_LOCAL_ACTIONS.md`. It separates those
actions from quarantine/index readiness and preserves an explicit read-only
option plus the existing protected fixture profile. Native/package acceptance
is pending. Ordinary Copy/Move/Delete are not promoted by this assignment.

Keep fresh filesystem observations, explicit user actions, no-replace
publication, cancellation/owned staging where applicable, no automatic elevation,
link/reparse semantics, and explicit terminal outcomes. Additional observed
parent checks are not a claim of atomic snapshot or complete concurrent-route
substitution resistance. Fault checks use generated fixtures, not personal files.

The next search setup work must distinguish bounded on-demand live search from
durable indexing consent. An indexed substring implementation is another
independent performance/correctness gate. Neither a hidden launch profile nor a
new setup screen alone closes the missing persistent policy and lifecycle work.
