# filemanager/engine/cmd/fileman-engine

Status: **OBSERVED development and M4 dogfood host**.

Requires either an explicit development sandbox or an exact host-bound manifest, constructs one service, and selects JSONL or authenticated local transport.

No package documentation comment is present.

## Invariants

- No empty or ambient root default.
- Installed roots come only from a validated manifest.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/deployment`
- `filemanager/engine/internal/observation/fsevents`
- `filemanager/engine/internal/observation/rdcw`
- `filemanager/engine/internal/sandbox`
- `filemanager/engine/internal/service`
- `filemanager/engine/internal/transport`

## Declarations

### callLocal

Kind: `function`. Source: `cmd/fileman-engine/main.go:271`.

```go
func callLocal(args []string, output io.Writer) error
```

No declaration documentation comment is present.

### createManifest

Kind: `function`. Source: `cmd/fileman-engine/main.go:189`.

```go
func createManifest(args []string, output io.Writer) error
```

No declaration documentation comment is present.

### ensureManifestAdmission

Kind: `function`. Source: `cmd/fileman-engine/main.go:143`.

```go
func ensureManifestAdmission(ctx context.Context, engine *service.Service, manifest deployment.Manifest) error
```

No declaration documentation comment is present.

### main

Kind: `function`. Source: `cmd/fileman-engine/main.go:27`.

```go
func main()
```

No declaration documentation comment is present.

### manifestAdmissionCommitter

Kind: `function`. Source: `cmd/fileman-engine/main.go:172`.

```go
func manifestAdmissionCommitter(manifest deployment.Manifest) func(transport.Request, transport.Response) error
```

No declaration documentation comment is present.

### run

Kind: `function`. Source: `cmd/fileman-engine/main.go:34`.

```go
func run(args []string, input io.Reader, output io.Writer) error
```

No declaration documentation comment is present.

### runLaunchd

Kind: `function`. Source: `cmd/fileman-engine/main.go:110`.

```go
func runLaunchd(args []string) error
```

No declaration documentation comment is present.

### serve

Kind: `function`. Source: `cmd/fileman-engine/main.go:311`.

```go
func serve(input io.Reader, output io.Writer, guard *sandbox.Guard) error
```

No declaration documentation comment is present.

### serveConfigured

Kind: `function`. Source: `cmd/fileman-engine/main.go:326`.

```go
func serveConfigured(input io.Reader, output io.Writer, guard *sandbox.Guard, options serveOptions) error
```

No declaration documentation comment is present.

### serveWithStore

Kind: `function`. Source: `cmd/fileman-engine/main.go:315`.

```go
func serveWithStore(input io.Reader, output io.Writer, guard *sandbox.Guard, storeRoot string) error
```

No declaration documentation comment is present.

### splitNonempty

Kind: `function`. Source: `cmd/fileman-engine/main.go:301`.

```go
func splitNonempty(value string) []string
```

No declaration documentation comment is present.

### writeLaunchdPlist

Kind: `function`. Source: `cmd/fileman-engine/main.go:67`.

```go
func writeLaunchdPlist(args []string, output io.Writer) error
```

No declaration documentation comment is present.

### writeManifestAtomically

Kind: `function`. Source: `cmd/fileman-engine/main.go:233`.

```go
func writeManifestAtomically(path string, payload []byte) error
```

No declaration documentation comment is present.

### serveOptions

Kind: `struct`. Source: `cmd/fileman-engine/main.go:319`.

```go
type serveOptions struct
```

No declaration documentation comment is present.
