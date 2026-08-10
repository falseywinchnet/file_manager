package service

import (
	"context"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/exact"
	"filemanager/engine/internal/ranking"
)

// Query searches one committed exact catalogue generation and returns
// evidence-bearing results with a generation-bound continuation cursor.
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

// QueryLive searches an already-approved filesystem scope without consulting
// or creating a catalogue. Continuation state is process-local, bounded, and
// discarded on expiry, root-policy change, completion, or shutdown.
func (s *Service) QueryLive(ctx context.Context, query api.LiveQuery) (api.LiveQueryResponse, error) {
	ctx, finish, err := s.beginOperation(ctx, operationQuery, "")
	if err != nil {
		return api.LiveQueryResponse{}, err
	}
	defer finish()
	if err := ctx.Err(); err != nil {
		return api.LiveQueryResponse{}, err
	}
	snapshot := s.store.Snapshot()
	projection, exists := snapshot.Projection(query.Scope.RootID)
	if !exists {
		return api.LiveQueryResponse{}, api.NewFault(api.ErrorUnapprovedRoot, "live-query root is not in the approved root policy")
	}
	resolved, err := s.guard.ResolveRoot(projection.Spec)
	if err != nil || resolved != projection.Spec.Path {
		return api.LiveQueryResponse{}, api.WrapFault(api.ErrorUnapprovedRoot, "approved live-query root no longer resolves inside the sandbox", err)
	}
	owns := func(root api.RootID, absolute string) bool {
		return snapshot.Owns(root, absolute) && s.guard.Allows(root, absolute)
	}
	return s.live.Query(ctx, projection.Spec, query, owns)
}

// Inspect returns one exact record and its stored provenance. A hard-linked
// object requires an observed path to disambiguate the binding.
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

// Integrity checks the current exact catalogue or durable generation without
// repairing it or mutating source files.
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
