# M4 File Manager services and installed Engine evidence — 2026-08-10

Status: **MEASURED contained deployment, adapter, search and service-control
pass; native visual walkthrough remains a separate frontend result**.

## Environment and artifacts

- host: nearby M4 Mac mini, macOS arm64, user-scoped LaunchAgents;
- contained root: `/Users/joshuahkuttenkuler/Developer/CodexRuns/fmsandbox`;
- Engine label/deployment/root: `com.filemanager.engine.fm1-contained` /
  `fm1-contained` / `fm1-contained`;
- Engine durable leaf:
  `~/Library/Application Support/FileManager/Engine/fm1-contained`;
- Engine recreatable runtime: `~/Library/Caches/com.filemanager.engine.fm1`;
- signed installed Engine SHA-256:
  `7c3a99ef52f1b3bbdf865c96387f53938505b33c17897cdc590120245c4e32b3`;
- installed Orchestrator SHA-256:
  `d8b95297c73050519cd4726bf7025d33e76b9aa3712caf822f2ddbc8dd689f99`;
- Orchestrator settings leaf:
  `~/Library/Application Support/com.filemanager.orchestrator-settings`.

The Engine manifest is host/UID/object-identity bound, excludes `.quarantine`,
keeps its store outside the root, and publishes separate `0600` query/admin
sockets and tokens inside a `0700` directory. Orchestrator uses its existing
launchd-adopted ORC1 endpoint and settings directory.

## Positive observations

1. `go test ./...` passed for all Engine packages after replacing the literal
   `m4-dogfood` supervisor-capability check with the explicit host-bound
   launchd construction path.
2. Orchestrator library tests passed 65/65, including instance+generation
   service-command binding and settings-primary symlink refusal.
3. Frontend model/operation/internal-drag tests passed 3/3 after the typed
   Services page was linked and installed.
4. `orchestrator.services.snapshot` returned exactly two services. Engine was
   `ready`, generation 1, `manual_reconcile`, transport `engine.local.v1`, root
   `fm1-contained`; Orchestrator was `ready` on `orchestrator.local`.
5. The independent C++17 probe parsed 25 contracts, 28 capabilities, settings
   revision 0, two services, and Engine `ready/manual_reconcile` without
   linking Rust.
6. Engine integrity check and reconcile succeeded through
   `orchestrator.services.command`. Exact catalogue search for `read-me.txt`
   then returned two results.
7. Engine restart succeeded, launchd reactivated it, and its instance changed
   from `b62ab5eef0961834bf2495bf6c5a344e` to
   `df285fb30ca0919991d8b07e3f636125`. An admin command carrying the first ID
   was rejected as stale.
8. Orchestrator restart succeeded through its own service command and ORC1
   reactivation. Its instance changed from
   `729510520d3f613ed6b7e0f2822ff6de` to
   `11233f99aecf5678e89c269ddf8f5123`. Replaying the old instance with the new
   process's equal generation 1 was rejected as stale.
9. The newly constructed contained Engine reports
   `engine.supervisor.launchd=available`, revision `host-bound-v1`; it does not
   rely on a deployment-name exception.

## Retained negative results

### Unix socket path length

The first contained runtime used
`~/Library/Application Support/FileManager/Engine/fm1-contained/runtime`.
Darwin rejected the resulting query socket with `bind: invalid argument` and
launchd restarted the failed process repeatedly. The deployment was booted out,
the manifest was regenerated with the short private cache runtime, and the
durable store/manifest remained at the original Application Support leaf.

### Replacement code signing

The first newly built Engine replacement passed tests but was installed before
receiving a local signature. launchd recorded `OS_REASON_CODESIGNING`, the
contained label entered `spawn scheduled`, and a direct invocation was killed
with signal 9. The old separate `m4-dogfood` process remained alive. The exact
new binary was ad-hoc signed with `codesign --sign -`, verified on disk, and
only `com.filemanager.engine.fm1-contained` was kicked. It then reached
`state = running` and served authenticated version/status calls. Distribution
signing and notarization remain open; this result is only the local M4 profile.

## Rollback evidence

Hash-named backups remain beside the installed binaries:

- Engine: `fileman-engine.previous-403fd7cd-fm1-launchd-capability`;
- Orchestrator: `orchestrator.previous-b03c1bb8-instance-gap` and earlier
  staged rollback copies.

The contained Engine has an independent label, manifest, runtime and durable
store. It can be booted out without stopping the older `m4-dogfood` instance or
deleting the protected dogfood filesystem tree.
