// Package similarity defines the optional, versioned boundary through which a
// future Kolmogrov fixed-width hash family may propose candidates. It does not
// implement or select a hash, index, distance, or ranking technique.
package similarity

import (
	"context"
	"time"

	"filemanager/engine/api"
)

const InterfaceVersion = "similarity-channel.v0"

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
}

type QueryInput struct {
	Text   string  `json:"text"`
	Fields []Field `json:"fields"`
}

type Budget struct {
	CandidateLimit uint32        `json:"candidate_limit"`
	Deadline       time.Duration `json:"deadline"`
}

type Contribution struct {
	Name  string  `json:"name"`
	Score float64 `json:"score"`
}

type Candidate struct {
	Anchor        Anchor         `json:"anchor"`
	RawScore      float64        `json:"raw_score"`
	Contributions []Contribution `json:"contributions,omitempty"`
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
