# filemanager/engine/internal/ranking

Status: **OBSERVED deterministic exact mapping**.

Maps exact matches and records into evidence-preserving result order without pretending later fusion is implemented.

Package ranking converts channel-preserving candidates into public results. M1 has one exact tier; no unrelated raw scores are fused here.

## Invariants

- Exact evidence remains named and reconstructable.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/catalog`
- `filemanager/engine/internal/exact`

## Declarations

### Exact

Kind: `function`. Source: `internal/ranking/exact.go:11`.

```go
func Exact(matches []exact.Match, generation api.Generation) []api.Result
```

No declaration documentation comment is present.

### Record

Kind: `function`. Source: `internal/ranking/exact.go:19`.

```go
func Record(record catalog.Record, generation api.Generation, rank int, evidence []api.Evidence) api.Result
```

No declaration documentation comment is present.
