# filemanager/engine/internal/exact

Status: **OBSERVED M1 exact subset**.

Bounded exact candidates, metadata filters, deterministic sorting, inspection, and generation-bound cursors over reference or durable readers.

Package exact implements M1 exact candidate generation and metadata predicates. It deliberately rejects residual text until the lexical engine defines that language.

## Invariants

- Candidate-budget overflow is an error, never silent truncation.
- Cursors bind to generation and normalized query.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/catalog`
- `filemanager/engine/internal/identity`
- `filemanager/engine/internal/workload`

## Declarations

### DefaultLimit

Kind: `constant`. Source: `internal/exact/query.go:25`.

```go
DefaultLimit        = 100
```

No declaration documentation comment is present.

### MaximumCandidates

Kind: `constant`. Source: `internal/exact/query.go:27`.

```go
MaximumCandidates   = 100_000
```

No declaration documentation comment is present.

### MaximumLimit

Kind: `constant`. Source: `internal/exact/query.go:26`.

```go
MaximumLimit        = 1000
```

No declaration documentation comment is present.

### cursorSchemaVersion

Kind: `constant`. Source: `internal/exact/query.go:28`.

```go
cursorSchemaVersion = 1
```

No declaration documentation comment is present.

### Inspect

Kind: `function`. Source: `internal/exact/query.go:321`.

```go
func Inspect(snapshot *catalog.Snapshot, ref api.ObjectRef) (catalog.Record, error)
```

No declaration documentation comment is present.

### InspectIndex

Kind: `function`. Source: `internal/exact/query.go:332`.

```go
func InspectIndex(snapshot *catalog.Snapshot, rootID api.RootID, source Index, ref api.ObjectRef) (catalog.Record, error)
```

No declaration documentation comment is present.

### Query

Kind: `function`. Source: `internal/exact/query.go:134`.

```go
func Query(ctx context.Context, snapshot *catalog.Snapshot, query api.Query) (matches []Match, next string, err error)
```

No declaration documentation comment is present.

### QueryIndex

Kind: `function`. Source: `internal/exact/query.go:145`.

```go
func QueryIndex(ctx context.Context, snapshot *catalog.Snapshot, generation api.Generation, rootID api.RootID, source Index, query api.Query) (matches []Match, next string, err error)
```

No declaration documentation comment is present.

### canPageExactName

Kind: `function`. Source: `internal/exact/query.go:490`.

```go
func canPageExactName(snapshot *catalog.Snapshot, scopeRelative string, query api.Query, p predicates, order []api.SortKey) bool
```

No declaration documentation comment is present.

### candidateIndices

Kind: `function`. Source: `internal/exact/query.go:473`.

```go
func candidateIndices(ctx context.Context, source Index, predicates predicates) ([]uint32, bool, error)
```

No declaration documentation comment is present.

### compareField

Kind: `function`. Source: `internal/exact/query.go:576`.

```go
func compareField(left, right catalog.Row, field string) int
```

No declaration documentation comment is present.

### compareInt64

Kind: `function`. Source: `internal/exact/query.go:591`.

```go
func compareInt64(left, right int64) int
```

No declaration documentation comment is present.

### contains

Kind: `function`. Source: `internal/exact/query.go:683`.

```go
func contains(root, path string) bool
```

No declaration documentation comment is present.

### cursorOffset

Kind: `function`. Source: `internal/exact/query.go:648`.

```go
func cursorOffset(encoded string, generation api.Generation, fingerprint string, candidates int) (int, error)
```

No declaration documentation comment is present.

### eligible

Kind: `function`. Source: `internal/exact/query.go:496`.

```go
func eligible(snapshot *catalog.Snapshot, root api.RootSpec, row catalog.Row, scope string, descendants bool, p predicates) bool
```

No declaration documentation comment is present.

### encodeCursor

Kind: `function`. Source: `internal/exact/query.go:675`.

```go
func encodeCursor(value cursor) (string, error)
```

No declaration documentation comment is present.

### evidenceCount

Kind: `function`. Source: `internal/exact/query.go:601`.

```go
func evidenceCount(p predicates) int
```

No declaration documentation comment is present.

### fillEvidence

Kind: `function`. Source: `internal/exact/query.go:615`.

```go
func fillEvidence(result []api.Evidence, record catalog.Record, p predicates)
```

No declaration documentation comment is present.

### fingerprint

Kind: `function`. Source: `internal/exact/query.go:634`.

```go
func fingerprint(query api.Query, root api.RootSpec, scope string, order []api.SortKey) string
```

No declaration documentation comment is present.

### isCanonicalPathOrder

Kind: `function`. Source: `internal/exact/query.go:630`.

```go
func isCanonicalPathOrder(order []api.SortKey) bool
```

No declaration documentation comment is present.

### less

Kind: `function`. Source: `internal/exact/query.go:559`.

```go
func less(left, right catalog.Row, order []api.SortKey) bool
```

No declaration documentation comment is present.

### normalizeOrder

Kind: `function`. Source: `internal/exact/query.go:535`.

```go
func normalizeOrder(order []api.SortKey) ([]api.SortKey, error)
```

No declaration documentation comment is present.

### parseInteger

Kind: `function`. Source: `internal/exact/query.go:465`.

```go
func parseInteger(value, field string) (int64, error)
```

No declaration documentation comment is present.

### parseNonnegative

Kind: `function`. Source: `internal/exact/query.go:454`.

```go
func parseNonnegative(value, field string) (int64, error)
```

No declaration documentation comment is present.

### parsePredicates

Kind: `function`. Source: `internal/exact/query.go:394`.

```go
func parsePredicates(root api.RootSpec, filters map[string]string) (predicates, error)
```

No declaration documentation comment is present.

### scopePath

Kind: `function`. Source: `internal/exact/query.go:521`.

```go
func scopePath(root api.RootSpec, requested string) (string, error)
```

No declaration documentation comment is present.

### Index

Kind: `interface`. Source: `internal/exact/query.go:57`.

```go
type Index interface
```

Index is the exact, generation-pinned read surface shared by the exhaustive M1 control and the off-heap M2 segment reader. Storage failures remain explicit errors; they are never translated into an empty result.

### referenceIndex.CandidateAll

Kind: `method`. Source: `internal/exact/query.go:69`.

```go
func (r referenceIndex) CandidateAll(ctx context.Context, maximum int) ([]uint32, bool, error)
```

No declaration documentation comment is present.

### referenceIndex.CandidateID

Kind: `method`. Source: `internal/exact/query.go:111`.

```go
func (r referenceIndex) CandidateID(id api.ObjectID, maximum int) ([]uint32, bool, error)
```

No declaration documentation comment is present.

### referenceIndex.CandidateName

Kind: `method`. Source: `internal/exact/query.go:88`.

```go
func (r referenceIndex) CandidateName(name string, maximum int) ([]uint32, bool, error)
```

No declaration documentation comment is present.

### referenceIndex.CandidateNamePage

Kind: `method`. Source: `internal/exact/query.go:96`.

```go
func (r referenceIndex) CandidateNamePage(name string, offset, limit, maximum int) ([]uint32, int, bool, error)
```

No declaration documentation comment is present.

### referenceIndex.PathIndex

Kind: `method`. Source: `internal/exact/query.go:119`.

```go
func (r referenceIndex) PathIndex(path string) (uint32, bool, error)
```

No declaration documentation comment is present.

### referenceIndex.Record

Kind: `method`. Source: `internal/exact/query.go:129`.

```go
func (r referenceIndex) Record(index uint32) (catalog.Record, bool, error)
```

No declaration documentation comment is present.

### referenceIndex.Row

Kind: `method`. Source: `internal/exact/query.go:124`.

```go
func (r referenceIndex) Row(index uint32) (catalog.Row, bool, error)
```

No declaration documentation comment is present.

### Match

Kind: `struct`. Source: `internal/exact/query.go:31`.

```go
type Match struct
```

No declaration documentation comment is present.

### cursor

Kind: `struct`. Source: `internal/exact/query.go:37`.

```go
type cursor struct
```

No declaration documentation comment is present.

### predicates

Kind: `struct`. Source: `internal/exact/query.go:44`.

```go
type predicates struct
```

No declaration documentation comment is present.

### referenceIndex

Kind: `struct`. Source: `internal/exact/query.go:67`.

```go
type referenceIndex struct
```

No declaration documentation comment is present.
