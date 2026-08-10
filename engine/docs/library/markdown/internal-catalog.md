# filemanager/engine/internal/catalog

Status: **OBSERVED M1 reference**.

Object-plus-binding algebra, immutable reference shards, root projections, canonical serialization, and exact integrity oracle.

Package catalog owns the exhaustive, immutable M1 reference catalogue. It freezes exact object-plus-binding semantics while leaving durable block layout to the M2 comparison program.

## Invariants

- Objects are distinct from path bindings.
- Compact ordinals are generation-local.
- Filesystem observations remain authoritative.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/identity`

## Declarations

### LightModeImmediateChildren

Kind: `constant`. Source: `internal/catalog/algebra.go:8`.

```go
const LightModeImmediateChildren uint32 = 65_536
```

No declaration documentation comment is present.

### canonicalSchema

Kind: `constant`. Source: `internal/catalog/catalog.go:23`.

```go
const canonicalSchema = "fileman-reference-object-binding-v1"
```

No declaration documentation comment is present.

### Check

Kind: `function`. Source: `internal/catalog/integrity.go:10`.

```go
func Check(snapshot *Snapshot) api.IntegrityReport
```

No declaration documentation comment is present.

### NewShard

Kind: `function`. Source: `internal/catalog/catalog.go:100`.

```go
func NewShard(root api.RootSpec, rootObject Object, observations []ObservedBinding) (*Shard, error)
```

No declaration documentation comment is present.

### NewStore

Kind: `function`. Source: `internal/catalog/catalog.go:525`.

```go
func NewStore() *Store
```

No declaration documentation comment is present.

### cloneProjections

Kind: `function`. Source: `internal/catalog/catalog.go:651`.

```go
func cloneProjections(source map[api.RootID]Projection) map[api.RootID]Projection
```

No declaration documentation comment is present.

### contains

Kind: `function`. Source: `internal/catalog/catalog.go:672`.

```go
func contains(root, path string) bool
```

No declaration documentation comment is present.

### lightDirectoryCount

Kind: `function`. Source: `internal/catalog/catalog.go:195`.

```go
func lightDirectoryCount(bindings []Binding, objectCount int, threshold uint32) uint64
```

No declaration documentation comment is present.

### newSnapshot

Kind: `function`. Source: `internal/catalog/catalog.go:618`.

```go
func newSnapshot(generation api.Generation, roots map[api.RootID]Projection) *Snapshot
```

No declaration documentation comment is present.

### relativeContains

Kind: `function`. Source: `internal/catalog/catalog.go:682`.

```go
func relativeContains(root, path string) bool
```

No declaration documentation comment is present.

### sameObject

Kind: `function`. Source: `internal/catalog/catalog.go:226`.

```go
func sameObject(left, right Object) bool
```

No declaration documentation comment is present.

### sameRootSet

Kind: `function`. Source: `internal/catalog/catalog.go:659`.

```go
func sameRootSet(snapshot *Snapshot, roots []api.RootSpec) bool
```

No declaration documentation comment is present.

### validateObservedBinding

Kind: `function`. Source: `internal/catalog/catalog.go:209`.

```go
func validateObservedBinding(observation ObservedBinding) error
```

No declaration documentation comment is present.

### Projection.Manifest

Kind: `method`. Source: `internal/catalog/algebra.go:41`.

```go
func (p Projection) Manifest() RootManifest
```

No declaration documentation comment is present.

### Record.Metadata

Kind: `method`. Source: `internal/catalog/catalog.go:79`.

```go
func (r Record) Metadata() api.Metadata
```

No declaration documentation comment is present.

### Record.ObjectID

Kind: `method`. Source: `internal/catalog/catalog.go:77`.

```go
func (r Record) ObjectID() api.ObjectID
```

No declaration documentation comment is present.

### Shard.BindingAt

Kind: `method`. Source: `internal/catalog/catalog.go:252`.

```go
func (s *Shard) BindingAt(index uint32) (Binding, string, bool)
```

No declaration documentation comment is present.

### Shard.Bindings

Kind: `method`. Source: `internal/catalog/catalog.go:239`.

```go
func (s *Shard) Bindings() []Binding
```

No declaration documentation comment is present.

### Shard.CanonicalBytes

Kind: `method`. Source: `internal/catalog/catalog.go:349`.

```go
func (s *Shard) CanonicalBytes() ([]byte, error)
```

No declaration documentation comment is present.

### Shard.Digest

Kind: `method`. Source: `internal/catalog/catalog.go:234`.

```go
func (s *Shard) Digest() [sha256.Size]byte
```

No declaration documentation comment is present.

### Shard.IDOrdinalAt

Kind: `method`. Source: `internal/catalog/catalog.go:266`.

```go
func (s *Shard) IDOrdinalAt(index uint32) (uint32, bool)
```

No declaration documentation comment is present.

### Shard.IDRange

Kind: `method`. Source: `internal/catalog/catalog.go:333`.

```go
func (s *Shard) IDRange(id api.ObjectID) []uint32
```

No declaration documentation comment is present.

### Shard.Len

Kind: `method`. Source: `internal/catalog/catalog.go:232`.

```go
func (s *Shard) Len() int
```

No declaration documentation comment is present.

### Shard.LightDirectoryCount

Kind: `method`. Source: `internal/catalog/catalog.go:236`.

```go
func (s *Shard) LightDirectoryCount() uint64
```

No declaration documentation comment is present.

### Shard.NameOrdinalAt

Kind: `method`. Source: `internal/catalog/catalog.go:259`.

```go
func (s *Shard) NameOrdinalAt(index uint32) (uint32, bool)
```

No declaration documentation comment is present.

### Shard.NameRange

Kind: `method`. Source: `internal/catalog/catalog.go:303`.

```go
func (s *Shard) NameRange(name string) []uint32
```

No declaration documentation comment is present.

### Shard.ObjectAt

Kind: `method`. Source: `internal/catalog/catalog.go:245`.

```go
func (s *Shard) ObjectAt(index uint32) (Object, bool)
```

ObjectAt, BindingAt, NameOrdinalAt, and IDOrdinalAt expose generation-local values to the durable writer without constructing a second complete heap-resident mirror. The returned values are copies; their ordinals have no meaning in another generation.

### Shard.ObjectCount

Kind: `method`. Source: `internal/catalog/catalog.go:233`.

```go
func (s *Shard) ObjectCount() int
```

No declaration documentation comment is present.

### Shard.Objects

Kind: `method`. Source: `internal/catalog/catalog.go:238`.

```go
func (s *Shard) Objects() []Object
```

No declaration documentation comment is present.

### Shard.Path

Kind: `method`. Source: `internal/catalog/catalog.go:313`.

```go
func (s *Shard) Path(path string) (Record, bool)
```

No declaration documentation comment is present.

### Shard.PathIndex

Kind: `method`. Source: `internal/catalog/catalog.go:321`.

```go
func (s *Shard) PathIndex(path string) (uint32, bool)
```

No declaration documentation comment is present.

### Shard.Record

Kind: `method`. Source: `internal/catalog/catalog.go:273`.

```go
func (s *Shard) Record(index uint32) (Record, bool)
```

No declaration documentation comment is present.

### Shard.Root

Kind: `method`. Source: `internal/catalog/catalog.go:231`.

```go
func (s *Shard) Root() api.RootSpec
```

No declaration documentation comment is present.

### Shard.RootObject

Kind: `method`. Source: `internal/catalog/catalog.go:235`.

```go
func (s *Shard) RootObject() Object
```

No declaration documentation comment is present.

### Shard.Row

Kind: `method`. Source: `internal/catalog/catalog.go:280`.

```go
func (s *Shard) Row(index uint32) (Row, bool)
```

No declaration documentation comment is present.

### Shard.computeDigest

Kind: `method`. Source: `internal/catalog/catalog.go:357`.

```go
func (s *Shard) computeDigest() ([sha256.Size]byte, error)
```

No declaration documentation comment is present.

### Shard.join

Kind: `method`. Source: `internal/catalog/catalog.go:293`.

```go
func (s *Shard) join(index uint32) Record
```

No declaration documentation comment is present.

### Shard.writeCanonical

Kind: `method`. Source: `internal/catalog/catalog.go:367`.

```go
func (s *Shard) writeCanonical(output io.Writer) error
```

No declaration documentation comment is present.

### Snapshot.Owner

Kind: `method`. Source: `internal/catalog/catalog.go:467`.

```go
func (s *Snapshot) Owner(path string) (api.RootSpec, bool)
```

No declaration documentation comment is present.

### Snapshot.Owns

Kind: `method`. Source: `internal/catalog/catalog.go:482`.

```go
func (s *Snapshot) Owns(root api.RootID, path string) bool
```

No declaration documentation comment is present.

### Snapshot.OwnsProjected

Kind: `method`. Source: `internal/catalog/catalog.go:490`.

```go
func (s *Snapshot) OwnsProjected(root api.RootID, path string) bool
```

No declaration documentation comment is present.

### Snapshot.OwnsRelative

Kind: `method`. Source: `internal/catalog/catalog.go:505`.

```go
func (s *Snapshot) OwnsRelative(root api.RootID, relativePath string) bool
```

No declaration documentation comment is present.

### Snapshot.Projection

Kind: `method`. Source: `internal/catalog/catalog.go:462`.

```go
func (s *Snapshot) Projection(id api.RootID) (Projection, bool)
```

No declaration documentation comment is present.

### Store.ApplyRoots

Kind: `method`. Source: `internal/catalog/catalog.go:533`.

```go
func (s *Store) ApplyRoots(roots []api.RootSpec) (*Snapshot, bool, error)
```

No declaration documentation comment is present.

### Store.MarkStale

Kind: `method`. Source: `internal/catalog/catalog.go:601`.

```go
func (s *Store) MarkStale(root api.RootID, warning string) (*Snapshot, error)
```

No declaration documentation comment is present.

### Store.Publish

Kind: `method`. Source: `internal/catalog/catalog.go:575`.

```go
func (s *Store) Publish(root api.RootID, shard *Shard) (*Snapshot, error)
```

No declaration documentation comment is present.

### Store.Snapshot

Kind: `method`. Source: `internal/catalog/catalog.go:531`.

```go
func (s *Store) Snapshot() *Snapshot
```

No declaration documentation comment is present.

### canonicalEncoder.byte

Kind: `method`. Source: `internal/catalog/catalog.go:411`.

```go
func (e *canonicalEncoder) byte(value byte)
```

No declaration documentation comment is present.

### canonicalEncoder.i64

Kind: `method`. Source: `internal/catalog/catalog.go:426`.

```go
func (e *canonicalEncoder) i64(value int64)
```

No declaration documentation comment is present.

### canonicalEncoder.string

Kind: `method`. Source: `internal/catalog/catalog.go:428`.

```go
func (e *canonicalEncoder) string(value string)
```

No declaration documentation comment is present.

### canonicalEncoder.u32

Kind: `method`. Source: `internal/catalog/catalog.go:416`.

```go
func (e *canonicalEncoder) u32(value uint32)
```

No declaration documentation comment is present.

### canonicalEncoder.u64

Kind: `method`. Source: `internal/catalog/catalog.go:421`.

```go
func (e *canonicalEncoder) u64(value uint64)
```

No declaration documentation comment is present.

### canonicalEncoder.write

Kind: `method`. Source: `internal/catalog/catalog.go:405`.

```go
func (e *canonicalEncoder) write(value []byte)
```

No declaration documentation comment is present.

### Binding

Kind: `struct`. Source: `internal/catalog/catalog.go:47`.

```go
type Binding struct
```

Binding connects one object to one parent object under an exact name. Ordinals are valid only inside their committed generation.

### Object

Kind: `struct`. Source: `internal/catalog/catalog.go:27`.

```go
type Object struct
```

Object contains intrinsic observations for one platform object. Names and paths belong to Binding, never to Object.

### ObservationWatermark

Kind: `struct`. Source: `internal/catalog/algebra.go:13`.

```go
type ObservationWatermark struct
```

ObservationWatermark identifies the native observation boundary incorporated by a generation. M1 full scans leave Adapter and Opaque empty; event adapters must not fabricate a replay cursor where the platform supplies none.

### ObservedBinding

Kind: `struct`. Source: `internal/catalog/catalog.go:38`.

```go
type ObservedBinding struct
```

ObservedBinding is scanner input. Parent and Object are exact platform observations; RelativePath is a checked reference projection used to build the ordered path index.

### Projection

Kind: `struct`. Source: `internal/catalog/catalog.go:445`.

```go
type Projection struct
```

No declaration documentation comment is present.

### Record

Kind: `struct`. Source: `internal/catalog/catalog.go:55`.

```go
type Record struct
```

Record is a joined read view. Shards store Object, Binding, and ordered paths separately; query/ranking code receives this view only for selected rows.

### RootManifest

Kind: `struct`. Source: `internal/catalog/algebra.go:30`.

```go
type RootManifest struct
```

No declaration documentation comment is present.

### Row

Kind: `struct`. Source: `internal/catalog/catalog.go:66`.

```go
type Row struct
```

No declaration documentation comment is present.

### Shard

Kind: `struct`. Source: `internal/catalog/catalog.go:89`.

```go
type Shard struct
```

No declaration documentation comment is present.

### Snapshot

Kind: `struct`. Source: `internal/catalog/catalog.go:453`.

```go
type Snapshot struct
```

No declaration documentation comment is present.

### Store

Kind: `struct`. Source: `internal/catalog/catalog.go:520`.

```go
type Store struct
```

No declaration documentation comment is present.

### Volume

Kind: `struct`. Source: `internal/catalog/algebra.go:23`.

```go
type Volume struct
```

Volume is the lightweight routing/availability object above approved-root shards. Native is adapter evidence (currently st_dev or volume serial), not a promise of eternal cross-mount identity.

### canonicalEncoder

Kind: `struct`. Source: `internal/catalog/catalog.go:398`.

```go
type canonicalEncoder struct
```

No declaration documentation comment is present.
