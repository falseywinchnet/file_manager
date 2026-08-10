# M4 LaunchAgent dogfood 001

Status: **MEASURED pass for ADR-015's named M4 deployment**.

Date: 2026-08-10.

## Environment and scope

- Host: Apple-silicon M4 Mac mini reached by `m4mini-awdl`
- OS: macOS 26.5 (build 25F71), arm64
- Service uid: 501 (`joshuahkuttenkuler`)
- Host UUID binding: `4FEB3A7D-D020-3910-B7AF-232C78D32651`
- Approved root: `/Users/joshuahkuttenkuler/Developer/CodexBuilds/file_manager-2d80cdb86b7d`
- Deployment: `m4-dogfood`; per-user LaunchAgent
  `com.filemanager.engine.m4-dogfood`
- Ingestion: manual authoritative reconciliation; FSEvents was not enabled
- Store/runtime: private Application Support state outside the approved root

The Neo checkout remained authoritative. `m4build` synchronized it before all
M4 builds and trials. No `sudo`, root daemon, whole-home admission, or remote
source edit was used.

## Build and automated controls

The M4 passed:

```sh
go test ./...
go test -race ./...
go vet ./...
CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build ./...
CGO_ENABLED=0 GOOS=windows GOARCH=amd64 go build ./...
python3 -m unittest discover -s tools -p 'test_*.py'
python3 tools/generate_library_docs.py --check
```

The race build emitted existing Darwin linker `LC_DYSYMTAB` warnings but all
tests passed. Automated negative cases covered loose-mode and symlink
manifests, wrong host/root identity, root-policy widening, excluded subtrees,
bad frame magic/size, and query-to-admin method denial.

## Installed lifecycle results

1. The installer built the M4 binary, emitted a host-bound `0600` manifest,
   generated and linted the LaunchAgent plist, bootstrapped it, and completed an
   authenticated query-endpoint status call.
2. Initial status reported `sandboxed=false`, `deployment=m4-dogfood`, the one
   exact approved root, a store outside that root, and not-ready because no
   generation had yet been published.
3. The first admin reconciliation completed in 20.090 seconds, published
   generation 1, and recorded 28,542 bindings.
4. An exact query for `M4_DOGFOOD_DEPLOYMENT.md` returned the expected source
   path and exact Darwin object/incarnation evidence from generation 1.
5. Integrity reported generation 1 healthy and repairable. Status reported
   ready, non-stale, and `root_policy_persistent=true`.
6. `engine.shutdown` drained the current instance. Launchd `KeepAlive` started
   a new instance, the query credential checksum changed, and generation 1 was
   recovered ready with the same 28,542 records.
7. `launchctl bootout` removed the job and an authenticated status call failed.
   Bootstrap/kickstart restored a new instance and generation 1.
8. `launchctl kill SIGKILL` produced another distinct instance and recovered
   generation 1 ready, demonstrating crash supervision rather than only a
   graceful-process path.
9. After the hardened no-follow manifest loader, corrected installer readiness
   gate, documentation, and evidence record were mirrored, a final
   authoritative reconciliation published generation 2 with 28,552 bindings.
10. The concurrency/admission hardening upgrade introduced a generation-bound
    `ADMISSION` record. Its first startup reconciled before endpoint publication,
    produced generation 4 with 30,632 bindings, and returned healthy integrity.
    A launchd restart rotated instance/credential identity and recovered
    generation 4 without a redundant publication.
11. The installed publication-acknowledgement hook advanced `ADMISSION` with a
    final manual reconciliation to generation 5 before returning success. The
    next launchd restart recovered generation 5 on the first readiness poll and
    did not rescan or create generation 6.

Observed instance ids included
`1aa6cfe81124c98f33f0dc0726d58294`,
`9c621fff425b2748d5de5e2982bd69d3`,
`5595e89f0f4ca4a822e23f5f4cb9a589`,
`16702dc4932a404ce814392d37245fba`, and
`f42c8c4f34cfb637deaa53d371c3d48c`.
The hardening upgrade additionally observed
`98603bd809262ce0f12c6ecbab8f59b2` and
`61ca6c10714a2e7452904ceb8ed354dd`.

Final filesystem protections were observed as:

- deployment leaf: same uid, `0700`;
- deployment manifest: same uid, regular, `0600`;
- query/admin sockets: same uid, socket, `0600`;
- query/admin tokens and discovery: same uid, regular, `0600`.
- admission record: same uid, regular, `0600`, finally bound to generation 5.

## Retained negative result

The first credential-rotation evidence command used a malformed nested `awk`
expression. It did restart the service but could not compare checksums. The
trial was not counted. A second trial used `cksum` without nested interpolation,
proved the credential changed, and captured distinct pre/post instance ids.

The first in-place reinstall waited only for the query socket node. A stale
socket from the replaced process satisfied that check before the new listener
accepted, and the immediate authenticated status call received connection
refused. The service recovered under launchd, but that installer run was not
counted. The readiness gate now retries a complete authenticated status call;
the corrected reinstall passed.

## Scope of inference

This closes installed launchd supervision, authenticated framed local
transport, manifest-bound non-sandbox root admission, restart recovery, and
reversal for the named M4 dogfood deployment. It does not establish general
macOS system-daemon packaging, update/rollback packaging, multi-user policy,
native NTFS/ext4 behavior, durable background currentness, lexical usefulness,
or release readiness. Manual reconciliation remains an explicit operational
limitation.
