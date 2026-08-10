# filemanager/engine/internal/observation

Status: **OBSERVED experimental portable core**.

Native cursor model and bounded coalescer that converts hints, gaps, and overflow into authoritative reconciliation work.

Package observation defines the portable correctness boundary between a native filesystem event adapter and authoritative catalogue reconciliation. Events are bounded hints. They never establish exact filesystem identity.

## Invariants

- Events are hints, never identity authority.
- Gaps force reconciliation.
- No durable per-event journal is introduced.

## Internal imports

- `filemanager/engine/api`

## Declarations

### KindCreate

Kind: `constant`. Source: `internal/observation/coalescer.go:42`.

```go
KindCreate Kind = 1 << iota
```

No declaration documentation comment is present.

### KindMetadata

Kind: `constant`. Source: `internal/observation/coalescer.go:44`.

```go
KindMetadata
```

No declaration documentation comment is present.

### KindRemove

Kind: `constant`. Source: `internal/observation/coalescer.go:46`.

```go
KindRemove
```

No declaration documentation comment is present.

### KindRename

Kind: `constant`. Source: `internal/observation/coalescer.go:45`.

```go
KindRename
```

No declaration documentation comment is present.

### KindRootInvalidated

Kind: `constant`. Source: `internal/observation/coalescer.go:47`.

```go
KindRootInvalidated
```

No declaration documentation comment is present.

### KindWrite

Kind: `constant`. Source: `internal/observation/coalescer.go:43`.

```go
KindWrite
```

No declaration documentation comment is present.

### validKinds

Kind: `constant`. Source: `internal/observation/coalescer.go:50`.

```go
const validKinds = KindCreate | KindWrite | KindMetadata | KindRename | KindRemove | KindRootInvalidated
```

No declaration documentation comment is present.

### DefaultLimits

Kind: `function`. Source: `internal/observation/coalescer.go:82`.

```go
func DefaultLimits() Limits
```

No declaration documentation comment is present.

### New

Kind: `function`. Source: `internal/observation/coalescer.go:168`.

```go
func New(limits Limits) (*Coalescer, error)
```

No declaration documentation comment is present.

### changeCost

Kind: `function`. Source: `internal/observation/coalescer.go:357`.

```go
func changeCost(key string, change Change) uint64
```

No declaration documentation comment is present.

### cloneCursor

Kind: `function`. Source: `internal/observation/coalescer.go:481`.

```go
func cloneCursor(cursor Cursor) Cursor
```

No declaration documentation comment is present.

### saturatingAdd

Kind: `function`. Source: `internal/observation/coalescer.go:487`.

```go
func saturatingAdd(left, right uint64) uint64
```

No declaration documentation comment is present.

### validRelativePath

Kind: `function`. Source: `internal/observation/coalescer.go:323`.

```go
func validRelativePath(value string) bool
```

No declaration documentation comment is present.

### validateBatch

Kind: `function`. Source: `internal/observation/coalescer.go:276`.

```go
func validateBatch(batch Batch, limits Limits) error
```

No declaration documentation comment is present.

### validateEvent

Kind: `function`. Source: `internal/observation/coalescer.go:297`.

```go
func validateEvent(event Event) error
```

No declaration documentation comment is present.

### Adapter

Kind: `interface`. Source: `internal/observation/adapter.go:16`.

```go
type Adapter interface
```

Adapter translates one native platform event source into portable chained batches. Platform-specific overflow, root replacement, and journal reset conditions must set Batch.Discontinuity; events remain hints only.

### CoverageReporter

Kind: `interface`. Source: `internal/observation/adapter.go:27`.

```go
type CoverageReporter interface
```

No declaration documentation comment is present.

### Coalescer.Deadline

Kind: `method`. Source: `internal/observation/coalescer.go:385`.

```go
func (c *Coalescer) Deadline() (time.Time, bool)
```

Deadline returns the next time work becomes due. A false result means the quiet coalescer needs no timer or polling loop.

### Coalescer.Due

Kind: `method`. Source: `internal/observation/coalescer.go:370`.

```go
func (c *Coalescer) Due(now time.Time) bool
```

No declaration documentation comment is present.

### Coalescer.Finish

Kind: `method`. Source: `internal/observation/coalescer.go:434`.

```go
func (c *Coalescer) Finish(workID uint64, success bool, failureReason string) error
```

Finish advances only a volatile reconciliation cursor. Persistence requires the generation manifest to commit this cursor with all exact components.

### Coalescer.Ingest

Kind: `method`. Source: `internal/observation/coalescer.go:206`.

```go
func (c *Coalescer) Ingest(now time.Time, batch Batch) (Outcome, error)
```

No declaration documentation comment is present.

### Coalescer.Initialize

Kind: `method`. Source: `internal/observation/coalescer.go:178`.

```go
func (c *Coalescer) Initialize(initial Cursor, requireReconcile bool, reason string) error
```

Initialize establishes the adapter subscription boundary. The service uses requireReconcile on startup until a cursor is committed atomically with an exact generation; an in-memory cursor must never imply restart currentness.

### Coalescer.RequireReconcile

Kind: `method`. Source: `internal/observation/coalescer.go:193`.

```go
func (c *Coalescer) RequireReconcile(reason string)
```

No declaration documentation comment is present.

### Coalescer.Snapshot

Kind: `method`. Source: `internal/observation/coalescer.go:460`.

```go
func (c *Coalescer) Snapshot() Snapshot
```

No declaration documentation comment is present.

### Coalescer.Take

Kind: `method`. Source: `internal/observation/coalescer.go:401`.

```go
func (c *Coalescer) Take(now time.Time, force bool) (Work, bool)
```

No declaration documentation comment is present.

### Coalescer.merge

Kind: `method`. Source: `internal/observation/coalescer.go:331`.

```go
func (c *Coalescer) merge(next Change) bool
```

No declaration documentation comment is present.

### Coalescer.overflow

Kind: `method`. Source: `internal/observation/coalescer.go:363`.

```go
func (c *Coalescer) overflow()
```

No declaration documentation comment is present.

### Coalescer.requireReconcile

Kind: `method`. Source: `internal/observation/coalescer.go:197`.

```go
func (c *Coalescer) requireReconcile(reason string)
```

No declaration documentation comment is present.

### Cursor.sameStream

Kind: `method`. Source: `internal/observation/coalescer.go:35`.

```go
func (c Cursor) sameStream(other Cursor) bool
```

No declaration documentation comment is present.

### Cursor.valid

Kind: `method`. Source: `internal/observation/coalescer.go:33`.

```go
func (c Cursor) valid() bool
```

No declaration documentation comment is present.

### Limits.validate

Kind: `method`. Source: `internal/observation/coalescer.go:92`.

```go
func (l Limits) validate() error
```

No declaration documentation comment is present.

### Batch

Kind: `struct`. Source: `internal/observation/coalescer.go:65`.

```go
type Batch struct
```

Batch is a chained adapter delivery. After must equal the last accepted Through cursor. This works with sparse native journal positions and makes a missing delivery explicit without assuming Position+1 continuity.

### Change

Kind: `struct`. Source: `internal/observation/coalescer.go:101`.

```go
type Change struct
```

Change is the bounded net hint retained for one root-relative address. Kinds is a history mask, not an assertion about final filesystem state.

### Coalescer

Kind: `struct`. Source: `internal/observation/coalescer.go:147`.

```go
type Coalescer struct
```

Coalescer is deliberately not internally synchronized. Its owner can keep ingestion, status snapshots, and work completion under one small mutex.

### Coverage

Kind: `struct`. Source: `internal/observation/adapter.go:22`.

```go
type Coverage struct
```

Coverage describes whether an adapter can support an exact-current claim after reconciliation. Omitted CoverageReporter fails closed as incomplete.

### Cursor

Kind: `struct`. Source: `internal/observation/coalescer.go:27`.

```go
type Cursor struct
```

Cursor is an adapter-owned position in one native observation stream. Epoch changes when an adapter can no longer compare positions with the prior stream. Position is ordered within an epoch but need not be contiguous.

### Event

Kind: `struct`. Source: `internal/observation/coalescer.go:55`.

```go
type Event struct
```

Event uses slash-separated paths relative to its approved root. A rename names both addresses. Coalescing preserves both sides as dirty paths; exact identity and final existence are resolved only by a filesystem scan.

### Limits

Kind: `struct`. Source: `internal/observation/coalescer.go:74`.

```go
type Limits struct
```

Limits bounds both adapter handoff and retained coalescer state. MaxOperations is a flush trigger; MaxChanges and MaxBytes are hard resident-state limits.

### Outcome

Kind: `struct`. Source: `internal/observation/coalescer.go:108`.

```go
type Outcome struct
```

No declaration documentation comment is present.

### Snapshot

Kind: `struct`. Source: `internal/observation/coalescer.go:130`.

```go
type Snapshot struct
```

No declaration documentation comment is present.

### Subscription

Kind: `struct`. Source: `internal/observation/adapter.go:8`.

```go
type Subscription struct
```

Subscription establishes an ordered journal boundary before any delivered batch. Closing Batches while ctx remains live means observation became unavailable; it is not evidence of an empty backlog.

### Work

Kind: `struct`. Source: `internal/observation/coalescer.go:119`.

```go
type Work struct
```

No declaration documentation comment is present.

### Kind

Kind: `type`. Source: `internal/observation/coalescer.go:39`.

```go
type Kind uint16
```

No declaration documentation comment is present.

### ErrInvalidBatch

Kind: `variable`. Source: `internal/observation/coalescer.go:20`.

```go
ErrInvalidBatch  = errors.New("invalid observation batch")
```

No declaration documentation comment is present.

### ErrInvalidCursor

Kind: `variable`. Source: `internal/observation/coalescer.go:19`.

```go
ErrInvalidCursor = errors.New("invalid observation cursor")
```

No declaration documentation comment is present.

### ErrWorkMismatch

Kind: `variable`. Source: `internal/observation/coalescer.go:21`.

```go
ErrWorkMismatch  = errors.New("observation work does not match the active batch")
```

No declaration documentation comment is present.
