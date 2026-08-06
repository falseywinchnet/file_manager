# Malkuth installer and packaging program

Status: **CANDIDATE platform technologies; DECIDED lifecycle and evidence laws**.

## Purpose

Build one coherent distribution from independently versioned applications,
services, libraries, resources, contracts, documentation, and manifests without
turning installation into hidden privilege or data admission.

## Suite package topology

A Malkuth manifest may contain:

- File Manager and admitted first-party applications;
- independently admitted Games modules and Lexicon corpus/data packages;
- user-scoped Orchestrator and its CLI/client projections;
- systemwide Engine service plus visible consent/status agent where admitted;
- private GUI.Forms runtime/resources required by applications;
- platform adapters/helpers with narrowly declared privilege;
- built-in theme, language, font, icon, help, and recovery resources;
- optional plugin SDK/tools—not an automatically enabled plugin catalogue;
- independently declared first-party plugins/external engines, including exact
  default-enabled state and grant profile;
- license inventory, SBOM, hashes/signatures, release notes, and compatibility
  manifest.

Independent developer packages remain separate. The end-user installer does not
expose GUI.Forms internals merely because applications use them.

## Cross-platform lifecycle contract

Every platform proves:

1. clean install as an ordinary user where possible;
2. explicit privilege request only for a named system operation;
3. first-run index-root consent before scanning;
4. Orchestrator/Engine discovery, activation, restart, and shutdown;
5. application launch and File Manager recovery path;
6. in-place compatible upgrade with settings/hive/catalogue migration or
   declared rebuild;
7. interrupted install/upgrade recovery;
8. downgrade rejection or explicit compatible rollback;
9. repair where the selected installer technology supports it;
10. uninstall that removes binaries/services/integration while separately
    asking whether to retain user settings, hives, indexes, and logs;
11. reinstall after either retained-data or full-removal path;
12. no network requirement for the installer unless the artifact is explicitly
    a bootstrapper and a full offline alternative exists.

## Platform candidates

### macOS

Candidates: signed/notarized `.dmg` application distribution, a signed `.pkg`
when service/helper placement needs installer semantics, or a composed pair.
Tests cover launchd user/system jobs, socket ownership, quarantine/Gatekeeper,
upgrades, removal, APFS identity, multiple user accounts, sleep/wake, and host
file-manager recovery.

Direct-distribution signing/notarization required for a safe public 1.0 is
separate from 3.0 Mac App Store enrollment and sandbox/review compliance.

### Windows

Candidates: MSI through a maintained toolchain, MSIX where service/shell needs
fit its constraints, or a signed installer plus portable diagnostic package.
Tests cover SCM tasks, named-pipe ACLs, install per-user versus per-machine,
Explorer/default-handler integration, upgrades/repair, NTFS identity, long
paths, Defender/SmartScreen, and complete uninstall.

### Linux

Candidates: native `.deb` and `.rpm`, plus one portable distribution such as
AppImage; Flatpak remains a separate sandbox/integration comparison. Tests cover
systemd user/system units, desktop/MIME integration, Wayland and X11 hosts,
package-manager upgrades/removal, ext4 identity, distro library independence,
and retained-data policy.

No candidate is selected merely because it produces a file. It must express the
actual service, privilege, update, integration, and uninstall model.

## Manifest and reproducibility

- exact source revisions, dependency locks, compiler/linker/tool versions;
- per-platform file inventory and component versions;
- ABI/protocol/schema compatibility ranges;
- deterministic asset/resource manifests;
- license notices and machine-readable SBOM;
- artifact digest and signature/notarization/certification status;
- supported OS/architecture matrix and exclusions;
- build/reproduction instructions and permissible nondeterminism;
- migration/rebuild and rollback rules;
- documentation/site release revision.

## Installer user experience

The installer expresses the mission:

- **Curious:** explains components, capability/privilege reasons, and provenance
  of the artifact being installed.
- **Concise:** smallest safe choice set; no marketing carousel, account, feed,
  store, or dark-pattern defaults.
- **Friendly:** obvious install location, disk use, service behavior, index
  consent, recovery, upgrade, and removal consequences.

Root selection and privacy questions are configuration, not an excuse to begin
scanning during installation. Defaults remain no for new drives/network roots.

## Security gates

- release builds and installers never contain development credentials or
  signing secrets;
- privileged helper surface is separately reviewed/fuzzed and minimal;
- package path traversal, symlink/reparse attacks, rollback, partial extraction,
  DLL/dylib search, environment injection, and uninstaller targeting receive
  hostile fixtures;
- signatures are verified before privileged installation and updates;
- update metadata is authenticated, rollback-aware, and not conflated with a
  future plugin catalogue;
- logs redact user paths/content by construction.

## Native acceptance matrix

Compatibility layers and cross-builds are useful preflight but earn no native
release credit. Each supported OS/architecture runs clean VM/machine install,
upgrade, failure injection, daily workflow, service restart, user switching,
sleep/wake, accessibility, and uninstall fixtures with captured artifacts.

## Open decisions

- exact platform package technologies and minimum versions;
- unified installer versus per-application choices;
- automatic update policy and cadence;
- component-selectable installation;
- direct-distribution signing requirements before store enrollment;
- portable builds and whether they may omit background services;
- retained user-data defaults at uninstall;
- CI/build farm, release custody, and secret-signing process.
