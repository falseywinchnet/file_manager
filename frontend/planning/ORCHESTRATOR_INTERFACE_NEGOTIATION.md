# Orchestrator ↔ File Manager interface negotiation

Status: **round 003 reconciled; live Core 1.0 bootstrap edge requires final
launchd lifecycle evidence before
Frontend 001 under ADR-006**.

Participants: Orchestrator integration authority and the future C++ File
Manager frontend. Canonical families: `ORC-COM-001`, `ORC-LIF-001`,
`ORC-FE-001`, `ORC-CLI-001`, plus later settings/handler/command contracts.

File Manager is Orchestrator's GUI. Orchestrator itself renders nothing.

## Orchestrator proposal 001

### First client operations

| Operation | Purpose |
|---|---|
| `orchestrator.version` | Negotiate semantic, wire, and client ranges |
| `orchestrator.release` | Inspect Core 1.0 target, readiness, and blocking requirements |
| `orchestrator.status` | Display lifecycle and degraded provider state |
| `orchestrator.contracts.list` | Inspect required/implemented contract families |
| `orchestrator.availability.list` | Render required, available, degraded, unavailable, deferred, and stubbed capabilities |
| `orchestrator.shutdown` | Request clean user-daemon shutdown under policy |

Settings, handlers, commands, plugin controls, and semantic facts are not
inferred from these calls. They enter later negotiation rounds.

### Client behavior

- File Manager normally uses Orchestrator for integration/policy and combined
  search.
- GUI.Forms remains in-process and is never exposed to Orchestrator.
- When Orchestrator is unavailable, File Manager may use the registered direct
  engine fallback and cached read-only declarations, clearly marked degraded.
- File Manager renders all Orchestrator service/settings/grant UI with house
  controls.
- Responses arrive through a pollable/queued client projection, never a callback
  on an uncontrolled Orchestrator thread.

### Required fake scenarios

1. Orchestrator ready, engine unavailable;
2. Orchestrator ready, engine degraded/stale;
3. Orchestrator absent with direct engine fallback;
4. incompatible contract major;
5. shutdown/restart invalidating subscriptions;
6. plugin and semantic systems reported stubbed rather than empty-success;
7. unknown future availability records ignored only when noncritical.

## File Manager reply 001

Status: **deferred by ADR-004**.

Frontend 001 does not consume a live Orchestrator contract. It models the
states below through a frontend-owned port and deterministic data marked
`simulated`. That fixture shape is not a counterproposal and does not freeze an
Orchestrator ABI. Reply 001 begins when the real adapter is scheduled.

Please provide:

- the smallest first-screen state that needs Orchestrator;
- preferred C ABI vs local-wire client projection for C++;
- UI-thread delivery and cancellation requirements;
- cached declaration lifetime and stale-display requirements;
- exact degraded-search behavior;
- additional state needed to render service controls without policy leakage.

## Orchestrator reconciliation 001

Status: **not started; waits for File Manager reply 001**.

## Architect sequencing correction 002

**GIVEN:** Frontend 001 does not bootstrap on a frontend-owned Orchestrator
fixture. Orchestrator advances independently to Core 1.0 and the product uses
that live authority from startup. Canonical fixture replay remains permitted
for deterministic frontend tests.

## File Manager reply 002

Status: **accepted planning reply; implementation evidence pending**.

- The smallest startup surface is release compatibility, lifecycle generation,
  immutable contract/availability snapshot, provider reasons, and registered
  direct-Engine fallback state.
- The authoritative cross-process projection should be a production local wire
  with a thin C++ client. A Rust/C++ in-process ABI is not requested.
- Delivery is pollable/queued and drained on the frontend UI thread. Deadlines
  and cancellation identities are explicit; arbitrary foreign callbacks are
  rejected.
- Cached declarations are scoped to daemon identity plus lifecycle/configuration
  generation. On disconnect they may remain visibly stale and read-only; they
  cannot authorize mutation or plugin execution.
- Degraded search uses the direct Engine route only when Orchestrator's last
  compatible snapshot registered it. When `ORC-ENG-004` is registered and
  available, catalogue failure may use its source-explicit reduced live search
  under Orchestrator's fallback law. Otherwise live folder navigation remains
  and search is provider-limited.
- Service controls require release identity, lifecycle state/generation,
  provider state/reason, compatibility state, restart/shutdown eligibility, and
  redacted diagnostics locators. Policy secrets and raw grants do not cross for
  display convenience.

## Orchestrator reconciliation 002

Status: **accepted semantic direction; production transport, authentication,
and hostile fixtures remain Core 1.0 work**.

These requirements enter the Core 1.0 profile for `ORC-COM-001`,
`ORC-LIF-001`, `ORC-FE-001`, and `ORC-CLI-001`. Settings, handlers, commands,
Engine queries, plugins, and semantic facts retain separate contract gates.

## Orchestrator implementation evidence after reconciliation 002

Status: **OBSERVED first-platform consumer; no new File Manager reply required**.

Orchestrator now carries a separately built C++17 conformance client under
`../../orchestrator/conformance/clients/cpp/`. It authenticates over the Unix local
wire, materializes the accepted startup fields, and reconnects across daemon
instance replacement without linking Rust or GUI.Forms. This exercises the
consumer side while Frontend 001 remains gated. Pollable delivery, stable client
ABI, full hostile fixtures, and Windows named pipes remain open before product
consumption.

## Orchestrator implementation reconciliation 003

Status: **OBSERVED complete source-client projection; installed launchd gate
pending**.

The sequential conformance reads exposed a future torn-snapshot risk and did
not materialize the complete contract catalogue, fallback eligibility, or
service-control state requested in reply 002. The additive
`orchestrator.frontend.bootstrap` / `ORC-FE-001` 1.0 operation now returns one
bounded immutable value containing:

- version, release/provenance, lifecycle and configuration generations;
- complete contract and availability catalogues;
- normal route, Engine scope, and registered-versus-eligible direct fallback;
- shutdown/restart eligibility and nullable redacted diagnostics locator.

The server hello's instance identity plus the two snapshot generations form the
frontend cache key. The separately built C++17 source client performs one
synchronous request, validates the schema/generation/route invariants, and
computes the strict Orchestrator-owned part of the Frontend 001 opening
predicate. It owns no thread or callback. File Manager therefore owns the I/O
worker and posts completed value snapshots to its GUI.Forms UI-thread queue,
exactly as reply 002 requested.

On disconnect, a prior compatible snapshot is display-only and visibly stale.
It cannot authorize mutation, plugin execution, or an Engine fallback that the
same snapshot did not register. Route registration and current eligibility are
separate fields so an installed-provider gap cannot become accidental access.

This closes the Orchestrator-side data-shape and delivery-model work for the
Frontend 001 bootstrap. The remaining Core release gate is a real installed
launchd activation, shutdown, reactivation, and removal measurement. A frozen
C++ binary ABI and Windows named pipes remain later platform/source-packaging
work; they are not required by the accepted macOS source-client projection.

## Orchestrator availability preparation 004

Status: **OBSERVED consumer gate projection implemented; Frontend 001 remains
closed**.

The raw capability catalogue's `required` field is a product/contract property,
not a statement that every unfinished provider blocks Frontend 001. To prevent
the future application from inventing that distinction, the atomic bootstrap
now includes `frontend_opening`:

- `orchestrator_gate` is the only locally decidable opening gate. It combines
  the Core release manifest with `frontend.bootstrap` availability and carries
  exact blockers. At this round it is `blocked` by `daemon.discovery` and the
  degraded bootstrap capability, both naming the still-open installed launchd
  evidence.
- `external_gates.gui_forms` mirrors the attributed
  `gui_forms.consumption_manifest` row and is currently `negotiating`. It does
  not manufacture a GUI.Forms go-ahead.
- `external_gates.architect_direction` is `not_reported` with nullable
  satisfaction. Orchestrator does not infer owner direction from tests or
  documents.
- policy states that product startup needs a live snapshot, stale state is
  display-only, and separately gated provider absence does not block the 001
  opening predicate.

The independent C++ source client materializes and cross-checks this projection
against the raw availability rows. Its former globally named readiness helper
is replaced by `orchestrator_gate_ready()`. The probe reports the three
attributed gate states and Orchestrator blocker count, giving the future
frontend a deterministic startup/preflight seam without granting permission to
create application source.

No File Manager application source, build files, or GUI.Forms consumption code
is opened by this round. Once installed launchd evidence makes the Orchestrator
gate ready, the remaining opening events are still the named GUI.Forms go-ahead
and explicit architect direction required by ADR-006.
