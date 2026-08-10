# filemanager/engine/internal/scan

Status: **OBSERVED metadata-only reference scanner**.

Capability-rooted authoritative traversal that produces exact catalogue shards without reading file contents or following directory symlinks.

Package scan performs metadata-only observation through an os.Root. It does not open file contents, follow directory symlinks, or write into source roots.

## Invariants

- Traversal stays beneath the explicit sandbox or manifest-bound root.
- Installed traversal verifies identity from the opened root handle.
- Source trees are read-only.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/catalog`
- `filemanager/engine/internal/identity`

## Declarations

### objectKind

Kind: `function`. Source: `internal/scan/scanner.go:120`.

```go
func objectKind(mode os.FileMode) api.ObjectKind
```

No declaration documentation comment is present.

### observedObject

Kind: `function`. Source: `internal/scan/scanner.go:113`.

```go
func observedObject(observed identity.Observation, info os.FileInfo) catalog.Object
```

No declaration documentation comment is present.

### Scanner.Scan

Kind: `method`. Source: `internal/scan/scanner.go:24`.

```go
func (Scanner) Scan(ctx context.Context, root api.RootSpec, owns OwnsFunc) (*catalog.Shard, error)
```

No declaration documentation comment is present.

### Scanner.ScanApproved

Kind: `method`. Source: `internal/scan/scanner.go:31`.

```go
func (Scanner) ScanApproved(ctx context.Context, root api.RootSpec, expectedObjectID string, owns OwnsFunc) (*catalog.Shard, error)
```

ScanApproved binds traversal to the exact directory object admitted by an installed manifest. The comparison uses the already-open os.Root handle, so a path replacement cannot race between validation and traversal.

### Scanner.scan

Kind: `method`. Source: `internal/scan/scanner.go:38`.

```go
func (Scanner) scan(ctx context.Context, root api.RootSpec, expectedObjectID string, owns OwnsFunc) (*catalog.Shard, error)
```

No declaration documentation comment is present.

### Scanner

Kind: `struct`. Source: `internal/scan/scanner.go:20`.

```go
type Scanner struct
```

No declaration documentation comment is present.

### OwnsFunc

Kind: `type`. Source: `internal/scan/scanner.go:18`.

```go
type OwnsFunc func(api.RootID, string) bool
```

No declaration documentation comment is present.

### ErrRootIdentityChanged

Kind: `variable`. Source: `internal/scan/scanner.go:22`.

```go
var ErrRootIdentityChanged = errors.New("approved root object identity changed")
```

No declaration documentation comment is present.
