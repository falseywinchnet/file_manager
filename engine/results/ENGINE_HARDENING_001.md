# Engine concurrency and admission hardening 001

Status: **OBSERVED defects corrected; MEASURED automated and M4 installed
upgrade controls passed**.

Date: 2026-08-10.

## Review scope

The pass followed the active Orchestrator-to-Engine request boundary through
`cmd/fileman-engine`, `internal/transport`, service lifecycle and reconciliation,
installed root admission, live traversal, and durable recovery. It deliberately
did not change the Orchestrator contract, ranking model, generation format, or
unadmitted observation/storage candidates.

## Corrected findings

1. **OBSERVED resource race:** local endpoints spawned one goroutine for every
   accepted socket. An authenticated or idle client could retain a handler
   indefinitely, and shutdown waited without first closing accepted sockets.
   Query/admin connections are now separately capped at 32/4, carry bounded
   handshake, idle-read, request, and write time, and are closed before handler
   drain.
2. **OBSERVED credential-availability bug:** a second process attempted token
   rotation before proving it could own both endpoints. Endpoint acquisition is
   now completed first; a duplicate process cannot replace a live listener or
   rotate its credentials.
3. **OBSERVED framing bug:** frame output assumed one `Write` consumed the whole
   header/payload. The writer now completes short writes or returns
   `io.ErrShortWrite`; a one-byte writer is a regression control.
4. **OBSERVED authority/recovery bug:** the host manifest's exclusions were not
   bound to the recovered generation. Tightening an exclusion could therefore
   reopen an older checked generation containing now-excluded records. A private,
   atomic `ADMISSION` record binds the exact manifest authority digest to one
   generation. Missing or mismatched admission causes authoritative
   reconciliation before endpoints open.
5. **OBSERVED root check/open race:** installed startup validated the directory
   path's identity, but a later scan or live query opened the path again without
   comparing that actual handle. Installed traversal now compares identity from
   the already-opened `os.Root`; replacement at the same address fails closed.
6. **OBSERVED live-query exclusion bug:** an installed live query could start
   directly inside a manifest-excluded subtree. The service now rejects the
   starting scope before constructing a live session.
7. **OBSERVED parsing and durability weaknesses:** `call-local` accepted trailing
   JSON; token reads accepted insufficiently validated files; private writes did
   not consistently complete temporary-file sync/rename discipline. These paths
   now reject trailing data, require an exact regular `0600` 256-bit hexadecimal
   token, and use synced atomic replacement. Manifest and admission replacement
   also sync the parent directory.
8. **OBSERVED avoidable synchronization indirection:** lifecycle mutexes were
   heap pointers. They are now ordinary owned `sync.Mutex` values behind one
   lifecycle pointer, removing needless allocation and accidental copy pressure.
9. **OBSERVED test race:** the event-storm control stopped waiting when the
   volatile watermark advanced without including the generation transition in
   its predicate, then asserted the separately sampled generation immediately.
   The wait now names both required postconditions; a fast scheduler can no
   longer turn its observation window into a false failure.
10. **OBSERVED hidden catch:** the FSEvents C callback recovered every panic to
    tolerate a late callback after `cgo.Handle` deletion. Recovery is now
    isolated to that one handle lookup. Expected teardown remains contained;
    unrelated callback defects are no longer silently converted into dropped
    work.
11. **OBSERVED acknowledgment gap:** startup wrote the policy-generation marker,
    but a later installed reconcile/rebuild could advance the catalogue without
    advancing the marker. Successful local administrative publication now
    commits admission before acknowledging the result; a marker failure becomes
    a retryable redacted fault instead of a silently stale recovery boundary.

## Automated evidence

The M4 arm64 mirror passed:

```sh
go test ./...
go test -race ./...
go vet ./...
CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build ./...
CGO_ENABLED=0 GOOS=windows GOARCH=amd64 go build ./...
go test -race -count=10 ./internal/transport ./internal/service ./internal/live ./internal/generation
```

Focused regressions cover duplicate startup, idle-client shutdown, short writes,
admission digest ordering and generation binding, exclusion-policy upgrade,
opened-root replacement, excluded live scope, and trailing client JSON. The
existing Darwin race-linker `LC_DYSYMTAB` warnings remained non-fatal.

## Installed M4 upgrade evidence

The authoritative Neo tree was mirrored with `m4build`, and the existing
`com.filemanager.engine.m4-dogfood` LaunchAgent was upgraded in place. Because
the new store had no `ADMISSION` record, startup reconciled before publishing
its sockets. The authenticated installer readiness call then reported:

- lifecycle `ready`, `sandboxed=false`, deployment `m4-dogfood`;
- generation 4 with 30,632 records and persistent root policy;
- exact approved root unchanged and engine state outside it.

The `ADMISSION` record was a same-user regular `0600` file naming generation 4.
Both sockets remained `0600` Unix sockets; both tokens and discovery remained
regular `0600` files. Authenticated integrity returned generation 4 healthy and
repairable.

An administrative shutdown exercised launchd replacement. Instance id changed
from `98603bd809262ce0f12c6ecbab8f59b2` to
`61ca6c10714a2e7452904ceb8ed354dd`; the 64-byte query-token checksum changed,
while the recovered checked generation remained 4 and ready. This restart did
not manufacture generation 5, demonstrating that a matching admission record
allows checked recovery without a redundant source publication.

After the acknowledgement hook was added, a final installed manual reconcile
published generation 5 in 24.834 seconds and returned 30,632 records. The
`ADMISSION` record already named generation 5 when the successful response was
observed. A following launchd restart became callable on the first 100 ms poll,
recovered generation 5 ready, and left the marker at generation 5; no redundant
generation or startup scan was produced.

## Scope of inference

This is a bounded professional-hardening slice, not a general release claim.
It does not add status streaming, request multiplexing, session-scoped remote
cancellation, SCM/systemd adapters, multi-user authorization, automatic exact
background currentness, or a durable root-policy revision in the primary
generation manifest. The `ADMISSION` record is an M4 installed projection until
that broader format and contract work is explicitly admitted.
