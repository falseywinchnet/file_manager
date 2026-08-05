package service

import (
	"context"
	"crypto/rand"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"errors"
	"sort"
	"sync"
	"time"

	"filemanager/engine/api"
)

type operationClass uint8

const (
	operationQuery operationClass = iota + 1
	operationAdministrative
)

type serviceLifecycle struct {
	mu             *sync.Mutex
	shutdown       *sync.Mutex
	state          api.LifecycleState
	generation     uint64
	instanceID     string
	startedAt      time.Time
	active         uint32
	activeQueries  uint32
	activeAdmin    uint32
	phase          api.WorkPhase
	drained        chan struct{}
	drainedClosed  bool
	lifetime       context.Context
	cancelLifetime context.CancelFunc
}

func newServiceLifecycle() (serviceLifecycle, error) {
	var identity [16]byte
	if _, err := rand.Read(identity[:]); err != nil {
		return serviceLifecycle{}, errors.New("create service instance identity: " + err.Error())
	}
	lifetime, cancel := context.WithCancel(context.Background())
	return serviceLifecycle{
		mu: &sync.Mutex{}, shutdown: &sync.Mutex{}, state: api.LifecycleReady, generation: 1,
		instanceID: hex.EncodeToString(identity[:]), startedAt: time.Now().UTC(), phase: api.WorkIdle,
		drained: make(chan struct{}), lifetime: lifetime, cancelLifetime: cancel,
	}, nil
}

func (s *Service) beginOperation(ctx context.Context, class operationClass, phase api.WorkPhase) (context.Context, func(), error) {
	s.lifecycle.mu.Lock()
	if s.lifecycle.state != api.LifecycleReady {
		state := s.lifecycle.state
		s.lifecycle.mu.Unlock()
		return nil, nil, api.NewFault(api.ErrorMethodUnavailable, "engine service is "+string(state))
	}
	s.lifecycle.active++
	if class == operationQuery {
		s.lifecycle.activeQueries++
	} else {
		s.lifecycle.activeAdmin++
		if phase != "" {
			s.lifecycle.phase = phase
		}
	}
	lifetime := s.lifecycle.lifetime
	s.lifecycle.mu.Unlock()

	operationContext, cancel := context.WithCancel(ctx)
	stopLifetime := context.AfterFunc(lifetime, cancel)
	var once sync.Once
	finish := func() {
		once.Do(func() {
			stopLifetime()
			cancel()
			s.finishOperation(class)
		})
	}
	return operationContext, finish, nil
}

func (s *Service) finishOperation(class operationClass) {
	s.lifecycle.mu.Lock()
	defer s.lifecycle.mu.Unlock()
	if s.lifecycle.active != 0 {
		s.lifecycle.active--
	}
	if class == operationQuery && s.lifecycle.activeQueries != 0 {
		s.lifecycle.activeQueries--
	}
	if class == operationAdministrative && s.lifecycle.activeAdmin != 0 {
		s.lifecycle.activeAdmin--
		if s.lifecycle.activeAdmin == 0 && s.lifecycle.state == api.LifecycleReady {
			s.lifecycle.phase = api.WorkIdle
		}
	}
	if s.lifecycle.state == api.LifecycleDraining && s.lifecycle.active == 0 && !s.lifecycle.drainedClosed {
		close(s.lifecycle.drained)
		s.lifecycle.drainedClosed = true
	}
}

func (s *Service) lifecycleStatus() (api.LifecycleStatus, api.WorkStatus) {
	s.lifecycle.mu.Lock()
	defer s.lifecycle.mu.Unlock()
	phase := s.lifecycle.phase
	if s.lifecycle.state == api.LifecycleDraining {
		phase = api.WorkDraining
	}
	return api.LifecycleStatus{
			State: s.lifecycle.state, Generation: s.lifecycle.generation,
			InstanceID: s.lifecycle.instanceID, StartedAt: s.lifecycle.startedAt,
		}, api.WorkStatus{
			Phase: phase, ActiveRequests: s.lifecycle.active,
			ActiveQueries: s.lifecycle.activeQueries, ActiveAdministrative: s.lifecycle.activeAdmin,
			BackgroundIngestion: false, BacklogKnown: false, Currentness: api.CurrentnessManual,
		}
}

// Shutdown transitions the process instance to draining, cancels service-owned
// operation contexts, waits for their bounded cleanup, and closes the current
// exact reader. Restart is intentionally a supervisor operation that constructs
// a new process instance and recovers committed state.
func (s *Service) Shutdown(ctx context.Context) (api.LifecycleStatus, error) {
	s.lifecycle.shutdown.Lock()
	defer s.lifecycle.shutdown.Unlock()

	s.lifecycle.mu.Lock()
	switch s.lifecycle.state {
	case api.LifecycleStopped:
		status, _ := s.lifecycleStatusLocked()
		s.lifecycle.mu.Unlock()
		return status, nil
	case api.LifecycleReady:
		s.lifecycle.state = api.LifecycleDraining
		s.lifecycle.generation++
		s.lifecycle.phase = api.WorkDraining
		s.lifecycle.cancelLifetime()
		if s.lifecycle.active == 0 && !s.lifecycle.drainedClosed {
			close(s.lifecycle.drained)
			s.lifecycle.drainedClosed = true
		}
	case api.LifecycleDraining:
	case api.LifecycleStarting, api.LifecycleFaulted:
		state := s.lifecycle.state
		s.lifecycle.mu.Unlock()
		return api.LifecycleStatus{}, api.NewFault(api.ErrorMethodUnavailable, "cannot stop engine service from "+string(state))
	}
	drained := s.lifecycle.drained
	s.lifecycle.mu.Unlock()

	select {
	case <-ctx.Done():
		return api.LifecycleStatus{}, ctx.Err()
	case <-drained:
	}
	if err := s.StopBackgroundObservation(ctx); err != nil {
		return api.LifecycleStatus{}, err
	}

	s.admin.Lock()
	s.readerMu.Lock()
	var closeErr error
	if s.reader != nil {
		closeErr = s.reader.Close()
		s.reader = nil
	}
	s.readerMu.Unlock()
	s.admin.Unlock()

	s.lifecycle.mu.Lock()
	if closeErr != nil {
		s.lifecycle.state = api.LifecycleFaulted
	} else {
		s.lifecycle.state = api.LifecycleStopped
	}
	s.lifecycle.generation++
	s.lifecycle.phase = api.WorkIdle
	status, _ := s.lifecycleStatusLocked()
	s.lifecycle.mu.Unlock()
	return status, closeErr
}

func (s *Service) lifecycleStatusLocked() (api.LifecycleStatus, api.WorkStatus) {
	return api.LifecycleStatus{
			State: s.lifecycle.state, Generation: s.lifecycle.generation,
			InstanceID: s.lifecycle.instanceID, StartedAt: s.lifecycle.startedAt,
		}, api.WorkStatus{
			Phase: s.lifecycle.phase, ActiveRequests: s.lifecycle.active,
			ActiveQueries: s.lifecycle.activeQueries, ActiveAdministrative: s.lifecycle.activeAdmin,
			BackgroundIngestion: false, BacklogKnown: false, Currentness: api.CurrentnessManual,
		}
}

func (s *Service) Version() api.VersionInfo {
	lifecycle, _ := s.lifecycleStatus()
	features := []string{"exact-reference", "integrity", "root-policy", "scan-reconcile", "service-lifecycle", "effective-configuration"}
	if s.Persistent() {
		features = append(features, "immutable-generation-v1")
	}
	if s.backgroundIngestionMode() != "manual_reconcile" {
		features = append(features, "background-observation-experimental")
	}
	return api.VersionInfo{
		Component: api.EngineComponentName, BuildVersion: api.EngineBuildVersion,
		Protocol: api.ProtocolVersion, InstanceID: lifecycle.InstanceID,
		Features: features, Capabilities: s.capabilities(),
	}
}

func (s *Service) Configuration() api.EffectiveConfiguration {
	snapshot := s.store.Snapshot()
	roots := make([]api.RootSpec, 0, len(snapshot.Roots))
	for _, projection := range snapshot.Roots {
		roots = append(roots, projection.Spec)
	}
	sort.Slice(roots, func(i, j int) bool { return roots[i].ID < roots[j].ID })
	persistentPolicy := false
	if s.durable != nil {
		s.readerMu.RLock()
		persistentPolicy = len(roots) == 1 && s.reader != nil && s.reader.Root() == roots[0]
		s.readerMu.RUnlock()
	}
	return s.effectiveConfiguration(roots, persistentPolicy)
}

func (s *Service) effectiveConfiguration(roots []api.RootSpec, persistentPolicy bool) api.EffectiveConfiguration {
	configuration := api.EffectiveConfiguration{
		Schema: api.EngineConfigurationSchema, SchemaMajor: 0, SchemaMinor: 1,
		Deployment: "development_sandbox", Sandboxed: true, Persistent: s.Persistent(),
		StoreOutsideRoots: s.durable == nil || !anyRootContains(roots, s.durableDirectory),
		IngestionMode:     s.backgroundIngestionMode(), RootPolicy: roots, RootPolicyPersistent: persistentPolicy,
	}
	digestMaterial := configuration
	digestMaterial.Digest = ""
	digestMaterial.RootPolicyPersistent = false
	encoded, _ := json.Marshal(digestMaterial)
	digest := sha256.Sum256(encoded)
	configuration.Digest = hex.EncodeToString(digest[:])
	return configuration
}

func anyRootContains(roots []api.RootSpec, path string) bool {
	for _, root := range roots {
		if pathContains(root.Path, path) {
			return true
		}
	}
	return false
}

func (s *Service) capabilities() []api.CapabilityStatus {
	persistentState := api.CapabilityUnavailable
	persistentReason := "persistent store is not enabled for this process instance"
	if s.Persistent() {
		persistentState, persistentReason = api.CapabilityAvailable, ""
	}
	return []api.CapabilityStatus{
		{ID: "engine.lifecycle", State: api.CapabilityAvailable, Revision: "0.1"},
		{ID: "engine.configuration.inspect", State: api.CapabilityAvailable, Revision: "0.1"},
		{ID: "engine.exact.catalogue", State: api.CapabilityAvailable, Revision: "reference-v1"},
		{ID: "engine.exact.query", State: api.CapabilityAvailable, Revision: "engine.v0"},
		{ID: "engine.ordered_metadata", State: api.CapabilityAvailable, Revision: "reference-v1"},
		{ID: "engine.root_policy", State: api.CapabilityAvailable, Revision: "development-sandbox-v0"},
		{ID: "engine.scan.reconcile", State: api.CapabilityAvailable, Revision: "metadata-full-scan-v0"},
		{ID: "engine.durable.immutable_generation", State: persistentState, Revision: "v1", Reason: persistentReason},
		{ID: "engine.delta_publication", State: api.CapabilityExperimental, Revision: "component-only", Reason: "not admitted to the live manifest or service"},
		{ID: "engine.lexical.index", State: api.CapabilityUnavailable, Reason: "M3 lexical dictionary and postings are not implemented"},
		{ID: "engine.content_feature.intake", State: api.CapabilityDeferred, Reason: "anchored fragment/provider input contract is not reconciled"},
		{ID: "engine.fragment.descriptor", State: api.CapabilityDeferred, Reason: "content-fragment boundaries, anchors, privacy policy, and descriptor lifecycle are not reconciled"},
		{ID: "engine.similarity.fixed_width", State: api.CapabilityExperimental, Revision: "history-tuple-0.1", Reason: "candidate channel is not admitted to the public planner"},
		{ID: "engine.background.observation", State: api.CapabilityExperimental, Revision: "portable-coalescer-0.2+macos-fsevents-0.2+windows-rdcw-0.1", Reason: "bounded macOS and Windows native adapters exist, but coverage is incomplete, the watermark is volatile, Linux is absent, and Windows has compatibility-only validation"},
		{ID: "engine.status.subscribe", State: api.CapabilityNegotiating, Reason: "bounded replay and overflow fixtures remain red"},
		{ID: "engine.transport.framed_local", State: api.CapabilityDeferred, Reason: "production authentication and framed transport are not implemented"},
		{ID: "contract.ORC-LIF-001", State: api.CapabilityNegotiating, Reason: "engine lifecycle projection awaits Orchestrator reconciliation"},
		{ID: "contract.ORC-ENG-001", State: api.CapabilityNegotiating, Reason: "exact subset exists; envelope and conformance are not frozen"},
		{ID: "contract.ORC-ENG-002", State: api.CapabilityNegotiating, Reason: "development admin subset exists; production authorization is not frozen"},
		{ID: "contract.ORC-ENG-003", State: api.CapabilityUnavailable, Reason: "status event stream is not implemented"},
	}
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
