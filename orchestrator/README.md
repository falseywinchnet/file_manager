# The Oracle

Status: **paper specification; implementation deliberately gated**.

The Oracle is File Manager's Rust control plane and master interoperability
authority. It specifies and will eventually coordinate the APIs and ABIs among:

- the C++ File Manager frontend;
- GUI.Forms;
- the standalone Go file/search engine;
- the Kolmogrov core fuzzy-candidate implementation;
- plugin supervisors and sandboxed workers;
- semantic-memory and provider hives;
- settings, handler, command, CLI, AI, and platform-integration clients.

The File Manager application is a program built on GUI.Forms, the Go engine, and
The Oracle. The Oracle does not absorb those projects. It owns the clean contract
map that allows them to interoperate without private-layout coupling.

## Paper-phase output

- [`spec/CONTRACT_REGISTRY.md`](spec/CONTRACT_REGISTRY.md) — master inventory of
  every cross-boundary contract and call direction.
- [`planning/MASTER_SPECIFICATION.md`](planning/MASTER_SPECIFICATION.md) — how
  semantic contracts, ABI projections, generated bindings, and project proposals
  relate.
- [`planning/AUTHORITY_AND_PROCESS_MODEL.md`](planning/AUTHORITY_AND_PROCESS_MODEL.md)
  — trusted processes, failure containment, authority, and restart behavior.
- [`planning/HIVE_AND_SETTINGS_MODEL.md`](planning/HIVE_AND_SETTINGS_MODEL.md) —
  durable memory, disposable provider data, registries, quotas, and erasure.
- [`planning/CONFORMANCE_AND_VERSIONING.md`](planning/CONFORMANCE_AND_VERSIONING.md)
  — version namespaces, fixtures, compatibility, and differential testing.
- [`planning/DELIVERY_SEQUENCE.md`](planning/DELIVERY_SEQUENCE.md) — why Oracle
  code waits and what unlocks each implementation stage.
- [`planning/FUTURE_THREAD_HANDOFF.md`](planning/FUTURE_THREAD_HANDOFF.md) — exact
  instructions for the later Oracle implementation task.

The existing [`../plugin_runtime/`](../plugin_runtime/) plan is retained as
research input for the sandbox supervisor, guest protocol, package lifecycle,
and hostile fixtures. It is not the project authority after this recharter.
