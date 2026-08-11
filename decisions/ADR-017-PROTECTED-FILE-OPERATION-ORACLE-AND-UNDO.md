# ADR-017: Protected file-operation oracle and one-step undo

Status: **accepted for protected-root implementation and M4 fault dogfood**.

Date: 2026-08-10.

Owner approval: the grand architect explicitly opened File Manager
implementation, permitted supporting repository extensions, declared the
prior blockers cleared, and directed full M4 dogfood. This record chooses the
most conservative operation law that advances that direction without admitting
daily personal roots or irreversible deletion.

## Question

What exact authority, identity, collision, deletion, and undo law may the first
File Manager mutation adapter implement inside a disposable protected root?

## Constraints

- **GIVEN:** filesystem observations, not Engine rows, authorize mutation.
- **GIVEN:** destructive campaigns remain inside explicit disposable roots
  until their separate promotion gates pass.
- **GIVEN:** a protected run refuses `/`, a home directory, a repository root,
  unresolved symlinks, and implicit or glob-derived targets.
- **GIVEN:** missing, replaced, permission-denied, partial, cancelled, and
  cross-volume outcomes are terminal facts, never empty success.
- **OBSERVED:** the first frontend already canonicalizes a protected root,
  refuses out-of-root navigation, lists symbolic links without following them,
  and applies only the newest navigation generation.
- **OBSERVED:** APFS supplies no-follow device/inode identity through `lstat`
  and atomic same-volume rename for the operation shapes admitted here.
- **HYPOTHESIS:** a separate, same-volume, harness-owned quarantine can provide
  deterministic recoverable deletion without writing product metadata into the
  user's browsed tree. It must be tested before daily-root promotion.

## Candidates

### A. Path-only direct operations and permanent deletion

Resolve a string path at command time and apply `remove`, rename, or copy.
This is small but cannot distinguish a replaced object and makes early delete
faults irreversible.

### B. Engine-authorized mutations

Use the indexed row or search result as the mutation identity. This confuses a
retrieval projection with authoritative filesystem state and violates the
program boundary.

### C. No-follow identity with explicit protected capability and quarantine

Bind every command to a canonical protected root plus an explicit mutation
opt-in. Capture leaf identity with no-follow device/inode observations, require
canonical non-symlink parent containment, revalidate identity immediately
before commit, refuse collisions, and move deleted objects atomically into a
separate same-volume harness quarantine. Retain one inverse operation and apply
it only if both source identity and destination vacancy still match.

## Decision

Choose C for the protected F4 profile.

The initial adapter admits only explicitly requested operations under one
canonical protected root and one separately supplied quarantine root. Both
roots are absolute, existing directories; neither may be `/`, the user's home,
the authoritative repository, or an unresolved symbolic-link route. The
frontend must receive an explicit mutation opt-in at process launch. Read-only
is the default.

An object identity is the no-follow `(device, inode, kind)` observation of the
leaf. Parent components are canonicalized and may not traverse symbolic links.
Every commit re-observes that identity. A mismatch returns `identity_changed`
without mutation. Symlink leaves may themselves be renamed or quarantined, but
their targets are never opened or traversed.

The first collision law is fail-closed: create, rename, move, copy, restore,
and undo never overwrite an existing destination. Replacement and merge are
separate future commands with explicit confirmation and fault evidence.

Deletion in this profile means an atomic same-volume move into the exact
harness-owned quarantine, not permanent removal and not an Engine catalogue
change. Cross-volume quarantine returns `cross_volume_unsupported`. The
adapter retains only the most recent inverse command as user-visible undo
authority. Older quarantine objects may remain recoverable but are not silently
reclassified as undoable.

New Folder selects a deterministic available name. Rename validates a single
basename and performs a same-parent rename. Move and copy require separate
staged-publication evidence before they are enabled in the native surface.

## Operation result law

Every request returns one terminal result containing:

- stable operation ID and kind;
- `success`, `cancelled`, `unavailable`, `conflict`, or `failed` terminal state;
- exact machine code and user-facing reason;
- protected root, original path, resulting path when any, and observed identity;
- whether a user-visible undo is currently available;
- whether any recoverable staged/quarantined object remains.

No failure becomes success merely because a later refresh cannot see the old
path. Directory refresh is a consequence of a successful operation, not proof
that the operation succeeded.

## Failure modes and tests

- Reject root, home, repository, relative, missing, and symlink-routed protected
  roots before enabling mutation.
- Replace an object between snapshot and commit; require `identity_changed` and
  preserve both objects.
- Exercise name collision, invalid basename, permission denial, disappearance,
  same-volume rename, cross-volume refusal, and cancellation before publication.
- Quarantine a regular file, directory tree, and symlink leaf without following
  a link target; undo each when the original path is vacant.
- Occupy the original path before undo; require conflict and retain the
  quarantined object.
- Start a second operation; prove the first inverse is no longer presented as
  one-step undo authority.
- Crash/restart recovery remains open: retained quarantine evidence is
  inspectable but no inverse is inferred until a journal ADR and recovery suite
  pass.

## Rejected options

- A is **REJECTED** because path identity is insufficient and permanent delete
  is not reversible during early dogfood.
- B is **REJECTED** because Engine retrieval state is not mutation authority.

## Reversal path

The protected adapter can later project to platform trash/recycle facilities
or an admitted durable operation journal without changing the public terminal
result law. Replacement, merge, cross-volume staged copy, multi-step undo, and
daily-root admission each require additive evidence and may not be inferred
from this decision.

## First implementation evidence

Status: **MEASURED on the M4 protected profile; broader fault matrix open**.

The C++ adapter and focused M4 test executable pass deterministic create/undo,
nonempty-create conflict, rename/undo, collision refusal, replacement identity
refusal, symlink-leaf quarantine without target traversal, directory-tree
quarantine, occupied-restore retry, staged regular-file and directory copy,
copied symlink leaves, copy cancellation/cleanup, recursive destination and
collision refusal, mid-traversal cancellation cleanup, injected partial
disk-full cleanup, exact permission denial, cross-volume move refusal,
same-volume move/undo, and one-step undo replacement. The deterministic fault
seam is constructor-owned and absent from product launches.
Navigation now derives stable IDs from no-follow device/inode identity and
cancels stale enumeration without publishing partial snapshots.

Screen Sharing dogfood used an explicit `0700` corpus and separate `0700`
same-device quarantine under `CodexRuns`. New Folder/Undo, inline Rename/Undo,
and two-step Delete/quarantine/Undo completed through the installed app bundle.
The native transfer shelf then copied `read-me.txt` into `Documents` through a
hidden destination-local stage, moved `ledger.csv` into `Images`, and restored
that move through the visible one-step Undo command. Remote observations proved
that the copy had a distinct inode, the move/undo retained the original inode,
and no `.fm-stage-*` object remained. The external `/etc/hosts` symlink remained
a non-followed leaf. A repeated Copy into the occupied destination produced a
native blocking dialog naming the exact source, destination, and
`destination_exists` code. Physical full-volume reproduction, cross-volume
staged copy, and crash recovery remain open and are not promoted by this
measurement.
