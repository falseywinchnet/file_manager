# Scaffold baseline 000

Status: **MEASURED**.

Date: 2026-08-05.

Environment:

- macOS (`darwin/arm64`);
- Apple M3;
- Go 1.26.5;
- fresh local build cache;
- no catalogue data and no production storage/query path yet.

Commands and outcomes:

```text
go test ./...                                      PASS
go test -race ./...                                PASS
go vet ./...                                       PASS
go test -run '^$' -bench . -benchmem ./...         PASS
```

Initial microbenchmark:

```text
BenchmarkOwnerAmongNestedRoots-8    137485    10023 ns/op    0 B/op    0 allocs/op
```

The benchmark routes one target through a 512-entry nested-root table. This is
a scaffold baseline, not an engine latency claim: it excludes storage, parsing,
filesystem observation, retrieval, rank fusion, transport, cold-cache behavior,
and concurrent mutation.

The JSONL launcher was also exercised against a disposable `mktemp` directory.
It returned `engine.v0`, reported `sandboxed: true`, exposed only the temporary
root, and accepted a clean shutdown. The temporary empty directory was removed.
