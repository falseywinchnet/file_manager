package service

import (
	"context"
	"errors"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/generation"
)

// Reconcile performs a complete authoritative metadata scan of an approved
// root and publishes a new generation only when its exact state changed.
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
	owns := func(root api.RootID, absolute string) bool {
		return snapshot.OwnsProjected(root, absolute) && s.guard.Allows(root, absolute)
	}
	shard, err := s.scanner.Scan(ctx, projection.Spec, owns)
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
