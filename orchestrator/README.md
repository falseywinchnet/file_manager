# Orchestrator

Status: **bootstrap kernel opened; provider contracts negotiated incrementally**.

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

## Current executable slice

The Rust bootstrap contains only provider-independent competence:

- common status/error and request/reply vocabulary;
- deterministic lifecycle;
- contract and availability catalogues;
- human and JSON CLI output;
- JSONL-over-stdio conformance service;
- explicit plugin and semantic-fact stubs.

It does not yet contain a real engine client, database, plugin worker, semantic
fact API, platform adapter, GUI, or durable user settings.

## Negotiated integration

[`negotiations/README.md`](negotiations/README.md) defines the ping-pong process.
Orchestrator places an interface note inside every affected project, the project
records its reply there, and Orchestrator reconciles accepted semantics into
[`spec/`](spec/). This resolves circular dependencies without turning unfinished
producer code into ABI.

## Governing material

- [`ADR-003`](../decisions/ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md)
  records the current authority and sequencing decision.
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
