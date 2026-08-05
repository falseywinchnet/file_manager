// Package api defines the transport-neutral contract of the File Manager
// catalogue and retrieval engine. It must not expose on-disk representation.
package api

import (
	"context"
	"time"
)

const ProtocolVersion = "engine.v0"

type RootID string
type ObjectID string
type Generation uint64

// ObjectRef identifies a platform object. Path is its observed address, not its
// identity. PlatformKey contains a platform-adapter-owned stable identity tuple.
type ObjectRef struct {
	Root        RootID            `json:"root"`
	ID          ObjectID          `json:"id"`
	Path        string            `json:"path"`
	PlatformKey map[string]string `json:"platform_key,omitempty"`
	Incarnation string            `json:"incarnation,omitempty"`
}

type Scope struct {
	Root        RootID `json:"root"`
	Path        string `json:"path,omitempty"`
	Descendants bool   `json:"descendants"`
}

type SortDirection string

const (
	SortAscending  SortDirection = "asc"
	SortDescending SortDirection = "desc"
)

// SortKey is deliberately limited to exact stored metadata in the reference
// engine. Relevance order belongs to the later ranking pipeline.
type SortKey struct {
	Field     string        `json:"field"`
	Direction SortDirection `json:"direction,omitempty"`
}

type Query struct {
	Text          string            `json:"text"`
	Scope         Scope             `json:"scope"`
	Limit         int               `json:"limit"`
	Cursor        string            `json:"cursor,omitempty"`
	ExactLiterals []string          `json:"exact_literals,omitempty"`
	Filters       map[string]string `json:"filters,omitempty"`
	Channels      []string          `json:"channels,omitempty"`
	Order         []SortKey         `json:"order,omitempty"`
}

type ObjectKind string

const (
	ObjectRegular   ObjectKind = "file"
	ObjectDirectory ObjectKind = "directory"
	ObjectSymlink   ObjectKind = "symlink"
	ObjectOther     ObjectKind = "other"
)

// Metadata contains only exact filesystem observations made by the scanner.
// It does not contain extracted, inferred, or provider-supplied fields.
type Metadata struct {
	Name             string     `json:"name"`
	Kind             ObjectKind `json:"kind"`
	Size             int64      `json:"size"`
	Mode             uint32     `json:"mode"`
	ModifiedUnixNano int64      `json:"modified_unix_nano"`
}

type EvidenceKind string

const (
	EvidenceExactName   EvidenceKind = "exact_name"
	EvidenceExactPath   EvidenceKind = "exact_path"
	EvidenceMetadata    EvidenceKind = "metadata"
	EvidenceLexical     EvidenceKind = "lexical"
	EvidenceFuzzy       EvidenceKind = "fuzzy"
	EvidenceStructural  EvidenceKind = "structural"
	EvidenceContent     EvidenceKind = "content_provider"
	EvidenceFederated   EvidenceKind = "federated"
	EvidenceUnavailable EvidenceKind = "offline_catalogue"
)

// Evidence preserves an individual channel's claim. Score is meaningful only
// under Calibration; consumers must not compare unrelated raw scores directly.
type Evidence struct {
	Kind        EvidenceKind   `json:"kind"`
	Channel     string         `json:"channel"`
	Score       float64        `json:"score"`
	Calibration string         `json:"calibration,omitempty"`
	Exact       bool           `json:"exact"`
	Inferred    bool           `json:"inferred"`
	Anchor      string         `json:"anchor,omitempty"`
	Details     map[string]any `json:"details,omitempty"`
	ObservedAt  time.Time      `json:"observed_at,omitempty"`
}

type Result struct {
	Object      ObjectRef  `json:"object"`
	Metadata    Metadata   `json:"metadata"`
	Generation  Generation `json:"generation"`
	Rank        int        `json:"rank"`
	Certainty   float64    `json:"certainty"`
	Unavailable bool       `json:"unavailable"`
	Evidence    []Evidence `json:"evidence"`
}

// RootSpec is an already user-approved projection root. The development
// service additionally requires every path to remain inside its sandbox.
type RootSpec struct {
	ID   RootID `json:"id"`
	Path string `json:"path"`
}

type RootState struct {
	Root               RootSpec   `json:"root"`
	Generation         Generation `json:"generation"`
	Records            uint64     `json:"records"`
	Indexed            bool       `json:"indexed"`
	Stale              bool       `json:"stale"`
	LightModeThreshold uint32     `json:"light_mode_threshold"`
	LightDirectories   uint64     `json:"light_directories"`
	Warning            string     `json:"warning,omitempty"`
}

type RootPlan struct {
	Roots                       []RootSpec `json:"roots"`
	CurrentConfigurationDigest  string     `json:"current_configuration_digest,omitempty"`
	ProposedConfigurationDigest string     `json:"proposed_configuration_digest,omitempty"`
	Changed                     bool       `json:"changed"`
}

type ReconcileReport struct {
	Root       RootID     `json:"root"`
	Generation Generation `json:"generation"`
	Records    uint64     `json:"records"`
	Published  bool       `json:"published"`
	StartedAt  time.Time  `json:"started_at"`
	FinishedAt time.Time  `json:"finished_at"`
}

type QueryResponse struct {
	Generation Generation `json:"generation"`
	Results    []Result   `json:"results"`
	NextCursor string     `json:"next_cursor,omitempty"`
	Partial    bool       `json:"partial"`
	Warnings   []string   `json:"warnings,omitempty"`
	Plan       QueryPlan  `json:"plan"`
}

type QueryPlan struct {
	Scopes          []Scope  `json:"scopes"`
	Channels        []string `json:"channels"`
	UnavailableRoot []RootID `json:"unavailable_roots,omitempty"`
	StaleRoot       []RootID `json:"stale_roots,omitempty"`
	ElapsedMicros   int64    `json:"elapsed_micros"`
}

type Status struct {
	Protocol      string                 `json:"protocol"`
	Generation    Generation             `json:"generation"`
	Ready         bool                   `json:"ready"`
	Sandboxed     bool                   `json:"sandboxed"`
	Lifecycle     LifecycleStatus        `json:"lifecycle"`
	Work          WorkStatus             `json:"work"`
	Configuration EffectiveConfiguration `json:"configuration"`
	Capabilities  []CapabilityStatus     `json:"capabilities"`
	Roots         []RootID               `json:"roots,omitempty"`
	RootStates    []RootState            `json:"root_states,omitempty"`
	Warnings      []string               `json:"warnings,omitempty"`
}

type IntegrityReport struct {
	Generation Generation `json:"generation"`
	Healthy    bool       `json:"healthy"`
	Problems   []string   `json:"problems,omitempty"`
	Repairable bool       `json:"repairable"`
}

// Engine is deliberately read-oriented. Root admission, reconciliation, and
// rebuild are administrative projection operations; file mutation is absent.
type Engine interface {
	Status(context.Context) (Status, error)
	Query(context.Context, Query) (QueryResponse, error)
	Inspect(context.Context, ObjectRef) (Result, error)
	Integrity(context.Context) (IntegrityReport, error)
}

// ManagedEngine adds process-instance and enacted-configuration controls. A
// supervisor constructs/starts and restarts the process; Shutdown is the
// engine-owned draining half of that lifecycle.
type ManagedEngine interface {
	Engine
	Version() VersionInfo
	Configuration() EffectiveConfiguration
	Shutdown(context.Context) (LifecycleStatus, error)
}
