# filemanager/engine/internal/live

Status: **OBSERVED ORC-ENG-004 development slice**.

Bounded metadata-only filesystem search with expiring process-local sessions, discovery-order pages, and explicit partial state.

Package live performs bounded, metadata-only filesystem search without constructing, consulting, or persisting a catalogue.

## Invariants

- No catalogue is created or consulted.
- An excluded starting scope and a replaced installed root fail closed.
- Directory links are returned but not traversed.
- State is bounded by depth, sessions, work, results, time, and bytes.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/identity`

## Declarations

### DefaultMaxOpenDirectories

Kind: `constant`. Source: `internal/live/manager.go:30`.

```go
DefaultMaxOpenDirectories = 8
```

No declaration documentation comment is present.

### DefaultMaxResponseBytes

Kind: `constant`. Source: `internal/live/manager.go:32`.

```go
DefaultMaxResponseBytes   = 256 * 1024
```

No declaration documentation comment is present.

### DefaultMaxResults

Kind: `constant`. Source: `internal/live/manager.go:22`.

```go
DefaultMaxResults         = 128
```

No declaration documentation comment is present.

### DefaultMaxStatCalls

Kind: `constant`. Source: `internal/live/manager.go:26`.

```go
DefaultMaxStatCalls       = 4_096
```

No declaration documentation comment is present.

### DefaultMaxVisitedEntries

Kind: `constant`. Source: `internal/live/manager.go:24`.

```go
DefaultMaxVisitedEntries  = 100_000
```

No declaration documentation comment is present.

### DefaultMaxWallTimeMS

Kind: `constant`. Source: `internal/live/manager.go:28`.

```go
DefaultMaxWallTimeMS      = 250
```

No declaration documentation comment is present.

### HardMaxOpenDirectories

Kind: `constant`. Source: `internal/live/manager.go:31`.

```go
HardMaxOpenDirectories    = 32
```

No declaration documentation comment is present.

### HardMaxResponseBytes

Kind: `constant`. Source: `internal/live/manager.go:33`.

```go
HardMaxResponseBytes      = 1024 * 1024
```

No declaration documentation comment is present.

### HardMaxResults

Kind: `constant`. Source: `internal/live/manager.go:23`.

```go
HardMaxResults            = 1_000
```

No declaration documentation comment is present.

### HardMaxStatCalls

Kind: `constant`. Source: `internal/live/manager.go:27`.

```go
HardMaxStatCalls          = 65_536
```

No declaration documentation comment is present.

### HardMaxVisitedEntries

Kind: `constant`. Source: `internal/live/manager.go:25`.

```go
HardMaxVisitedEntries     = 1_000_000
```

No declaration documentation comment is present.

### HardMaxWallTimeMS

Kind: `constant`. Source: `internal/live/manager.go:29`.

```go
HardMaxWallTimeMS         = 5_000
```

No declaration documentation comment is present.

### maxCursorBytes

Kind: `constant`. Source: `internal/live/manager.go:42`.

```go
maxCursorBytes            = 128
```

No declaration documentation comment is present.

### maxQueryIDBytes

Kind: `constant`. Source: `internal/live/manager.go:38`.

```go
maxQueryIDBytes           = 128
```

No declaration documentation comment is present.

### maxQueryTextBytes

Kind: `constant`. Source: `internal/live/manager.go:40`.

```go
maxQueryTextBytes         = 4_096
```

No declaration documentation comment is present.

### maxRelativePathBytes

Kind: `constant`. Source: `internal/live/manager.go:41`.

```go
maxRelativePathBytes      = 32_768
```

No declaration documentation comment is present.

### maxRootIDBytes

Kind: `constant`. Source: `internal/live/manager.go:39`.

```go
maxRootIDBytes            = 256
```

No declaration documentation comment is present.

### maxSessions

Kind: `constant`. Source: `internal/live/manager.go:34`.

```go
maxSessions               = 32
```

No declaration documentation comment is present.

### maxUnavailablePaths

Kind: `constant`. Source: `internal/live/manager.go:35`.

```go
maxUnavailablePaths       = 64
```

No declaration documentation comment is present.

### resultByteAllowance

Kind: `constant`. Source: `internal/live/manager.go:37`.

```go
resultByteAllowance       = 512
```

No declaration documentation comment is present.

### sessionTTL

Kind: `constant`. Source: `internal/live/manager.go:36`.

```go
sessionTTL                = 30 * time.Second
```

No declaration documentation comment is present.

### NewManager

Kind: `function`. Source: `internal/live/manager.go:84`.

```go
func NewManager() *Manager
```

No declaration documentation comment is present.

### clampBudget

Kind: `function`. Source: `internal/live/manager.go:185`.

```go
func clampBudget(request api.LiveQueryBudget) (api.LiveQueryBudget, error)
```

No declaration documentation comment is present.

### displayPath

Kind: `function`. Source: `internal/live/manager.go:475`.

```go
func displayPath(path string) string
```

No declaration documentation comment is present.

### newSession

Kind: `function`. Source: `internal/live/manager.go:207`.

```go
func newSession(root api.RootSpec, expectedObjectID string, query api.LiveQuery, now time.Time) (*session, error)
```

No declaration documentation comment is present.

### randomID

Kind: `function`. Source: `internal/live/manager.go:264`.

```go
func randomID() (string, error)
```

No declaration documentation comment is present.

### resultFor

Kind: `function`. Source: `internal/live/manager.go:381`.

```go
func resultFor(root api.RootSpec, relative, name string, info os.FileInfo, observed identity.Observation, rank int) api.Result
```

No declaration documentation comment is present.

### validateQuery

Kind: `function`. Source: `internal/live/manager.go:167`.

```go
func validateQuery(query api.LiveQuery) error
```

No declaration documentation comment is present.

### Manager.Close

Kind: `method`. Source: `internal/live/manager.go:100`.

```go
func (m *Manager) Close()
```

No declaration documentation comment is present.

### Manager.Query

Kind: `method`. Source: `internal/live/manager.go:102`.

```go
func (m *Manager) Query(ctx context.Context, root api.RootSpec, expectedObjectID string, query api.LiveQuery, owns OwnsFunc) (api.LiveQueryResponse, error)
```

No declaration documentation comment is present.

### Manager.Reset

Kind: `method`. Source: `internal/live/manager.go:88`.

```go
func (m *Manager) Reset()
```

No declaration documentation comment is present.

### Manager.expire

Kind: `method`. Source: `internal/live/manager.go:463`.

```go
func (m *Manager) expire(now time.Time)
```

No declaration documentation comment is present.

### Manager.expireToken

Kind: `method`. Source: `internal/live/manager.go:444`.

```go
func (m *Manager) expireToken(token string)
```

No declaration documentation comment is present.

### session.close

Kind: `method`. Source: `internal/live/manager.go:429`.

```go
func (s *session) close()
```

No declaration documentation comment is present.

### session.matches

Kind: `method`. Source: `internal/live/manager.go:272`.

```go
func (s *session) matches(query api.LiveQuery) bool
```

No declaration documentation comment is present.

### session.page

Kind: `method`. Source: `internal/live/manager.go:278`.

```go
func (s *session) page(ctx context.Context, budget api.LiveQueryBudget, owns OwnsFunc, started time.Time) (api.LiveQueryResponse, bool, error)
```

No declaration documentation comment is present.

### session.response

Kind: `method`. Source: `internal/live/manager.go:400`.

```go
func (s *session) response(results []api.Result, visited, stats uint64, started time.Time, complete bool) api.LiveQueryResponse
```

No declaration documentation comment is present.

### session.unavailablePath

Kind: `method`. Source: `internal/live/manager.go:421`.

```go
func (s *session) unavailablePath(path string)
```

No declaration documentation comment is present.

### session.warn

Kind: `method`. Source: `internal/live/manager.go:413`.

```go
func (s *session) warn(message string)
```

No declaration documentation comment is present.

### Manager

Kind: `struct`. Source: `internal/live/manager.go:47`.

```go
type Manager struct
```

No declaration documentation comment is present.

### directoryFrame

Kind: `struct`. Source: `internal/live/manager.go:73`.

```go
type directoryFrame struct
```

No declaration documentation comment is present.

### pendingEntry

Kind: `struct`. Source: `internal/live/manager.go:78`.

```go
type pendingEntry struct
```

No declaration documentation comment is present.

### session

Kind: `struct`. Source: `internal/live/manager.go:53`.

```go
type session struct
```

No declaration documentation comment is present.

### OwnsFunc

Kind: `type`. Source: `internal/live/manager.go:45`.

```go
type OwnsFunc func(api.RootID, string) bool
```

No declaration documentation comment is present.
