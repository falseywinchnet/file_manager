# Plugin Runtime operating instructions

Status: **FROZEN LEGACY RESEARCH INPUT**.

This subtree preserves the earlier repo-shaped study of the hostile plugin
boundary. Do not begin or continue production implementation here. The active
authority and future Rust implementation home are `../orchestrator/`; its Oracle
plugin-supervisor subsystem must import useful threat, capability, protocol, and
validation work through the contract proposal process.

Edits here are limited to provenance, factual correction, negative-result
retention, and explicit export into Oracle unless the grand architect reopens
the subtree. The repository root `AGENTS.md` and Oracle planning governance
remain authoritative.

## Status discipline

- This project is planning/research until the grand architect approves numbered
  decisions. Code written for an experiment does not decide the architecture.
- Every consequential claim uses GIVEN, OBSERVED, MEASURED, HYPOTHESIS,
  CANDIDATE, REJECTED, or DECIDED exactly as defined at the repository root.
- Put future accepted decisions under `decisions/`. Do not mark an ADR accepted
  without owner approval.
- Preserve failed isolation, codec, IPC, and packaging experiments with their
  environments and rejection gates.

## Hard boundaries

- Rust is the admitted language for the hostile wrapper/supervisor boundary.
- Third-party code never executes in the File Manager GUI process.
- Plugins provide information through fixed preview, thumbnail, virtual-system,
  and search contracts. They do not inject, replace, or restyle GUI controls.
- Plugin-originated file mutations are not admitted. A plugin cannot acquire a
  mutation capability by sending an unrecognized request.
- Core operation is offline and contains no web search, store, or remote-content
  discovery. Any future networked plugin power is a separate unresolved grant.
- Non-PNG rich-media decoders belong behind this boundary. PNG needed by the
  GUI/theme system is not owned here.
- Exact filesystem identity and stored records remain authoritative. Plugin
  search results are attributed proposals, never filesystem truth.

## Historical build-shape guardrail

The earlier intended shape was a standalone Rust workspace rooted here:

```text
plugin_runtime/
  Cargo.toml
  crates/
  sdk/
  tools/
  fixtures/
  experiments/
  decisions/
  planning/
```

Do not create that workspace here. Future libraries, helpers, tests, fixtures,
and SDK artifacts belong under Oracle after its implementation gate opens.

## Security engineering

- Parse and validate all untrusted bytes before allocating from claimed sizes.
- Default deny unknown messages, capabilities, extension points, and protocol
  versions. Never silently weaken a sandbox when an enforcement primitive is
  absent.
- Capabilities are unforgeable supervisor-issued handles or scoped grants, not
  plugin-declared strings trusted at runtime.
- Pass narrow handles/streams instead of ambient paths wherever the platform
  permits. Never grant a plugin the user's home directory for convenience.
- Bound wall time, CPU, memory, output dimensions, output bytes, message size,
  recursion, child processes, open handles, and derived-data storage.
- Logs and crash records omit file contents and redact paths by construction.
