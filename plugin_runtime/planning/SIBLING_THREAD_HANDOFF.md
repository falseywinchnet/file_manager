# First sibling-thread handoff

Status: **SUPERSEDED — DO NOT DISPATCH**.

The grand architect has moved production plugin supervision into the Oracle
repository at `../../orchestrator/`, where it can be specified with every other
cross-project API and ABI. This prompt is retained only as historical research
input. Use `../../orchestrator/planning/FUTURE_THREAD_HANDOFF.md` after Oracle's
implementation gate opens.

The superseded prompt follows verbatim for provenance:

```text
You are implementing milestone R0, the contract-and-threat laboratory for the
independent Rust Plugin Runtime subproject. Work only under:

  /Users/quentinkuttenkuler/file_manager/plugin_runtime/

Do not touch GUI.Forms, File Manager product code, or root planning files. Do
not create a nested .git repository. Preserve all existing dirty worktree
changes. Read, in order and completely:

  /Users/quentinkuttenkuler/file_manager/AGENTS.md
  /Users/quentinkuttenkuler/file_manager/planning/DECISION_PROTOCOL.md
  /Users/quentinkuttenkuler/file_manager/plugin_runtime/AGENTS.md
  /Users/quentinkuttenkuler/file_manager/plugin_runtime/README.md
  /Users/quentinkuttenkuler/file_manager/plugin_runtime/planning/REQUIREMENTS_LEDGER.md
  /Users/quentinkuttenkuler/file_manager/plugin_runtime/planning/THREAT_AND_CAPABILITY_MODEL.md
  /Users/quentinkuttenkuler/file_manager/plugin_runtime/planning/PROCESS_AND_PROTOCOL.md
  /Users/quentinkuttenkuler/file_manager/plugin_runtime/planning/EXTENSION_CONTRACTS.md
  /Users/quentinkuttenkuler/file_manager/plugin_runtime/planning/VALIDATION_AND_IMPLEMENTATION.md

This is an experiment, not an architecture decision. Label every new design
claim GIVEN, OBSERVED, MEASURED, HYPOTHESIS, CANDIDATE, REJECTED, or DECIDED.
Do not create or accept an ADR. Do not select a production codec, IPC stack,
WASM runtime, package format, OS sandbox, service topology, or stable ABI.

Implement a standalone Rust workspace proving only the logical protocol and
supervisor lifecycle. Use the current stable Rust edition admitted by the local
toolchain, pin dependencies, forbid unsafe Rust in R0 crates, and make the lower
workspace build without anything above plugin_runtime. Suggested structure:

  Cargo.toml
  crates/protocol-model/
  crates/supervisor-core/
  crates/lab-transport/
  fixtures/hostile-guests/
  experiments/r0/

Required behavior:

1. Define renderer-, OS-, and codec-neutral semantic types for plugin identity,
   package digest, contract/version range, request ID/nonce, source snapshot,
   capability grant, budgets, job request, terminal status, provenance, and
   resource accounting. Use checked size/time arithmetic and explicit bounded
   newtypes. Do not put paths, file writes, network, shell, UI handles, Rust
   pointers, callbacks, or arbitrary extension maps in the capability model.
2. Implement a deterministic supervisor job state machine with exactly one
   terminal result, idempotent cancellation, deadline enforcement using an
   injected monotonic clock, nonce/source-generation validation, capability
   intersection, output-budget enforcement, and fail-closed handling of unknown
   contract/message states.
3. Implement an in-memory/fake transport only. If serialization is useful for
   fixtures, use a deliberately labeled non-production laboratory encoding and
   keep it behind a Codec trait. Do not benchmark it as a production candidate.
4. Implement hostile scripted guests/fixtures for success, unsupported, panic
   translated to worker loss, never reply, ignores cancellation, oversized
   output, arithmetic overflow attempt, duplicate terminal reply, stale reply,
   wrong nonce, wrong source generation, out-of-order reply, undeclared
   capability, and a result attempting to claim a local path/identity.
5. Emit deterministic redacted event traces containing IDs/digests/status and
   resource counters but no contents, query text, secrets, or raw paths.
6. Add unit, property, and deterministic concurrency tests proving the R0 exit
   gate in VALIDATION_AND_IMPLEMENTATION.md. A randomized test must print and
   persist its seed on failure. No real filesystem outside a temporary fixture
   root and no network access in tests.
7. Add docs for the semantic model, replay commands, dependency locks, exact
   environment/toolchain, test inventory, MEASURED results, known limitations,
   and negative results. Explicitly state that R0 proves neither OS containment
   nor production IPC safety.

Quality gates:

  cargo fmt --check
  cargo clippy --workspace --all-targets --all-features -- -D warnings
  cargo test --workspace --all-features
  cargo doc --workspace --no-deps

Also run a dependency/license/advisory inventory using tools available locally;
if a tool is absent, record that as an unverified gap instead of installing a
global tool silently. Keep build outputs out of the source tree or git status.

Before editing, send me the exact files you intend to own. At completion report:
changed files, commands and outputs, test counts, measured environment, what R0
does and does not establish, rejected/failed attempts, and the next decision
gate. Do not continue into R1.
```

## Why R0 is first

R0 forces the authority model, failure semantics, version vocabulary, and host
independence to exist before an attractive codec, sandbox API, or plugin demo can
smuggle in permanent architecture. It is useful work even if every R1/R2
candidate changes, and it supplies one identical oracle for those comparisons.
