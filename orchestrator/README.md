# Orchestrator

Status: **headless Core 1.0 ready on macOS; provider contracts continue behind
independent capability gates**.

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

The first-party application-backbone proposal—Document Picker sessions,
application profiles, shared administration projection, local help resolution,
and transfer-flavor registration—is recorded in
[`proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md`](proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md).
It does not add those operations to Core 1.0 or put GUI code in this daemon.

The proposed Lexicon provider and host-mediated Archive Viewer/Image Converter
proofs are separately recorded in `../lexicon/` and
[`proposals/first_party_extensions/`](proposals/first_party_extensions/). They
do not open plugin execution, lexical lookup, filesystem write authority, or UI
injection in Core 1.0.

## Current executable slice

The executable source now separates the thin process entry, CLI/presentation,
stdio laboratory, bounded Unix daemon host, launchd activation, endpoint/session
framing, semantic kernel, and provider ports. The searchable local service
atlas under [`docs/library/`](docs/library/) documents those boundaries and
generates a Markdown mirror from the same reviewed manifest.

The Rust bootstrap contains only provider-independent competence:

- common status/error and request/reply vocabulary;
- deterministic lifecycle;
- contract and availability catalogues;
- human and JSON CLI output;
- JSONL-over-stdio conformance service;
- explicit plugin and semantic-fact stubs.

Engine semantic v0 and its capability gates are now frozen for experimental
implementation under ADR-007. ADR-008 separately requires catalogue-independent
Engine search. The Go provider now implements `engine.query_live`; the
provider-neutral broker, narrow fallback law, development child adapter, and
single frontend-facing `orchestrator.search` operation pass a real
zero-catalogue route. Availability is derived from the connected Engine's
advertised `engine.live.query` and `contract.ORC-ENG-004` capabilities. The
authenticated Rust integration test and independently compiled C++17 client
both pass without exposing catalogue/live selection to the frontend. Installed
The contained M4 profile now connects a separately installed, host-bound Go
Engine through its authenticated `ENG1` discovery and distinct query/admin
authorities. Exact search, status, integrity, reconcile and supervised restart
pass through Orchestrator; native NTFS/ext4 and other installed-platform
promotion evidence remain open. The bounded Rust development client also
passes version, status, exact query, and shutdown against a separately built Go
Engine process.
The first ADR-009 local-daemon slice now provides bounded `ORC1` framing,
private Unix discovery, an OS-random credential hello, instance verification,
and CLI status/shutdown across separate processes. ADR-011 adds a stable macOS
Application Support endpoint, launchd listener adoption, and bounded
activation/credential rediscovery. The installed LaunchAgent lifecycle now
passes first activation, shutdown, bounded supervisor reactivation, instance
rotation, bootout, and exact removal. Windows explicit-process named pipes are
now implemented as a development projection; installed supervision remains open.
Its fixed worker pool and bounded
pending-session queue prevent slow peers from creating unbounded threads or
blocking the control plane beyond the bounded handshake interval. Native peer
UID verification, no-follow durable endpoint publication, secure Rust secret
erasure, fail-closed worker panic containment, bounded stdio draining, and
non-authoritative live transport counters harden that boundary. A separately
built C++17 conformance client now reads the
atomic typed bootstrap snapshot—including contracts, availability, route/
fallback state, and service-control eligibility—verifies the golden digest,
and reconnects across daemon restarts. `ORC-SET-001` now supplies a bounded
durable scalar settings store selected by ADR-018, while `ORC-UI-001` supplies
immutable service facts and identity-bound closed commands selected by ADR-019.
The independently built C++17 client consumes both. A general database, plugin
worker, semantic fact API, and Orchestrator GUI remain absent and independently
reported; File Manager owns their presentation.

The release projection now carries a deterministic SHA-256 digest over the
manifest fields and embedded Core contract inputs. It is explicitly unsigned;
packaged-artifact signing remains outside this development build's claim.

The executable now reports the Core 1.0 bootstrap profile ready on macOS because
discovery/authentication, production local transport, installed launchd
lifecycle, independent client conformance, and stable contract horizons pass.
Separately gated providers remain truthfully reduced.

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
- [`ADR-018`](../decisions/ADR-018-ORCHESTRATOR-BOUNDED-ATOMIC-SETTINGS-STORE.md)
  selects the bounded atomic first settings store.
- [`ADR-019`](../decisions/ADR-019-CONTAINED-ENGINE-ADAPTER-AND-IDENTITY-BOUND-SERVICE-CONTROLS.md)
  selects the contained installed Engine route and service-command identity law.
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
- [`proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md`](proposals/application_backbone/DOCUMENT_PICKER_HELP_AND_TRANSFER.md)
  records the proposed multi-application picker/help/transfer edge.
- [`proposals/first_party_extensions/`](proposals/first_party_extensions/)
  records the host-mediated Archive Viewer/Image Converter proposal family.
- [`../lexicon/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../lexicon/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md)
  records the proposed exact lexical/corpus provider edge.

## Build

The crate's declared minimum Rust version is 1.87. The lockfile is
authoritative for dependency reproduction. Build products stay under ignored
component-local trees.

The Windows GNU development build can be verified with
`./orchestrator/tools/verify_windows.ps1`, supplying `-MingwBin` and `-RustBin`
when needed. It builds the native Rust CLI/laboratory and C++ source client.
Windows local daemon transport and separate Go Engine integration are available
for explicitly launched current-user processes. Installed supervision and durable
settings remain unavailable; this does not promote Windows to Core 1.0 readiness.
See [the Windows projection](spec/WINDOWS_LOCAL_PROJECTION.md) and
[native evidence](conformance/evidence/SHADOW_WINDOWS_PIPES_2026-09-29.md).
The release manifest's `first_platform: macos` remains historical macOS
evidence, not an assertion about the host running a development CLI.
See [the Shadow verification receipt](conformance/evidence/SHADOW_WINDOWS_2026-09-29.md).

The internal `AtomicBatchPool` is an available synchronous compute primitive,
not the blocking local-session host and not a plugin authority. Its hardened
C11/C++20 sibling is independently consumable from
`third_party/threadpool_atomic_fast`; both implementations share the safety
argument under `formal/` and the M4 benchmark workload.

```sh
rustup run stable cargo fmt --manifest-path orchestrator/Cargo.toml -- --check
rustup run stable cargo test --manifest-path orchestrator/Cargo.toml --locked
rustup run stable cargo clippy --manifest-path orchestrator/Cargo.toml --all-targets --all-features --locked -- -D warnings
rustup run stable cargo run --manifest-path orchestrator/Cargo.toml --locked -- status
rustup run stable cargo run --manifest-path orchestrator/Cargo.toml --release --bin orchestrator-worker-bench -- 10
```

The navigable, dependency-free source/service reference lives at
[`docs/library/index.html`](docs/library/index.html), with checked-in Markdown
mirrors for agents and text tools. Regenerate or check it with:

```sh
python3 orchestrator/tools/generate_service_docs.py
python3 orchestrator/tools/generate_service_docs.py --check
```

From the authoritative checkout, the complete M4 gate is one command:

```sh
/Users/ultimussecundai/.local/bin/m4build -- \
  /bin/sh orchestrator/tools/verify_m4.sh
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
  call-local services --json
cargo run --manifest-path orchestrator/Cargo.toml -- \
  call-local shutdown
```

`launchd-plist` prints (but does not install or load) the conformant LaunchAgent
definition for the current binary. Packaging creates the printed socket's
parent leaf as the user with mode `0700`, installs the binary and plist, and may
then use the same lifecycle exercised by the Core 1.0 installed trial:

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

For the end-to-end development route, `serve-local` may supervise that
separately built Engine over bounded JSONL and expose only the unified
`orchestrator.search` operation to authenticated local clients:

```sh
cargo run --manifest-path orchestrator/Cargo.toml -- serve-local \
  --runtime-dir /absolute/private/runtime \
  --engine-binary /absolute/path/fileman-engine \
  --engine-sandbox-root /absolute/disposable/sandbox \
  --engine-root-id docs \
  --engine-root-path /absolute/disposable/sandbox/source
```

All four Engine options are atomic. Their absence makes runtime live-search
availability `unavailable`; a connected child must advertise both
`engine.live.query` and `contract.ORC-ENG-004` as `available`. This development
route does not claim installed Engine discovery, query/admin authentication, or
platform service supervision.
