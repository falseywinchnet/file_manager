# filemanager/engine/api

Status: **ORC-ENG semantic v0 frozen for experimental implementation**.

Transport-neutral requests, responses, evidence, lifecycle, capabilities, configuration, and safe fault vocabulary.

Package api defines the transport-neutral contract of the File Manager catalogue and retrieval engine. It must not expose on-disk representation.

## Invariants

- No storage representation crosses this package.
- No source-file mutation operation exists.
- Unrelated raw channel scores are not directly comparable.

## Internal imports

- `None.`

## Declarations

### CapabilityAvailable

Kind: `constant`. Source: `api/service.go:33`.

```go
CapabilityAvailable    CapabilityState = "available"
```

No declaration documentation comment is present.

### CapabilityDeferred

Kind: `constant`. Source: `api/service.go:37`.

```go
CapabilityDeferred     CapabilityState = "deferred"
```

No declaration documentation comment is present.

### CapabilityExperimental

Kind: `constant`. Source: `api/service.go:34`.

```go
CapabilityExperimental CapabilityState = "available_experimental"
```

No declaration documentation comment is present.

### CapabilityNegotiating

Kind: `constant`. Source: `api/service.go:35`.

```go
CapabilityNegotiating  CapabilityState = "negotiating"
```

No declaration documentation comment is present.

### CapabilityUnavailable

Kind: `constant`. Source: `api/service.go:36`.

```go
CapabilityUnavailable  CapabilityState = "unavailable"
```

No declaration documentation comment is present.

### CurrentnessBaselineRequired

Kind: `constant`. Source: `api/service.go:74`.

```go
CurrentnessBaselineRequired       CurrentnessState = "baseline_required"
```

No declaration documentation comment is present.

### CurrentnessCatchingUp

Kind: `constant`. Source: `api/service.go:76`.

```go
CurrentnessCatchingUp             CurrentnessState = "catching_up"
```

No declaration documentation comment is present.

### CurrentnessCoverageIncomplete

Kind: `constant`. Source: `api/service.go:78`.

```go
CurrentnessCoverageIncomplete     CurrentnessState = "observation_coverage_incomplete"
```

No declaration documentation comment is present.

### CurrentnessCurrentVolatile

Kind: `constant`. Source: `api/service.go:77`.

```go
CurrentnessCurrentVolatile        CurrentnessState = "current_volatile"
```

No declaration documentation comment is present.

### CurrentnessManual

Kind: `constant`. Source: `api/service.go:73`.

```go
CurrentnessManual                 CurrentnessState = "manual_reconcile"
```

No declaration documentation comment is present.

### CurrentnessObservationUnavailable

Kind: `constant`. Source: `api/service.go:79`.

```go
CurrentnessObservationUnavailable CurrentnessState = "observation_unavailable"
```

No declaration documentation comment is present.

### CurrentnessReconciling

Kind: `constant`. Source: `api/service.go:75`.

```go
CurrentnessReconciling            CurrentnessState = "reconciling"
```

No declaration documentation comment is present.

### EngineBuildVersion

Kind: `constant`. Source: `api/service.go:7`.

```go
EngineBuildVersion        = "0.1.0-development"
```

No declaration documentation comment is present.

### EngineComponentName

Kind: `constant`. Source: `api/service.go:6`.

```go
EngineComponentName       = "engine"
```

No declaration documentation comment is present.

### EngineConfigurationSchema

Kind: `constant`. Source: `api/service.go:8`.

```go
EngineConfigurationSchema = "fileman.engine.effective-configuration"
```

No declaration documentation comment is present.

### ErrorAmbiguousObject

Kind: `constant`. Source: `api/errors.go:12`.

```go
ErrorAmbiguousObject    ErrorCode = "AMBIGUOUS_OBJECT"
```

No declaration documentation comment is present.

### ErrorGenerationExpired

Kind: `constant`. Source: `api/errors.go:15`.

```go
ErrorGenerationExpired  ErrorCode = "GENERATION_EXPIRED"
```

No declaration documentation comment is present.

### ErrorIntegrity

Kind: `constant`. Source: `api/errors.go:18`.

```go
ErrorIntegrity          ErrorCode = "INTEGRITY_FAILURE"
```

No declaration documentation comment is present.

### ErrorInternal

Kind: `constant`. Source: `api/errors.go:19`.

```go
ErrorInternal           ErrorCode = "INTERNAL"
```

No declaration documentation comment is present.

### ErrorInvalidQuery

Kind: `constant`. Source: `api/errors.go:9`.

```go
ErrorInvalidQuery       ErrorCode = "INVALID_QUERY"
```

No declaration documentation comment is present.

### ErrorInvalidRequest

Kind: `constant`. Source: `api/errors.go:8`.

```go
ErrorInvalidRequest     ErrorCode = "INVALID_REQUEST"
```

No declaration documentation comment is present.

### ErrorMethodUnavailable

Kind: `constant`. Source: `api/errors.go:10`.

```go
ErrorMethodUnavailable  ErrorCode = "METHOD_UNAVAILABLE"
```

No declaration documentation comment is present.

### ErrorNotFound

Kind: `constant`. Source: `api/errors.go:11`.

```go
ErrorNotFound           ErrorCode = "NOT_FOUND"
```

No declaration documentation comment is present.

### ErrorOutsideRoot

Kind: `constant`. Source: `api/errors.go:13`.

```go
ErrorOutsideRoot        ErrorCode = "OUTSIDE_ROOT"
```

No declaration documentation comment is present.

### ErrorResourceBudget

Kind: `constant`. Source: `api/errors.go:17`.

```go
ErrorResourceBudget     ErrorCode = "RESOURCE_BUDGET_EXCEEDED"
```

No declaration documentation comment is present.

### ErrorStaleConfiguration

Kind: `constant`. Source: `api/errors.go:16`.

```go
ErrorStaleConfiguration ErrorCode = "STALE_CONFIGURATION"
```

No declaration documentation comment is present.

### ErrorUnapprovedRoot

Kind: `constant`. Source: `api/errors.go:14`.

```go
ErrorUnapprovedRoot     ErrorCode = "UNAPPROVED_ROOT"
```

No declaration documentation comment is present.

### EvidenceContent

Kind: `constant`. Source: `api/types.go:85`.

```go
EvidenceContent     EvidenceKind = "content_provider"
```

No declaration documentation comment is present.

### EvidenceExactName

Kind: `constant`. Source: `api/types.go:79`.

```go
EvidenceExactName   EvidenceKind = "exact_name"
```

No declaration documentation comment is present.

### EvidenceExactPath

Kind: `constant`. Source: `api/types.go:80`.

```go
EvidenceExactPath   EvidenceKind = "exact_path"
```

No declaration documentation comment is present.

### EvidenceFederated

Kind: `constant`. Source: `api/types.go:86`.

```go
EvidenceFederated   EvidenceKind = "federated"
```

No declaration documentation comment is present.

### EvidenceFuzzy

Kind: `constant`. Source: `api/types.go:83`.

```go
EvidenceFuzzy       EvidenceKind = "fuzzy"
```

No declaration documentation comment is present.

### EvidenceLexical

Kind: `constant`. Source: `api/types.go:82`.

```go
EvidenceLexical     EvidenceKind = "lexical"
```

No declaration documentation comment is present.

### EvidenceMetadata

Kind: `constant`. Source: `api/types.go:81`.

```go
EvidenceMetadata    EvidenceKind = "metadata"
```

No declaration documentation comment is present.

### EvidenceStructural

Kind: `constant`. Source: `api/types.go:84`.

```go
EvidenceStructural  EvidenceKind = "structural"
```

No declaration documentation comment is present.

### EvidenceUnavailable

Kind: `constant`. Source: `api/types.go:87`.

```go
EvidenceUnavailable EvidenceKind = "offline_catalogue"
```

No declaration documentation comment is present.

### LifecycleDraining

Kind: `constant`. Source: `api/service.go:16`.

```go
LifecycleDraining LifecycleState = "draining"
```

No declaration documentation comment is present.

### LifecycleFaulted

Kind: `constant`. Source: `api/service.go:18`.

```go
LifecycleFaulted  LifecycleState = "faulted"
```

No declaration documentation comment is present.

### LifecycleReady

Kind: `constant`. Source: `api/service.go:15`.

```go
LifecycleReady    LifecycleState = "ready"
```

No declaration documentation comment is present.

### LifecycleStarting

Kind: `constant`. Source: `api/service.go:14`.

```go
LifecycleStarting LifecycleState = "starting"
```

No declaration documentation comment is present.

### LifecycleStopped

Kind: `constant`. Source: `api/service.go:17`.

```go
LifecycleStopped  LifecycleState = "stopped"
```

No declaration documentation comment is present.

### LiveFilesystemSource

Kind: `constant`. Source: `api/types.go:157`.

```go
const LiveFilesystemSource = "live_filesystem"
```

No declaration documentation comment is present.

### ObjectDirectory

Kind: `constant`. Source: `api/types.go:61`.

```go
ObjectDirectory ObjectKind = "directory"
```

No declaration documentation comment is present.

### ObjectOther

Kind: `constant`. Source: `api/types.go:63`.

```go
ObjectOther     ObjectKind = "other"
```

No declaration documentation comment is present.

### ObjectRegular

Kind: `constant`. Source: `api/types.go:60`.

```go
ObjectRegular   ObjectKind = "file"
```

No declaration documentation comment is present.

### ObjectSymlink

Kind: `constant`. Source: `api/types.go:62`.

```go
ObjectSymlink   ObjectKind = "symlink"
```

No declaration documentation comment is present.

### ProtocolVersion

Kind: `constant`. Source: `api/types.go:10`.

```go
const ProtocolVersion = "engine.v0"
```

No declaration documentation comment is present.

### SortAscending

Kind: `constant`. Source: `api/types.go:35`.

```go
SortAscending  SortDirection = "asc"
```

No declaration documentation comment is present.

### SortDescending

Kind: `constant`. Source: `api/types.go:36`.

```go
SortDescending SortDirection = "desc"
```

No declaration documentation comment is present.

### WorkDraining

Kind: `constant`. Source: `api/service.go:67`.

```go
WorkDraining       WorkPhase = "draining"
```

No declaration documentation comment is present.

### WorkIdle

Kind: `constant`. Source: `api/service.go:62`.

```go
WorkIdle           WorkPhase = "idle"
```

No declaration documentation comment is present.

### WorkIntegrityCheck

Kind: `constant`. Source: `api/service.go:66`.

```go
WorkIntegrityCheck WorkPhase = "integrity_check"
```

No declaration documentation comment is present.

### WorkPlanning

Kind: `constant`. Source: `api/service.go:63`.

```go
WorkPlanning       WorkPhase = "planning"
```

No declaration documentation comment is present.

### WorkRebuilding

Kind: `constant`. Source: `api/service.go:65`.

```go
WorkRebuilding     WorkPhase = "rebuilding"
```

No declaration documentation comment is present.

### WorkReconciling

Kind: `constant`. Source: `api/service.go:64`.

```go
WorkReconciling    WorkPhase = "reconciling"
```

No declaration documentation comment is present.

### NewFault

Kind: `function`. Source: `api/errors.go:39`.

```go
func NewFault(code ErrorCode, message string) *Fault
```

No declaration documentation comment is present.

### WrapFault

Kind: `function`. Source: `api/errors.go:43`.

```go
func WrapFault(code ErrorCode, message string, cause error) *Fault
```

No declaration documentation comment is present.

### Engine

Kind: `interface`. Source: `api/types.go:235`.

```go
type Engine interface
```

Engine is deliberately read-oriented. Root admission, reconciliation, and rebuild are administrative projection operations; file mutation is absent.

### ManagedEngine

Kind: `interface`. Source: `api/types.go:246`.

```go
type ManagedEngine interface
```

ManagedEngine adds process-instance and enacted-configuration controls. A supervisor constructs/starts and restarts the process; Shutdown is the engine-owned draining half of that lifecycle.

### Fault.Error

Kind: `method`. Source: `api/errors.go:30`.

```go
func (e *Fault) Error() string
```

No declaration documentation comment is present.

### Fault.Unwrap

Kind: `method`. Source: `api/errors.go:37`.

```go
func (e *Fault) Unwrap() error
```

No declaration documentation comment is present.

### CapabilityStatus

Kind: `struct`. Source: `api/service.go:42`.

```go
type CapabilityStatus struct
```

CapabilityStatus names product capability rather than a private package or data structure. Reason is required for every non-available state.

### EffectiveConfiguration

Kind: `struct`. Source: `api/service.go:107`.

```go
type EffectiveConfiguration struct
```

EffectiveConfiguration reports enacted service behavior. It deliberately excludes candidate tuning values that the running engine does not enforce.

### Evidence

Kind: `struct`. Source: `api/types.go:92`.

```go
type Evidence struct
```

Evidence preserves an individual channel's claim. Score is meaningful only under Calibration; consumers must not compare unrelated raw scores directly.

### Fault

Kind: `struct`. Source: `api/errors.go:24`.

```go
type Fault struct
```

Fault is safe to project across transports. Cause remains process-local so responses do not accidentally disclose paths or implementation details.

### IntegrityReport

Kind: `struct`. Source: `api/types.go:226`.

```go
type IntegrityReport struct
```

No declaration documentation comment is present.

### LifecycleStatus

Kind: `struct`. Source: `api/service.go:23`.

```go
type LifecycleStatus struct
```

LifecycleStatus describes one process instance. Generation is process-local; InstanceID changes across supervisor restarts and must be used with it.

### LiveQuery

Kind: `struct`. Source: `api/types.go:170`.

```go
type LiveQuery struct
```

No declaration documentation comment is present.

### LiveQueryBudget

Kind: `struct`. Source: `api/types.go:161`.

```go
type LiveQueryBudget struct
```

LiveQueryBudget is a per-page work constitution. The service clamps every field to its own hard ceiling; a caller cannot expand server policy.

### LiveQueryResponse

Kind: `struct`. Source: `api/types.go:193`.

```go
type LiveQueryResponse struct
```

LiveQueryResponse deliberately has no catalogue generation. Its exact observations were made during a mutable filesystem traversal identified by ScanID and ordered only within that scan.

### LiveQueryScope

Kind: `struct`. Source: `api/types.go:178`.

```go
type LiveQueryScope struct
```

No declaration documentation comment is present.

### LiveQueryWork

Kind: `struct`. Source: `api/types.go:184`.

```go
type LiveQueryWork struct
```

No declaration documentation comment is present.

### Metadata

Kind: `struct`. Source: `api/types.go:68`.

```go
type Metadata struct
```

Metadata contains only exact filesystem observations made by the scanner. It does not contain extracted, inferred, or provider-supplied fields.

### ObjectRef

Kind: `struct`. Source: `api/types.go:18`.

```go
type ObjectRef struct
```

ObjectRef identifies a platform object. Path is its observed address, not its identity. PlatformKey contains a platform-adapter-owned stable identity tuple.

### Query

Kind: `struct`. Source: `api/types.go:46`.

```go
type Query struct
```

No declaration documentation comment is present.

### QueryPlan

Kind: `struct`. Source: `api/types.go:204`.

```go
type QueryPlan struct
```

No declaration documentation comment is present.

### QueryResponse

Kind: `struct`. Source: `api/types.go:148`.

```go
type QueryResponse struct
```

No declaration documentation comment is present.

### ReconcileReport

Kind: `struct`. Source: `api/types.go:139`.

```go
type ReconcileReport struct
```

No declaration documentation comment is present.

### Result

Kind: `struct`. Source: `api/types.go:104`.

```go
type Result struct
```

No declaration documentation comment is present.

### RootPlan

Kind: `struct`. Source: `api/types.go:132`.

```go
type RootPlan struct
```

No declaration documentation comment is present.

### RootSpec

Kind: `struct`. Source: `api/types.go:116`.

```go
type RootSpec struct
```

RootSpec is an already user-approved projection root. The development service additionally requires every path to remain inside its sandbox.

### RootState

Kind: `struct`. Source: `api/types.go:121`.

```go
type RootState struct
```

No declaration documentation comment is present.

### Scope

Kind: `struct`. Source: `api/types.go:26`.

```go
type Scope struct
```

No declaration documentation comment is present.

### SortKey

Kind: `struct`. Source: `api/types.go:41`.

```go
type SortKey struct
```

SortKey is deliberately limited to exact stored metadata in the reference engine. Relevance order belongs to the later ranking pipeline.

### Status

Kind: `struct`. Source: `api/types.go:212`.

```go
type Status struct
```

No declaration documentation comment is present.

### VersionInfo

Kind: `struct`. Source: `api/service.go:49`.

```go
type VersionInfo struct
```

No declaration documentation comment is present.

### WorkStatus

Kind: `struct`. Source: `api/service.go:85`.

```go
type WorkStatus struct
```

WorkStatus is an honest snapshot of work this implementation can observe. A volatile reconciled watermark is not a committed watermark; currentness across restart requires the cursor and exact generation in one manifest.

### CapabilityState

Kind: `type`. Source: `api/service.go:30`.

```go
type CapabilityState string
```

No declaration documentation comment is present.

### CurrentnessState

Kind: `type`. Source: `api/service.go:70`.

```go
type CurrentnessState string
```

No declaration documentation comment is present.

### ErrorCode

Kind: `type`. Source: `api/errors.go:5`.

```go
type ErrorCode string
```

No declaration documentation comment is present.

### EvidenceKind

Kind: `type`. Source: `api/types.go:76`.

```go
type EvidenceKind string
```

No declaration documentation comment is present.

### Generation

Kind: `type`. Source: `api/types.go:14`.

```go
type Generation uint64
```

No declaration documentation comment is present.

### LifecycleState

Kind: `type`. Source: `api/service.go:11`.

```go
type LifecycleState string
```

No declaration documentation comment is present.

### ObjectID

Kind: `type`. Source: `api/types.go:13`.

```go
type ObjectID string
```

No declaration documentation comment is present.

### ObjectKind

Kind: `type`. Source: `api/types.go:57`.

```go
type ObjectKind string
```

No declaration documentation comment is present.

### RootID

Kind: `type`. Source: `api/types.go:12`.

```go
type RootID string
```

No declaration documentation comment is present.

### SortDirection

Kind: `type`. Source: `api/types.go:32`.

```go
type SortDirection string
```

No declaration documentation comment is present.

### WorkPhase

Kind: `type`. Source: `api/service.go:59`.

```go
type WorkPhase string
```

No declaration documentation comment is present.
