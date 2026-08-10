// Package service coordinates root policy, scanning, exact queries, and
// immutable reader generations without exposing catalogue representation.
package service

import (
	"context"
	"errors"
	"fmt"
	"sync"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/generation"
	"filemanager/engine/internal/live"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/scan"
)

// Service is the transport-neutral engine façade. It coordinates exact
// catalogue state, durable reader generations, live traversal, lifecycle, and
// optional background observation while keeping their representations private.
type Service struct {
	guard            *sandbox.Guard
	store            *catalog.Store
	scanner          metadataScanner
	lifecycle        *serviceLifecycle
	admin            sync.Mutex
	durable          *generation.Store
	durableDirectory string
	readerMu         sync.RWMutex
	reader           *generation.Reader
	recoveryProblems int
	pendingRecovery  []generation.RecoveryProblem
	generationFloor  api.Generation
	quarantined      int
	quarantineError  string
	background       backgroundObservation
	live             *live.Manager
}

type metadataScanner interface {
	Scan(context.Context, api.RootSpec, scan.OwnsFunc) (*catalog.Shard, error)
}

var _ api.Engine = (*Service)(nil)
var _ api.ManagedEngine = (*Service)(nil)

// New constructs an in-memory development service beneath an explicit
// sandbox guard.
func New(guard *sandbox.Guard) (*Service, error) {
	if guard == nil {
		return nil, errors.New("sandbox guard is required")
	}
	lifecycle, err := newServiceLifecycle()
	if err != nil {
		return nil, err
	}
	return &Service{
		guard: guard, store: catalog.NewStore(), scanner: scan.Scanner{},
		lifecycle: lifecycle, live: live.NewManager(),
	}, nil
}

// NewPersistent opens the one-root M2 durable service. The store directory
// must already exist and may not lie beneath the indexed source root.
func NewPersistent(guard *sandbox.Guard, directory string) (*Service, error) {
	engine, err := New(guard)
	if err != nil {
		return nil, err
	}
	durable, err := generation.OpenStore(directory, nil)
	if err != nil {
		return nil, err
	}
	engine.durable = durable
	engine.durableDirectory = durable.Directory()
	floor, err := durable.GenerationFloor()
	if err != nil {
		return nil, fmt.Errorf("read durable generation high-water mark: %w", err)
	}
	engine.generationFloor = floor
	head, err := durable.Probe()
	if errors.Is(err, generation.ErrNoGeneration) {
		_, recovery, recoveryErr := durable.RecoverDetailed()
		engine.recoveryProblems = len(recovery.Problems)
		engine.pendingRecovery = append([]generation.RecoveryProblem(nil), recovery.Problems...)
		if recoveryErr != nil && !errors.Is(recoveryErr, generation.ErrNoGeneration) {
			return nil, fmt.Errorf("inspect damaged durable generation: %w", recoveryErr)
		}
		return engine, nil
	}
	if err != nil {
		return nil, fmt.Errorf("probe durable generation: %w", err)
	}
	resolved, err := guard.ResolveRoot(head.Root)
	if err != nil || resolved != head.Root.Path || pathContains(head.Root.Path, engine.durableDirectory) {
		return nil, errors.New("committed durable root is not admissible in this sandbox")
	}
	if _, _, err := engine.store.ApplyRoots([]api.RootSpec{head.Root}); err != nil {
		return nil, fmt.Errorf("restore durable root policy: %w", err)
	}
	reader, recovery, err := durable.RecoverDetailed()
	engine.recoveryProblems = len(recovery.Problems)
	if errors.Is(err, generation.ErrNoGeneration) {
		engine.pendingRecovery = append([]generation.RecoveryProblem(nil), recovery.Problems...)
		return engine, nil
	}
	if err != nil {
		return nil, fmt.Errorf("recover durable generation: %w", err)
	}
	engine.reader = reader
	if len(recovery.Problems) != 0 {
		moved, quarantineErr := durable.Quarantine(recovery.Problems)
		engine.quarantined = len(moved)
		if quarantineErr != nil {
			engine.quarantineError = quarantineErr.Error()
			engine.pendingRecovery = append([]generation.RecoveryProblem(nil), recovery.Problems...)
		} else {
			engine.recoveryProblems = 0
		}
	}
	return engine, nil
}

// Persistent reports whether the service owns a durable generation store.
func (s *Service) Persistent() bool { return s.durable != nil }

// Close drains the service and releases its current reader generation.
func (s *Service) Close() error {
	_, err := s.Shutdown(context.Background())
	return err
}

// SandboxRoot returns the canonical development containment root.
func (s *Service) SandboxRoot() string { return s.guard.Root() }
