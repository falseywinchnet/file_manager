# filemanager/engine/internal/sandbox

Status: **OBSERVED development and installed admission guards**.

Canonical development containment plus immutable manifest id/path and exclusion enforcement for installed traversal.

Package sandbox confines development and test access to an explicit root.

## Invariants

- No unsafe default, force flag, or environment bypass.
- Installed administrative calls cannot widen manifest authority.

## Internal imports

- `filemanager/engine/api`

## Declarations

### New

Kind: `function`. Source: `internal/sandbox/guard.go:34`.

```go
func New(root string) (*Guard, error)
```

No declaration documentation comment is present.

### NewApproved

Kind: `function`. Source: `internal/sandbox/guard.go:63`.

```go
func NewApproved(deployment string, approved []ApprovedRoot) (*Guard, error)
```

NewApproved constructs a manifest-bound installed-service guard. This is deliberately not a bypass flag: only the exact canonical id/path pairs in approved can be configured, and exclusions can only reduce their scope.

### canonicalDirectory

Kind: `function`. Source: `internal/sandbox/guard.go:114`.

```go
func canonicalDirectory(path string) (string, error)
```

No declaration documentation comment is present.

### canonicalExclusions

Kind: `function`. Source: `internal/sandbox/guard.go:97`.

```go
func canonicalExclusions(values []string) ([]string, error)
```

No declaration documentation comment is present.

### contains

Kind: `function`. Source: `internal/sandbox/guard.go:236`.

```go
func contains(root, absolute string) bool
```

No declaration documentation comment is present.

### Guard.Allows

Kind: `method`. Source: `internal/sandbox/guard.go:163`.

```go
func (g *Guard) Allows(rootID api.RootID, absolute string) bool
```

Allows is the final per-entry admission predicate. Root ownership prevents overlapping projections; this predicate additionally prunes manifest exclusions before metadata is observed.

### Guard.Deployment

Kind: `method`. Source: `internal/sandbox/guard.go:137`.

```go
func (g *Guard) Deployment() string
```

No declaration documentation comment is present.

### Guard.Resolve

Kind: `method`. Source: `internal/sandbox/guard.go:216`.

```go
func (g *Guard) Resolve(candidate string) (string, error)
```

Resolve returns an absolute contained path. This lexical guard is followed by platform identity/symlink checks before production scanning is admitted.

### Guard.ResolveDirectory

Kind: `method`. Source: `internal/sandbox/guard.go:187`.

```go
func (g *Guard) ResolveDirectory(candidate string) (string, error)
```

ResolveDirectory resolves an existing directory through symlinks and then proves that the resolved directory remains under the development sandbox. Scanners subsequently use os.Root so later traversal cannot escape through a symlink swap.

### Guard.ResolveRoot

Kind: `method`. Source: `internal/sandbox/guard.go:142`.

```go
func (g *Guard) ResolveRoot(root api.RootSpec) (string, error)
```

ResolveRoot proves that a requested policy entry is exactly one of the immutable manifest admissions. Development mode retains contained-subroot behavior for disposable fixtures.

### Guard.Root

Kind: `method`. Source: `internal/sandbox/guard.go:133`.

```go
func (g *Guard) Root() string
```

No declaration documentation comment is present.

### Guard.Sandboxed

Kind: `method`. Source: `internal/sandbox/guard.go:135`.

```go
func (g *Guard) Sandboxed() bool
```

No declaration documentation comment is present.

### Guard.contains

Kind: `method`. Source: `internal/sandbox/guard.go:232`.

```go
func (g *Guard) contains(absolute string) bool
```

No declaration documentation comment is present.

### ApprovedRoot

Kind: `struct`. Source: `internal/sandbox/guard.go:27`.

```go
type ApprovedRoot struct
```

ApprovedRoot is an installed-service admission. ObjectID is checked by the deployment loader before construction; Guard then enforces the immutable id/path and relative exclusion policy for every operation.

### Guard

Kind: `struct`. Source: `internal/sandbox/guard.go:17`.

```go
type Guard struct
```

No declaration documentation comment is present.

### ErrOutsideRoot

Kind: `variable`. Source: `internal/sandbox/guard.go:15`.

```go
var ErrOutsideRoot = errors.New("path is outside sandbox root")
```

No declaration documentation comment is present.
