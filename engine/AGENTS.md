# File Manager engine operating instructions

This directory is an isolated Go subproject for the purpose-built File Manager
catalogue, index, and retrieval service. Work here must remain independently
buildable and must not edit the parent File Manager, `gui_forms/`,
`plugin_runtime/`, or `kolmogrov/` trees.

Before changing code, read in order:

1. `README.md`
2. `docs/CHARTER.md`
3. `docs/ARCHITECT_HANDOFF_001.md`
4. `docs/ARCHITECT_HANDOFF_002.md`
5. `docs/API_CONTRACT.md`
6. `docs/STORAGE_ENGINE_PROGRAM.md`
7. `docs/QUERY_AND_RANKING.md`
8. `docs/TEST_AND_BENCHMARK_PROTOCOL.md`
9. `docs/DELIVERY_PLAN.md`
10. `docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md` for any cross-project edge

The architect handoffs are accepted direction. Handoff 002 supplements handoff
001 and controls the Kolmogrov, semantic-memory, and Orchestrator-contract boundaries.
Where an older document still presents one of their decisions as an unresolved
candidate, the handoffs control. Controls and falsification gates remain
mandatory.

## Fixed product boundaries

- **GIVEN:** the engine is written in Go and is a purpose-built File Manager
  backend. SQLite, Xapian, and other stores are comparison controls, not hidden
  production dependencies.
- **GIVEN:** the filesystem is authoritative. The engine stores projections and
  exact observations; it never becomes the authority for file bytes or user
  mutations.
- **GIVEN:** a platform file object has authoritative platform identity. Paths
  are observed addresses. Content hashes express duplicate/version relations.
- **GIVEN:** the most-specific approved root owns each indexed object. Parent
  shards do not duplicate independently indexed child shards.
- **GIVEN:** navigation does not depend on this service. A missing or damaged
  engine may reduce search capability but may not disable folder navigation.
- **GIVEN:** the public interrogation/plugin API is read-oriented and exposes no
  file mutation primitive. Shell-originated mutations are external observations.
- **GIVEN:** AI semantic interpretation and personal memory are deferred to
  Orchestrator-managed providers and hives. The engine remains complete without
  models or embeddings. Its production fuzzy-candidate design nevertheless has
  a first-class Kolmogrov transfer target subject to proof and benchmark gates.
- **GIVEN:** no web search, web store, cloud discovery, or bundled web engine.

## Safety boundary

- Development and test binaries MUST require an explicit sandbox root and MUST
  refuse paths outside it.
- Tests MUST use temporary roots. Never scan the user's home folder, repository
  parent, mounted volumes, or real personal files.
- A production-root admission path may be added only after its approval manifest,
  identity checks, exclusion policy, and audit tests exist.
- The engine never writes inside an indexed source tree. Its store, temporary
  files, and benchmark results live under explicitly supplied engine paths.
- Do not add a `--force`, environment-variable bypass, or convenient unsafe
  default around the sandbox.

## Engineering discipline

- Use the repository evidence labels: **GIVEN**, **OBSERVED**, **MEASURED**,
  **HYPOTHESIS**, **CANDIDATE**, **REJECTED**, and **DECIDED**.
- Correctness precedes speed. Any operation that targets the wrong object is a
  release-blocking failure regardless of benchmark results.
- Preserve exact channel evidence and inferred rank evidence separately.
- No algorithm earns architecture status because it is clever. Compare it to a
  declared baseline on identical fixtures.
- Measurements record revision, OS, filesystem, hardware, corpus, cold/warm
  state, command, p50/p95/p99/max, CPU, resident bytes, bytes read/written, and
  index bytes per record.
- Keep negative results under `results/rejected/`; do not erase them.
- Prefer standard-library dependencies until a dependency wins a named gate.
- Run `go test ./...`, `go test -race ./...`, `go vet ./...`, and the relevant
  benchmarks before handoff. If a toolchain is unavailable, state that plainly;
  do not claim validation.

## Architecture hygiene

- The accepted target spine is object-plus-bindings, exclusive approved-root
  shards under volume manifests, and bounded immutable committed generations.
  Low-level codecs and optimizations remain experiment-driven.
- Exact catalogue, candidate generation, ranking, result cutoff, and transport
  are separate packages and tests.
- The transport layer may not leak storage representation into the API.
- The store must support atomic committed reader generations, integrity checks,
  quarantine, and rebuild before it may claim parity with SQLite.
- Fixed-width perceptual hashes only propose candidates. Every result maps back
  to an exact catalogue record and names its evidence.
- Optimize the exhaustive/correct reference first. Optimize allocation, layout,
  compression, concurrency, and SIMD only after equivalence tests exist.

## Change boundary

The lower project builds only an engine library and service binary. Parent
integration happens later through the versioned API. Do not add GUI code or
couple the engine to File Manager process internals.

Cross-project API and ABI semantics are registered in
`../orchestrator/spec/CONTRACT_REGISTRY.md`. Record replies and counterproposals
in `docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`; do not silently export a
storage layout or Go representation as the program ABI.
