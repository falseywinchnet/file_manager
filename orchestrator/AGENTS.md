# Orchestrator operating instructions

This directory is the Rust integration-authority project for File Manager. The
component and executable are named **Orchestrator**.

## Current phase: Core 1.0

The grand architect opened the provider-independent bootstrap kernel in
[`ADR-003`](../decisions/ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md).
Rust implementation is permitted and directed toward the headless Core 1.0
release profile in ADR-006: common contracts, lifecycle, release readiness,
availability/capability state, CLI projections, production local client
bootstrap, deterministic fixtures, fake peers, and conformance work. GUI.Forms
is not an Orchestrator dependency.

Real provider adapters advance only after their project-local negotiation note
records a reply and the relevant semantic contract/fixtures are sufficiently
complete. Plugin execution, plugin AI, and semantic-fact operations are stubs
until separately opened. Do not infer those APIs from older candidate plans.

Before editing, read in order:

1. `README.md`
2. `../decisions/ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md`
3. `../decisions/ADR-006-ORCHESTRATOR-CORE-1-0-FRONTEND-BOOTSTRAP.md`
4. `planning/MASTER_SPECIFICATION.md`
5. `spec/CONTRACT_REGISTRY.md`
6. `planning/AUTHORITY_AND_PROCESS_MODEL.md`
7. `planning/HIVE_AND_SETTINGS_MODEL.md`
8. `planning/CONFORMANCE_AND_VERSIONING.md`
9. `planning/DELIVERY_SEQUENCE.md`
10. `negotiations/README.md`
11. the relevant contract and project-local negotiation note

The root `AGENTS.md`, decision protocol, accepted ADRs, and negative product
definition remain authoritative.

## Integration authority

- Orchestrator owns the canonical requirement, semantic meaning, version,
  capability, availability, and conformance map for every cross-project edge.
- Orchestrator states what it needs in the affected project's
  `ORCHESTRATOR_INTERFACE_NEGOTIATION.md`. The project records its reply there;
  Orchestrator reconciles the next round and canonical contract.
- A project-local negotiation note is dialogue, not a second specification.
  Accepted semantics live under `spec/` and executable projections identify
  their source revision.
- GUI.Forms, the Go engine, the C++ frontend, platform adapters, and plugin SDKs
  may counterpropose and implement their sides. They may not create an
  unregistered cross-boundary call.
- No Rust, Go, or C++ native layout crosses a language or process boundary.
  Cross-process contracts use versioned messages. In-process language-neutral
  seams use explicit C ABIs with fixed-width types and ownership rules.
- Bidirectional cooperation uses requests, replies, and bounded event streams;
  never foreign-thread callbacks or reciprocal raw function-pointer webs.

## Runtime boundary

- The engine is systemwide; Orchestrator is user-scoped, lazy, and restartable.
- File Manager consumes Orchestrator as its normal integration/policy surface.
  A registered direct engine route is degraded fallback only.
- Orchestrator has no GUI. File Manager renders Orchestrator settings, service
  state, grants, and controls using GUI.Forms.
- The initial public API is the CLI. Human users, local AI tools, and developer
  agents invoking it share the local user's authority. Future plugin-AI
  authority is a different, currently stubbed capability surface.
- Orchestrator owns capability and availability truth. Required but absent
  functionality is reported as unavailable or stubbed, never silently faked.

## Reef boundary

The trusted Orchestrator may own policy, registries, hives, routing, validation,
quotas, and worker supervision. It may not:

- render File Manager controls or own a GUI;
- own filesystem bytes or become the file-mutation authority;
- absorb the Go catalogue/index store;
- run third-party native code or AI models in its trusted process;
- expose plugin workers directly to GUI.Forms or engine internals;
- make web discovery, a plugin store, or remote content a core ambient service;
- let provider records overwrite exact filesystem facts or explicit
  user-authored memory.

## Change discipline

- Keep every contract entry small enough to test independently.
- Mark missing services, timeouts, stale generations, partial results,
  capability denial, stubs, and version mismatch explicitly.
- A call is incomplete until cancellation, backpressure, ownership, bounds, and
  version mismatch are specified.
- Preserve alternatives and rejected proposals. Generated bindings never become
  semantic authority.
- Use `cargo fmt --check`, `cargo test`, and `cargo clippy --all-targets
  --all-features -- -D warnings` before handoff.
