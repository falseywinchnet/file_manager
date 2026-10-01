# ORC-PCK-001 trusted first-party local projection

Date: 2026-09-29. **GIVEN:** the owner approved a usable offline first-party
picker without a daemon, Engine or catalogue dependency. This bounded development
source projection is registered; future daemon/plugin capability services remain
proposal-only.

Frontend owns the installed `FileManagerDocumentPicker` C++ controller/view
package consumed with the matching GUI.Forms SDK. It does not link File Manager's
executable or freeze a C++ binary ABI. Other applications use owned popups, not
File Manager's persistent panels. GUI.Forms owns presentation mechanics, not policy.

`DocumentPickerAuthority::{unavailable, orchestrator_session,trusted_local_host}`
defaults unavailable. A trusted first-party host explicitly grants its local
source with nonempty application identity, protected root, at most 32 additional
admitted roots, purpose/profile, cardinality, filters and hidden policy. No drive
is inferred. The historical session-valid bool defaults false and maps only
explicit caller input to daemon-session source. Local authority invents neither
a daemon session nor plugin privilege or Engine permission.

Each owned presentation has one grant lifetime. Revoke/restore retains the same
source/scope; it cannot widen either. Presenting UI does not renew authority.
Accept/cancel completes once and revokes it; owner close cancels. The host
explicitly restores the same grant for another owned opening. Unspecified or
revoked authority fails closed.

Navigation/acceptance enforce containment and no-link routes, with the bounded
trusted-local directory-navigation amendment below. Acceptance rechecks
exact object/directory identity and revision. Overwrite confirmation binds the
observed destination identity/revision; changes require a fresh decision. The
picker returns selection only, never file contents, writes or reservations.
The host revalidates immediately before its own I/O and owns collision policy.
Errors/overwrite requests leave the presentation open. Hidden visibility is per
caller, not an implicit shared preference.

**OBSERVED:** implementation and standalone installed consumer are documented in
`../../../frontend/docs/document_picker.md`. Focused Windows tests cover authority,
scope, stale identity, overwrite and presentation lifecycle. Native ownership,
focus, accessibility and three-platform acceptance remain separate gates.

## Trusted-local directory-link navigation amendment — 2026-10-01

**OBSERVED coordinator report:** SwiftEdit Mac dogfood cannot enter directory
symlinks. Source inspection confirms the view disables symlink rows, controller
selection rejects them, and `read_directory` refuses link routes. The current
contract previously also refused them. The coordinator accepted the following
bounded development fix scope under the existing owner dogfood directive; no
additional owner permission gate is introduced. This acceptance does not claim
that the existing implementation already permits following links.

**OBSERVED coordinator reconciliation:** only an explicitly granted `trusted_local_host`
picker may resolve a requested existing directory route to its canonical target
and navigate there when that target is within an already admitted root. Root
admission remains explicit; link resolution cannot add a root or widen a grant.
The resulting browser location, directory identity and returned observations
refer to the canonical target, not the link alias. Navigation through a displayed
link is distinct from accepting the link object. Unavailable/revoked authority
cannot gain this exception; `orchestrator_session` retains its refusal policy.

Resolution failure, a dangling/cyclic link, non-directory target, out-of-root
target or changed/unavailable target must refuse without publishing a successful
navigation. Re-observe the canonical target through the existing contained,
no-link read path before publishing its snapshot. Canonicalization alone is not
a race-free I/O authority or reservation. Acceptance still revalidates directory
and selected object identity/revision; the host revalidates before its own I/O.

Final file-link acceptance was outside this directory-only amendment; the
separately assigned Open/Import amendment below addresses that next scope.
Select Folder may accept the freshly revalidated canonical
directory after navigation, not return the alias as an accepted link. Save/Export
from that directory still accept only a valid basename under the canonical
parent, retain no-link route/leaf checks, and bind overwrite confirmation to the
exact observed destination identity/revision. Following a directory for browsing
does not authorize following an existing file link for writing or replacement.

ADR-017 protected mutation and ADR-020 protected-root actions remain unchanged.
Keep the exception at the picker authority boundary; do not relax the shared
filesystem reader or File Manager mutation model globally. Before adapter freeze,
record focused fixtures for in-root and explicitly
admitted cross-root directory targets, outside-root/dangling/cyclic/file targets,
revoked/session authority, changed targets, final file-link refusal and Save/
Export overwrite revalidation from the canonical directory. Exact source review
against `planning/PROGRAMMING_HOUSE_STYLE.md` and native Mac evidence remain
separate from this semantic review. No implementation or availability promotion
is made here.

**OBSERVED assignment:** the coordinator owns the controller/view fix and focused
tests. Source review and test/native receipts remain pending; this is acceptance
of a development scope, not a new architecture decision or SDK availability.

## Trusted-local Open/Import file-alias amendment — accepted development scope

**OBSERVED coordinator direction:** the owner's symlink complaint includes files,
not only directories. The coordinator has assigned itself the bounded next fix
under that existing user intent. This section prepares its canonical semantics
accepted by the coordinator; it adds no owner permission or research gate and claims
no implemented or installed behavior.

Only a currently granted `trusted_local_host` presentation in Open File, Open
Files or Import Files may select a visible file alias for this path. Selection
retains the alias's browser-snapshot identity/revision; it is not yet acceptance
of either the alias or its target. At acceptance, revalidate the current browser
directory and the alias against that snapshot, freshly resolve the target and
require a regular file under an explicitly admitted root. Return the canonical
target path with its fresh exact identity/revision, never the alias path paired
with target identity. Existing profile cardinality, filters, visibility and
single-completion rules remain in force; resolution does not admit another root.
Filename/type filters and hidden visibility apply to the displayed alias; the
resolved target may have a different basename. These are presentation filters,
not content validation or filesystem authority.

Outside-root, broken/cyclic, changed-since-snapshot, unavailable and nonregular
targets refuse, as does revoked/unavailable authority. Failed acceptance returns
no partial accepted selection and keeps the presentation open. Fresh resolution
and identity observation are not an I/O reservation: the consumer performs its
existing no-follow I/O and immediate identity/revision revalidation on the
returned canonical target. A race or replacement must not fall back to opening
the alias route or silently choose another object.

Save/Export retain no-follow leaf and route guards and exact overwrite binding;
this Open/Import exception does not authorize file-link writes. The Orchestrator
session projection remains no-follow. Existing directory-alias navigation and
canonical Select Folder behavior remain unchanged, as do ADR-017 protected
mutation and ADR-020 protected-root actions. No general filesystem-model policy
change, root inference, public API addition or consumer privilege is implied.

**OBSERVED assignment:** the coordinator owns the private target-resolution
adapter extending the directory helper, frontend native-file/controller/view
changes, focused tests and consumer documentation. This chat owns this canonical
record only. Required focused evidence includes admitted same/cross-root regular
targets, alias replacement/retargeting since snapshot, broken/cyclic/outside-root
and nonregular targets, authority loss, mixed multi-selection failure, canonical
result identity, unchanged Save/Export and session refusals, and preservation of
directory navigation. Exact authored source review against
`planning/PROGRAMMING_HOUSE_STYLE.md` and test/native receipts remain separate
acceptance evidence. No SDK or platform availability is promoted by this scope.
