# filemanager/engine/internal/identity

Status: **OBSERVED platform adapters with remaining promotion gates**.

Typed platform identity and incarnation evidence for macOS, Linux, and Windows observations.

Package identity extracts exact platform object keys from filesystem observations. Paths never participate in object identity.

## Invariants

- Paths are observed addresses, not identity.
- Platform evidence is retained rather than collapsed into a false universal key.

## Internal imports

- `None.`

## Declarations

### PlatformDarwin

Kind: `constant`. Source: `internal/identity/identity.go:16`.

```go
PlatformDarwin
```

No declaration documentation comment is present.

### PlatformFixture

Kind: `constant`. Source: `internal/identity/identity.go:19`.

```go
PlatformFixture
```

No declaration documentation comment is present.

### PlatformLinux

Kind: `constant`. Source: `internal/identity/identity.go:17`.

```go
PlatformLinux
```

No declaration documentation comment is present.

### PlatformUnknown

Kind: `constant`. Source: `internal/identity/identity.go:15`.

```go
PlatformUnknown Platform = iota
```

No declaration documentation comment is present.

### PlatformWindows

Kind: `constant`. Source: `internal/identity/identity.go:18`.

```go
PlatformWindows
```

No declaration documentation comment is present.

### Compare

Kind: `function`. Source: `internal/identity/identity.go:133`.

```go
func Compare(left, right Observation) int
```

No declaration documentation comment is present.

### Observe

Kind: `function`. Source: `internal/identity/identity.go:176`.

```go
func Observe(root *os.Root, name string, info os.FileInfo) (Observation, error)
```

Observe is implemented per target because an os.FileInfo alone does not expose a portable authoritative object key. root and name keep any handle acquisition confined to the approved root.

### ParseObjectID

Kind: `function`. Source: `internal/identity/identity.go:92`.

```go
func ParseObjectID(encoded string) (Observation, error)
```

No declaration documentation comment is present.

### observe

Kind: `function`. Source: `internal/identity/identity_darwin.go:10`.

```go
func observe(_ *os.Root, _ string, info os.FileInfo) (Observation, error)
```

No declaration documentation comment is present.

### observe

Kind: `function`. Source: `internal/identity/identity_linux.go:11`.

```go
func observe(_ *os.Root, _ string, info os.FileInfo) (Observation, error)
```

No declaration documentation comment is present.

### observe

Kind: `function`. Source: `internal/identity/identity_windows.go:11`.

```go
func observe(root *os.Root, name string, info os.FileInfo) (Observation, error)
```

No declaration documentation comment is present.

### Observation.Fields

Kind: `method`. Source: `internal/identity/identity.go:62`.

```go
func (o Observation) Fields() map[string]string
```

No declaration documentation comment is present.

### Observation.IncarnationString

Kind: `method`. Source: `internal/identity/identity.go:85`.

```go
func (o Observation) IncarnationString() string
```

No declaration documentation comment is present.

### Observation.Key

Kind: `method`. Source: `internal/identity/identity.go:47`.

```go
func (o Observation) Key() Key
```

No declaration documentation comment is present.

### Observation.Namespace

Kind: `method`. Source: `internal/identity/identity.go:70`.

```go
func (o Observation) Namespace() string
```

No declaration documentation comment is present.

### Observation.ObjectID

Kind: `method`. Source: `internal/identity/identity.go:55`.

```go
func (o Observation) ObjectID() string
```

No declaration documentation comment is present.

### Observation.VolumeKey

Kind: `method`. Source: `internal/identity/identity.go:51`.

```go
func (o Observation) VolumeKey() VolumeKey
```

No declaration documentation comment is present.

### Incarnation

Kind: `struct`. Source: `internal/identity/identity.go:29`.

```go
type Incarnation struct
```

No declaration documentation comment is present.

### Key

Kind: `struct`. Source: `internal/identity/identity.go:40`.

```go
type Key struct
```

No declaration documentation comment is present.

### Observation

Kind: `struct`. Source: `internal/identity/identity.go:22`.

```go
type Observation struct
```

No declaration documentation comment is present.

### VolumeKey

Kind: `struct`. Source: `internal/identity/identity.go:35`.

```go
type VolumeKey struct
```

No declaration documentation comment is present.

### Platform

Kind: `type`. Source: `internal/identity/identity.go:12`.

```go
type Platform uint8
```

No declaration documentation comment is present.
