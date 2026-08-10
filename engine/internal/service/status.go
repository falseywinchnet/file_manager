package service

import (
	"context"
	"fmt"
	"sort"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
)

// Status returns a point-in-time lifecycle, capability, root, generation, and
// currentness snapshot. Readiness is reduced honestly by stale or incomplete
// observation state.
func (s *Service) Status(_ context.Context) (api.Status, error) {
	if s.durable != nil {
		return s.decorateStatus(s.persistentStatus()), nil
	}
	snapshot := s.store.Snapshot()
	status := api.Status{
		Protocol: api.ProtocolVersion, Generation: snapshot.Generation,
		Sandboxed: s.guard.Sandboxed(), Ready: len(snapshot.Roots) != 0,
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
		status.Warnings = append(status.Warnings, "no approved roots configured")
	}
	return s.decorateStatus(status), nil
}

func (s *Service) persistentStatus() api.Status {
	s.readerMu.RLock()
	defer s.readerMu.RUnlock()
	snapshot := s.store.Snapshot()
	status := api.Status{Protocol: api.ProtocolVersion, Sandboxed: s.guard.Sandboxed(), Ready: len(snapshot.Roots) == 1}
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
		status.Warnings = append(status.Warnings, "no approved root configured")
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

func (s *Service) decorateStatus(status api.Status) api.Status {
	status.Lifecycle, status.Work = s.lifecycleStatus()
	status.Configuration = s.Configuration()
	status.Capabilities = s.capabilities()
	background, stale, warning := s.backgroundWorkStatus(time.Now())
	background.Phase = status.Work.Phase
	background.ActiveRequests = status.Work.ActiveRequests
	background.ActiveQueries = status.Work.ActiveQueries
	background.ActiveAdministrative = status.Work.ActiveAdministrative
	status.Work = background
	if warning != "" {
		status.Warnings = append(status.Warnings, warning)
	}
	if stale {
		status.Ready = false
		for index := range status.RootStates {
			status.RootStates[index].Stale = true
			if status.RootStates[index].Warning == "" {
				if background.Currentness == api.CurrentnessCoverageIncomplete {
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
