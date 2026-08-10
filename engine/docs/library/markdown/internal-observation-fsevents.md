# filemanager/engine/internal/observation/fsevents

Status: **OBSERVED experimental macOS adapter; coverage incomplete**.

FSEvents subscription projected into the portable cursor/batch model with explicit coverage limitation.

Package fsevents provides the experimental macOS native observation adapter. Other platforms expose the same constructor as explicitly unavailable.

## Invariants

- Incomplete final-hard-link removal coverage prevents an exact-current claim.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/observation`

## Declarations

### DefaultConfig

Kind: `function`. Source: `internal/observation/fsevents/config.go:20`.

```go
func DefaultConfig() Config
```

No declaration documentation comment is present.

### New

Kind: `function`. Source: `internal/observation/fsevents/fsevents_stub.go:10`.

```go
func New([]api.RootSpec, Config) (observation.Adapter, error)
```

No declaration documentation comment is present.

### New

Kind: `function`. Source: `internal/observation/fsevents/fsevents_darwin.go:88`.

```go
func New(roots []api.RootSpec, config Config) (observation.Adapter, error)
```

No declaration documentation comment is present.

### adapterFromHandle

Kind: `function`. Source: `internal/observation/fsevents/fsevents_darwin.go:235`.

```go
func adapterFromHandle(token uintptr) (adapter *Adapter, ok bool)
```

No declaration documentation comment is present.

### goFilemanFSEventsCallback

Kind: `function`. Source: `internal/observation/fsevents/fsevents_darwin.go:198`.

```go
func goFilemanFSEventsCallback(token C.uintptr_t, count C.size_t, rawPaths, rawFlags unsafe.Pointer, through C.uint64_t)
```

export goFilemanFSEventsCallback

### newEpoch

Kind: `function`. Source: `internal/observation/fsevents/fsevents_darwin.go:369`.

```go
func newEpoch() (string, error)
```

No declaration documentation comment is present.

### sendBatch

Kind: `function`. Source: `internal/observation/fsevents/fsevents_darwin.go:310`.

```go
func sendBatch(ctx context.Context, output chan<- observation.Batch, batch observation.Batch) bool
```

No declaration documentation comment is present.

### Adapter.ObservationCoverage

Kind: `method`. Source: `internal/observation/fsevents/fsevents_darwin.go:115`.

```go
func (a *Adapter) ObservationCoverage() observation.Coverage
```

No declaration documentation comment is present.

### Adapter.Subscribe

Kind: `method`. Source: `internal/observation/fsevents/fsevents_darwin.go:125`.

```go
func (a *Adapter) Subscribe(ctx context.Context) (observation.Subscription, error)
```

No declaration documentation comment is present.

### Adapter.deliver

Kind: `method`. Source: `internal/observation/fsevents/fsevents_darwin.go:263`.

```go
func (a *Adapter) deliver(ctx context.Context, initial observation.Cursor, output chan<- observation.Batch)
```

No declaration documentation comment is present.

### Adapter.ownedPath

Kind: `method`. Source: `internal/observation/fsevents/fsevents_darwin.go:357`.

```go
func (a *Adapter) ownedPath(value string) (api.RootSpec, string, bool)
```

No declaration documentation comment is present.

### Adapter.recordDrop

Kind: `method`. Source: `internal/observation/fsevents/fsevents_darwin.go:249`.

```go
func (a *Adapter) recordDrop(position uint64)
```

No declaration documentation comment is present.

### Adapter.runNative

Kind: `method`. Source: `internal/observation/fsevents/fsevents_darwin.go:147`.

```go
func (a *Adapter) runNative(ctx context.Context, since uint64, started chan<- error)
```

No declaration documentation comment is present.

### Adapter.translate

Kind: `method`. Source: `internal/observation/fsevents/fsevents_darwin.go:319`.

```go
func (a *Adapter) translate(items []nativeItem) ([]observation.Event, bool)
```

No declaration documentation comment is present.

### Config.validate

Kind: `method`. Source: `internal/observation/fsevents/config.go:27`.

```go
func (c Config) validate() error
```

No declaration documentation comment is present.

### Adapter

Kind: `struct`. Source: `internal/observation/fsevents/fsevents_darwin.go:66`.

```go
type Adapter struct
```

No declaration documentation comment is present.

### Config

Kind: `struct`. Source: `internal/observation/fsevents/config.go:12`.

```go
type Config struct
```

No declaration documentation comment is present.

### nativeBatch

Kind: `struct`. Source: `internal/observation/fsevents/fsevents_darwin.go:77`.

```go
type nativeBatch struct
```

No declaration documentation comment is present.

### nativeItem

Kind: `struct`. Source: `internal/observation/fsevents/fsevents_darwin.go:83`.

```go
type nativeItem struct
```

No declaration documentation comment is present.

### ErrUnavailable

Kind: `variable`. Source: `internal/observation/fsevents/config.go:10`.

```go
var ErrUnavailable = errors.New("macOS FSEvents observation adapter is unavailable")
```

No declaration documentation comment is present.

### _

Kind: `variable`. Source: `internal/observation/fsevents/fsevents_darwin.go:377`.

```go
var _ observation.Adapter = (*Adapter)(nil)
```

No declaration documentation comment is present.

### _

Kind: `variable`. Source: `internal/observation/fsevents/fsevents_darwin.go:378`.

```go
var _ observation.CoverageReporter = (*Adapter)(nil)
```

No declaration documentation comment is present.
