// Package service coordinates root policy, scanning, exact queries, and
// immutable reader generations without exposing catalogue representation.
package service

import (
	"context"
	"errors"
	"fmt"
	"path/filepath"
	"sort"
	"strings"
	"sync"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/exact"
	"filemanager/engine/internal/generation"
	"filemanager/engine/internal/ranking"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/scan"
)

type Service struct {
	guard            *sandbox.Guard
	store            *catalog.Store
	scanner          metadataScanner
	lifecycle        serviceLifecycle
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
}

type metadataScanner interface {
	Scan(context.Context, api.RootSpec, scan.OwnsFunc) (*catalog.Shard, error)
}

var _ api.Engine = (*Service)(nil)
var _ api.ManagedEngine = (*Service)(nil)

func New(guard *sandbox.Guard) (*Service, error) {
	if guard == nil {
		return nil, errors.New("sandbox guard is required")
	}
	lifecycle, err := newServiceLifecycle()
	if err != nil {
		return nil, err
	}
	return &Service{guard: guard, store: catalog.NewStore(), scanner: scan.Scanner{}, lifecycle: lifecycle}, nil
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
	resolved, err := guard.ResolveDirectory(head.Root.Path)
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

func (s *Service) Persistent() bool { return s.durable != nil }

func (s *Service) Close() error {
	_, err := s.Shutdown(context.Background())
	return err
}

func (s *Service) SandboxRoot() string { return s.guard.Root() }

func (s *Service) PlanRoots(ctx context.Context, roots []api.RootSpec) (api.RootPlan, error) {
	ctx, finish, err := s.beginOperation(ctx, operationAdministrative, api.WorkPlanning)
	if err != nil {
		return api.RootPlan{}, err
	}
	defer finish()
	if err := ctx.Err(); err != nil {
		return api.RootPlan{}, err
	}
	return s.planRoots(roots)
}

func (s *Service) planRoots(roots []api.RootSpec) (api.RootPlan, error) {
	if s.durable != nil && len(roots) > 1 {
		return api.RootPlan{}, api.NewFault(api.ErrorInvalidRequest, "the M2 durable service currently admits exactly one root")
	}
	canonical := make([]api.RootSpec, len(roots))
	for index, root := range roots {
		if root.ID == "" {
			return api.RootPlan{}, api.NewFault(api.ErrorInvalidRequest, "root id is required")
		}
		resolved, err := s.guard.ResolveDirectory(root.Path)
		if err != nil {
			code := api.ErrorInvalidRequest
			if errors.Is(err, sandbox.ErrOutsideRoot) {
				code = api.ErrorOutsideRoot
			}
			return api.RootPlan{}, api.WrapFault(code, fmt.Sprintf("root %q is not an admissible sandbox directory", root.ID), err)
		}
		canonical[index] = api.RootSpec{ID: root.ID, Path: resolved}
		if s.durable != nil && pathContains(resolved, s.durableDirectory) {
			return api.RootPlan{}, api.NewFault(api.ErrorInvalidRequest, "engine store must remain outside the indexed source root")
		}
	}
	sort.Slice(canonical, func(i, j int) bool { return canonical[i].ID < canonical[j].ID })
	// Reuse Store validation without mutating service state.
	validation := catalog.NewStore()
	if _, _, err := validation.ApplyRoots(canonical); err != nil {
		return api.RootPlan{}, api.WrapFault(api.ErrorInvalidRequest, "root plan is invalid", err)
	}
	current := s.Configuration()
	proposed := s.effectiveConfiguration(canonical, false)
	return api.RootPlan{
		Roots: canonical, CurrentConfigurationDigest: current.Digest,
		ProposedConfigurationDigest: proposed.Digest, Changed: current.Digest != proposed.Digest,
	}, nil
}

func (s *Service) ApplyRoots(ctx context.Context, roots []api.RootSpec) (api.RootPlan, error) {
	return s.applyRoots(ctx, roots, "")
}

// ApplyRootsExpected prevents a stale administrative controller from
// overwriting a root policy planned against another effective configuration.
func (s *Service) ApplyRootsExpected(ctx context.Context, roots []api.RootSpec, expectedConfigurationDigest string) (api.RootPlan, error) {
	if expectedConfigurationDigest == "" {
		return api.RootPlan{}, api.NewFault(api.ErrorInvalidRequest, "expected configuration digest is required")
	}
	return s.applyRoots(ctx, roots, expectedConfigurationDigest)
}

func (s *Service) applyRoots(ctx context.Context, roots []api.RootSpec, expectedConfigurationDigest string) (api.RootPlan, error) {
	ctx, finish, err := s.beginOperation(ctx, operationAdministrative, api.WorkPlanning)
	if err != nil {
		return api.RootPlan{}, err
	}
	defer finish()
	s.admin.Lock()
	defer s.admin.Unlock()
	if err := ctx.Err(); err != nil {
		return api.RootPlan{}, err
	}
	if expectedConfigurationDigest != "" {
		current := s.Configuration().Digest
		if current != expectedConfigurationDigest {
			return api.RootPlan{}, api.NewFault(api.ErrorStaleConfiguration, "root policy was planned against a different effective configuration")
		}
	}
	plan, err := s.planRoots(roots)
	if err != nil {
		return api.RootPlan{}, err
	}
	if _, _, err := s.store.ApplyRoots(plan.Roots); err != nil {
		return api.RootPlan{}, api.WrapFault(api.ErrorInvalidRequest, "apply root plan", err)
	}
	s.invalidateBackground("approved root policy changed")
	if s.durable != nil {
		s.readerMu.Lock()
		keep := len(plan.Roots) == 1 && s.reader != nil && s.reader.Root() == plan.Roots[0]
		if !keep && s.reader != nil {
			_ = s.reader.Close()
			s.reader = nil
		}
		s.readerMu.Unlock()
	}
	return plan, nil
}

func (s *Service) Reconcile(ctx context.Context, rootID api.RootID) (api.ReconcileReport, error) {
	ctx, finish, err := s.beginOperation(ctx, operationAdministrative, api.WorkReconciling)
	if err != nil {
		return api.ReconcileReport{}, err
	}
	defer finish()
	return s.reconcile(ctx, rootID, false)
}

func (s *Service) reconcile(ctx context.Context, rootID api.RootID, forcePublication bool) (api.ReconcileReport, error) {
	s.admin.Lock()
	defer s.admin.Unlock()
	started := time.Now().UTC()
	snapshot := s.store.Snapshot()
	projection, exists := snapshot.Projection(rootID)
	if !exists {
		return api.ReconcileReport{}, api.NewFault(api.ErrorUnapprovedRoot, "reconcile root is not configured")
	}
	shard, err := s.scanner.Scan(ctx, projection.Spec, snapshot.OwnsProjected)
	if err != nil {
		if contextErr := ctx.Err(); contextErr != nil {
			return api.ReconcileReport{}, contextErr
		}
		_, _ = s.store.MarkStale(rootID, "last reconciliation failed")
		return api.ReconcileReport{}, api.WrapFault(api.ErrorInternal, "reconcile approved root", err)
	}
	if s.durable != nil {
		// A complete authoritative scan may discover no exact-state change. Do
		// not turn that into a generation, manifest, sync, or cleanup write.
		// Pending recovery evidence is excluded because its preservation work
		// must still run through the existing checked-publication path.
		if !forcePublication && len(s.pendingRecovery) == 0 && s.quarantineError == "" {
			s.readerMu.RLock()
			current := s.reader
			if current != nil && current.Root() == projection.Spec && current.Digest() == shard.Digest() {
				generationID := current.Metadata().Generation
				s.readerMu.RUnlock()
				return api.ReconcileReport{
					Root: rootID, Generation: generationID, Records: uint64(shard.Len()), Published: false,
					StartedAt: started, FinishedAt: time.Now().UTC(),
				}, nil
			}
			s.readerMu.RUnlock()
		}
		nextGeneration := s.generationFloor + 1
		if nextGeneration == 0 {
			return api.ReconcileReport{}, api.NewFault(api.ErrorIntegrity, "durable generation counter is exhausted")
		}
		if head, err := s.durable.Probe(); err == nil {
			if head.Metadata.Generation > s.generationFloor {
				s.generationFloor = head.Metadata.Generation
				nextGeneration = s.generationFloor + 1
			}
		} else if !errors.Is(err, generation.ErrNoGeneration) {
			return api.ReconcileReport{}, api.WrapFault(api.ErrorIntegrity, "probe durable generation", err)
		}
		if nextGeneration == 0 {
			return api.ReconcileReport{}, api.NewFault(api.ErrorIntegrity, "durable generation counter is exhausted")
		}
		reader, err := s.durable.Publish(nextGeneration, shard)
		if err != nil {
			return api.ReconcileReport{}, api.WrapFault(api.ErrorInternal, "publish durable catalogue generation", err)
		}
		moved := []generation.Quarantined(nil)
		var quarantineErr error
		if len(s.pendingRecovery) != 0 {
			moved, quarantineErr = s.durable.Quarantine(s.pendingRecovery)
		}
		s.readerMu.Lock()
		old := s.reader
		s.reader = reader
		s.generationFloor = nextGeneration
		s.quarantined += len(moved)
		if quarantineErr == nil {
			s.recoveryProblems = 0
			s.pendingRecovery = nil
			s.quarantineError = ""
		} else {
			s.quarantineError = quarantineErr.Error()
		}
		if old != nil {
			_ = old.Close()
		}
		s.readerMu.Unlock()
		return api.ReconcileReport{
			Root: rootID, Generation: nextGeneration, Records: uint64(shard.Len()), Published: true,
			StartedAt: started, FinishedAt: time.Now().UTC(),
		}, nil
	}
	committed, err := s.store.Publish(rootID, shard)
	if err != nil {
		return api.ReconcileReport{}, api.WrapFault(api.ErrorInternal, "publish catalogue generation", err)
	}
	return api.ReconcileReport{
		Root: rootID, Generation: committed.Generation, Records: uint64(shard.Len()), Published: true,
		StartedAt: started, FinishedAt: time.Now().UTC(),
	}, nil
}

// Rebuild performs a full authoritative scan and publishes it through the same
// immutable commit path as reconciliation. The last valid reader remains
// available until publication succeeds.
func (s *Service) Rebuild(ctx context.Context, rootID api.RootID) (api.ReconcileReport, error) {
	ctx, finish, err := s.beginOperation(ctx, operationAdministrative, api.WorkRebuilding)
	if err != nil {
		return api.ReconcileReport{}, err
	}
	defer finish()
	return s.reconcile(ctx, rootID, true)
}

func (s *Service) Status(_ context.Context) (api.Status, error) {
	if s.durable != nil {
		return s.decorateStatus(s.persistentStatus()), nil
	}
	snapshot := s.store.Snapshot()
	status := api.Status{
		Protocol: api.ProtocolVersion, Generation: snapshot.Generation,
		Sandboxed: true, Ready: len(snapshot.Roots) != 0,
	}
	ids := make([]api.RootID, 0, len(snapshot.Roots))
	for id := range snapshot.Roots {
		ids = append(ids, id)
	}
	sort.Slice(ids, func(i, j int) bool { return ids[i] < ids[j] })
	for _, id := range ids {
		projection := snapshot.Roots[id]
		state := api.RootState{
			Root: projection.Spec, Generation: projection.Generation,
			Indexed: projection.Shard != nil, Stale: projection.Stale, Warning: projection.Warning,
			LightModeThreshold: catalog.LightModeImmediateChildren,
		}
		if projection.Shard != nil {
			state.Records = uint64(projection.Shard.Len())
			state.LightDirectories = projection.Shard.LightDirectoryCount()
		}
		status.Roots = append(status.Roots, id)
		status.RootStates = append(status.RootStates, state)
		if !state.Indexed || state.Stale {
			status.Ready = false
		}
		if state.Warning != "" {
			status.Warnings = append(status.Warnings, fmt.Sprintf("root %q: %s", id, state.Warning))
		}
	}
	if len(snapshot.Roots) == 0 {
		status.Warnings = append(status.Warnings, "no approved sandbox roots configured")
	}
	return s.decorateStatus(status), nil
}

func (s *Service) persistentStatus() api.Status {
	s.readerMu.RLock()
	defer s.readerMu.RUnlock()
	snapshot := s.store.Snapshot()
	status := api.Status{Protocol: api.ProtocolVersion, Sandboxed: true, Ready: len(snapshot.Roots) == 1}
	ids := make([]api.RootID, 0, len(snapshot.Roots))
	for id := range snapshot.Roots {
		ids = append(ids, id)
	}
	sort.Slice(ids, func(i, j int) bool { return ids[i] < ids[j] })
	for _, id := range ids {
		projection := snapshot.Roots[id]
		state := api.RootState{Root: projection.Spec, LightModeThreshold: catalog.LightModeImmediateChildren}
		if s.reader != nil && s.reader.Root() == projection.Spec {
			metadata := s.reader.Metadata()
			state.Generation = metadata.Generation
			state.Records = metadata.BindingCount
			state.LightDirectories = metadata.LightDirectories
			state.Indexed = true
			status.Generation = metadata.Generation
		} else {
			state.Stale = true
			state.Warning = "root requires durable reconciliation"
			status.Ready = false
		}
		status.Roots = append(status.Roots, id)
		status.RootStates = append(status.RootStates, state)
		if state.Warning != "" {
			status.Warnings = append(status.Warnings, fmt.Sprintf("root %q: %s", id, state.Warning))
		}
	}
	if len(snapshot.Roots) == 0 {
		status.Ready = false
		status.Warnings = append(status.Warnings, "no approved sandbox root configured")
	}
	if s.recoveryProblems != 0 {
		detail := "a full rebuild is required before quarantine"
		if s.reader != nil {
			detail = "serving the last valid generation"
		}
		status.Warnings = append(status.Warnings, fmt.Sprintf(
			"%d durable recovery candidate(s) failed validation; %s", s.recoveryProblems, detail,
		))
	}
	if s.quarantined != 0 {
		status.Warnings = append(status.Warnings, fmt.Sprintf(
			"%d rejected durable artifact(s) were preserved in the engine quarantine", s.quarantined,
		))
	}
	if s.quarantineError != "" {
		status.Warnings = append(status.Warnings, "durable evidence quarantine is incomplete: "+s.quarantineError)
	}
	return status
}

func (s *Service) Query(ctx context.Context, query api.Query) (api.QueryResponse, error) {
	ctx, finish, err := s.beginOperation(ctx, operationQuery, "")
	if err != nil {
		return api.QueryResponse{}, err
	}
	defer finish()
	started := time.Now()
	if err := ctx.Err(); err != nil {
		return api.QueryResponse{}, err
	}
	snapshot := s.store.Snapshot()
	var matches []exact.Match
	var cursor string
	var generationID api.Generation
	var queryErr error
	if s.durable != nil {
		s.readerMu.RLock()
		defer s.readerMu.RUnlock()
		if s.reader == nil {
			return api.QueryResponse{}, api.NewFault(api.ErrorMethodUnavailable, "query scope has no checked durable generation")
		}
		generationID = s.reader.Metadata().Generation
		matches, cursor, queryErr = exact.QueryIndex(ctx, snapshot, generationID, s.reader.Root().ID, s.reader, query)
	} else {
		generationID = snapshot.Generation
		matches, cursor, queryErr = exact.Query(ctx, snapshot, query)
	}
	if queryErr != nil {
		return api.QueryResponse{}, queryErr
	}
	if err := ctx.Err(); err != nil {
		return api.QueryResponse{}, err
	}
	response := api.QueryResponse{
		Generation: generationID,
		Results:    ranking.Exact(matches, generationID),
		NextCursor: cursor,
		Plan: api.QueryPlan{
			Scopes: []api.Scope{query.Scope}, Channels: []string{"exact"},
			ElapsedMicros: time.Since(started).Microseconds(),
		},
	}
	if projection, ok := snapshot.Projection(query.Scope.Root); s.durable == nil && ok && projection.Stale {
		response.Plan.StaleRoot = []api.RootID{query.Scope.Root}
		response.Warnings = append(response.Warnings, "query used a stale catalogue generation")
	}
	if s.backgroundStale() {
		if len(response.Plan.StaleRoot) == 0 {
			response.Plan.StaleRoot = []api.RootID{query.Scope.Root}
		}
		response.Warnings = append(response.Warnings, "query used a generation not reconciled through the active observation stream")
	}
	return response, nil
}

func (s *Service) Inspect(ctx context.Context, ref api.ObjectRef) (api.Result, error) {
	ctx, finish, operationErr := s.beginOperation(ctx, operationQuery, "")
	if operationErr != nil {
		return api.Result{}, operationErr
	}
	defer finish()
	if err := ctx.Err(); err != nil {
		return api.Result{}, err
	}
	snapshot := s.store.Snapshot()
	var record catalog.Record
	var generationID api.Generation
	var err error
	if s.durable != nil {
		s.readerMu.RLock()
		defer s.readerMu.RUnlock()
		if s.reader == nil {
			return api.Result{}, api.NewFault(api.ErrorMethodUnavailable, "inspect requires a checked durable generation")
		}
		generationID = s.reader.Metadata().Generation
		record, err = exact.InspectIndex(snapshot, s.reader.Root().ID, s.reader, ref)
	} else {
		generationID = snapshot.Generation
		record, err = exact.Inspect(snapshot, ref)
	}
	if err != nil {
		return api.Result{}, err
	}
	evidence := []api.Evidence{{Kind: api.EvidenceMetadata, Channel: "exact", Score: 1, Calibration: "exact-v0", Exact: true, Anchor: record.Path}}
	return ranking.Record(record, generationID, 1, evidence), nil
}

func (s *Service) Integrity(ctx context.Context) (api.IntegrityReport, error) {
	ctx, finish, operationErr := s.beginOperation(ctx, operationAdministrative, api.WorkIntegrityCheck)
	if operationErr != nil {
		return api.IntegrityReport{}, operationErr
	}
	defer finish()
	if err := ctx.Err(); err != nil {
		return api.IntegrityReport{}, err
	}
	if s.durable == nil {
		return catalog.Check(s.store.Snapshot()), nil
	}
	s.readerMu.RLock()
	defer s.readerMu.RUnlock()
	if s.reader == nil {
		return api.IntegrityReport{Healthy: false, Repairable: true, Problems: []string{"no checked durable generation"}}, nil
	}
	metadata := s.reader.Metadata()
	report := api.IntegrityReport{Generation: metadata.Generation, Healthy: true, Repairable: true}
	if err := s.reader.Check(metadata.SegmentDigest); err != nil {
		report.Healthy = false
		report.Problems = []string{"durable generation checksum mismatch"}
	}
	return report, nil
}

func pathContains(root, path string) bool {
	if root == path {
		return true
	}
	relative, err := filepath.Rel(root, path)
	return err == nil && relative != ".." && !strings.HasPrefix(relative, ".."+string(filepath.Separator))
}
