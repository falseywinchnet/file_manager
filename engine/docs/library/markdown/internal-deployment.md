# filemanager/engine/internal/deployment

Status: **OBSERVED M4-only production-root admission**.

Validates same-uid 0600 manifests bound to Darwin host UUID, exact root object identity, exclusions, private engine-owned state, and a checked generation's admission digest.

Package deployment validates the host-bound authority manifest used by an installed Engine service. It is the production replacement for the disposable development sandbox, not a bypass around root admission.

## Invariants

- A manifest copied to another host or uid fails closed.
- Root identity and canonical path are rechecked at every start and from the opened traversal handle.
- A recovered generation is not served under a different exclusion policy.
- Engine state remains outside indexed roots.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/identity`
- `filemanager/engine/internal/sandbox`

## Declarations

### ManifestMajor

Kind: `constant`. Source: `internal/deployment/manifest.go:25`.

```go
ManifestMajor  = 1
```

No declaration documentation comment is present.

### ManifestSchema

Kind: `constant`. Source: `internal/deployment/manifest.go:24`.

```go
ManifestSchema = "fileman.engine.deployment"
```

No declaration documentation comment is present.

### admissionFile

Kind: `constant`. Source: `internal/deployment/manifest.go:27`.

```go
admissionFile  = "ADMISSION"
```

No declaration documentation comment is present.

### admissionSchema

Kind: `constant`. Source: `internal/deployment/manifest.go:59`.

```go
const admissionSchema = "fileman.engine.admission.v1"
```

No declaration documentation comment is present.

### maxManifest

Kind: `constant`. Source: `internal/deployment/manifest.go:26`.

```go
maxManifest    = 1 << 20
```

No declaration documentation comment is present.

### AdmissionMatches

Kind: `function`. Source: `internal/deployment/manifest.go:237`.

```go
func AdmissionMatches(storeRoot, digest string, generation api.Generation) (bool, error)
```

AdmissionMatches proves that the recovered generation was produced under the current root/exclusion authority. A missing marker is a safe cache miss; malformed or loosely protected state fails closed.

### CommitAdmission

Kind: `function`. Source: `internal/deployment/manifest.go:278`.

```go
func CommitAdmission(storeRoot, digest string, generation api.Generation) error
```

CommitAdmission atomically records the policy/generation pair only after a checked reconciliation has published successfully.

### CurrentHost

Kind: `function`. Source: `internal/deployment/manifest.go:61`.

```go
func CurrentHost() (Host, error)
```

No declaration documentation comment is present.

### LoadSecure

Kind: `function`. Source: `internal/deployment/manifest.go:89`.

```go
func LoadSecure(path string) (Manifest, error)
```

LoadSecure reads a regular, same-uid, 0600 manifest without following a manifest-path symlink, rejects trailing JSON, and validates it for this host.

### RootObjectID

Kind: `function`. Source: `internal/deployment/manifest.go:70`.

```go
func RootObjectID(path string) (string, error)
```

RootObjectID observes the exact root directory identity through os.Root.

### Validate

Kind: `function`. Source: `internal/deployment/manifest.go:128`.

```go
func Validate(manifest *Manifest, host Host) error
```

No declaration documentation comment is present.

### contains

Kind: `function`. Source: `internal/deployment/manifest.go:355`.

```go
func contains(root, path string) bool
```

No declaration documentation comment is present.

### exactDirectory

Kind: `function`. Source: `internal/deployment/manifest.go:323`.

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

Kind: `function`. Source: `internal/deployment/manifest.go:341`.

```go
func requirePrivateDirectory(path string) error
```

No declaration documentation comment is present.

### Manifest.AdmissionDigest

Kind: `method`. Source: `internal/deployment/manifest.go:199`.

```go
func (m Manifest) AdmissionDigest() string
```

AdmissionDigest identifies the security-relevant root authority independently of JSON field or list order. Store and runtime placement are validated separately and do not affect which source records a generation may contain.

### Manifest.Guard

Kind: `method`. Source: `internal/deployment/manifest.go:180`.

```go
func (m Manifest) Guard() (*sandbox.Guard, error)
```

No declaration documentation comment is present.

### Manifest.RootSpecs

Kind: `method`. Source: `internal/deployment/manifest.go:188`.

```go
func (m Manifest) RootSpecs() []api.RootSpec
```

No declaration documentation comment is present.

### Host

Kind: `struct`. Source: `internal/deployment/manifest.go:30`.

```go
type Host struct
```

No declaration documentation comment is present.

### Manifest

Kind: `struct`. Source: `internal/deployment/manifest.go:42`.

```go
type Manifest struct
```

No declaration documentation comment is present.

### Root

Kind: `struct`. Source: `internal/deployment/manifest.go:35`.

```go
type Root struct
```

No declaration documentation comment is present.

### admissionRecord

Kind: `struct`. Source: `internal/deployment/manifest.go:53`.

```go
type admissionRecord struct
```

No declaration documentation comment is present.
