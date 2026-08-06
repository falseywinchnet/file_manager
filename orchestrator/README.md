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
and CLI status/shutdown across separate processes. ADR-011 adds a stable macOS
Application Support endpoint, launchd listener adoption, and bounded
activation/credential rediscovery. An actual installed LaunchAgent lifecycle
and the Windows named-pipe gate remain open. Its fixed worker pool and bounded
pending-session queue prevent slow peers from creating unbounded threads or
blocking the control plane beyond the bounded handshake interval. Native peer
UID verification, no-follow durable endpoint publication, secure Rust secret
erasure, fail-closed worker panic containment, bounded stdio draining, and
non-authoritative live transport counters harden that boundary. A separately
built C++17 conformance client now reads the
atomic typed bootstrap snapshot—including contracts, availability, route/
fallback state, and service-control eligibility—verifies the golden digest,
and reconnects across daemon restarts. A database, plugin worker,
semantic fact API, GUI, and durable user settings remain absent and
independently reported.

The release projection now carries a deterministic SHA-256 digest over the
manifest fields and embedded Core contract inputs. It is explicitly unsigned;
packaged-artifact signing remains outside this development build's claim.

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
- [`ADR-010`](../decisions/ADR-010-ORCHESTRATOR-CORE-1-0-COMPATIBILITY-HORIZON.md)
  starts the macOS Core contract/wire 1.0 line and defines Core 1.x compatibility.
- [`ADR-011`](../decisions/ADR-011-ORCHESTRATOR-MACOS-LAUNCHD-ACTIVATION.md)
  defines stable macOS discovery, launchd socket adoption, and activation retry.
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

On macOS, the Rust CLI and independent C++ client derive the same stable private
Application Support location. `--runtime-dir` remains available as an explicit
test or development override:

```sh
cargo run --manifest-path orchestrator/Cargo.toml -- \
  serve-local
cargo run --manifest-path orchestrator/Cargo.toml -- \
  call-local status --json
cargo run --manifest-path orchestrator/Cargo.toml -- \
  call-local shutdown
```

`launchd-plist` prints (but does not install or load) a LaunchAgent definition
for the current binary. Packaging must first create the printed socket's parent
leaf as the user with mode `0700`, install the binary and plist, and then perform
the remaining real activation/removal gate:

```sh
cargo run --manifest-path orchestrator/Cargo.toml -- launchd-plist
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
