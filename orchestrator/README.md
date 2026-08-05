# Orchestrator

Status: **headless Core 1.0 implementation active; provider contracts negotiated
incrementally**.

Orchestrator is File Manager's user-scoped Rust integration authority. It owns
the required/available capability map and the canonical interoperability
contracts among:

- the C++ File Manager frontend and GUI.Forms;
- the systemwide Go file/search engine;
- the Kolmogrov core candidate channel;
- future plugin supervisors and sandboxed workers;
- future semantic/provider hives;
- settings, handlers, commands, CLI, audit, and platform integrations.

File Manager normally consumes Orchestrator for integration and policy.
GUI.Forms remains its direct in-process UI dependency, and a registered direct
engine path remains available for degraded fallback. Orchestrator has no GUI;
File Manager renders its settings and service controls.

Orchestrator has no dependency on GUI.Forms. ADR-006 directs it toward a named
Core 1.0 release before Frontend 001 bootstraps against the live service.

## Current executable slice

The Rust bootstrap contains only provider-independent competence:

- common status/error and request/reply vocabulary;
- deterministic lifecycle;
- contract and availability catalogues;
- human and JSON CLI output;
- JSONL-over-stdio conformance service;
- explicit plugin and semantic-fact stubs.

Engine semantic v0 and its capability gates are now frozen for experimental
implementation under ADR-007. ADR-008 separately requires catalogue-independent
Engine search; Orchestrator now has a provider-neutral draft port and tested
fallback law, while the real live-search provider contract and implementation
remain negotiating. A bounded Rust development client now passes version,
status, exact query, and shutdown against a separately built Go Engine process.
The first ADR-009 local-daemon slice now provides bounded `ORC1` framing,
private Unix discovery, an OS-random credential hello, instance verification,
and CLI status/shutdown across separate processes. It remains pre-production
until concurrency, hostile/cross-version, launchd session-location, independent
client, and Windows named-pipe gates pass. A database, plugin worker, semantic
fact API, GUI, and durable user settings remain absent and independently
reported.

The executable reports progress toward the Core 1.0 bootstrap profile. A target
declaration is not a readiness claim: discovery/authentication, production
local transport, independent client conformance, and stable contract horizons
remain required before the release can report ready.

## Negotiated integration

[`negotiations/README.md`](negotiations/README.md) defines the ping-pong process.
Orchestrator places an interface note inside every affected project, the project
records its reply there, and Orchestrator reconciles accepted semantics into
[`spec/`](spec/). This resolves circular dependencies without turning unfinished
producer code into ABI.

## Governing material

- [`ADR-003`](../decisions/ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md)
  records the current authority and sequencing decision.
- [`ADR-006`](../decisions/ADR-006-ORCHESTRATOR-CORE-1-0-FRONTEND-BOOTSTRAP.md)
  records the Core 1.0 target and live frontend bootstrap dependency.
- [`ADR-007`](../decisions/ADR-007-ENGINE-ORCHESTRATOR-SEMANTIC-V0-AND-CAPABILITY-GATES.md)
  records the Engine semantic-v0 and capability-gated integration boundary.
- [`ADR-008`](../decisions/ADR-008-CATALOGUE-INDEPENDENT-LIVE-SEARCH.md)
  records the required bounded catalogue-independent fallback and routing law.
- [`ADR-009`](../decisions/ADR-009-ORCHESTRATOR-LOCAL-WIRE-DISCOVERY-AND-SESSION-AUTH.md)
  records the local wire, discovery, and session-authentication decision.
- [`spec/CONTRACT_REGISTRY.md`](spec/CONTRACT_REGISTRY.md) is the master
  inventory.
- [`planning/MASTER_SPECIFICATION.md`](planning/MASTER_SPECIFICATION.md) defines
  semantic contract and projection doctrine.
- [`planning/AUTHORITY_AND_PROCESS_MODEL.md`](planning/AUTHORITY_AND_PROCESS_MODEL.md)
  defines runtime boundaries.
- [`planning/CONFORMANCE_AND_VERSIONING.md`](planning/CONFORMANCE_AND_VERSIONING.md)
  defines fixtures and compatibility.
- [`planning/DELIVERY_SEQUENCE.md`](planning/DELIVERY_SEQUENCE.md) defines the
  incremental bootstrap and adapter gates.

## Build

```sh
cargo fmt --check --manifest-path orchestrator/Cargo.toml
cargo test --manifest-path orchestrator/Cargo.toml
cargo clippy --manifest-path orchestrator/Cargo.toml --all-targets --all-features -- -D warnings
cargo run --manifest-path orchestrator/Cargo.toml -- status
```

The current Unix local-daemon slice requires an explicit short private runtime
leaf until the launchd/session adapter owns its location:

```sh
cargo run --manifest-path orchestrator/Cargo.toml -- \
  serve-local --runtime-dir /absolute/private/runtime-leaf
cargo run --manifest-path orchestrator/Cargo.toml -- \
  call-local status --runtime-dir /absolute/private/runtime-leaf --json
cargo run --manifest-path orchestrator/Cargo.toml -- \
  call-local shutdown --runtime-dir /absolute/private/runtime-leaf
```

The bounded JSONL development adapter can be probed against an explicitly built
Engine binary and disposable sandbox without selecting JSONL as production IPC:

```sh
cargo run --manifest-path orchestrator/Cargo.toml \
  --example engine_jsonl_probe -- /path/to/fileman-engine /path/to/disposable-sandbox
```

Supplying an indexed subroot and exact filename additionally exercises canonical
root plan/apply, reconciliation, and the typed `ORC-ENG-001` adapter:

```sh
cargo run --manifest-path orchestrator/Cargo.toml \
  --example engine_jsonl_probe -- /path/to/fileman-engine \
  /path/to/disposable-sandbox /path/to/disposable-sandbox/source ledger.txt
```
