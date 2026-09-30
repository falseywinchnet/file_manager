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

Navigation/acceptance enforce containment and no-link routes. Acceptance rechecks
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
