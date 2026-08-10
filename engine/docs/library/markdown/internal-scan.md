# filemanager/engine/internal/scan

Status: **OBSERVED metadata-only reference scanner**.

Capability-rooted authoritative traversal that produces exact catalogue shards without reading file contents or following directory symlinks.

Package scan performs metadata-only observation through an os.Root. It does not open file contents, follow directory symlinks, or write into source roots.

## Invariants

- Traversal stays beneath the explicit sandbox.
- Source trees are read-only.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/catalog`
- `filemanager/engine/internal/identity`

## Declarations

### objectKind

Kind: `function`. Source: `internal/scan/scanner.go:100`.

```go
func objectKind(mode os.FileMode) api.ObjectKind
```

No declaration documentation comment is present.

### observedObject

Kind: `function`. Source: `internal/scan/scanner.go:93`.

```go
func observedObject(observed identity.Observation, info os.FileInfo) catalog.Object
```

No declaration documentation comment is present.

### Scanner.Scan

Kind: `method`. Source: `internal/scan/scanner.go:21`.

```go
func (Scanner) Scan(ctx context.Context, root api.RootSpec, owns OwnsFunc) (*catalog.Shard, error)
```

No declaration documentation comment is present.

### Scanner

Kind: `struct`. Source: `internal/scan/scanner.go:19`.

```go
type Scanner struct
```

No declaration documentation comment is present.

### OwnsFunc

Kind: `type`. Source: `internal/scan/scanner.go:17`.

```go
type OwnsFunc func(api.RootID, string) bool
```

No declaration documentation comment is present.
