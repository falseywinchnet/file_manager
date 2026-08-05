# Plugin Runtime

Status: **frozen legacy research source; superseded as an implementation home**.

This subtree records the earlier Rust containment, capability, and
interoperability study for File Manager extensions. Its valuable threat model,
capability analysis, hostile fixtures, and protocol questions are retained.
Production implementation now belongs to the Oracle plugin-supervisor subsystem
under [`../orchestrator/`](../orchestrator/), whose contract registry reconciles
the engine, frontend, plugin, hive, CLI, and platform boundaries together.

Do not dispatch the old sibling handoff or create a competing runtime here.

## Confirmed purpose

- **GIVEN:** contain third-party/native preview failures outside File Manager.
- **GIVEN:** admit only preview, thumbnail, virtual-system, and search provider
  extension classes at present.
- **GIVEN:** plugins contribute information, not GUI controls or house style.
- **GIVEN:** non-PNG media decoding belongs in plugins.
- **GIVEN:** File Manager core has no web search, store, or remote discovery.
- **GIVEN:** Rust is preferred for the hostile wrapper/sandbox boundary.

Nothing preserved here selects a wire codec, sandbox primitive, package format, trust
tier, update channel, ABI horizon, or WebAssembly/native execution model. Those
remain candidates until their declared gates run and an approved decision
record accepts them.

## Reading order

1. [`planning/README.md`](planning/README.md)
2. [`planning/REQUIREMENTS_LEDGER.md`](planning/REQUIREMENTS_LEDGER.md)
3. [`planning/THREAT_AND_CAPABILITY_MODEL.md`](planning/THREAT_AND_CAPABILITY_MODEL.md)
4. [`planning/PROCESS_AND_PROTOCOL.md`](planning/PROCESS_AND_PROTOCOL.md)
5. [`planning/EXTENSION_CONTRACTS.md`](planning/EXTENSION_CONTRACTS.md)
6. [`planning/LIFECYCLE_AND_DISTRIBUTION.md`](planning/LIFECYCLE_AND_DISTRIBUTION.md)
7. [`planning/VALIDATION_AND_IMPLEMENTATION.md`](planning/VALIDATION_AND_IMPLEMENTATION.md)
8. [`planning/SIBLING_THREAD_HANDOFF.md`](planning/SIBLING_THREAD_HANDOFF.md)

## Historical independence contract

Future C/C++ and Go consumers call a stable supervisor-facing C boundary or a
versioned local protocol. Plugins never link GUI.Forms, the C++ GUI process, or
the Go indexer's internal data structures. Host adapters translate host objects
to explicit protocol values and scoped handles.
