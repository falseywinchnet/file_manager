package service

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"errors"
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
	s.live.Close()

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
