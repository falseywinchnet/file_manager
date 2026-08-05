package service

import (
	"context"
	"errors"
	"fmt"
	"sort"
	"sync"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/observation"
)

type BackgroundPolicy struct {
	Coalescer observation.Limits
	RetryMin  time.Duration
	RetryMax  time.Duration
}

func DefaultBackgroundPolicy() BackgroundPolicy {
	return BackgroundPolicy{
		Coalescer: observation.DefaultLimits(),
		RetryMin:  time.Second,
		RetryMax:  30 * time.Second,
	}
}

type backgroundObservation struct {
	mu         sync.Mutex
	starting   bool
	controller *backgroundController
}

type backgroundController struct {
	mu        sync.Mutex
	ctx       context.Context
	cancel    context.CancelFunc
	done      chan struct{}
	wake      chan struct{}
	batches   <-chan observation.Batch
	coalescer *observation.Coalescer
	policy    BackgroundPolicy

	running                 bool
	lastError               string
	retryAt                 time.Time
	retryDelay              time.Duration
	completeForExactCurrent bool
	coverageLimitation      string
}

// StartBackgroundObservation attaches one platform adapter to the service.
// It is an internal integration seam until a native adapter, authorization,
// and the atomic manifest-watermark schema pass their gates.
func (s *Service) StartBackgroundObservation(ctx context.Context, adapter observation.Adapter, policy BackgroundPolicy) error {
	if adapter == nil {
		return errors.New("background observation adapter is required")
	}
	if policy.RetryMin <= 0 || policy.RetryMax < policy.RetryMin {
		return errors.New("background retry bounds are invalid")
	}
	coalescer, err := observation.New(policy.Coalescer)
	if err != nil {
		return err
	}

	operationContext, finish, err := s.beginOperation(ctx, operationAdministrative, api.WorkPlanning)
	if err != nil {
		return err
	}
	defer finish()
	if len(s.store.Snapshot().Roots) == 0 {
		return api.NewFault(api.ErrorInvalidRequest, "background observation requires an approved root")
	}

	s.background.mu.Lock()
	if s.background.starting || (s.background.controller != nil && s.background.controller.isRunning()) {
		s.background.mu.Unlock()
		return api.NewFault(api.ErrorInvalidRequest, "background observation is already running")
	}
	s.background.starting = true
	s.background.mu.Unlock()
	defer func() {
		s.background.mu.Lock()
		s.background.starting = false
		s.background.mu.Unlock()
	}()

	s.lifecycle.mu.Lock()
	lifetime := s.lifecycle.lifetime
	s.lifecycle.mu.Unlock()
	backgroundContext, cancel := context.WithCancel(lifetime)
	stopStartupCancellation := context.AfterFunc(operationContext, cancel)
	subscription, err := adapter.Subscribe(backgroundContext)
	stopStartupCancellation()
	if err != nil {
		cancel()
		return fmt.Errorf("subscribe native observation adapter: %w", err)
	}
	if subscription.Batches == nil {
		cancel()
		return errors.New("native observation adapter returned no batch channel")
	}
	if err := coalescer.Initialize(
		subscription.Initial,
		true,
		"restart/start has no observation watermark committed with the exact generation",
	); err != nil {
		cancel()
		return err
	}
	if err := operationContext.Err(); err != nil {
		cancel()
		return err
	}

	controller := &backgroundController{
		ctx: backgroundContext, cancel: cancel, done: make(chan struct{}), wake: make(chan struct{}, 1),
		batches: subscription.Batches, coalescer: coalescer, policy: policy,
		running: true, retryDelay: policy.RetryMin,
		coverageLimitation: "native observation adapter did not declare exact-current coverage",
	}
	if reporter, ok := adapter.(observation.CoverageReporter); ok {
		coverage := reporter.ObservationCoverage()
		controller.completeForExactCurrent = coverage.CompleteForExactCurrent
		controller.coverageLimitation = coverage.Limitation
		if !coverage.CompleteForExactCurrent && controller.coverageLimitation == "" {
			controller.coverageLimitation = "native observation coverage is incomplete"
		}
	}
	s.background.mu.Lock()
	s.background.controller = controller
	s.background.mu.Unlock()
	go s.runBackgroundObservation(controller)
	return nil
}

func (c *backgroundController) isRunning() bool {
	c.mu.Lock()
	defer c.mu.Unlock()
	return c.running
}

// StopBackgroundObservation stops adapter consumption without stopping query
// service. Restarting observation establishes a new baseline and reconciles.
func (s *Service) StopBackgroundObservation(ctx context.Context) error {
	s.background.mu.Lock()
	controller := s.background.controller
	s.background.mu.Unlock()
	if controller == nil {
		return nil
	}
	controller.cancel()
	select {
	case <-ctx.Done():
		return ctx.Err()
	case <-controller.done:
		return nil
	}
}

func (s *Service) runBackgroundObservation(controller *backgroundController) {
	var workers sync.WaitGroup
	workers.Add(2)
	go func() {
		defer workers.Done()
		s.pumpBackgroundObservations(controller)
	}()
	go func() {
		defer workers.Done()
		s.reconcileBackgroundObservations(controller)
	}()
	workers.Wait()
	controller.mu.Lock()
	controller.running = false
	controller.mu.Unlock()
	close(controller.done)
}

func (s *Service) pumpBackgroundObservations(controller *backgroundController) {
	for {
		select {
		case <-controller.ctx.Done():
			return
		case batch, open := <-controller.batches:
			if !open {
				controller.mu.Lock()
				controller.coalescer.RequireReconcile("native observation stream closed")
				controller.lastError = "native observation stream closed"
				controller.mu.Unlock()
				controller.cancel()
				return
			}
			controller.mu.Lock()
			_, err := controller.coalescer.Ingest(time.Now(), batch)
			if err != nil {
				controller.lastError = err.Error()
			}
			controller.mu.Unlock()
			select {
			case controller.wake <- struct{}{}:
			default:
			}
		}
	}
}

func (s *Service) reconcileBackgroundObservations(controller *backgroundController) {
	timer := time.NewTimer(time.Hour)
	defer stopTimer(timer)
	stopTimer(timer)
	for {
		timerChannel := s.armBackgroundTimer(controller, timer)
		select {
		case <-controller.ctx.Done():
			stopTimer(timer)
			return
		case <-controller.wake:
			stopTimer(timer)
		case <-timerChannel:
			s.runBackgroundWork(controller)
		}
	}
}

func (s *Service) armBackgroundTimer(controller *backgroundController, timer *time.Timer) <-chan time.Time {
	controller.mu.Lock()
	defer controller.mu.Unlock()
	deadline, armed := controller.coalescer.Deadline()
	if !armed {
		return nil
	}
	now := time.Now()
	if deadline.IsZero() || deadline.Before(now) {
		deadline = now
	}
	if controller.retryAt.After(deadline) {
		deadline = controller.retryAt
	}
	stopTimer(timer)
	timer.Reset(deadline.Sub(now))
	return timer.C
}

func (s *Service) runBackgroundWork(controller *backgroundController) {
	now := time.Now()
	controller.mu.Lock()
	if controller.retryAt.After(now) {
		controller.mu.Unlock()
		return
	}
	work, ok := controller.coalescer.Take(now, false)
	controller.mu.Unlock()
	if !ok {
		return
	}

	roots, scopeWarning := s.backgroundRoots(work)
	var workErr error
	for _, root := range roots {
		_, err := s.Reconcile(controller.ctx, root)
		if err != nil {
			workErr = err
			break
		}
	}
	controller.mu.Lock()
	defer controller.mu.Unlock()
	if workErr != nil {
		_ = controller.coalescer.Finish(work.ID, false, "background reconciliation failed")
		controller.lastError = workErr.Error()
		controller.retryAt = time.Now().Add(controller.retryDelay)
		controller.retryDelay *= 2
		if controller.retryDelay > controller.policy.RetryMax {
			controller.retryDelay = controller.policy.RetryMax
		}
		return
	}
	_ = controller.coalescer.Finish(work.ID, true, "")
	controller.lastError = scopeWarning
	controller.retryAt = time.Time{}
	controller.retryDelay = controller.policy.RetryMin
}

func (s *Service) backgroundRoots(work observation.Work) ([]api.RootID, string) {
	snapshot := s.store.Snapshot()
	approved := make(map[api.RootID]struct{}, len(snapshot.Roots))
	for root := range snapshot.Roots {
		approved[root] = struct{}{}
	}
	selected := make(map[api.RootID]struct{})
	unknown := false
	if !work.RequiresFullScan {
		for _, change := range work.Changes {
			if _, ok := approved[change.Root]; !ok {
				unknown = true
				continue
			}
			selected[change.Root] = struct{}{}
		}
	}
	if work.RequiresFullScan || unknown {
		selected = approved
	}
	roots := make([]api.RootID, 0, len(selected))
	for root := range selected {
		roots = append(roots, root)
	}
	sort.Slice(roots, func(i, j int) bool { return roots[i] < roots[j] })
	if unknown {
		return roots, "native adapter delivered an event for an unapproved root"
	}
	return roots, ""
}

func (s *Service) invalidateBackground(reason string) {
	s.background.mu.Lock()
	controller := s.background.controller
	s.background.mu.Unlock()
	if controller == nil {
		return
	}
	controller.mu.Lock()
	if controller.running {
		controller.coalescer.RequireReconcile(reason)
	}
	controller.mu.Unlock()
	select {
	case controller.wake <- struct{}{}:
	default:
	}
}

func (s *Service) backgroundIngestionMode() string {
	s.background.mu.Lock()
	controller := s.background.controller
	s.background.mu.Unlock()
	if controller != nil && controller.isRunning() {
		return "native_adapter_experimental"
	}
	return "manual_reconcile"
}

func (s *Service) backgroundStale() bool {
	_, stale, _ := s.backgroundWorkStatus(time.Now())
	return stale
}

func (s *Service) backgroundWorkStatus(now time.Time) (api.WorkStatus, bool, string) {
	s.background.mu.Lock()
	controller := s.background.controller
	s.background.mu.Unlock()
	if controller == nil {
		return api.WorkStatus{Currentness: api.CurrentnessManual}, false, ""
	}
	controller.mu.Lock()
	defer controller.mu.Unlock()
	snapshot := controller.coalescer.Snapshot()
	status := api.WorkStatus{
		BackgroundIngestion: controller.running,
		BacklogKnown:        controller.running && snapshot.Initialized,
		PendingObservations: snapshot.PendingObservations,
		ObservationGap:      snapshot.ReconcileRequired,
		ObservationSource:   snapshot.Observed.Source,
		ObservationEpoch:    snapshot.Observed.Epoch,
		ObservedWatermark:   snapshot.Observed.Position,
		WatermarkDurable:    false,
		ObservationError:    controller.lastError,
		CoverageIncomplete:  !controller.completeForExactCurrent,
	}
	if snapshot.HasReconciled {
		status.ReconciledWatermark = snapshot.Reconciled.Position
	}
	if !snapshot.Oldest.IsZero() && now.After(snapshot.Oldest) {
		status.OldestObservationMS = uint64(now.Sub(snapshot.Oldest) / time.Millisecond)
	}
	switch {
	case !controller.running:
		status.Currentness = api.CurrentnessObservationUnavailable
	case snapshot.InFlight:
		status.Currentness = api.CurrentnessReconciling
	case snapshot.ReconcileRequired || !snapshot.HasReconciled:
		status.Currentness = api.CurrentnessBaselineRequired
	case snapshot.PendingObservations != 0 || !sameObservationStream(snapshot.Reconciled, snapshot.Observed) || snapshot.Reconciled.Position != snapshot.Observed.Position:
		status.Currentness = api.CurrentnessCatchingUp
	case status.CoverageIncomplete:
		status.Currentness = api.CurrentnessCoverageIncomplete
	default:
		status.Currentness = api.CurrentnessCurrentVolatile
	}
	warning := ""
	if status.Currentness == api.CurrentnessCoverageIncomplete {
		warning = controller.coverageLimitation + "; the exact catalogue is reconciled through delivered observations but currentness remains incomplete"
	} else if status.Currentness != api.CurrentnessCurrentVolatile {
		warning = "background observation has not reconciled the exact catalogue through its latest volatile watermark"
	} else if !status.WatermarkDurable {
		warning = "background currentness is volatile; the live generation manifest does not yet commit its observation watermark"
	}
	return status, status.Currentness != api.CurrentnessCurrentVolatile, warning
}

func sameObservationStream(left, right observation.Cursor) bool {
	return left.Source == right.Source && left.Epoch == right.Epoch
}

func stopTimer(timer *time.Timer) {
	if !timer.Stop() {
		select {
		case <-timer.C:
		default:
		}
	}
}
