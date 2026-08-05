package service

import (
	"context"
	"os"
	"path/filepath"
	"sync/atomic"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/observation"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/scan"
)

type channelObservationAdapter struct {
	initial observation.Cursor
	batches chan observation.Batch
}

type blockingMetadataScanner struct {
	delegate scan.Scanner
	started  chan struct{}
	release  chan struct{}
	calls    atomic.Int32
}

func (s *blockingMetadataScanner) Scan(ctx context.Context, root api.RootSpec, owns scan.OwnsFunc) (*catalog.Shard, error) {
	s.calls.Add(1)
	s.started <- struct{}{}
	select {
	case <-ctx.Done():
		return nil, ctx.Err()
	case <-s.release:
		return s.delegate.Scan(ctx, root, owns)
	}
}

func (a *channelObservationAdapter) Subscribe(context.Context) (observation.Subscription, error) {
	return observation.Subscription{Initial: a.initial, Batches: a.batches}, nil
}

func TestBackgroundObservationBaselineGapAndStopState(t *testing.T) {
	engine, sandboxPath := testService(t)
	defer engine.Close()
	source := filepath.Join(sandboxPath, "observed")
	if err := os.Mkdir(source, 0o700); err != nil {
		t.Fatal(err)
	}
	firstPath := filepath.Join(source, "first.txt")
	if err := os.WriteFile(firstPath, []byte("first"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}

	adapter := &channelObservationAdapter{
		initial: observation.Cursor{Source: "fixture", Epoch: "boot-1", Position: 10},
		batches: make(chan observation.Batch, 16),
	}
	policy := backgroundTestPolicy()
	if err := engine.StartBackgroundObservation(context.Background(), adapter, policy); err != nil {
		t.Fatal(err)
	}
	baseline := waitForBackgroundState(t, engine, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCurrentVolatile && status.Work.ReconciledWatermark == 10
	})
	if !baseline.Ready || !baseline.Work.BacklogKnown || !baseline.Work.BackgroundIngestion || baseline.Work.WatermarkDurable {
		t.Fatalf("baseline status=%+v", baseline)
	}
	if baseline.Configuration.IngestionMode != "native_adapter_experimental" {
		t.Fatalf("baseline configuration=%+v", baseline.Configuration)
	}

	secondPath := filepath.Join(source, "second.txt")
	if err := os.WriteFile(secondPath, []byte("second"), 0o600); err != nil {
		t.Fatal(err)
	}
	adapter.batches <- observation.Batch{
		After:   observation.Cursor{Source: "fixture", Epoch: "boot-1", Position: 11},
		Through: observation.Cursor{Source: "fixture", Epoch: "boot-1", Position: 50},
		Events:  []observation.Event{{Root: "docs", Kind: observation.KindCreate, Path: "second.txt"}},
	}
	recovered := waitForBackgroundState(t, engine, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCurrentVolatile && status.Work.ReconciledWatermark == 50
	})
	if !recovered.Ready || recovered.Work.ObservationGap {
		t.Fatalf("recovered status=%+v", recovered)
	}
	query, err := engine.Query(context.Background(), api.Query{
		Scope: api.Scope{Root: "docs", Descendants: true}, Filters: map[string]string{"name": "second.txt"},
	})
	if err != nil || len(query.Results) != 1 || len(query.Plan.StaleRoot) != 0 {
		t.Fatalf("recovered query=%+v err=%v", query, err)
	}

	if err := engine.StopBackgroundObservation(context.Background()); err != nil {
		t.Fatal(err)
	}
	stopped, err := engine.Status(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	if stopped.Work.BackgroundIngestion || stopped.Work.BacklogKnown || stopped.Work.Currentness != api.CurrentnessObservationUnavailable || stopped.Ready {
		t.Fatalf("stopped observation status=%+v", stopped)
	}
}

func TestBackgroundEventStormBatchesOneDurablePublicationAndThenStaysQuiet(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "store")
	for _, directory := range []string{source, storePath} {
		if err := os.Mkdir(directory, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	filePath := filepath.Join(source, "storm.txt")
	if err := os.WriteFile(filePath, []byte("before"), 0o600); err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	defer engine.Close()
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	adapter := &channelObservationAdapter{
		initial: observation.Cursor{Source: "fixture", Epoch: "storm", Position: 0},
		batches: make(chan observation.Batch, 4),
	}
	policy := backgroundTestPolicy()
	policy.Coalescer.MaxBatchEvents = 2048
	policy.Coalescer.MaxOperations = 1000
	if err := engine.StartBackgroundObservation(context.Background(), adapter, policy); err != nil {
		t.Fatal(err)
	}
	baseline := waitForBackgroundState(t, engine, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCurrentVolatile && status.Generation != 0
	})
	beforeFiles := lifecycleStoreState(t, storePath)
	if err := os.WriteFile(filePath, []byte("after-and-longer"), 0o600); err != nil {
		t.Fatal(err)
	}
	events := make([]observation.Event, 1000)
	for index := range events {
		events[index] = observation.Event{Root: "docs", Kind: observation.KindWrite, Path: "storm.txt"}
	}
	adapter.batches <- observation.Batch{
		After:   adapter.initial,
		Through: observation.Cursor{Source: "fixture", Epoch: "storm", Position: 9000},
		Events:  events,
	}
	updated := waitForBackgroundState(t, engine, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCurrentVolatile && status.Work.ReconciledWatermark == 9000
	})
	if updated.Generation != baseline.Generation+1 {
		t.Fatalf("storm generation=%d, baseline=%d", updated.Generation, baseline.Generation)
	}
	if updated.Work.PendingObservations != 0 || updated.Work.ObservationGap {
		t.Fatalf("storm status=%+v", updated)
	}
	afterFiles := lifecycleStoreState(t, storePath)
	if afterFiles == beforeFiles {
		t.Fatal("changed authoritative state did not publish a generation")
	}

	time.Sleep(4 * policy.Coalescer.MaxAge)
	quietFiles := lifecycleStoreState(t, storePath)
	quiet, err := engine.Status(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	if quietFiles != afterFiles || quiet.Generation != updated.Generation || quiet.Work.Currentness != api.CurrentnessCurrentVolatile {
		t.Fatalf("quiet engine wrote or drifted: before=%+v after=%+v status=%+v", afterFiles, quietFiles, quiet)
	}
}

func TestRootPolicyChangeForcesBackgroundBaseline(t *testing.T) {
	engine, sandboxPath := testService(t)
	defer engine.Close()
	first := filepath.Join(sandboxPath, "first")
	second := filepath.Join(sandboxPath, "second")
	for _, directory := range []string{first, second} {
		if err := os.Mkdir(directory, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "first", Path: first}}); err != nil {
		t.Fatal(err)
	}
	adapter := &channelObservationAdapter{
		initial: observation.Cursor{Source: "fixture", Epoch: "roots", Position: 7},
		batches: make(chan observation.Batch, 1),
	}
	if err := engine.StartBackgroundObservation(context.Background(), adapter, backgroundTestPolicy()); err != nil {
		t.Fatal(err)
	}
	waitForBackgroundState(t, engine, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCurrentVolatile
	})
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "second", Path: second}}); err != nil {
		t.Fatal(err)
	}
	status := waitForBackgroundState(t, engine, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCurrentVolatile && len(status.Roots) == 1 && status.Roots[0] == "second"
	})
	if !status.Ready {
		t.Fatalf("root-change reconciliation status=%+v", status)
	}
}

func TestObservationsArrivingDuringScanRemainBacklogged(t *testing.T) {
	engine, sandboxPath := testService(t)
	defer engine.Close()
	source := filepath.Join(sandboxPath, "during-scan")
	if err := os.Mkdir(source, 0o700); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	adapter := &channelObservationAdapter{
		initial: observation.Cursor{Source: "fixture", Epoch: "concurrent", Position: 0},
		batches: make(chan observation.Batch, 8),
	}
	policy := backgroundTestPolicy()
	policy.Coalescer.MaxOperations = 1
	if err := engine.StartBackgroundObservation(context.Background(), adapter, policy); err != nil {
		t.Fatal(err)
	}
	waitForBackgroundState(t, engine, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCurrentVolatile
	})

	gate := &blockingMetadataScanner{started: make(chan struct{}, 2), release: make(chan struct{}, 2)}
	engine.scanner = gate
	if err := os.WriteFile(filepath.Join(source, "first.txt"), []byte("first"), 0o600); err != nil {
		t.Fatal(err)
	}
	adapter.batches <- observation.Batch{
		After: adapter.initial, Through: observation.Cursor{Source: "fixture", Epoch: "concurrent", Position: 1},
		Events: []observation.Event{{Root: "docs", Kind: observation.KindCreate, Path: "first.txt"}},
	}
	select {
	case <-gate.started:
	case <-time.After(5 * time.Second):
		t.Fatal("first background scan did not start")
	}
	if err := os.WriteFile(filepath.Join(source, "second.txt"), []byte("second"), 0o600); err != nil {
		t.Fatal(err)
	}
	adapter.batches <- observation.Batch{
		After:   observation.Cursor{Source: "fixture", Epoch: "concurrent", Position: 1},
		Through: observation.Cursor{Source: "fixture", Epoch: "concurrent", Position: 2},
		Events:  []observation.Event{{Root: "docs", Kind: observation.KindCreate, Path: "second.txt"}},
	}
	backlogged := waitForBackgroundState(t, engine, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessReconciling && status.Work.PendingObservations == 2 && status.Work.ObservedWatermark == 2
	})
	if backlogged.Ready {
		t.Fatalf("in-flight backlog reported ready: %+v", backlogged)
	}
	gate.release <- struct{}{}
	select {
	case <-gate.started:
	case <-time.After(5 * time.Second):
		t.Fatal("arrival during scan did not schedule a second pass")
	}
	gate.release <- struct{}{}
	current := waitForBackgroundState(t, engine, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCurrentVolatile && status.Work.ReconciledWatermark == 2
	})
	if !current.Ready || gate.calls.Load() != 2 {
		t.Fatalf("converged status=%+v scan calls=%d", current, gate.calls.Load())
	}
}

func backgroundTestPolicy() BackgroundPolicy {
	policy := DefaultBackgroundPolicy()
	policy.Coalescer.MaxAge = 20 * time.Millisecond
	policy.RetryMin = 10 * time.Millisecond
	policy.RetryMax = 100 * time.Millisecond
	return policy
}

func waitForBackgroundState(t *testing.T, engine *Service, ready func(api.Status) bool) api.Status {
	t.Helper()
	deadline := time.Now().Add(5 * time.Second)
	var last api.Status
	for time.Now().Before(deadline) {
		status, err := engine.Status(context.Background())
		if err != nil {
			t.Fatal(err)
		}
		last = status
		if ready(status) {
			return status
		}
		time.Sleep(time.Millisecond)
	}
	t.Fatalf("background state timeout; last=%+v", last)
	return api.Status{}
}
