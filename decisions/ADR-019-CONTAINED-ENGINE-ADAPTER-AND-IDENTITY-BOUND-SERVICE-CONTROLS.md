# ADR-019: contained Engine adapter and identity-bound service controls

Status: **accepted for File Manager 1.0 contained M4 dogfood**.

Date: 2026-08-10.

Owner approval: the grand architect directed File Manager implementation and
M4 dogfood, authorized the necessary adjacent repository work, and stated that
the blocking gates were cleared. This decision is limited to the contained
File Manager profile and does not promote every Engine platform adapter.

## Question

How should the File Manager frontend reach the installed Go Engine and present
bounded Orchestrator/Engine administration without embedding policy, replaying
non-idempotent operations, or confusing stale service state with current
authority?

## GIVEN constraints

- File Manager normally reaches Engine through Orchestrator. Direct Engine
  fallback remains separately registered and gated.
- Engine owns its query/admin authority split, host-bound root manifest,
  durable catalogue and local `ENG1` transport. Orchestrator may adapt those
  contracts but may not bypass them.
- File Manager owns GUI composition. Orchestrator may return service facts and
  a closed command allowlist, never arbitrary controls or layout.
- File operations remain authoritative at the filesystem. Engine results are
  candidates that must be re-contained and re-observed before presentation or
  mutation.
- Restart, rebuild and shutdown are not safe to infer or replay from a stale
  snapshot.
- The source checkout on Neo remains authoritative; M4 products and writable
  profiles live outside the mirror under the user-scoped installation and
  `CodexRuns` locations.

## Candidates

### A. Spawn a development Engine child per frontend or Orchestrator process

This reuses the JSONL development adapter but duplicates catalogue lifetime,
does not exercise the installed query/admin authority split, and makes GUI
restart semantics own provider lifetime accidentally.

### B. Let File Manager connect directly to Engine for normal search and admin

This shortens one path but duplicates discovery, authentication, fallback and
administrative policy in the C++ application. It also violates the normal
Orchestrator integration route.

### C. Install a contained host-bound Engine and adapt it through Orchestrator

Engine runs as a distinct user LaunchAgent with one approved root, durable
store, short private runtime path, separate query/admin sockets and credentials.
Orchestrator validates the installed discovery record, same-user private
objects, peer UID, protocol and instance correlation for each call. Query calls
may retry once after rediscovery because they are idempotent. Administrative
calls are never automatically replayed. File Manager consumes typed C++
`ORC-FE-001`, `ORC-SET-001`, and `ORC-UI-001` values on its own worker.

## Measurements and observations

- **OBSERVED:** the first runtime directory beneath the long Application
  Support deployment leaf failed Darwin Unix-socket bind with `EINVAL`. Moving
  only the ephemeral endpoint directory to the private short cache path
  `~/Library/Caches/com.filemanager.engine.fm1` made both `ENG1` sockets usable;
  the durable store and manifest remained under Application Support.
- **OBSERVED:** the first replacement Go binary passed its full test suite but
  launchd rejected it with `OS_REASON_CODESIGNING`. Applying a verified ad-hoc
  local signature restored the contained LaunchAgent. This is retained as a
  packaging failure, not erased from the evidence.
- **MEASURED:** the M4 Engine suite passed across every Go package after the
  host-bound launchd capability fix. Orchestrator library tests passed 65/65,
  frontend focused tests passed 3/3, and the independent C++ client parsed two
  installed services and returned two exact `read-me.txt` catalogue matches.
- **MEASURED:** Engine restart rotated instance
  `b62ab5eef0961834bf2495bf6c5a344e` to
  `df285fb30ca0919991d8b07e3f636125`; a command carrying the old identity was
  rejected as stale. Orchestrator restart rotated
  `729510520d3f613ed6b7e0f2822ff6de` to
  `11233f99aecf5678e89c269ddf8f5123`; replay with the old identity and the
  coincidentally equal generation was also rejected.

The named run is recorded in
`orchestrator/conformance/evidence/M4_FILE_MANAGER_SERVICES_2026-08-10.md`.

## Decision

Choose C for the contained File Manager 1.0 profile.

The Engine deployment ID and approved root ID are `fm1-contained`; the root is
the explicit `CodexRuns/fmsandbox` tree and excludes `.quarantine`. The durable
store is outside the indexed root. The short cache directory contains only
recreatable discovery, socket and credential objects.

`ORC-UI-001` 1.0 has two operations:

- `orchestrator.services.snapshot` returns an immutable, bounded list of
  service identity, lifecycle state, readiness, generation, transport,
  currentness, admitted roots and closed command descriptors;
- `orchestrator.services.command` accepts exactly one listed command and binds
  it to the observed service identity. Orchestrator commands require both ORC1
  process `instance_id` and lifecycle generation. Engine commands require the
  current Engine instance ID; reconcile/rebuild additionally require an
  admitted root ID.

The admitted first command set is Orchestrator restart/shutdown and Engine
integrity check/reconcile/rebuild/restart. File Manager confirms rebuild,
restart and shutdown, disables unavailable commands, and refreshes facts after
terminal completion. A stale or unavailable state authorizes nothing.

## Failure modes

- Missing, non-private, wrong-owner, symbolic-link or inconsistent Engine
  discovery objects make the adapter unavailable.
- Query transport failure permits one rediscovery/retry of the same correlated
  idempotent request. Remote Engine faults and all admin calls are returned
  without retry.
- A changed Engine or Orchestrator process identity returns `stale` before the
  requested effect.
- A root command without the currently reported admitted root is invalid.
- If Engine is absent, Orchestrator and direct filesystem navigation remain
  available while service/search state is explicitly unavailable.
- If launchd rejects an unsigned replacement, retain the failure, restore or
  sign the exact local artifact, verify its signature, and restart only the
  affected label.

## Rejected options

- Reject A as the product route; retain it only for bounded development and
  hostile conformance.
- Reject B as the normal route because it moves authentication and policy into
  the frontend. The previously registered direct fallback remains a degraded
  read path only when a compatible Orchestrator snapshot declares it eligible.
- Reject deployment-name special cases for capability truth. Host-bound
  launchd construction, not the literal `m4-dogfood` string, now supplies the
  Engine supervisor projection.
- Reject generation-only Orchestrator commands because lifecycle generation
  restarts at one in a new process and therefore cannot identify a daemon by
  itself.

## Reversal path

Boot out `com.filemanager.engine.fm1-contained`, restore the hash-named Engine
binary backup and remove only the contained manifest/runtime/store after an
explicit data-retention choice. Restore the hash-named Orchestrator binary and
its existing LaunchAgent without changing the settings store. The C++ client
and frontend can retain typed unavailable service rows while `ORC-UI-001` is
disabled. A later event-stream or platform transport may replace polling only
after preserving the same identity/currentness rules.

## Unresolved edges

- Windows installed named-pipe and Linux installed transport evidence;
- signed distribution/notarization beyond the local ad-hoc M4 artifact;
- bounded service progress/event replay and resynchronization;
- durable general administrative audit records;
- daily-root promotion and a packaged uninstall/data-retention UI.
