# ADR-012: Malkuth suite identity and release horizons

Status: **accepted**.

Date: 2026-08-06.

Owner approval: the grand architect named the unified package **Malkuth**,
supplied the mission words **Curious, Concise, Friendly**, and directed the
1.0, 2.0, and 3.0 release horizons recorded here.

## Question

What identity frames the unified applications and services after they can be
dogfooded together, and what work distinguishes its first three public release
horizons?

## GIVEN constraints

- File Manager, Orchestrator, Engine, Kolmogrov-derived capability, GUI.Forms,
  and later first-party applications must be presentable as one coherent
  package without erasing their independent build/version boundaries.
- The suite name is **Malkuth**, because it governs the physical existence of
  files and reveals their provenance and location.
- The mission is **Curious, Concise, Friendly**.
- After File Manager dogfood proves claimed capabilities, public release work
  requires online documentation, cross-platform installers, and a polished
  showcase website.
- Malkuth 1.0 makes the integrated system work and dogfoods it on Windows,
  Linux, and macOS; fixes as many bugs as possible; establishes stability; and
  supplies basic installers and documentation.
- Malkuth 2.0 preserves stable calls while reducing mutexes, memory, coarse or
  duplicated machinery (“big dumb”), and correctness/performance debt across
  frameworks. It develops the plugin wrapper plus secure LAN-only cross-platform
  tracking and remote-file availability.
- Malkuth 3.0 adds formal verification and further hardening, wider platform
  support, decent touch capability, more configuration, more fonts, and
  app-store distribution signing.

## Workloads and failure modes

- A suite label must not imply that every research project or optional app is
  release-ready.
- A polished website must not claim functionality supported only by mocks,
  cross-builds, Wine, or design documents.
- Online documentation must not become a runtime network dependency or replace
  local contextual help.
- Installers must preserve opt-in indexing, explicit privilege boundaries,
  clean upgrade/uninstall, and separately supervised Engine/Orchestrator roles.
- “Reduce mutexes” must not become unmeasured lock-free churn or weaken
  correctness.
- “Reduce big dumb” must not become aesthetic rewriting of stable code; it means
  measured removal of coarse locks, duplicate representations, needless
  allocations/copies, oversized dependencies, broad invalidation, and redundant
  process/protocol work.
- LAN availability must not silently become Internet discovery, ambient remote
  browsing, or a cloud account.
- Formal verification must name the model, implementation relation, assumptions,
  and checked property; a proof label cannot cover an unrelated binary.

## Candidates

### A — market each program independently with no suite identity

Preserves local autonomy but hides the shared backbone, duplicates release
explanations, and gives users no coherent compatibility or installer story.

### B — replace every component/product name with one Malkuth executable

Looks unified superficially but erases useful boundaries, turns one version into
a false compatibility claim, and encourages a monolithic package/runtime.

### C — Malkuth as suite/distribution identity over independently versioned parts

File Manager, Paint, Text Editor, Orchestrator, Engine, GUI.Forms, and research
artifacts retain honest names and versions. A Malkuth release manifest pins the
compatible application/runtime set, documentation, installers, provenance,
licenses, and conformance evidence.

## Evidence and measurements

The name, mission words, and release horizons are **GIVEN**, not measured. The
current Engine and GUI.Forms repositories contain partial cross-platform,
packaging, performance, and dogfood evidence; those records do not yet establish
Malkuth 1.0. Each release gate requires its own native-machine evidence matrix.

## Decision

Choose C.

### Identity

**Malkuth** is the suite, distribution, public website identity, documentation
family, and compatibility manifest. It does not rename File Manager,
Orchestrator, Engine, GUI.Forms, Kolmogrov, Paint, or Text Editor.

The name's metaphysical language is product framing, not a data-authority
override: the filesystem/platform remains authoritative for file bytes and
identity; Malkuth reveals, organizes, operates on, and explains that physical
file world through declared capabilities.

### Mission

> **Curious, Concise, Friendly.**
>
> Malkuth is curious about the files that physically exist: where they are, how
> they arrived, what can act on them, and which evidence supports that account.
> It is concise in the commands, explanations, and machinery it places between
> a person and their work. It is friendly by being quick, legible, accessible,
> reversible where promised, and honest about what it knows. It reveals
> provenance and location without turning the desktop into a feed, a cloud
> account, or a browser.

### Versioning

- Suite versions are release-train/compatibility-manifest versions.
- Components retain independent semantic/ABI/protocol/schema versions.
- A Malkuth manifest names exact artifacts, component ranges, platform support,
  contract availability, licenses/SBOM, installer revision, documentation
  revision, and evidence status.
- Paint and Text Editor are admitted first-party suite applications, but their
  inclusion in Malkuth 1.0 remains an explicit release-scope decision. They do
  not silently block File Manager 1.0 merely because their plans exist.

### Release horizons

The normative interpretation lives in
[`../malkuth/planning/RELEASE_HORIZONS.md`](../malkuth/planning/RELEASE_HORIZONS.md).

## Why the other candidates lost

Independent marketing fails to explain the unified backbone and compatible
distribution. One monolithic Malkuth product would turn product polish into
architectural coupling and make one version number lie about every internal
contract.

## Consequences

- `malkuth/` becomes a planning-only release/presentation program until its
  implementation gates open.
- Documentation, installers, website, release manifests, license/SBOM records,
  screenshots, and claims are treated as tested release artifacts.
- Paint and Text Editor receive independent planning subprojects with
  interview-first starts and explicit implementation gates.
- The polished website and online docs may use ordinary web technology; no web
  engine enters the desktop applications.
- App-store submission/signing is a 3.0 horizon. Direct-distribution integrity,
  ordinary platform signing/notarization required for safe 1.0 installation,
  and store-specific signing remain distinct.

## Reversal and migration path

Individual application names and component versions remain independent, so a
future suite rename changes release metadata, website/installer branding, and
documentation navigation rather than internal file formats or service ABIs.
Release horizon contents can move through an approved amendment without
renumbering component protocols.

## Unresolved edges

- Exact Malkuth 1.0 application inclusion set, especially Paint and Text Editor.
- Final logo, wordmark, domain, trademark clearance, and pronunciation guidance.
- Website/docs generator and host.
- Platform installer technologies and minimum OS versions.
- Whether direct macOS/Windows public distribution requires signing before the
  3.0 app-store horizon—which is likely operationally necessary but must be
  separated from store enrollment.
- Exact LAN topology, pairing, cryptography, revocation, and remote-file
  availability semantics for 2.0.
- Formal methods, proof targets, and supported touch/platform matrix for 3.0.
