package service

import (
	"context"
	"fmt"
	"slices"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/generation"
)

// statusSnapshot retains an immutable catalogue and copies reader/controller
// values. It retains no reader handle or mutable controller storage.
type statusSnapshot struct {
	catalogue        *catalog.Snapshot
	hasReader        bool
	readerRoot       api.RootSpec
	readerMetadata   generation.Metadata
	recoveryProblems int
	quarantined      int
	quarantineError  string
	background       backgroundStatusSnapshot
	capturedAt       time.Time
	lifecycle        api.LifecycleStatus
	work             api.WorkStatus
}

// Status returns catalogue and observation state captured together, with
// lifecycle, capabilities and configuration. ApplyRoots still publishes policy
// before invalidating observation; that administrative transition is not atomic.
func (s *Service) Status(_ context.Context) (api.Status, error) {
	var captured statusSnapshot = s.captureStatus()
	var status api.Status = s.projectStatus(captured)
	return status, nil
}

func (s *Service) captureStatus() statusSnapshot {
	var captured statusSnapshot = statusSnapshot{}
	// Lock order matches durable Query: readerMu precedes background.mu and
	// controller.mu. Never take admin here: reconciliation holds it during scans.
	// Holding controller.mu prevents Finish from advancing the watermark between
	// the catalogue load and the observation copy. Reader metadata is copied
	// while pinned by readerMu; no reader survives this capture.
	s.readerMu.RLock()
	s.background.mu.Lock()
	var controller *backgroundController = s.background.controller
	if controller != nil {
		controller.mu.Lock()
	}
	captured.catalogue = s.store.Snapshot()
	if s.reader != nil {
		captured.hasReader = true
		captured.readerRoot = s.reader.Root()
		captured.readerMetadata = s.reader.Metadata()
	}
	captured.recoveryProblems = s.recoveryProblems
	captured.quarantined = s.quarantined
	captured.quarantineError = s.quarantineError
	if controller != nil {
		captured.background = controller.captureStatusLocked()
	}
	captured.capturedAt = time.Now()
	if controller != nil {
		controller.mu.Unlock()
	}
	s.background.mu.Unlock()
	s.readerMu.RUnlock()
	captured.lifecycle, captured.work = s.lifecycleStatus()
	return captured
}

func (s *Service) projectStatus(captured statusSnapshot) api.Status {
	var ids []api.RootID = make([]api.RootID, 0, len(captured.catalogue.Roots))
	var id api.RootID = ""
	for id = range captured.catalogue.Roots {
		ids = append(ids, id)
	}
	slices.Sort(ids)
	var roots []api.RootSpec = make([]api.RootSpec, len(ids))
	var index int = 0
	for index = 0; index < len(ids); index++ {
		roots[index] = captured.catalogue.Roots[ids[index]].Spec
	}
	var status api.Status = api.Status{}
	if s.durable != nil {
		status = s.persistentStatus(captured, ids)
	} else {
		status = s.volatileStatus(captured, ids)
	}
	status.Lifecycle = captured.lifecycle
	var stale bool = false
	var warning string = ""
	status.Work, stale, warning = captured.background.project(captured.capturedAt)
	status.Work.Phase = captured.work.Phase
	status.Work.ActiveRequests = captured.work.ActiveRequests
	status.Work.ActiveQueries = captured.work.ActiveQueries
	status.Work.ActiveAdministrative = captured.work.ActiveAdministrative
	var ingestionMode string = "manual_reconcile"
	if status.Work.BackgroundIngestion {
		ingestionMode = "native_adapter_experimental"
	}
	var persistentPolicy bool = s.durable != nil && len(roots) == 1 && captured.hasReader && captured.readerRoot == roots[0]
	status.Configuration = s.effectiveConfigurationWithIngestion(roots, persistentPolicy, ingestionMode)
	status.Capabilities = s.capabilities()
	if warning != "" {
		status.Warnings = append(status.Warnings, warning)
	}
	if stale {
		status.Ready = false
		for index = 0; index < len(status.RootStates); index++ {
			status.RootStates[index].Stale = true
			if status.RootStates[index].Warning == "" {
				if status.Work.Currentness == api.CurrentnessCoverageIncomplete {
					status.RootStates[index].Warning = "background observation coverage is incomplete"
				} else {
					status.RootStates[index].Warning = "background currentness requires authoritative reconciliation"
				}
			}
		}
	}
	if status.Lifecycle.State != api.LifecycleReady {
		status.Ready = false
	}
	return status
}

func (s *Service) volatileStatus(captured statusSnapshot, ids []api.RootID) api.Status {
	var snapshot *catalog.Snapshot = captured.catalogue
	var status api.Status = api.Status{
		Protocol: api.ProtocolVersion, Generation: snapshot.Generation,
		Sandboxed: s.guard.Sandboxed(), Ready: len(ids) != 0,
	}
	if len(ids) != 0 {
		status.Roots = ids
		status.RootStates = make([]api.RootState, len(ids))
	}
	var index int = 0
	for index = 0; index < len(ids); index++ {
		var id api.RootID = ids[index]
		var projection catalog.Projection = snapshot.Roots[id]
		var state api.RootState = api.RootState{
			Root: projection.Spec, Generation: projection.Generation,
			Indexed: projection.Shard != nil, Stale: projection.Stale, Warning: projection.Warning,
			LightModeThreshold: catalog.LightModeImmediateChildren,
		}
		if projection.Shard != nil {
			var count int = projection.Shard.Len()
			state.Records = uint64(count)
			state.LightDirectories = projection.Shard.LightDirectoryCount()
		}
		status.RootStates[index] = state
		if !state.Indexed || state.Stale {
			status.Ready = false
		}
		if state.Warning != "" {
			var warning string = fmt.Sprintf("root %q: %s", id, state.Warning)
			status.Warnings = append(status.Warnings, warning)
		}
	}
	if len(ids) == 0 {
		status.Warnings = append(status.Warnings, "no approved roots configured")
	}
	return status
}

func (s *Service) persistentStatus(captured statusSnapshot, ids []api.RootID) api.Status {
	var status api.Status = api.Status{
		Protocol: api.ProtocolVersion, Sandboxed: s.guard.Sandboxed(), Ready: len(ids) == 1,
	}
	if len(ids) != 0 {
		status.Roots = ids
		status.RootStates = make([]api.RootState, len(ids))
	}
	var index int = 0
	for index = 0; index < len(ids); index++ {
		var id api.RootID = ids[index]
		var projection catalog.Projection = captured.catalogue.Roots[id]
		var state api.RootState = api.RootState{Root: projection.Spec, LightModeThreshold: catalog.LightModeImmediateChildren}
		if captured.hasReader && captured.readerRoot == projection.Spec {
			state.Generation = captured.readerMetadata.Generation
			state.Records = captured.readerMetadata.BindingCount
			state.LightDirectories = captured.readerMetadata.LightDirectories
			state.Indexed = true
			status.Generation = captured.readerMetadata.Generation
		} else {
			state.Stale = true
			state.Warning = "root requires durable reconciliation"
			status.Ready = false
		}
		status.RootStates[index] = state
		if state.Warning != "" {
			var warning string = fmt.Sprintf("root %q: %s", id, state.Warning)
			status.Warnings = append(status.Warnings, warning)
		}
	}
	if len(ids) == 0 {
		status.Ready = false
		status.Warnings = append(status.Warnings, "no approved root configured")
	}
	if captured.recoveryProblems != 0 {
		var detail string = "a full rebuild is required before quarantine"
		if captured.hasReader {
			detail = "serving the last valid generation"
		}
		var warning string = fmt.Sprintf("%d durable recovery candidate(s) failed validation; %s", captured.recoveryProblems, detail)
		status.Warnings = append(status.Warnings, warning)
	}
	if captured.quarantined != 0 {
		var warning string = fmt.Sprintf("%d rejected durable artifact(s) were preserved in the engine quarantine", captured.quarantined)
		status.Warnings = append(status.Warnings, warning)
	}
	if captured.quarantineError != "" {
		var warning string = "durable evidence quarantine is incomplete: " + captured.quarantineError
		status.Warnings = append(status.Warnings, warning)
	}
	return status
}
