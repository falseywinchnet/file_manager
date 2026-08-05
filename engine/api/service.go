package api

import "time"

const (
	EngineComponentName       = "engine"
	EngineBuildVersion        = "0.1.0-development"
	EngineConfigurationSchema = "fileman.engine.effective-configuration"
)

type LifecycleState string

const (
	LifecycleStarting LifecycleState = "starting"
	LifecycleReady    LifecycleState = "ready"
	LifecycleDraining LifecycleState = "draining"
	LifecycleStopped  LifecycleState = "stopped"
	LifecycleFaulted  LifecycleState = "faulted"
)

// LifecycleStatus describes one process instance. Generation is process-local;
// InstanceID changes across supervisor restarts and must be used with it.
type LifecycleStatus struct {
	State      LifecycleState `json:"state"`
	Generation uint64         `json:"generation"`
	InstanceID string         `json:"instance_id"`
	StartedAt  time.Time      `json:"started_at"`
}

type CapabilityState string

const (
	CapabilityAvailable    CapabilityState = "available"
	CapabilityExperimental CapabilityState = "available_experimental"
	CapabilityNegotiating  CapabilityState = "negotiating"
	CapabilityUnavailable  CapabilityState = "unavailable"
	CapabilityDeferred     CapabilityState = "deferred"
)

// CapabilityStatus names product capability rather than a private package or
// data structure. Reason is required for every non-available state.
type CapabilityStatus struct {
	ID       string          `json:"id"`
	State    CapabilityState `json:"state"`
	Revision string          `json:"revision,omitempty"`
	Reason   string          `json:"reason,omitempty"`
}

type VersionInfo struct {
	Component        string             `json:"component"`
	BuildVersion     string             `json:"build_version"`
	Protocol         string             `json:"protocol"`
	InstanceID       string             `json:"instance_id"`
	ContractFamilies []string           `json:"contract_families,omitempty"`
	Features         []string           `json:"features"`
	Capabilities     []CapabilityStatus `json:"capabilities"`
}

type WorkPhase string

const (
	WorkIdle           WorkPhase = "idle"
	WorkPlanning       WorkPhase = "planning"
	WorkReconciling    WorkPhase = "reconciling"
	WorkRebuilding     WorkPhase = "rebuilding"
	WorkIntegrityCheck WorkPhase = "integrity_check"
	WorkDraining       WorkPhase = "draining"
)

type CurrentnessState string

const (
	CurrentnessManual                 CurrentnessState = "manual_reconcile"
	CurrentnessBaselineRequired       CurrentnessState = "baseline_required"
	CurrentnessReconciling            CurrentnessState = "reconciling"
	CurrentnessCatchingUp             CurrentnessState = "catching_up"
	CurrentnessCurrentVolatile        CurrentnessState = "current_volatile"
	CurrentnessCoverageIncomplete     CurrentnessState = "observation_coverage_incomplete"
	CurrentnessObservationUnavailable CurrentnessState = "observation_unavailable"
)

// WorkStatus is an honest snapshot of work this implementation can observe.
// A volatile reconciled watermark is not a committed watermark; currentness
// across restart requires the cursor and exact generation in one manifest.
type WorkStatus struct {
	Phase                WorkPhase        `json:"phase"`
	ActiveRequests       uint32           `json:"active_requests"`
	ActiveQueries        uint32           `json:"active_queries"`
	ActiveAdministrative uint32           `json:"active_administrative"`
	BackgroundIngestion  bool             `json:"background_ingestion"`
	BacklogKnown         bool             `json:"backlog_known"`
	PendingObservations  uint64           `json:"pending_observations,omitempty"`
	OldestObservationMS  uint64           `json:"oldest_observation_ms,omitempty"`
	ObservationGap       bool             `json:"observation_gap,omitempty"`
	Currentness          CurrentnessState `json:"currentness"`
	ObservationSource    string           `json:"observation_source,omitempty"`
	ObservationEpoch     string           `json:"observation_epoch,omitempty"`
	ObservedWatermark    uint64           `json:"observed_watermark,omitempty"`
	ReconciledWatermark  uint64           `json:"reconciled_watermark,omitempty"`
	WatermarkDurable     bool             `json:"watermark_durable"`
	CoverageIncomplete   bool             `json:"coverage_incomplete,omitempty"`
	ObservationError     string           `json:"observation_error,omitempty"`
}

// EffectiveConfiguration reports enacted service behavior. It deliberately
// excludes candidate tuning values that the running engine does not enforce.
type EffectiveConfiguration struct {
	SchemaMajor          uint16     `json:"schema_major"`
	SchemaMinor          uint16     `json:"schema_minor"`
	Schema               string     `json:"schema"`
	Digest               string     `json:"digest"`
	Deployment           string     `json:"deployment"`
	Sandboxed            bool       `json:"sandboxed"`
	Persistent           bool       `json:"persistent"`
	StoreOutsideRoots    bool       `json:"store_outside_indexed_roots"`
	IngestionMode        string     `json:"ingestion_mode"`
	RootPolicy           []RootSpec `json:"root_policy"`
	RootPolicyPersistent bool       `json:"root_policy_persistent"`
}
