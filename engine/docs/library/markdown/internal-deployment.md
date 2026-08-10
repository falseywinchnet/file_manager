# filemanager/engine/internal/deployment

Status: **OBSERVED M4-only production-root admission**.

Validates same-uid 0600 manifests bound to Darwin host UUID, exact root object identity, exclusions, and private engine-owned state.

Package deployment validates the host-bound authority manifest used by an installed Engine service. It is the production replacement for the disposable development sandbox, not a bypass around root admission.

## Invariants

- A manifest copied to another host or uid fails closed.
- Root identity and canonical path are rechecked at every start.
- Engine state remains outside indexed roots.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/identity`
- `filemanager/engine/internal/sandbox`

## Declarations

### ManifestMajor

Kind: `constant`. Source: `internal/deployment/manifest.go:23`.

```go
ManifestMajor  = 1
```

No declaration documentation comment is present.

### ManifestSchema

Kind: `constant`. Source: `internal/deployment/manifest.go:22`.

```go
ManifestSchema = "fileman.engine.deployment"
```

No declaration documentation comment is present.

### maxManifest

Kind: `constant`. Source: `internal/deployment/manifest.go:24`.

```go
maxManifest    = 1 << 20
```

No declaration documentation comment is present.

### CurrentHost

Kind: `function`. Source: `internal/deployment/manifest.go:50`.

```go
func CurrentHost() (Host, error)
```

No declaration documentation comment is present.

### LoadSecure

Kind: `function`. Source: `internal/deployment/manifest.go:78`.

```go
func LoadSecure(path string) (Manifest, error)
```

LoadSecure reads a regular, same-uid, 0600 manifest without following a manifest-path symlink, rejects trailing JSON, and validates it for this host.

### RootObjectID

Kind: `function`. Source: `internal/deployment/manifest.go:59`.

```go
func RootObjectID(path string) (string, error)
```

RootObjectID observes the exact root directory identity through os.Root.

### Validate

Kind: `function`. Source: `internal/deployment/manifest.go:117`.

```go
func Validate(manifest *Manifest, host Host) error
```

No declaration documentation comment is present.

### contains

Kind: `function`. Source: `internal/deployment/manifest.go:217`.

```go
func contains(root, path string) bool
```

No declaration documentation comment is present.

### exactDirectory

Kind: `function`. Source: `internal/deployment/manifest.go:185`.

```go
func exactDirectory(path string) (string, error)
```

No declaration documentation comment is present.

### fileOwner

Kind: `function`. Source: `internal/deployment/owner_windows.go:7`.

```go
func fileOwner(os.FileInfo) (int, bool)
```

No declaration documentation comment is present.

### fileOwner

Kind: `function`. Source: `internal/deployment/owner_unix.go:10`.

```go
func fileOwner(info os.FileInfo) (int, bool)
```

No declaration documentation comment is present.

### hostUUID

Kind: `function`. Source: `internal/deployment/host_other.go:7`.

```go
func hostUUID() (string, error)
```

No declaration documentation comment is present.

### hostUUID

Kind: `function`. Source: `internal/deployment/host_darwin.go:10`.

```go
func hostUUID() (string, error)
```

No declaration documentation comment is present.

### openManifestNoFollow

Kind: `function`. Source: `internal/deployment/open_manifest_windows.go:7`.

```go
func openManifestNoFollow(path string) (*os.File, error)
```

No declaration documentation comment is present.

### openManifestNoFollow

Kind: `function`. Source: `internal/deployment/open_manifest_unix.go:10`.

```go
func openManifestNoFollow(path string) (*os.File, error)
```

No declaration documentation comment is present.

### requirePrivateDirectory

Kind: `function`. Source: `internal/deployment/manifest.go:203`.

```go
func requirePrivateDirectory(path string) error
```

No declaration documentation comment is present.

### Manifest.Guard

Kind: `method`. Source: `internal/deployment/manifest.go:169`.

```go
func (m Manifest) Guard() (*sandbox.Guard, error)
```

No declaration documentation comment is present.

### Manifest.RootSpecs

Kind: `method`. Source: `internal/deployment/manifest.go:177`.

```go
func (m Manifest) RootSpecs() []api.RootSpec
```

No declaration documentation comment is present.

### Host

Kind: `struct`. Source: `internal/deployment/manifest.go:27`.

```go
type Host struct
```

No declaration documentation comment is present.

### Manifest

Kind: `struct`. Source: `internal/deployment/manifest.go:39`.

```go
type Manifest struct
```

No declaration documentation comment is present.

### Root

Kind: `struct`. Source: `internal/deployment/manifest.go:32`.

```go
type Root struct
```

No declaration documentation comment is present.
