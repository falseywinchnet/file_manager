# Accepted architecture decisions

Decision records are authoritative only when their status is **accepted** and
their owner approval names the grand architect's direction. Candidate research
and existing implementation do not silently create decisions.

| Record | Status | Scope |
|---|---|---|
| [`ADR-001-ENGINE-REFERENCE-AND-STORAGE-SPINE.md`](ADR-001-ENGINE-REFERENCE-AND-STORAGE-SPINE.md) | accepted | Go engine object/binding model, root/volume topology, immutable-generation storage spine, observation/publication policy, structures, ranking baseline, migration, API projections, external/network placement, large-directory mode, and initial performance constitution |
| [`ADR-002-ORACLE-CONTRACT-AUTHORITY-AND-PROGRAM-SEQUENCE.md`](ADR-002-ORACLE-CONTRACT-AUTHORITY-AND-PROGRAM-SEQUENCE.md) | superseded | Historical paper-first Oracle name and global dependency gate |
| [`ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md`](ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md) | accepted | Orchestrator naming, integration authority, project-local ABI negotiation, user-scoped bootstrap kernel, CLI authority, fallbacks, and stub boundaries |
| [`ADR-004-FRONTEND-001-LOCATION-AND-OPENING-GATE.md`](ADR-004-FRONTEND-001-LOCATION-AND-OPENING-GATE.md) | partially superseded | `frontend/` ownership and Design DNA 006 remain accepted; its GUI.Forms-only opening predicate is superseded by ADR-006 |
| [`ADR-006-ORCHESTRATOR-CORE-1-0-FRONTEND-BOOTSTRAP.md`](ADR-006-ORCHESTRATOR-CORE-1-0-FRONTEND-BOOTSTRAP.md) | accepted | Headless Orchestrator Core 1.0 release target, truthful provider availability, live frontend bootstrap dependency, and independent GUI.Forms progress |
| [`ADR-007-ENGINE-ORCHESTRATOR-SEMANTIC-V0-AND-CAPABILITY-GATES.md`](ADR-007-ENGINE-ORCHESTRATOR-SEMANTIC-V0-AND-CAPABILITY-GATES.md) | accepted | Stable experimental Engine semantic v0, capability-gated integration, manual-reconcile distinction, packaging boundary, and nonblocking platform-promotion gates |
| [`ADR-008-CATALOGUE-INDEPENDENT-LIVE-SEARCH.md`](ADR-008-CATALOGUE-INDEPENDENT-LIVE-SEARCH.md) | accepted | Required bounded catalogue-independent Engine search, independent `ORC-ENG-004` semantics, and catalogue-to-live fallback law; partially supersedes ADR-007's optional classification |
| [`ADR-009-ORCHESTRATOR-LOCAL-WIRE-DISCOVERY-AND-SESSION-AUTH.md`](ADR-009-ORCHESTRATOR-LOCAL-WIRE-DISCOVERY-AND-SESSION-AUTH.md) | accepted for implementation | Bounded framed local wire, private discovery endpoint, credential hello, Unix first projection, and remaining platform/concurrency promotion gates |
| [`ADR-010-ORCHESTRATOR-CORE-1-0-COMPATIBILITY-HORIZON.md`](ADR-010-ORCHESTRATOR-CORE-1-0-COMPATIBILITY-HORIZON.md) | accepted; first macOS artifact conformant | macOS first-platform release, Core contract/wire 1.0 line, Core 1.x additive compatibility law, and retained 0.1 rejection corpus |
| [`ADR-011-ORCHESTRATOR-MACOS-LAUNCHD-ACTIVATION.md`](ADR-011-ORCHESTRATOR-MACOS-LAUNCHD-ACTIVATION.md) | accepted; installed evidence passed | stable per-user macOS discovery, launchd socket adoption, restart credential rotation, bounded client activation retry, and installed lifecycle evidence |
| [`ADR-012-MALKUTH-SUITE-IDENTITY-AND-RELEASE-HORIZONS.md`](ADR-012-MALKUTH-SUITE-IDENTITY-AND-RELEASE-HORIZONS.md) | accepted | Malkuth suite identity, Curious/Concise/Friendly mission, independent-component release manifests, and 1.0/2.0/3.0 horizons |
| [`ADR-013-FUTURE-UTILITY-SCOPE-AND-SURFACE-TOPOLOGY.md`](ADR-013-FUTURE-UTILITY-SCOPE-AND-SURFACE-TOPOLOGY.md) | accepted | Future Games/Lexicon/plugin/built-in scope, desktop boundary, and the rule that persistent side panels belong only to File Manager and its embedded picker/browser |
| [`ADR-014-GUI-FORMS-INITIALIZATION-AND-HANDLE-LIFECYCLE.md`](ADR-014-GUI-FORMS-INITIALIZATION-AND-HANDLE-LIFECYCLE.md) | accepted for implementation | Portable GUI.Forms host, managed presentation, retained attachment, native identity, and compatibility-handle lifecycle order |
| [`ADR-015-ENGINE-M4-DOGFOOD-ROOT-ADMISSION-AND-LAUNCHD.md`](ADR-015-ENGINE-M4-DOGFOOD-ROOT-ADMISSION-AND-LAUNCHD.md) | accepted; named M4 installed evidence passed | Host-bound Engine root manifest, per-user launchd supervision, authenticated query/admin sockets, and exact reversal scope |

Each record retains its alternatives, consequences, reversal path, and
unresolved implementation edges. Engine workers consume ADR-001 through
`../engine/docs/ARCHITECT_HANDOFF_001.md`, the retained Kolmogrov/semantic
boundary through `../engine/docs/ARCHITECT_HANDOFF_002.md`, current integration
authority through ADR-003, the Core 1.0 gate through ADR-006, and the frozen
Engine semantic-v0 handoff through ADR-007, as corrected for required live
search by ADR-008, and the Engine negotiation ledger.
