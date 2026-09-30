# Windows Engine named-pipe integration

Status: **OBSERVED user-process integration; native temporary-fixture checks
MEASURED on Shadow, 2026-09-29 local / 2026-09-30 UTC**. The owner directed
actual Windows search integration. Engine reply 008 in
`ORCHESTRATOR_INTERFACE_NEGOTIATION.md` records the Windows projection agreed
with the Orchestrator integration chat. This is not SCM installation or full
Windows platform promotion.

## Start an explicitly authorized root

Build from `engine/` with Go 1.24 or newer. Pinned vendored Windows dependencies
permit an offline build:

```powershell
go build -p 2 -trimpath -o .build/fileman-engine.exe ./cmd/fileman-engine
```

Create an existing parent directory for Engine-owned state outside the source
root. The following example uses variables for explicit caller-selected paths;
it does not choose or scan personal folders. Source must already exist.

```powershell
$engine = 'C:\absolute\path\fileman-engine.exe'
$source = 'C:\explicitly-approved\source'
$stateParent = 'C:\explicitly-selected\engine-state'
$manifest = Join-Path $stateParent 'policy\manifest.json'
$runtime = Join-Path $stateParent 'runtime'

& $engine create-windows-manifest `
  --deployment-id desktop-search --root-id docs --root-path $source `
  --runtime-dir $runtime --output $manifest

& $engine serve-windows --manifest $manifest
```

The `policy` and `runtime` leaves are created with current-user-only protected
ACLs. Existing broad/inherited ACL directories are rejected, not modified.
Parent directories must exist. Runtime and manifest paths must be absolute and
cannot traverse reparse points. State cannot be created inside the source root.
The manifest binds the Windows host MachineGuid, user SID, root ID, canonical
path and exact filesystem object identity. No default root is admitted.

Default startup is **live-only**: it applies root policy without scanning or
creating a persistent catalogue. `engine.query_live` performs bounded on-demand
name/path traversal. Query calls cannot expand the manifest's root authority.

Persistent indexing requires both `--index-enabled` and `--store-root` when
creating the manifest, with a separate private state leaf outside the source.
The first profile admits exactly one persistent root. On every indexed startup,
Engine reconciles before endpoint discovery is published. This makes changed
exclusions effective before recovered results become queryable. Startup remains
bounded by a five-minute reconciliation context; blocked native I/O remains a
separate cancellation gate. No Windows admission-marker fast path is claimed.
An explicit `--exclude private,cache` narrows scope. Background watchers are not
automatically started by this profile, so indexed results retain manual-reconcile
currentness. Root changes require a new manifest and process restart.

## Query and stop

Use another terminal, or have Orchestrator launch and supervise the separate
process without a visible console. The process is not registered with SCM.

```powershell
& $engine call-local --runtime-dir $runtime --request `
  '{"id":"live","method":"engine.query_live","params":{"query_id":"one","scope":{"root_id":"docs","descendants":true},"text":"needle"}}'

& $engine call-local --runtime-dir $runtime --authority admin --request `
  '{"id":"stop","method":"engine.shutdown"}'
```

Orchestrator consumes `<runtime>\discovery.json`; the Go client is a diagnostic
and conformance tool. See Engine reply 008 for exact fields. Query/admin named
pipes use existing ENG1 framing and hello. Tokens are distinct 256-bit hex
credentials, rotated each start and removed at graceful exit. Discovery names
SID, PID, instance identity, both pipe names and token paths. It never includes
the credentials themselves. A query credential cannot authenticate on admin,
and query dispatch cannot shut down, reconcile or configure roots.

The Windows account is the trust boundary. The runtime directory/files are
owner/SID/DACL checked. Pipes allow only the current SID and reject remote
clients. Before sending credentials, the client validates actual server PID and
process-token SID and opens with identification-only SQOS. The server checks
actual client process-token SID. Runtime and store locks prevent concurrent
writers using the same state paths. Bounded shared dispatch, read/write deadlines,
context cancellation, and draining connection closure remain in force.

## Dependency decision and evidence

**CANDIDATE compared:** handwritten synchronous or overlapped Win32 pipe adapter
versus a pinned Microsoft go-winio adapter over the existing `net.Conn` boundary.
The former would require independently implementing IOCP, handle-close races and
deadlines. **Selected for this implementation slice:** go-winio v0.6.2 and
x/sys v0.30.0, pinned in go.mod/go.sum and vendored with upstream licenses
(64 files, approximately 1.7 MB). They introduce no external runtime service or
DLL and do not change the Engine wire or semantic contracts. x/sys v0.30.0
retains the project's Go 1.24 minimum.

**OBSERVED upstream mechanisms:** `vendor/github.com/Microsoft/go-winio/pipe.go`
uses `FILE_PIPE_REJECT_REMOTE_CLIENTS` and exclusive `FILE_CREATE` for the initial
listener. Engine supplies the explicit current-SID DACL. Library calls are
confined to the Windows transport/security adapter. Reversal: replace that
adapter while retaining `net.Conn`, ENG1, manifest and semantic tests; no index
format change is required.

**MEASURED final native validation:**

- `go build -p 2 ./...`, `go vet -p 2 ./...`: pass.
- `go test -p 2 -count=1 -json ./...`: 173 test/subtest passes, 20 existing skips.
- `go test -race -p 2 -count=1 -json ./...`: pass, no reported races.
- New tests cover owner-only state round-trip, broad ACL rejection, bounded
  reads, exclusive lock/pinned-directory behavior, real pipe live query without
  catalogue, wrong server PID, query/admin credential separation, query admin
  denial, idle-client shutdown, cancelled dial, live/indexed startup and restart,
  changed exclusions, wrong host/SID, root replacement, and source-state refusal.
- A separately launched native executable created a manifest for a generated
  temporary source, served live `needle.txt` with `generation=0` and
  `source=live_filesystem`, then shut down through its admin endpoint with exit 0.
- Linux amd64 cross-build and Darwin arm64 command-test compile pass; neither
  establishes native execution or macOS regression completion.

Raw ignored receipts: `.build/windows-validation/windows-pipes/`,
`pipe-process-live.json`, `pipe-process-stop.json`, `pipe-discovery.json`,
`pipe-process.stderr`, and the Linux/Darwin cross logs. Tests ran Go 1.27.1 on
Windows 11/NTFS, GOMAXPROCS=2, `-p 2`, shared MinGW GCC for race checks. No real
personal files were scanned and no services or OS security settings were changed.
The generated manual-smoke fixture path is recorded in `pipe-fixture-root.txt`.

## Remaining gates

The Orchestrator independent-client/end-to-end application route must pass its
own conformance checks; Go self-client success alone does not establish that
route. Windows reparse-point file identity, full NTFS oracle, exact-current
watcher coverage and durable watermarks remain open. So do broad root/resource
campaigns, blocked native I/O, cross-user/remote-host adversarial campaigns,
service installation/update/uninstall, signing and release packaging. No lexical
or fuzzy search capability is added. No automatic root consent or index creation
is inferred from launching File Manager.
