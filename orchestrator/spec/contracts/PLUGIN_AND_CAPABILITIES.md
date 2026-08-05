# ORC-PLG: plugin, worker, and capability contracts

Status: **paper import from legacy plugin-runtime research**.

The Rust Oracle supervises killable workers. Third-party/native code never runs
inside the Oracle, frontend, GUI.Forms, or Go engine.

Retained requirements:

- capabilities are supervisor-issued scoped handles, not trusted strings;
- inputs are bounded streams/handles and immutable identity snapshots;
- every job has deadline, cancellation, CPU/memory/output/open-handle budgets;
- replies bind request nonce, plugin/package/version, source generation, grant,
  and provenance;
- unknown capability, version, or critical flag fails closed;
- worker death contains descendants and revokes handles;
- plugins return data or declarative registrations, never GUI objects.

Expanded Oracle-era extension classes include bounded preview, thumbnail,
virtual-system, search, extracted-field, semantic-memory proposal, handler
evidence, and declarative command contracts. Expansion does not imply file
mutation, network, UI injection, or OS-association authority.

The detailed sandbox/codec/package candidates remain in
`../../../plugin_runtime/planning/` until integrated here through decisions.
