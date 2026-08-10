# M4-only installed dogfood deployment

Status: **DECIDED by ADR-015; installed M4 evidence passed on 2026-08-10**.

This projection runs the Engine as the logged-in M4 user's native LaunchAgent.
It is deliberately not a root LaunchDaemon. It removes the disposable sandbox
envelope only for exact roots in a host-bound deployment manifest; it does not
make arbitrary filesystem paths admissible.

## Authority objects

| Object | Required protection | Purpose |
|---|---|---|
| `deployment.json` | same uid, regular file, `0600` | host UUID, uid, deployment id, store/runtime paths, exact approved root identities and exclusions |
| Application Support deployment leaf | same uid, `0700` | durable store, runtime, and logs outside indexed source |
| `query.sock`, `admin.sock` | Unix sockets, `0600` | separated local authorities |
| `query.token`, `admin.token` | `0600`, replaced each process start | independent 256-bit session credentials |
| `discovery.json` | `0600`, replaced each process start | instance identity and endpoint paths |
| store `ADMISSION` | same uid, regular file, `0600` | binds the checked generation to the exact manifest root identities and exclusions that produced it |
| LaunchAgent plist | user LaunchAgents directory, `0644` | `RunAtLoad`/`KeepAlive` process supervision |

Both endpoints validate the Unix peer uid before parsing a credential. Local
messages use an eight-byte `ENG1` header containing a big-endian bounded JSON
length. The current maximum is 1 MiB. Query/admin connection counts are capped
at 32/4; authentication, idle-frame, request, and write deadlines prevent an
accepted connection from holding an unbounded process resource. Shutdown closes
accepted sockets before waiting for their goroutines. The query endpoint admits
version/status/configuration, query, live-query, and inspect. The admin endpoint
admits version/status/configuration, root plan/apply, reconcile, rebuild,
integrity, and shutdown.

The service opens and identifies the approved root before scanning or starting
a live traversal. Path replacement after manifest validation therefore fails
closed instead of scanning the object that happened to appear at the same
address. On startup, the `ADMISSION` record must name both the recovered
generation and the digest of the current host/root/exclusion authority. A
missing or mismatched record forces authoritative reconciliation before either
socket is published. This is a policy-generation binding, not a clean-shutdown
marker. A successful installed reconcile or rebuild advances the record before
its response is acknowledged, so the next restart can recover that generation
without manufacturing another source scan.

## M4 installation

From the authoritative Neo checkout, mirror and execute the installer on the
M4 against the deterministic mirrored repository root:

```sh
/Users/ultimussecundai/.local/bin/m4build -- \
  /bin/sh engine/tools/install_macos_m4_dogfood.sh \
  /Users/joshuahkuttenkuler/Developer/CodexBuilds/file_manager-2d80cdb86b7d
```

The installer builds with the M4 Go toolchain, installs the binary, creates and
validates the host-bound manifest, lints the plist, bootstraps the exact
LaunchAgent, waits for the query socket, and performs an authenticated status
call. An upgrade whose admission policy is not yet bound may spend the readiness
window doing the mandatory startup reconciliation. It never requests `sudo`.

Useful authenticated calls on the M4 are:

```sh
RUNTIME="$HOME/Library/Application Support/FileManager/Engine/m4-dogfood/runtime"
"$HOME/.local/bin/fileman-engine" call-local --runtime-dir "$RUNTIME" \
  --authority admin \
  --request '{"id":"reconcile","method":"engine.scan_reconcile","params":{"root":"m4-file-manager-source"}}'
"$HOME/.local/bin/fileman-engine" call-local --runtime-dir "$RUNTIME" \
  --authority query \
  --request '{"id":"status","method":"engine.status"}'
```

Manual reconciliation is truthful policy for this deployment. Native
FSEvents remains coverage-incomplete and is not silently enabled by launchd.

## Stop and reversal

`engine.shutdown` drains the current process; `KeepAlive` intentionally starts
a new instance with new endpoint credentials. To stop supervision and remove
volatile endpoints:

```sh
/Users/ultimussecundai/.local/bin/m4build -- \
  /bin/sh engine/tools/uninstall_macos_m4_dogfood.sh
```

The uninstaller preserves the deployment manifest, checked generations, logs,
and shared binary. This retains recovery and negative-result evidence. Their
removal is a separate explicit purge, not part of ordinary bootout.
