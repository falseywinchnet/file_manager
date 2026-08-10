# filemanager/engine/internal/observation/rdcw

Status: **OBSERVED compatibility slice; native NTFS promotion red**.

Windows ReadDirectoryChangesW subscription projected into the portable cursor/batch model.

Package rdcw provides the experimental Windows ReadDirectoryChangesW observation adapter. Other platforms expose an unavailable constructor.

## Invariants

- Compatibility evidence is not native NTFS promotion evidence.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/observation`

## Declarations

### fileFlagBackupSemantics

Kind: `constant`. Source: `internal/observation/rdcw/rdcw_windows.go:27`.

```go
fileFlagBackupSemantics = 0x02000000
```

No declaration documentation comment is present.

### fileFlagOverlapped

Kind: `constant`. Source: `internal/observation/rdcw/rdcw_windows.go:28`.

```go
fileFlagOverlapped      = 0x40000000
```

No declaration documentation comment is present.

### fileListDirectory

Kind: `constant`. Source: `internal/observation/rdcw/rdcw_windows.go:23`.

```go
fileListDirectory       = 0x0001
```

No declaration documentation comment is present.

### fileShareDelete

Kind: `constant`. Source: `internal/observation/rdcw/rdcw_windows.go:26`.

```go
fileShareDelete         = 0x0004
```

No declaration documentation comment is present.

### fileShareRead

Kind: `constant`. Source: `internal/observation/rdcw/rdcw_windows.go:24`.

```go
fileShareRead           = 0x0001
```

No declaration documentation comment is present.

### fileShareWrite

Kind: `constant`. Source: `internal/observation/rdcw/rdcw_windows.go:25`.

```go
fileShareWrite          = 0x0002
```

No declaration documentation comment is present.

### infinite

Kind: `constant`. Source: `internal/observation/rdcw/rdcw_windows.go:29`.

```go
infinite                = 0xffffffff
```

No declaration documentation comment is present.

### DefaultConfig

Kind: `function`. Source: `internal/observation/rdcw/config.go:16`.

```go
func DefaultConfig() Config
```

No declaration documentation comment is present.

### New

Kind: `function`. Source: `internal/observation/rdcw/rdcw_stub.go:10`.

```go
func New([]api.RootSpec, Config) (observation.Adapter, error)
```

No declaration documentation comment is present.

### New

Kind: `function`. Source: `internal/observation/rdcw/rdcw_windows.go:50`.

```go
func New(roots []api.RootSpec, config Config) (observation.Adapter, error)
```

No declaration documentation comment is present.

### deliver

Kind: `function`. Source: `internal/observation/rdcw/rdcw_windows.go:228`.

```go
func deliver( ctx context.Context, initial observation.Cursor, raw <-chan rawBatch, output chan<- observation.Batch, dropWake <-chan struct
```

No declaration documentation comment is present.

### issueRead

Kind: `function`. Source: `internal/observation/rdcw/rdcw_windows.go:151`.

```go
func issueRead(item *watcher) error
```

No declaration documentation comment is present.

### newEpoch

Kind: `function`. Source: `internal/observation/rdcw/rdcw_windows.go:333`.

```go
func newEpoch() (string, error)
```

No declaration documentation comment is present.

### parseNotifications

Kind: `function`. Source: `internal/observation/rdcw/rdcw_windows.go:278`.

```go
func parseNotifications(root api.RootID, buffer []byte, bytes uint32, maxEvents int) ([]observation.Event, bool)
```

No declaration documentation comment is present.

### sendBatch

Kind: `function`. Source: `internal/observation/rdcw/rdcw_windows.go:269`.

```go
func sendBatch(ctx context.Context, output chan<- observation.Batch, batch observation.Batch) bool
```

No declaration documentation comment is present.

### Adapter.ObservationCoverage

Kind: `method`. Source: `internal/observation/rdcw/rdcw_windows.go:78`.

```go
func (*Adapter) ObservationCoverage() observation.Coverage
```

No declaration documentation comment is present.

### Adapter.Subscribe

Kind: `method`. Source: `internal/observation/rdcw/rdcw_windows.go:88`.

```go
func (a *Adapter) Subscribe(ctx context.Context) (observation.Subscription, error)
```

No declaration documentation comment is present.

### Adapter.offer

Kind: `method`. Source: `internal/observation/rdcw/rdcw_windows.go:216`.

```go
func (a *Adapter) offer(raw chan<- rawBatch, batch rawBatch, dropWake chan<- struct
```

No declaration documentation comment is present.

### Adapter.openWatcher

Kind: `method`. Source: `internal/observation/rdcw/rdcw_windows.go:132`.

```go
func (a *Adapter) openWatcher(root api.RootSpec, port syscall.Handle, key uint32) (*watcher, error)
```

No declaration documentation comment is present.

### Adapter.runCompletions

Kind: `method`. Source: `internal/observation/rdcw/rdcw_windows.go:166`.

```go
func (a *Adapter) runCompletions( ctx context.Context, port syscall.Handle, watchers []*watcher, raw chan<- rawBatch, dropWake chan<- struct
```

No declaration documentation comment is present.

### Config.validate

Kind: `method`. Source: `internal/observation/rdcw/config.go:25`.

```go
func (c Config) validate() error
```

No declaration documentation comment is present.

### Adapter

Kind: `struct`. Source: `internal/observation/rdcw/rdcw_windows.go:32`.

```go
type Adapter struct
```

No declaration documentation comment is present.

### Config

Kind: `struct`. Source: `internal/observation/rdcw/config.go:9`.

```go
type Config struct
```

No declaration documentation comment is present.

### rawBatch

Kind: `struct`. Source: `internal/observation/rdcw/rdcw_windows.go:45`.

```go
type rawBatch struct
```

No declaration documentation comment is present.

### watcher

Kind: `struct`. Source: `internal/observation/rdcw/rdcw_windows.go:38`.

```go
type watcher struct
```

No declaration documentation comment is present.

### ErrUnavailable

Kind: `variable`. Source: `internal/observation/rdcw/config.go:7`.

```go
var ErrUnavailable = errors.New("Windows ReadDirectoryChangesW observation adapter is unavailable")
```

No declaration documentation comment is present.

### _

Kind: `variable`. Source: `internal/observation/rdcw/rdcw_windows.go:341`.

```go
var _ observation.Adapter = (*Adapter)(nil)
```

No declaration documentation comment is present.

### _

Kind: `variable`. Source: `internal/observation/rdcw/rdcw_windows.go:342`.

```go
var _ observation.CoverageReporter = (*Adapter)(nil)
```

No declaration documentation comment is present.
