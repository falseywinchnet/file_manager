# filemanager/engine/internal/workload

Status: **OBSERVED**.

Canonical generated logical corpora and digests shared by reference and control implementations.

Package workload defines generated, non-sensitive logical corpora shared by the reference catalogue and future M2 controls.

## Invariants

- Controls consume identical logical records and correctness digests.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/catalog`
- `filemanager/engine/internal/identity`

## Declarations

### SchemaVersion

Kind: `constant`. Source: `internal/workload/workload.go:18`.

```go
const SchemaVersion = "fileman-workload-v1"
```

No declaration documentation comment is present.

### CorrectnessV1

Kind: `function`. Source: `internal/workload/workload.go:38`.

```go
func CorrectnessV1() Corpus
```

CorrectnessV1 is deliberately small and adversarial. Repeated Object 11 is a hard link with bindings under different parents; Object 12 is a symlink, not an alias for its lexical target name.

### ScaleV1

Kind: `function`. Source: `internal/workload/workload.go:59`.

```go
func ScaleV1(fileCount, directorySize, repeatedEvery int) (Corpus, error)
```

ScaleV1 generates a deterministic metadata-only corpus. directorySize must be positive. When repeatedEvery is positive it must equal directorySize, so exactly one binding per directory receives the basename "repeated".

### fixtureIdentity

Kind: `function`. Source: `internal/workload/workload.go:146`.

```go
func fixtureIdentity(object uint64) identity.Observation
```

No declaration documentation comment is present.

### Corpus.Digest

Kind: `method`. Source: `internal/workload/workload.go:91`.

```go
func (c Corpus) Digest() [sha256.Size]byte
```

No declaration documentation comment is present.

### Corpus.ReferenceShard

Kind: `method`. Source: `internal/workload/workload.go:118`.

```go
func (c Corpus) ReferenceShard(root api.RootSpec) (*catalog.Shard, error)
```

No declaration documentation comment is present.

### workloadEncoder.string

Kind: `method`. Source: `internal/workload/workload.go:155`.

```go
func (e *workloadEncoder) string(value string)
```

No declaration documentation comment is present.

### Corpus

Kind: `struct`. Source: `internal/workload/workload.go:30`.

```go
type Corpus struct
```

No declaration documentation comment is present.

### Entry

Kind: `struct`. Source: `internal/workload/workload.go:20`.

```go
type Entry struct
```

No declaration documentation comment is present.

### workloadEncoder

Kind: `struct`. Source: `internal/workload/workload.go:150`.

```go
type workloadEncoder struct
```

No declaration documentation comment is present.
