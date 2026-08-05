# ADR-002: Oracle contract authority and program sequence

Status: **superseded by
[`ADR-003`](ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md)**.

The repository topology, Rust boundary, and contract-authority findings remain
historical inputs. The component name, integration authority, negotiation
workflow, and implementation sequencing are replaced by ADR-003.

Date: 2026-08-05.

Owner approval: the grand architect directed a paper-first `orchestrator/`
repository called The Oracle, a waiting `file_manager/` frontend repository, and
the dependency order recorded here.

## Question

Where do the API/ABI specifications and runtime coordination among GUI.Forms,
the Go engine, Kolmogrov, the C++ frontend, hives, settings, CLI, platform
integration, and hostile plugin wrappers live; and in what order are these
systems built?

## GIVEN constraints

- The Oracle must own, interoperate, and reciprocate all cross-project APIs and
  ABIs during development.
- All boundaries must be organized sanitarily in one paper repository consulted
  by every subproject.
- The File Manager frontend is an application running on GUI.Forms, the engine,
  and The Oracle; it does not absorb them.
- GUI.Forms reaches semicompletion first, while the engine becomes substantially
  usable.
- The Oracle remains on paper until those inputs are stable enough to consume.
- Oracle implementation then supplies hives, settings, CLI, plugin supervision,
  associations/commands, interoperability, and conformance.
- The frontend is built and dogfooded only after those foundations become
  manageable and the architect transitions direction to it.
- The Oracle/runtime and hostile boundary are written in Rust; that does not
  make untrusted native plugins safe in-process.
- Kolmogrov is a core engine fuzzy-candidate mechanism, distinct from semantic
  memory and AI interpretation.

## Candidates

### A. Let every subproject own and evolve its interfaces independently

Fast locally, but produces reciprocal private calls, version drift, duplicated
types, unsafe ABI assumptions, and an integration crisis in the frontend.

### B. Make the frontend the integration authority

Tempting because it consumes everything, but forces product code to become the
plugin supervisor, hive owner, settings authority, API broker, and compatibility
layer.

### C. Expand the narrow plugin-runtime project without rechartering it

Preserves existing files, but its declared non-goals reject handlers, metadata
deposits, semantic storage, orchestration, and public API ownership. Its name and
instructions would contradict its actual authority.

### D. Paper-first Oracle contract repository plus waiting frontend

One registry defines every cross-project semantic contract and projection. The
Rust Oracle later implements policy, hives, routing and plugin supervision;
foundation projects remain independent providers/consumers. The frontend waits
for explicit consumption snapshots.

## Decision

Choose D.

### Repository topology

- `orchestrator/` is the paper-first repository for The Oracle.
- `orchestrator/spec/CONTRACT_REGISTRY.md` is the canonical API/ABI inventory.
- `orchestrator/spec/contracts/` owns language/transport-neutral semantics.
- `orchestrator/proposals/` receives missing-boundary proposals from
  subprojects.
- `orchestrator/bindings/` and `orchestrator/conformance/` remain empty of code
  until the applicable paper gates open.
- `file_manager/` is the future C++ frontend and remains paper-only until Gate
  F0 and explicit architect approval.
- `plugin_runtime/` is frozen legacy research input. Its sandbox, capability,
  protocol, package, fixture, and validation work is integrated later into the
  Oracle's plugin-supervisor subsystem; it is no longer a separate
  implementation authority.

### Meaning of Oracle API ownership

The Oracle registry owns the semantic map of every boundary, but the runtime
does not need to proxy every hot call. GUI.Forms remains an in-process frontend
dependency. The frontend and Oracle may each consume registered private engine
contracts. The Oracle owns the public AI/plugin/CLI interoperability surface,
capability enforcement, hives, registries, routing and worker supervision.

Bidirectional cooperation uses versioned commands, replies and bounded event
streams—not raw reciprocal callbacks, native language layouts, or foreign
function calls on uncontrolled threads.

### Runtime authority

- C++ frontend: application interaction and composition.
- GUI.Forms: retained UI mechanics and C ABI implementation.
- Go engine: exact file-object catalogue, lexical retrieval, and core Kolmogrov
  candidate index.
- Rust Oracle: public API, contract negotiation, policy, registries, semantic and
  provider hives, settings, CLI, handler/command declarations, query routing,
  plugin supervision, audit and platform-integration coordination.
- Plugin workers: one bounded granted job; never trusted-process execution.
- First-party platform adapters: explicit OS registration/integration work.

### Development order

1. GUI.Forms advances to a named semicomplete consumption snapshot.
2. Engine advances to a named useful exact-query/generation/API snapshot and
   preserves a first-class Kolmogrov seam.
3. Kolmogrov research continues independently; early engine controls do not
   demote its intended core role.
4. Oracle is developed on paper: registry, contracts, authority, versions,
   threat model, failure semantics, and proposal workflow.
5. After Gate O0, implement Oracle contract laboratory, then core services,
   hives, settings, CLI, plugin reef and first-party integration.
6. After Gate F0 and explicit approval, implement the C++ frontend.
7. Dogfood first on macOS, then carry the same contracts to Windows and Linux.

## Why the other candidates lost

- Independent interface ownership makes the eventual frontend the accidental
  specification and hides cross-language errors until integration.
- Frontend ownership concentrates policy, storage, plugins and presentation in
  one process/project.
- The old plugin-runtime charter is deliberately hostile-boundary-only and
  cannot truthfully govern the full system without a new authority model.
- Building Oracle or frontend before provider contracts stabilize rewards
  private reach-throughs and repeated rewrites.

## Consequences

- All new cross-project operations require an Oracle registry entry/proposal.
- Subprojects continue independently and export consumption snapshots plus
  fixtures rather than asking the frontend to depend on internals.
- The Oracle carries a large specification burden before Rust implementation;
  this is intentional because it owns the most dangerous coordination surface.
- The frontend begins later but integrates against coherent mocks and contracts.
- “Oracle” as an unqualified test-helper name becomes ambiguous and should be
  replaced by qualified reference/conformance terminology over time.

## Reversal and migration path

The semantic registry and fixtures remain useful if the Oracle daemon is later
split into services or one hot path becomes direct. Runtime routing may change
without changing object identity, authority, capabilities, errors, or the
registered provider/consumer contracts. A future ADR may split process roles;
it may not reintroduce unregistered private cross-project calls.

## Unresolved edges

- Exact IDL and wire codec.
- Whether the frontend uses a direct registered engine query path in production
  or always traverses an Oracle query broker.
- Exact stable C ABI projections for Oracle and GUI.Forms clients.
- Plugin command mutation classes beyond read-only/open-with behavior.
- Semantic-memory write authority for external AI versus user-confirmed facts.
- Hive physical stores, sync and encryption.
- Platform sandbox and integration mechanisms.
- Numeric Oracle/plugin/hive budgets.
