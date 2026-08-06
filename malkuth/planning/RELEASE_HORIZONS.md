# Malkuth release horizons

Date: 2026-08-06.

Status: **DECIDED horizons; exact application inclusion and numerical budgets
remain open**.

## Version model

A Malkuth suite version is a release manifest that pins independently versioned
applications, services, libraries, schemas, documentation, installers, and
evidence. Component APIs do not inherit a suite major automatically.

## Malkuth 1.0 — works, survives daily use, can be obtained

### Intent

Get the integrated system working; dogfood it natively on Windows, Linux, and
macOS; remove as many material bugs as possible; establish stability; and ship
basic installers plus complete release documentation.

### Required lanes

1. **Integration:** File Manager, GUI.Forms, Orchestrator, Engine, platform
   adapters, handlers, settings, picker/help surfaces, and admitted plugins
   report compatible real capabilities rather than fixture success.
2. **Native dogfood:** named physical Windows, Linux, and macOS machines perform
   the same daily workflows, service restarts, sleep/wake, display changes,
   file-operation corpus, accessibility paths, and degraded-provider cases.
3. **Stability:** crash, hang, corruption, wrong-object mutation, lost-setting,
   stale-search, installer, and uninstall failures have severity, reproduction,
   owner, disposition, and release blocker rules.
4. **Installers:** at least one supported install/update/uninstall path per OS,
   with explicit index consent and separate Engine/Orchestrator lifecycle.
5. **Documentation:** versioned online user, administrator, CLI/API, plugin,
   troubleshooting, privacy, and release documentation; minimal local help does
   not require the network.
6. **Website:** polished, accessible, static-first public showcase and downloads
   surface using real release-candidate evidence.
7. **Supply chain:** pinned sources, reproducible/offline build instructions,
   artifact hashes, license notices, SBOM, provenance manifest, and signature
   status stated exactly.

### 1.0 exclusion law

Paint, Text Editor, Games, and Lexicon may join 1.0 only by explicit release-
scope decision and their own passed gates. Archive Viewer, Image Converter,
OCR, LAN federation, general plugin ecosystems, final Kolmogrov proof closure,
app-store publication, broad touch redesign, and every future font/script pack
do not silently block File Manager 1.0.

## Malkuth 2.0 — same calls, tighter machine

### Intent

Preserve externally consumed calls while tightening memory, speed, correctness,
and internal structure across the frameworks and services. Complete the hostile
plugin wrapper and introduce secure LAN-only tracking and remote-file
availability.

### “Reduce mutexes” discipline

- inventory every lock, owner, protected invariant, contention profile, hold
  time, and tail-latency effect;
- first reduce shared state, critical-section breadth, duplicate work, and
  blocking I/O under locks;
- compare sharding, ownership transfer, immutable snapshots, queues, RCU-like
  reads, and actor/service boundaries against the current correct reference;
- retain locks where they are clearer and unmeasured contention is immaterial;
- require race, deadlock, starvation, fairness, shutdown, and equivalence tests;
- never claim progress from lock count alone.

### “Reduce big dumb” discipline

Measure and remove coarse full-tree scans, full-window redraw, duplicated
catalogues/state, eager copies, oversized allocations, redundant serialization,
overbroad process wakeups, repeated parsing, giant dependencies, and adapters
that translate the same fact repeatedly. Preserve simple correct machinery when
replacement complexity does not win a declared workload.

### Stable-call law

- Malkuth 2.0 does not use an internal-polish release as an excuse to break
  public CLI, local-wire, C ABI, DML, picker, plugin, handler, settings, or file
  format contracts.
- Existing 1.x clients continue directly or through a bounded compatibility
  adapter with conformance fixtures and a declared retirement horizon.
- Internal representations may change radically behind the same semantics.

### Plugin reef

Implement package identity, grants, sandboxed workers, hostile protocols,
preview/thumbnail/search/virtual/provider classes actually admitted by
contracts, quotas, crash containment, update/revocation, and audit. Plugin code
never enters trusted GUI, Orchestrator, or Engine processes.

The first-party Archive Viewer and Image Converter are preferred reef dogfood
profiles once their contracts/gates pass. Archive Viewer exercises virtual
hierarchies, an external process such as 7z, embedded browser reuse and trusted
extraction publication. Image Converter exercises bounded create-new output.
Neither grants ambient filesystem writes or plugin-supplied UI. OCR remains
deferred.

### LAN file availability

- explicit user pairing and machine identity;
- authenticated encrypted transport and revocation;
- no ambient Internet discovery, account, relay, or web-store dependency;
- source machine retains full catalogue/content authority;
- remote availability, staleness, provenance, permissions, and partial failure
  remain visible;
- remote file access is capability-scoped and auditable;
- local coarse catalogues require explicit policy;
- copy/open operations distinguish remote read from local materialization.

Exact discovery, pairing, crypto, transport, conflict, offline-cache, and remote
mutation rules require later ADRs and threat models.

## Malkuth 3.0 — verified, hardened, broader input/distribution

### Intent

Add formal verification and further hardening, broaden supported platforms,
deliver decent touch capability, expand configuration and font/script coverage,
and satisfy app-store signing/distribution programs.

### Formal verification targets

Prioritize small critical mechanisms rather than claiming a proof of the whole
desktop:

- capability/grant and plugin job state machines;
- local/LAN protocol framing, authentication, replay, cancellation, and
  revocation;
- file identity/ownership and wrong-object mutation prevention;
- immutable generation publication, recovery, and migration;
- settings/hive transaction and provenance authority;
- installer/update state transitions and downgrade behavior;
- selected Kolmogrov formal claims where implementation correspondence exists.

Candidate methods include executable reference models, property/state-machine
testing, TLA+/PlusCal, Alloy, Lean/Coq, model checking, symbolic execution, and
language-specific verification tools. Selection follows target/property fit.

### Wider surface

- named additional OS versions/architectures only after native conformance;
- touch hit geometry, gestures, selection, drag, keyboard coexistence, IME, and
  assistive-tech testing without redesigning the desktop into tablet cards;
- more configuration through bounded schemas, not arbitrary UI/code injection;
- additional signed/versioned font and script packs with fallback, security,
  licensing, shaping, and accessibility evidence;
- store-specific manifests, entitlements, sandbox exceptions, signing,
  notarization/certification, privacy declarations, and review compliance.

## Promotion procedure

Each horizon uses
[`RELEASE_MANIFEST_AND_ACCEPTANCE.md`](RELEASE_MANIFEST_AND_ACCEPTANCE.md).
A date or percentage never promotes a release. The grand architect approves the
artifact/evidence manifest and every carried blocker or exclusion.
