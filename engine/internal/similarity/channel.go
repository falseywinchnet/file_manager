// Package similarity defines the optional, versioned boundary through which a
// future Kolmogrov fixed-width hash family may propose candidates. It does not
// implement or select a hash, index, distance, or ranking technique.
package similarity

import (
	"context"
	"fmt"
	"time"

	"filemanager/engine/api"
)

const InterfaceVersion = "similarity-channel.v0"

// TerminalStatus is the explicit availability/result state of an experimental
// similarity projection. An unavailable or exhausted projection must never be
// reported as a successful empty candidate set.
type TerminalStatus string

const (
	StatusAvailableExperimental TerminalStatus = "available_experimental"
	StatusUnavailable           TerminalStatus = "unavailable"
	StatusUnsupportedInput      TerminalStatus = "unsupported_input"
	StatusOverCapacity          TerminalStatus = "over_capacity"
	StatusConfigurationMismatch TerminalStatus = "configuration_mismatch"
	StatusRebuildRequired       TerminalStatus = "rebuild_required"
	StatusBudgetExceeded        TerminalStatus = "budget_exceeded"
	StatusCancelled             TerminalStatus = "cancelled"
	StatusCorruptProjection     TerminalStatus = "corrupt_projection"
	StatusInternalFailure       TerminalStatus = "internal_failure"
)

type ChannelError struct {
	Status TerminalStatus
	Detail string
}

func (e *ChannelError) Error() string {
	return fmt.Sprintf("similarity %s: %s", e.Status, e.Detail)
}

type Configuration struct {
	Family           string `json:"family"`
	Revision         string `json:"revision"`
	WidthBytes       uint32 `json:"width_bytes"`
	ParametersDigest string `json:"parameters_digest"`
}

// Anchor binds every lossy sketch back to an exact catalogue observation.
type Anchor struct {
	Root       api.RootID     `json:"root"`
	Object     api.ObjectID   `json:"object"`
	Path       string         `json:"path"`
	Generation api.Generation `json:"generation"`
}

type Field struct {
	Name  string `json:"name"`
	Value string `json:"value"`
}

type Input struct {
	Anchor Anchor  `json:"anchor"`
	Fields []Field `json:"fields"`
}

type Sketch struct {
	Anchor        Anchor        `json:"anchor"`
	Configuration Configuration `json:"configuration"`
	Bytes         []byte        `json:"bytes"`
	Addresses     []Address     `json:"addresses,omitempty"`
}

type AddressKind string

const (
	AddressHistory AddressKind = "history"
	AddressFull    AddressKind = "full"
)

// Address is one independently retrievable fixed-width location. Multiple
// history addresses remain coupled internally by Key; splitting Key into
// separately accepted coordinates would destroy the common-history witness.
type Address struct {
	Kind         AddressKind `json:"kind"`
	SourceLength uint16      `json:"source_length"`
	Key          uint64      `json:"key"`
}

type QueryInput struct {
	Text   string  `json:"text"`
	Fields []Field `json:"fields"`
}

type Budget struct {
	CandidateLimit    uint32        `json:"candidate_limit"`
	PostingEntryLimit uint32        `json:"posting_entry_limit"`
	ProbeLimit        uint32        `json:"probe_limit"`
	Deadline          time.Duration `json:"deadline"`
}

type Contribution struct {
	Name  string  `json:"name"`
	Score float64 `json:"score"`
}

type Candidate struct {
	Anchor        Anchor              `json:"anchor"`
	Ordinal       uint32              `json:"ordinal"`
	RawScore      float64             `json:"raw_score"`
	Contributions []Contribution      `json:"contributions,omitempty"`
	Evidence      []CandidateEvidence `json:"evidence,omitempty"`
}

// CandidateEvidence reports hash work and provenance without inventing a
// relevance score. Exact edit/transposition adjudication is a later layer.
type CandidateEvidence struct {
	Channel       string   `json:"channel"`
	LengthPlans   []string `json:"length_plans"`
	MatchedKeys   []uint64 `json:"matched_keys,omitempty"`
	CandidateOnly bool     `json:"candidate_only"`
}

// Encoder and CandidateSource remain CANDIDATE interfaces until Kolmogrov has
// a winning configuration and the engine has transfer/equivalence evidence.
type Encoder interface {
	Configuration(context.Context) (Configuration, error)
	EncodeBatch(context.Context, []Input) ([]Sketch, error)
	EncodeQuery(context.Context, QueryInput) (Sketch, error)
}

type CandidateSource interface {
	Configuration(context.Context) (Configuration, error)
	Candidates(context.Context, Sketch, Budget) ([]Candidate, error)
	EraseGeneration(context.Context, api.Generation) error
}
