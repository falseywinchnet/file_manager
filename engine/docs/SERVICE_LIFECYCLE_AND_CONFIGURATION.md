# Engine service lifecycle and configuration

Status: **OBSERVED development lifecycle/configuration projection; GIVEN
systemwide service boundary; CANDIDATE native supervisors, authenticated IPC,
and background ingestion**.

## Service boundary

The service is the complete local catalogue, indexing, and retrieval engine.
Its organizing responsibilities are exact filesystem identity, authorized-root
projection, durable checked generations, lexical and metadata indexes,
bounded candidate channels, evidence-preserving ranking, query/inspection, and
background observation/reconciliation.

Content-fragment descriptors and similarity hashes, including a Kolmogrov
implementation where it wins its declared workloads, are inputs to bounded
candidate generation. They are not filesystem identity, stored-record
authority, a relevance model, or the lifecycle/storage spine. A candidate must
resolve back to an exact object, binding, generation, and source anchor before
it can contribute evidence. Failure or absence of a descriptor component must
leave exact catalogue recovery and exact query correct.

Content extraction is not admitted implicitly. A future intake contract must
name the provider, approved root, object identity, committed generation,
fragment boundary and anchor, content-policy decision, descriptor algorithm and
parameters, and rebuild compatibility. This keeps useful content/fragment
hashing available without giving a hash authority it does not possess.

## Process lifecycle

The lifecycle belongs to one process instance:

| State | Entry and behavior |
|---|---|
| `starting` | **CANDIDATE:** supervisor has constructed the process; schema, manifests, roots, and last checked generation are being recovered. No readiness claim yet. |
| `ready` | **OBSERVED:** checked startup completed. New bounded query and administrative projection work may be admitted. |
| `draining` | **OBSERVED:** shutdown rejects new work, cancels service-owned operation contexts, and waits for bounded cleanup. Existing exact status remains inspectable. |
| `stopped` | **OBSERVED:** active operations drained and the current exact reader closed. Shutdown is idempotent and does not write a heartbeat or clean-stop record. |
| `faulted` | **OBSERVED terminal state shape:** shutdown could not close a resource cleanly. Production recovery/reporting policy remains open. |

`start` is process construction plus checked recovery, not a request sent to an
already running engine. `stop` is the engine's draining `Shutdown` operation.
`restart` is supervisor-owned stop/process replacement/start. It creates a new
cryptographically random `instance_id`; committed catalogue and enacted
configuration identity survive when their checked generation survives. There
is no in-process pseudo-restart that might retain hidden state.

The current constructor reaches `ready` synchronously or returns an error.
Exposing observable `starting` progress requires the production supervisor and
endpoint contract; the enum is reserved but the implementation does not
fabricate an interval it cannot report.

## Configuration ownership and application

Configuration is split by authority:

- installation/supervisor configuration selects the store, endpoint,
  deployment identity, resource ceilings, and platform adapter;
- Orchestrator/user policy authorizes roots and content/provider policy;
- the engine validates and projects already-authorized policy;
- experiment parameters remain candidates until an accepted decision record
  admits them.

`engine.configuration_get` reports only enacted facts. Its SHA-256 digest is a
stable compare-and-apply identity over schema, deployment mode, persistence,
store placement, ingestion mode, and canonical root policy. Dynamic health
such as whether the policy has reached a checked durable generation is reported
separately as `root_policy_persistent` and is deliberately excluded from the
digest.

`engine.root_plan` returns the current digest, proposed digest, canonical roots,
and whether the proposal changes configuration. Canonical
`engine.root_apply` requires `expected_configuration_digest`; a controller
planned against older state receives `STALE_CONFIGURATION` and must re-plan.
The unqualified `root.apply` alias remains a development fixture and does not
define the cross-project safety contract.

**OBSERVED limitation:** root policy is process memory until persistent
reconciliation commits it with a generation. A crash before that commit loses
the uncommitted projection, so production root apply acknowledgement and the
manifest's root-policy revision remain unresolved together. The digest is
content identity, not a substitute for the eventual authoritative revision.

## Work and background state

`engine.status` reports lifecycle, active request counts, current
administrative phase, effective configuration, roots/generation/integrity, and
the enumerated capability ledger. Default ingestion remains
`manual_reconcile`, for which `background_ingestion` and `backlog_known` are
false. The opt-in development macOS path reports
`native_adapter_experimental`, explicit currentness state, pending count/age,
gap, adapter source/epoch, observed/reconciled positions, and
`watermark_durable=false`. It never reports a zero OS-event backlog after the
adapter stops.

**OBSERVED experimental implementation:** native observations are coalesced in bounded volatile memory;
a quiet engine performs no periodic catalogue write, query telemetry write,
heartbeat write, or durable status-event append. Observation watermark and
root policy revision still must commit atomically with component digests in the
generation manifest. That commit is not implemented, so every adapter start
forces a baseline scan and may claim only process-local `current_volatile`.
Gaps, native overflow, cursor/epoch discontinuity, root invalidation, and
bounded-memory overflow force authoritative reconciliation.

Maximum age, operation count, byte budget, run count, and compaction thresholds
are workload experiment points. Foreground query never triggers compaction and
waits only for a short manifest publication exclusion.

## Capability inventory

The runtime capability list is deliberately broader than implemented method
names so consumers can distinguish absent work from a temporarily unavailable
feature:

| Capability group | Current state |
|---|---|
| exact catalogue/query, ordered metadata, root planning, manual scan | **OBSERVED available** |
| immutable checked generation | **OBSERVED available only in persistent mode** |
| delta publication | **OBSERVED experimental component with isolated atomic manifest/recovery; not in live manifest/service** |
| lexical dictionary/postings | **unavailable; M3** |
| anchored content-feature intake and fragment descriptors | **deferred pending authority/privacy/anchor contract** |
| fixed-width similarity candidate channel | **OBSERVED experimental and disabled from the public planner** |
| native observation, watermarks, coalescing | **OBSERVED experimental portable core plus macOS FSEvents; watermark not durable** |
| bounded status subscription | **negotiating** |
| authenticated framed local transport | **deferred** |

The exact catalogue stays authoritative across every row. A descriptor channel
may be rebuilt, disabled, upgraded, or rejected independently.

## Native service adapters

These are **CANDIDATE** adapters, not implemented install promises:

| OS | Supervisor/start-stop candidate | Endpoint candidate |
|---|---|---|
| macOS | `launchd` system daemon; `launchctl` installs/starts/stops/restarts | launchd/root-owned Unix-domain query and admin sockets |
| Windows | Service Control Manager service; SCM start/stop/restart with bounded stop deadline | ACL-separated named pipes and peer-token verification |
| Linux | `systemd` system service/socket unit; `systemctl` lifecycle | owner/mode-separated `AF_UNIX` sockets plus peer credentials |

Native adapters must test install, upgrade, disabled boot, crash-loop limits,
drain deadlines, endpoint replacement, stale clients, and clean uninstall.
Wine and Lima are compatibility oracles only; native APFS, NTFS, and ext4
campaigns remain promotion gates.

## Verification gates

- restart changes `instance_id` while retaining the last checked generation and
  effective-configuration digest;
- shutdown cancels/drains active work, rejects new work, and writes no durable
  clean-stop marker;
- stale configuration apply cannot mutate roots;
- process kill at every publish boundary recovers exactly old or new checked
  state, never a mixture;
- idle, no-change reconciliation, and status subscription produce zero durable
  catalogue writes;
- query and admin endpoints cannot exchange authority;
- fragment/similarity component loss does not alter exact recovery or identity.

Implementation locators: `api/service.go`, `api/types.go`,
`internal/service/lifecycle.go`, `internal/service/service.go`, and
`internal/transport/jsonl.go`. Tests are in
`internal/service/lifecycle_test.go` and `internal/transport/jsonl_test.go`.
