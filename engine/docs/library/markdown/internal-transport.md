# filemanager/engine/internal/transport

Status: **OBSERVED development JSONL and M4 authenticated local projections**.

Decodes bounded requests, separates query/admin authorities, validates same-uid peers and rotated credentials, caps connections and time, dispatches canonical methods, and supports an installed acknowledgement hook without importing deployment policy.

Package transport projects the engine object model onto newline-delimited JSON. It contains no catalogue or ranking logic.

## Invariants

- Every JSONL or ENG1 payload is capped at 1 MiB.
- Query/admin accepted connections are capped at 32/4 and idle sockets cannot block shutdown.
- A failed duplicate startup cannot rotate live credentials.
- Installed acknowledgement failure is returned as a redacted fault.
- Transport contains no catalogue or ranking logic.
- Query and admin endpoints cannot exchange authority.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/sandbox`
- `filemanager/engine/internal/service`

## Declarations

### AuthorityAdmin

Kind: `constant`. Source: `internal/transport/jsonl.go:29`.

```go
AuthorityAdmin       Authority = "admin"
```

No declaration documentation comment is present.

### AuthorityDevelopment

Kind: `constant`. Source: `internal/transport/jsonl.go:27`.

```go
AuthorityDevelopment Authority = "development"
```

No declaration documentation comment is present.

### AuthorityQuery

Kind: `constant`. Source: `internal/transport/jsonl.go:28`.

```go
AuthorityQuery       Authority = "query"
```

No declaration documentation comment is present.

### LocalProtocol

Kind: `constant`. Source: `internal/transport/local.go:25`.

```go
LocalProtocol = "engine.local.v1"
```

No declaration documentation comment is present.

### MaxJSONLFrameBytes

Kind: `constant`. Source: `internal/transport/jsonl.go:32`.

```go
const MaxJSONLFrameBytes = 1_048_576
```

No declaration documentation comment is present.

### frameHeader

Kind: `constant`. Source: `internal/transport/local.go:27`.

```go
frameHeader   = 8
```

No declaration documentation comment is present.

### frameMagic

Kind: `constant`. Source: `internal/transport/local.go:26`.

```go
frameMagic    = "ENG1"
```

No declaration documentation comment is present.

### handshakeTimeout

Kind: `constant`. Source: `internal/transport/local.go:31`.

```go
handshakeTimeout    = 5 * time.Second
```

No declaration documentation comment is present.

### idleFrameTimeout

Kind: `constant`. Source: `internal/transport/local.go:32`.

```go
idleFrameTimeout    = 30 * time.Second
```

No declaration documentation comment is present.

### maxAdminConnections

Kind: `constant`. Source: `internal/transport/local.go:30`.

```go
maxAdminConnections = 4
```

No declaration documentation comment is present.

### maxQueryConnections

Kind: `constant`. Source: `internal/transport/local.go:29`.

```go
maxQueryConnections = 32
```

No declaration documentation comment is present.

### requestTimeout

Kind: `constant`. Source: `internal/transport/local.go:33`.

```go
requestTimeout      = 30 * time.Second
```

No declaration documentation comment is present.

### writeTimeout

Kind: `constant`. Source: `internal/transport/local.go:34`.

```go
writeTimeout        = 5 * time.Second
```

No declaration documentation comment is present.

### CallLocal

Kind: `function`. Source: `internal/transport/local.go:293`.

```go
func CallLocal(ctx context.Context, runtimeDir string, authority Authority, request Request) (Response, error)
```

No declaration documentation comment is present.

### Files

Kind: `function`. Source: `internal/transport/local.go:76`.

```go
func Files(runtimeDir string) EndpointFiles
```

No declaration documentation comment is present.

### Serve

Kind: `function`. Source: `internal/transport/jsonl.go:59`.

```go
func Serve(ctx context.Context, input io.Reader, output io.Writer, engine *service.Service) error
```

No declaration documentation comment is present.

### ServeLocal

Kind: `function`. Source: `internal/transport/local.go:87`.

```go
func ServeLocal(ctx context.Context, runtimeDir string, engine *service.Service) error
```

ServeLocal creates distinct same-user query/admin endpoints. Credentials are random per process instance and are never accepted on the other endpoint.

### ServeLocalWithOptions

Kind: `function`. Source: `internal/transport/local.go:91`.

```go
func ServeLocalWithOptions(ctx context.Context, runtimeDir string, engine *service.Service, options LocalOptions) error
```

No declaration documentation comment is present.

### cleanupRuntime

Kind: `function`. Source: `internal/transport/local.go:494`.

```go
func cleanupRuntime(files EndpointFiles)
```

No declaration documentation comment is present.

### constantToken

Kind: `function`. Source: `internal/transport/local.go:419`.

```go
func constantToken(left, right string) bool
```

No declaration documentation comment is present.

### decodeParams

Kind: `function`. Source: `internal/transport/jsonl.go:210`.

```go
func decodeParams(raw json.RawMessage, target any) error
```

No declaration documentation comment is present.

### dispatch

Kind: `function`. Source: `internal/transport/jsonl.go:85`.

```go
func dispatch(ctx context.Context, engine *service.Service, incoming Request) (Response, bool)
```

No declaration documentation comment is present.

### dispatchAuthorized

Kind: `function`. Source: `internal/transport/jsonl.go:89`.

```go
func dispatchAuthorized(ctx context.Context, engine *service.Service, incoming Request, authority Authority) (Response, bool)
```

No declaration documentation comment is present.

### handleLocal

Kind: `function`. Source: `internal/transport/local.go:238`.

```go
func handleLocal(ctx context.Context, connection net.Conn, engine *service.Service, authority Authority, token string, shutdown func(), options LocalOptions)
```

No declaration documentation comment is present.

### listenPrivate

Kind: `function`. Source: `internal/transport/local.go:434`.

```go
func listenPrivate(path string) (net.Listener, error)
```

No declaration documentation comment is present.

### methodAllowed

Kind: `function`. Source: `internal/transport/jsonl.go:181`.

```go
func methodAllowed(authority Authority, method string) bool
```

No declaration documentation comment is present.

### newConnectionSet

Kind: `function`. Source: `internal/transport/local.go:210`.

```go
func newConnectionSet() *connectionSet
```

No declaration documentation comment is present.

### peerUID

Kind: `function`. Source: `internal/transport/peer_darwin_nocgo.go:10`.

```go
func peerUID(net.Conn) (int, error)
```

No declaration documentation comment is present.

### peerUID

Kind: `function`. Source: `internal/transport/peer_other.go:10`.

```go
func peerUID(net.Conn) (int, error)
```

No declaration documentation comment is present.

### peerUID

Kind: `function`. Source: `internal/transport/peer_linux.go:11`.

```go
func peerUID(connection net.Conn) (int, error)
```

No declaration documentation comment is present.

### peerUID

Kind: `function`. Source: `internal/transport/peer_darwin.go:21`.

```go
func peerUID(connection net.Conn) (int, error)
```

No declaration documentation comment is present.

### publicFault

Kind: `function`. Source: `internal/transport/jsonl.go:228`.

```go
func publicFault(err error) *Fault
```

No declaration documentation comment is present.

### readFrame

Kind: `function`. Source: `internal/transport/local.go:334`.

```go
func readFrame(reader io.Reader, target any) error
```

No declaration documentation comment is present.

### readToken

Kind: `function`. Source: `internal/transport/local.go:400`.

```go
func readToken(path string) (string, error)
```

No declaration documentation comment is present.

### rotateToken

Kind: `function`. Source: `internal/transport/local.go:388`.

```go
func rotateToken(path string) (string, error)
```

No declaration documentation comment is present.

### serveEndpoint

Kind: `function`. Source: `internal/transport/local.go:163`.

```go
func serveEndpoint( ctx context.Context, listener net.Listener, engine *service.Service, authority Authority, token string, maximumConnections int, connections *connectionSet, handlers *sync.WaitGroup, shutdown func(), errorsOut chan<- error, options LocalOptions, )
```

No declaration documentation comment is present.

### verifyRuntimeDirectory

Kind: `function`. Source: `internal/transport/local.go:423`.

```go
func verifyRuntimeDirectory(path string) error
```

No declaration documentation comment is present.

### writeFrame

Kind: `function`. Source: `internal/transport/local.go:361`.

```go
func writeFrame(writer io.Writer, value any) error
```

No declaration documentation comment is present.

### writePrivate

Kind: `function`. Source: `internal/transport/local.go:470`.

```go
func writePrivate(path string, payload []byte) error
```

No declaration documentation comment is present.

### writePrivateJSON

Kind: `function`. Source: `internal/transport/local.go:461`.

```go
func writePrivateJSON(path string, value any) error
```

No declaration documentation comment is present.

### connectionSet.Add

Kind: `method`. Source: `internal/transport/local.go:214`.

```go
func (s *connectionSet) Add(connection net.Conn)
```

No declaration documentation comment is present.

### connectionSet.CloseAll

Kind: `method`. Source: `internal/transport/local.go:226`.

```go
func (s *connectionSet) CloseAll()
```

No declaration documentation comment is present.

### connectionSet.Remove

Kind: `method`. Source: `internal/transport/local.go:220`.

```go
func (s *connectionSet) Remove(connection net.Conn)
```

No declaration documentation comment is present.

### Discovery

Kind: `struct`. Source: `internal/transport/local.go:46`.

```go
type Discovery struct
```

No declaration documentation comment is present.

### EndpointFiles

Kind: `struct`. Source: `internal/transport/local.go:37`.

```go
type EndpointFiles struct
```

No declaration documentation comment is present.

### Fault

Kind: `struct`. Source: `internal/transport/jsonl.go:40`.

```go
type Fault struct
```

No declaration documentation comment is present.

### LocalOptions

Kind: `struct`. Source: `internal/transport/local.go:57`.

```go
type LocalOptions struct
```

No declaration documentation comment is present.

### Request

Kind: `struct`. Source: `internal/transport/jsonl.go:18`.

```go
type Request struct
```

No declaration documentation comment is present.

### Response

Kind: `struct`. Source: `internal/transport/jsonl.go:34`.

```go
type Response struct
```

No declaration documentation comment is present.

### connectionSet

Kind: `struct`. Source: `internal/transport/local.go:205`.

```go
type connectionSet struct
```

No declaration documentation comment is present.

### hello

Kind: `struct`. Source: `internal/transport/local.go:64`.

```go
type hello struct
```

No declaration documentation comment is present.

### helloResponse

Kind: `struct`. Source: `internal/transport/local.go:70`.

```go
type helloResponse struct
```

No declaration documentation comment is present.

### legacyVersion

Kind: `struct`. Source: `internal/transport/jsonl.go:45`.

```go
type legacyVersion struct
```

No declaration documentation comment is present.

### reconcileParams

Kind: `struct`. Source: `internal/transport/jsonl.go:55`.

```go
type reconcileParams struct
```

No declaration documentation comment is present.

### rootParams

Kind: `struct`. Source: `internal/transport/jsonl.go:50`.

```go
type rootParams struct
```

No declaration documentation comment is present.

### Authority

Kind: `type`. Source: `internal/transport/jsonl.go:24`.

```go
type Authority string
```

No declaration documentation comment is present.
