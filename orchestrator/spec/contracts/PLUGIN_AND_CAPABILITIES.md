# ORC-PLG: plugin, worker, and capability contracts

Status: **stubbed; legacy plugin-runtime research retained for later import**.

The future Rust Orchestrator supervisor will own killable workers. No worker,
package, grant, plugin AI, or extension operation exists in the bootstrap.
Third-party/native code never runs inside Orchestrator, the frontend, GUI.Forms,
or the Go engine.

Retained requirements:

- capabilities are supervisor-issued scoped handles, not trusted strings;
- inputs are bounded streams/handles and immutable identity snapshots;
- every job has deadline, cancellation, CPU/memory/output/open-handle budgets;
- replies bind request nonce, plugin/package/version, source generation, grant,
  and provenance;
- unknown capability, version, or critical flag fails closed;
- worker death contains descendants and revokes handles;
- plugins return data or declarative registrations, never GUI objects.

Candidate later extension classes include bounded preview, thumbnail,
virtual-system, search, extracted-field, plugin AI, handler evidence, and
declarative commands. Semantic-fact operations wait for the architect's
separate design. Candidate retention does not admit file mutation, network, UI
injection, or OS-association authority.

The detailed sandbox/codec/package candidates remain in
`../../../plugin_runtime/planning/` until integrated here through decisions.
