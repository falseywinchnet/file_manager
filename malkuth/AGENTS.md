# Malkuth release-program operating instructions

This directory is the planning and eventual release-engineering home for the
Malkuth suite. It is not a desktop runtime component and does not absorb the
independent File Manager, Paint, Text Editor, Games, Lexicon, Orchestrator,
Engine, GUI.Forms, or Kolmogrov projects.

## Current phase: planning only

Do not create website implementation, documentation-generator configuration,
installer projects, signing automation, publishing credentials, package feeds,
download endpoints, or release binaries until the applicable gates in
`planning/RELEASE_HORIZONS.md` and the grand architect's direction open them.

Before editing, read:

1. `README.md`
2. `../decisions/ADR-012-MALKUTH-SUITE-IDENTITY-AND-RELEASE-HORIZONS.md`
3. `../decisions/ADR-013-FUTURE-UTILITY-SCOPE-AND-SURFACE-TOPOLOGY.md`
4. `planning/MISSION_AND_NARRATIVE.md`
5. `planning/RELEASE_HORIZONS.md`
6. the relevant documentation, installer, or website plan
7. the root `AGENTS.md` and decision protocol

## Release truth

- A Malkuth version is a compatibility/release manifest, not permission to make
  every component share one ABI or source version.
- Every public claim names the real artifact and evidence. Mocks, design boards,
  cross-builds, Wine, and simulated providers are labelled.
- Screenshots and demonstrations come from reproducible shipping-candidate
  builds; no composited feature fiction.
- Online documentation and the polished website are external release artifacts,
  never a bundled desktop web engine or a runtime network dependency.
- Installer tests cover clean install, upgrade, repair where supported,
  downgrade refusal/recovery, service lifecycle, user-data retention, and clean
  uninstall on native target systems.
- Never place signing keys, publishing tokens, store credentials, private update
  keys, or production secrets in this repository.
- Preserve negative packaging, signing, documentation, accessibility, and
  website results under labelled records.
