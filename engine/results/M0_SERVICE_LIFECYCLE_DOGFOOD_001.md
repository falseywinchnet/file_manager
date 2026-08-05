# M0 service lifecycle/configuration dogfood 001

Status: **MEASURED development service projection; not native supervisor,
transport, or power-loss evidence**.

Date: 2026-08-05. Host: Apple M3, 8 GiB, macOS/Darwin 23.6.0,
Go 1.26.5 darwin/arm64. Command:

```sh
FILEMAN_ENGINE_MEASURE=1 go test ./benchmarks \
  -run '^TestServiceLifecycleResourceDogfood$' -count=1 -v
```

The workload created a disposable sandbox, one-file authorized source, and an
engine-owned persistent store outside the source. It committed generation 1,
repeated read-only service projections 10,000 times each, performed an
unchanged authoritative reconciliation, and drained shutdown.

| Operation | Total time | Time/call | Allocated/call | Mallocs/call |
|---|---:|---:|---:|---:|
| effective configuration | 19.389 ms | 1.938 µs | 753.12 B | 7.003 |
| version/capability projection | 4.628 ms | 0.462 µs | 1,696.00 B | 3.000 |
| complete status projection | 19.729 ms | 1.972 µs | 2,297.77 B | 12.008 |

The unchanged reconciliation completed in 323.542 µs, returned generation 1
with `published=false`, and left the observed durable store at two files and
1,062 bytes with unchanged file count, byte count, and maximum modification
time. Draining shutdown completed in 64.292 µs and left the same durable state.

Separate conformance tests prove that shutdown rejects new work, cancels and
waits for admitted operations, becomes idempotently stopped, and that restart
changes process instance identity while preserving checked configuration.
They also prove that a stale root-policy digest cannot mutate service state and
that a forced rebuild still publishes even when exact state is unchanged.

## Interpretation

- **OBSERVED:** read-only lifecycle/configuration/status calls are microsecond
  scale with small bounded allocations on the one-root development instance.
- **OBSERVED:** an exact digest match bypasses segment creation, manifest
  publication, sync, rename, and reclamation in the persistent reconciliation
  path. Shutdown writes no clean-stop marker.
- **GIVEN:** this no-op rule applies to the complete exact catalogue state; it
  is not coupled to a fuzzy/similarity component.
- **NOT MEASURED:** native launchd/SCM/systemd behavior, IPC authentication,
  idle hours, device power use, filesystem metadata below the application,
  NTFS/ext4, process-kill durability, or physical NAND writes.

## Verification matrix

- **PASS:** `go test ./...`, `go test -race ./...`, and `go vet ./...` on
  macOS/arm64;
- **PASS:** Linux/amd64 and Windows/amd64 `go build ./...`;
- **PASS compatibility oracle:** all 13 test-bearing Windows/amd64 packages
  under Wine 11.10 on host APFS;
- **PASS compatibility oracle:** all 13 test-bearing Linux/arm64 packages in
  Lima with test temporary data on guest `/var/tmp`.

Wine and Lima do not establish native service installation, NTFS/ext4
durability, platform identity, or power-loss safety.

Implementation: `internal/service/lifecycle.go` and
`internal/service/service.go`. Measurement:
`benchmarks/service_lifecycle_test.go`. Conformance:
`internal/service/lifecycle_test.go` and
`internal/transport/jsonl_test.go`.
