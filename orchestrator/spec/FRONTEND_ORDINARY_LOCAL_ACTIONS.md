# Ordinary local New Folder and Rename

Status: **development implementation contract; native/package acceptance pending**.
Date: 2026-10-03.

## Authority and scope

**GIVEN:** the owner's Shadow direction requires ordinary real-world File
Manager use, without requiring an isolated development profile. Core navigation
and file operations remain usable without Engine or Orchestrator augmentation.
Indexing remains opt-in. Fault campaigns continue to use generated fixtures.

**OBSERVED at audit baseline `6e224d0`:** ordinary startup creates no file-operation controller.
The only controller requires an existing separate same-volume quarantine and
rejects Home/repository roots, including for New Folder and Rename. Navigation
already permits locations beyond the launch root. See `frontend/src/main.cpp`,
`application.cpp::mutation_scope_active`, and `FileOperationService` construction.

This contract opens a bounded correction: ordinary New Folder, same-parent
Rename, and their identity-checked one-step Undo. It does not promote ordinary
Copy, Move, Delete, Trash, cross-volume operations, or durable undo. ADR-017
continues to govern its explicit protected fixture profile; it is not silently
redefined. No cross-process operation, plugin capability, Engine authority, or
stable C++ ABI is added. The frontend source package and consumers rebuild
together if its source API changes.

## Ordinary action semantics

- Normal product startup selects the ordinary local action policy. An explicit
  read-only launch remains available. The protected mutation arguments select
  their existing distinct policy, with contradictory options rejected.
- New Folder uses the current displayed directory; Rename uses the selected
  filesystem object and a single validated basename. Search-provider identity
  does not authorize an action. Actions run only after the corresponding user
  command, never while composing a menu or opening a directory.
- No quarantine, catalogue, service, root ID, or blanket filesystem-root grant
  is needed. Each queued command owns its exact path, observed parent identity,
  and selected source identity where applicable. Observe the parent when the
  command is requested, before queueing; a replacement before execution refuses
  the command. Directory identity excludes mutable child-count/mtime facts.
- Absolute observable directory routes are required. Symbolic-link/reparse
  parent routes remain refused. Native permissions govern access; no elevation,
  permission edits, or implicit retry against another path. Home/repository
  *contents* are normal locations, not refused because of their category.
- Root objects themselves are not rename targets. A valid link leaf may be
  renamed as a leaf; its target is not traversed. Empty or invalid basenames
  remain errors. Existing basename selection and unchanged-name no-op behavior
  are preserved.
- Creation and rename reuse no-replace publication. An occupied or unobservable
  destination is not vacancy. No replace/merge fallback is admitted. A successful
  creation must have an observable directory identity before granting Undo.
- Undo owns the original/current paths and relevant parent/source identities.
  Revalidate both the route and identity before an inverse operation. New Folder
  Undo removes only the same still-empty directory; it never removes children.
  Rename Undo refuses an occupied original name. Failure preserves retryable
  undo authority when the existing result law permits it. Undo is in-memory and
  ends at application exit; it is not crash recovery or durable Trash.
- Commands publish the existing explicit terminal result. A postcondition
  failure must report that an object may have been changed/retained, not claim
  that nothing happened. Navigation refresh alone is not proof of success.
- UI availability is per operation. Enabling New Folder/Rename must not enable
  protected Copy/Move/Delete or internal transfer indirectly. Context menus,
  keyboard handlers, property editing, drag paths, and direct method calls must
  enforce the same policy. Describe unavailable operations accurately.

## Limits and acceptance

**OBSERVED limitation:** existing path observation followed by a native operation
is not an atomic filesystem snapshot. Additional parent identity checks reject
observed replacements; they do not prove resistance to every concurrent route
substitution. Keep this limitation explicit, review native parent-handle options,
and do not claim complete race resistance from no-replace publication alone.

Implementation acceptance requires generated-fixture checks of ordinary launch
without services/quarantine, Home/repository-category contents, navigation away
from launch location, successful create/rename/undo, name collision, stale parent
and source, link routes, root-object refusal, nonempty-folder undo refusal, and
unchanged protected/read-only behavior. Exercise the UI command paths as well as
the adapter. No fault campaign modifies personal files.

Review all authored implementation/tests/tooling against the full
`planning/PROGRAMMING_HOUSE_STYLE.md`, with exact scope and remaining limitations.
Run focused Windows correctness followed by native Windows/macOS/Linux CI and
ordinary packaged-entry-point checks before a delivery claim. This contract
is permission to implement and test; it is not evidence that those gates passed.
