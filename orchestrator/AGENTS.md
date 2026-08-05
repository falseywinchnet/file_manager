# The Oracle operating instructions

This directory is the paper-first Rust control-plane project for File Manager.
The component is named **The Oracle**; `orchestrator/` is its repository path.

## Current phase

The Oracle is specification-only until the dependency gates in
`planning/DELIVERY_SEQUENCE.md` are satisfied and the grand architect explicitly
opens implementation. Do not create `Cargo.toml`, Rust sources, generated
bindings, executable fixtures, or package artifacts during the paper phase.

Before editing, read in order:

1. `README.md`
2. `planning/MASTER_SPECIFICATION.md`
3. `spec/CONTRACT_REGISTRY.md`
4. `planning/AUTHORITY_AND_PROCESS_MODEL.md`
5. `planning/HIVE_AND_SETTINGS_MODEL.md`
6. `planning/CONFORMANCE_AND_VERSIONING.md`
7. `planning/DELIVERY_SEQUENCE.md`
8. the relevant contract under `spec/contracts/`

The root `AGENTS.md`, decision protocol, accepted ADRs, and negative product
definition remain authoritative.

## Contract authority

- This repository owns the canonical registry of every cross-project API, ABI,
  wire protocol, event stream, capability, and version namespace.
- Owning the registry does not mean implementing every endpoint. The registry
  names the semantic owner, provider, consumers, transport, authority, version,
  failure behavior, and conformance artifacts.
- GUI.Forms, the Go engine, the C++ frontend, platform adapters, and plugin SDKs
  may propose or implement their side of a contract. They may not invent an
  unregistered cross-boundary call in local code.
- During the paper phase, subprojects contribute proposed changes under
  `proposals/`; accepted changes are integrated into `spec/` with an explicit
  decision/provenance entry.
- No Rust, Go, or C++ native layout crosses a language or process boundary.
  Cross-process contracts use versioned messages. In-process language-neutral
  seams use an explicit C ABI with fixed-width types and ownership rules.
- Bidirectional cooperation uses requests, replies, and event streams. Do not
  implement foreign-thread callbacks or reciprocal raw function-pointer webs.

## Reef boundary

The trusted Oracle may own policy, registries, hives, routing, validation,
quotas, and worker supervision. It may not:

- render File Manager controls;
- own filesystem bytes or become the file-mutation authority;
- absorb the Go catalogue/index store;
- run third-party native code or AI models in its trusted process;
- expose plugin workers directly to GUI.Forms or engine internals;
- make web discovery, a plugin store, or remote content a core ambient service;
- let a provider's derived records overwrite exact filesystem facts or explicit
  user-authored memory.

## Naming discipline

“The Oracle” means this runtime component. Use `reference model`, `identity
fixture`, `conformance evaluator`, or a qualified term such as `filesystem
oracle` for tests; do not use an unqualified `oracle` for a test helper.

## Change discipline

- Keep every contract entry small enough to test independently.
- Mark unknown fields, missing services, timeouts, stale generations, partial
  results, and capability denial explicitly.
- A call is not specified until cancellation, backpressure, ownership, resource
  bounds, and version mismatch behavior are specified.
- Preserve alternatives and rejected proposals. Do not let a generated binding
  become the source of truth over the semantic contract.
