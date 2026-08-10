# filemanager/engine/internal/ownership

Status: **OBSERVED**.

Routes each path to its most-specific approved root and prevents parent/child shard duplication.

Package ownership routes each path to its most-specific approved root.

## Invariants

- An independently approved child root prunes the parent projection.

## Internal imports

- `None.`

## Declarations

### Router.Owner

Kind: `method`. Source: `internal/ownership/router.go:51`.

```go
func (r *Router) Owner(path string) (Root, error)
```

No declaration documentation comment is present.

### Router.OwnerCleanAbsolute

Kind: `method`. Source: `internal/ownership/router.go:62`.

```go
func (r *Router) OwnerCleanAbsolute(path string) (Root, error)
```

OwnerCleanAbsolute avoids repeated absolute-path normalization in scanners that already construct canonical absolute paths.

### Router.Replace

Kind: `method`. Source: `internal/ownership/router.go:22`.

```go
func (r *Router) Replace(roots []Root) error
```

No declaration documentation comment is present.

### Router.ownerCleanAbsolute

Kind: `method`. Source: `internal/ownership/router.go:69`.

```go
func (r *Router) ownerCleanAbsolute(path string) (Root, error)
```

No declaration documentation comment is present.

### Root

Kind: `struct`. Source: `internal/ownership/router.go:12`.

```go
type Root struct
```

No declaration documentation comment is present.

### Router

Kind: `struct`. Source: `internal/ownership/router.go:17`.

```go
type Router struct
```

No declaration documentation comment is present.

### ErrNoOwner

Kind: `variable`. Source: `internal/ownership/router.go:10`.

```go
var ErrNoOwner = errors.New("no approved root owns path")
```

No declaration documentation comment is present.
