# filemanager/engine/internal/service

Status: **OBSERVED development service**.

The cohesive façade over lifecycle, configuration, capabilities, approved roots, reconciliation, checked readers, exact/live query, status, and experimental observation/similarity seams.

Package service coordinates root policy, scanning, exact queries, and immutable reader generations without exposing catalogue representation.

## Invariants

- Administrative publication is serialized.
- Queries pin one immutable reader generation.
- Shutdown cancels and drains admitted work before closing readers.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/catalog`
- `filemanager/engine/internal/exact`
- `filemanager/engine/internal/generation`
- `filemanager/engine/internal/live`
- `filemanager/engine/internal/observation`
- `filemanager/engine/internal/observation/fsevents`
- `filemanager/engine/internal/observation/rdcw`
- `filemanager/engine/internal/ranking`
- `filemanager/engine/internal/sandbox`
- `filemanager/engine/internal/scan`
- `filemanager/engine/internal/similarity`

## Declarations

### operationAdministrative

Kind: `constant`. Source: `internal/service/lifecycle.go:18`.

```go
operationAdministrative
```

No declaration documentation comment is present.

### operationQuery

Kind: `constant`. Source: `internal/service/lifecycle.go:17`.

```go
operationQuery operationClass = iota + 1
```

No declaration documentation comment is present.

### DefaultBackgroundPolicy

Kind: `function`. Source: `internal/service/background.go:21`.

```go
func DefaultBackgroundPolicy() BackgroundPolicy
```

No declaration documentation comment is present.

### New

Kind: `function`. Source: `internal/service/service.go:50`.

```go
func New(guard *sandbox.Guard) (*Service, error)
```

New constructs an in-memory development service beneath an explicit sandbox guard.

### NewPersistent

Kind: `function`. Source: `internal/service/service.go:66`.

```go
func NewPersistent(guard *sandbox.Guard, directory string) (*Service, error)
```

NewPersistent opens the one-root M2 durable service. The store directory must already exist and may not lie beneath the indexed source root.

### anyRootContains

Kind: `function`. Source: `internal/service/configuration.go:65`.

```go
func anyRootContains(roots []api.RootSpec, path string) bool
```

No declaration documentation comment is present.

### newServiceLifecycle

Kind: `function`. Source: `internal/service/lifecycle.go:38`.

```go
func newServiceLifecycle() (serviceLifecycle, error)
```

No declaration documentation comment is present.

### pathContains

Kind: `function`. Source: `internal/service/paths.go:8`.

```go
func pathContains(root, path string) bool
```

No declaration documentation comment is present.

### sameObservationStream

Kind: `function`. Source: `internal/service/background.go:403`.

```go
func sameObservationStream(left, right observation.Cursor) bool
```

No declaration documentation comment is present.

### stopTimer

Kind: `function`. Source: `internal/service/background.go:407`.

```go
func stopTimer(timer *time.Timer)
```

No declaration documentation comment is present.

### metadataScanner

Kind: `interface`. Source: `internal/service/service.go:41`.

```go
type metadataScanner interface
```

No declaration documentation comment is present.

### Service.ApplyRoots

Kind: `method`. Source: `internal/service/roots.go:66`.

```go
func (s *Service) ApplyRoots(ctx context.Context, roots []api.RootSpec) (api.RootPlan, error)
```

ApplyRoots applies a validated development root policy without a stale-plan precondition. Cross-project clients use ApplyRootsExpected instead.

### Service.ApplyRootsExpected

Kind: `method`. Source: `internal/service/roots.go:72`.

```go
func (s *Service) ApplyRootsExpected(ctx context.Context, roots []api.RootSpec, expectedConfigurationDigest string) (api.RootPlan, error)
```

ApplyRootsExpected prevents a stale administrative controller from overwriting a root policy planned against another effective configuration.

### Service.Close

Kind: `method`. Source: `internal/service/service.go:129`.

```go
func (s *Service) Close() error
```

Close drains the service and releases its current reader generation.

### Service.Configuration

Kind: `method`. Source: `internal/service/configuration.go:33`.

```go
func (s *Service) Configuration() api.EffectiveConfiguration
```

Configuration returns the enacted configuration and a stable digest. Root currentness is health state and is intentionally excluded from that digest.

### Service.Inspect

Kind: `method`. Source: `internal/service/query.go:99`.

```go
func (s *Service) Inspect(ctx context.Context, ref api.ObjectRef) (api.Result, error)
```

Inspect returns one exact record and its stored provenance. A hard-linked object requires an observed path to disambiguate the binding.

### Service.Integrity

Kind: `method`. Source: `internal/service/query.go:133`.

```go
func (s *Service) Integrity(ctx context.Context) (api.IntegrityReport, error)
```

Integrity checks the current exact catalogue or durable generation without repairing it or mutating source files.

### Service.Persistent

Kind: `method`. Source: `internal/service/service.go:126`.

```go
func (s *Service) Persistent() bool
```

Persistent reports whether the service owns a durable generation store.

### Service.PinSimilarityGeneration

Kind: `method`. Source: `internal/service/similarity_generation.go:31`.

```go
func (s *Service) PinSimilarityGeneration(ctx context.Context, rootID api.RootID) (*SimilarityGeneration, error)
```

PinSimilarityGeneration acquires a stable exact record source for an experimental projection. It is intentionally internal and does not enlarge the engine transport contract.

### Service.PlanRoots

Kind: `method`. Source: `internal/service/roots.go:16`.

```go
func (s *Service) PlanRoots(ctx context.Context, roots []api.RootSpec) (api.RootPlan, error)
```

PlanRoots validates and canonicalizes an already-authorized root policy without mutating service state.

### Service.Query

Kind: `method`. Source: `internal/service/query.go:15`.

```go
func (s *Service) Query(ctx context.Context, query api.Query) (api.QueryResponse, error)
```

Query searches one committed exact catalogue generation and returns evidence-bearing results with a generation-bound continuation cursor.

### Service.QueryLive

Kind: `method`. Source: `internal/service/query.go:73`.

```go
func (s *Service) QueryLive(ctx context.Context, query api.LiveQuery) (api.LiveQueryResponse, error)
```

QueryLive searches an already-approved filesystem scope without consulting or creating a catalogue. Continuation state is process-local, bounded, and discarded on expiry, root-policy change, completion, or shutdown.

### Service.Rebuild

Kind: `method`. Source: `internal/service/reconciliation.go:119`.

```go
func (s *Service) Rebuild(ctx context.Context, rootID api.RootID) (api.ReconcileReport, error)
```

Rebuild performs a full authoritative scan and publishes it through the same immutable commit path as reconciliation. The last valid reader remains available until publication succeeds.

### Service.Reconcile

Kind: `method`. Source: `internal/service/reconciliation.go:14`.

```go
func (s *Service) Reconcile(ctx context.Context, rootID api.RootID) (api.ReconcileReport, error)
```

Reconcile performs a complete authoritative metadata scan of an approved root and publishes a new generation only when its exact state changed.

### Service.SandboxRoot

Kind: `method`. Source: `internal/service/service.go:135`.

```go
func (s *Service) SandboxRoot() string
```

SandboxRoot returns the canonical development containment root.

### Service.Shutdown

Kind: `method`. Source: `internal/service/lifecycle.go:125`.

```go
func (s *Service) Shutdown(ctx context.Context) (api.LifecycleStatus, error)
```

Shutdown transitions the process instance to draining, cancels service-owned operation contexts, waits for their bounded cleanup, and closes the current exact reader. Restart is intentionally a supervisor operation that constructs a new process instance and recovers committed state.

### Service.StartBackgroundObservation

Kind: `method`. Source: `internal/service/background.go:56`.

```go
func (s *Service) StartBackgroundObservation(ctx context.Context, adapter observation.Adapter, policy BackgroundPolicy) error
```

StartBackgroundObservation attaches one platform adapter to the service. It is an internal integration seam until a native adapter, authorization, and the atomic manifest-watermark schema pass their gates.

### Service.Status

Kind: `method`. Source: `internal/service/status.go:16`.

```go
func (s *Service) Status(_ context.Context) (api.Status, error)
```

Status returns a point-in-time lifecycle, capability, root, generation, and currentness snapshot. Readiness is reduced honestly by stale or incomplete observation state.

### Service.StopBackgroundObservation

Kind: `method`. Source: `internal/service/background.go:147`.

```go
func (s *Service) StopBackgroundObservation(ctx context.Context) error
```

StopBackgroundObservation stops adapter consumption without stopping query service. Restarting observation establishes a new baseline and reconciles.

### Service.Version

Kind: `method`. Source: `internal/service/configuration.go:14`.

```go
func (s *Service) Version() api.VersionInfo
```

Version reports build, protocol, contract, feature, instance, and capability identity without exposing storage representation.

### Service.applyRoots

Kind: `method`. Source: `internal/service/roots.go:79`.

```go
func (s *Service) applyRoots(ctx context.Context, roots []api.RootSpec, expectedConfigurationDigest string) (api.RootPlan, error)
```

No declaration documentation comment is present.

### Service.armBackgroundTimer

Kind: `method`. Source: `internal/service/background.go:227`.

```go
func (s *Service) armBackgroundTimer(controller *backgroundController, timer *time.Timer) <-chan time.Time
```

No declaration documentation comment is present.

### Service.backgroundIngestionMode

Kind: `method`. Source: `internal/service/background.go:335`.

```go
func (s *Service) backgroundIngestionMode() string
```

No declaration documentation comment is present.

### Service.backgroundRoots

Kind: `method`. Source: `internal/service/background.go:286`.

```go
func (s *Service) backgroundRoots(work observation.Work) ([]api.RootID, string)
```

No declaration documentation comment is present.

### Service.backgroundStale

Kind: `method`. Source: `internal/service/background.go:345`.

```go
func (s *Service) backgroundStale() bool
```

No declaration documentation comment is present.

### Service.backgroundWorkStatus

Kind: `method`. Source: `internal/service/background.go:350`.

```go
func (s *Service) backgroundWorkStatus(now time.Time) (api.WorkStatus, bool, string)
```

No declaration documentation comment is present.

### Service.beginOperation

Kind: `method`. Source: `internal/service/lifecycle.go:51`.

```go
func (s *Service) beginOperation(ctx context.Context, class operationClass, phase api.WorkPhase) (context.Context, func(), error)
```

No declaration documentation comment is present.

### Service.capabilities

Kind: `method`. Source: `internal/service/capabilities.go:5`.

```go
func (s *Service) capabilities() []api.CapabilityStatus
```

No declaration documentation comment is present.

### Service.decorateStatus

Kind: `method`. Source: `internal/service/status.go:111`.

```go
func (s *Service) decorateStatus(status api.Status) api.Status
```

No declaration documentation comment is present.

### Service.effectiveConfiguration

Kind: `method`. Source: `internal/service/configuration.go:49`.

```go
func (s *Service) effectiveConfiguration(roots []api.RootSpec, persistentPolicy bool) api.EffectiveConfiguration
```

No declaration documentation comment is present.

### Service.finishOperation

Kind: `method`. Source: `internal/service/lifecycle.go:83`.

```go
func (s *Service) finishOperation(class operationClass)
```

No declaration documentation comment is present.

### Service.invalidateBackground

Kind: `method`. Source: `internal/service/background.go:317`.

```go
func (s *Service) invalidateBackground(reason string)
```

No declaration documentation comment is present.

### Service.lifecycleStatus

Kind: `method`. Source: `internal/service/lifecycle.go:104`.

```go
func (s *Service) lifecycleStatus() (api.LifecycleStatus, api.WorkStatus)
```

No declaration documentation comment is present.

### Service.lifecycleStatusLocked

Kind: `method`. Source: `internal/service/lifecycle.go:186`.

```go
func (s *Service) lifecycleStatusLocked() (api.LifecycleStatus, api.WorkStatus)
```

No declaration documentation comment is present.

### Service.persistentStatus

Kind: `method`. Source: `internal/service/status.go:56`.

```go
func (s *Service) persistentStatus() api.Status
```

No declaration documentation comment is present.

### Service.planRoots

Kind: `method`. Source: `internal/service/roots.go:28`.

```go
func (s *Service) planRoots(roots []api.RootSpec) (api.RootPlan, error)
```

No declaration documentation comment is present.

### Service.pumpBackgroundObservations

Kind: `method`. Source: `internal/service/background.go:181`.

```go
func (s *Service) pumpBackgroundObservations(controller *backgroundController)
```

No declaration documentation comment is present.

### Service.reconcile

Kind: `method`. Source: `internal/service/reconciliation.go:23`.

```go
func (s *Service) reconcile(ctx context.Context, rootID api.RootID, forcePublication bool) (api.ReconcileReport, error)
```

No declaration documentation comment is present.

### Service.reconcileBackgroundObservations

Kind: `method`. Source: `internal/service/background.go:209`.

```go
func (s *Service) reconcileBackgroundObservations(controller *backgroundController)
```

No declaration documentation comment is present.

### Service.runBackgroundObservation

Kind: `method`. Source: `internal/service/background.go:163`.

```go
func (s *Service) runBackgroundObservation(controller *backgroundController)
```

No declaration documentation comment is present.

### Service.runBackgroundWork

Kind: `method`. Source: `internal/service/background.go:246`.

```go
func (s *Service) runBackgroundWork(controller *backgroundController)
```

No declaration documentation comment is present.

### SimilarityGeneration.Anchor

Kind: `method`. Source: `internal/service/similarity_generation.go:89`.

```go
func (g *SimilarityGeneration) Anchor(ordinal uint32) (similarity.Anchor, bool)
```

No declaration documentation comment is present.

### SimilarityGeneration.Close

Kind: `method`. Source: `internal/service/similarity_generation.go:122`.

```go
func (g *SimilarityGeneration) Close() error
```

No declaration documentation comment is present.

### SimilarityGeneration.ExactGeneration

Kind: `method`. Source: `internal/service/similarity_generation.go:84`.

```go
func (g *SimilarityGeneration) ExactGeneration() api.Generation
```

No declaration documentation comment is present.

### SimilarityGeneration.Filename

Kind: `method`. Source: `internal/service/similarity_generation.go:99`.

```go
func (g *SimilarityGeneration) Filename(ordinal uint32) (string, bool)
```

No declaration documentation comment is present.

### SimilarityGeneration.Generation

Kind: `method`. Source: `internal/service/similarity_generation.go:83`.

```go
func (g *SimilarityGeneration) Generation() api.Generation
```

No declaration documentation comment is present.

### SimilarityGeneration.RecordCount

Kind: `method`. Source: `internal/service/similarity_generation.go:87`.

```go
func (g *SimilarityGeneration) RecordCount() uint32
```

No declaration documentation comment is present.

### SimilarityGeneration.Root

Kind: `method`. Source: `internal/service/similarity_generation.go:82`.

```go
func (g *SimilarityGeneration) Root() api.RootSpec
```

No declaration documentation comment is present.

### SimilarityGeneration.record

Kind: `method`. Source: `internal/service/similarity_generation.go:111`.

```go
func (g *SimilarityGeneration) record(ordinal uint32) (catalog.Record, bool)
```

No declaration documentation comment is present.

### backgroundController.isRunning

Kind: `method`. Source: `internal/service/background.go:139`.

```go
func (c *backgroundController) isRunning() bool
```

No declaration documentation comment is present.

### BackgroundPolicy

Kind: `struct`. Source: `internal/service/background.go:15`.

```go
type BackgroundPolicy struct
```

No declaration documentation comment is present.

### Service

Kind: `struct`. Source: `internal/service/service.go:22`.

```go
type Service struct
```

Service is the transport-neutral engine façade. It coordinates exact catalogue state, durable reader generations, live traversal, lifecycle, and optional background observation while keeping their representations private.

### SimilarityGeneration

Kind: `struct`. Source: `internal/service/similarity_generation.go:18`.

```go
type SimilarityGeneration struct
```

SimilarityGeneration is a private, read-only lease on one authoritative exact generation. Experimental projections consume its ordinals but never own identity, paths, or filenames. Close releases the durable segment pin.

### backgroundController

Kind: `struct`. Source: `internal/service/background.go:35`.

```go
type backgroundController struct
```

No declaration documentation comment is present.

### backgroundObservation

Kind: `struct`. Source: `internal/service/background.go:29`.

```go
type backgroundObservation struct
```

No declaration documentation comment is present.

### serviceLifecycle

Kind: `struct`. Source: `internal/service/lifecycle.go:21`.

```go
type serviceLifecycle struct
```

No declaration documentation comment is present.

### operationClass

Kind: `type`. Source: `internal/service/lifecycle.go:14`.

```go
type operationClass uint8
```

No declaration documentation comment is present.

### _

Kind: `variable`. Source: `internal/service/service.go:45`.

```go
var _ api.Engine = (*Service)(nil)
```

No declaration documentation comment is present.

### _

Kind: `variable`. Source: `internal/service/service.go:46`.

```go
var _ api.ManagedEngine = (*Service)(nil)
```

No declaration documentation comment is present.

### _

Kind: `variable`. Source: `internal/service/similarity_generation.go:131`.

```go
var _ similarity.HistoryTupleRecordResolver = (*SimilarityGeneration)(nil)
```

No declaration documentation comment is present.

### _

Kind: `variable`. Source: `internal/service/similarity_generation.go:132`.

```go
var _ similarity.HistoryTupleGenerationResolver = (*SimilarityGeneration)(nil)
```

No declaration documentation comment is present.
