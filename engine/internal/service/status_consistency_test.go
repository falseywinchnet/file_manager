package service

import (
	"context"
	"os"
	"path/filepath"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/observation"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/scan"
)

// The fixture owns the service and scanner gate. Cleanup releases the scan
// before draining the service, including after a failed assertion.
type statusConsistencyFixture struct {
	engine   *Service
	scanner  *blockingMetadataScanner
	released bool
}

func newStatusConsistencyFixture(t *testing.T, persistent bool) *statusConsistencyFixture {
	t.Helper()
	var sandboxPath string = t.TempDir()
	var source string = filepath.Join(sandboxPath, "source")
	var err error = os.Mkdir(source, 0o700)
	if err != nil {
		t.Fatal(err)
	}
	var filePath string = filepath.Join(source, "first.txt")
	var contents []byte = []byte("first")
	err = os.WriteFile(filePath, contents, 0o600)
	if err != nil {
		t.Fatal(err)
	}
	var guard *sandbox.Guard = nil
	guard, err = sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	var engine *Service = nil
	if persistent {
		var storePath string = filepath.Join(sandboxPath, "store")
		err = os.Mkdir(storePath, 0o700)
		if err != nil {
			t.Fatal(err)
		}
		engine, err = NewPersistent(guard, storePath)
	} else {
		engine, err = New(guard)
	}
	if err != nil {
		t.Fatal(err)
	}
	var scanner *blockingMetadataScanner = &blockingMetadataScanner{
		delegate: scan.Scanner{}, started: make(chan struct{}, 1), release: make(chan struct{}),
	}
	var fixture *statusConsistencyFixture = &statusConsistencyFixture{engine: engine, scanner: scanner, released: false}
	t.Cleanup(fixture.close)
	engine.scanner = scanner
	var ctx context.Context = context.Background()
	var roots []api.RootSpec = []api.RootSpec{{ID: "docs", Path: source}}
	_, err = engine.ApplyRoots(ctx, roots)
	if err != nil {
		t.Fatal(err)
	}
	var adapter *channelObservationAdapter = &channelObservationAdapter{
		initial: observation.Cursor{Source: "fixture", Epoch: "status-capture", Position: 10},
		batches: make(chan observation.Batch, 1),
	}
	var policy BackgroundPolicy = backgroundTestPolicy()
	err = engine.StartBackgroundObservation(ctx, adapter, policy)
	if err != nil {
		t.Fatal(err)
	}
	var deadline *time.Timer = time.NewTimer(5 * time.Second)
	defer deadline.Stop()
	select {
	case <-scanner.started:
	case <-deadline.C:
		t.Fatal("baseline scan did not reach its fixture gate")
	}
	return fixture
}

func (fixture *statusConsistencyFixture) releaseScan() {
	if !fixture.released {
		close(fixture.scanner.release)
		fixture.released = true
	}
}

func (fixture *statusConsistencyFixture) close() {
	fixture.releaseScan()
	_ = fixture.engine.Close()
}

func statusConsistencyCurrent(status api.Status) bool {
	var current bool = status.Work.Currentness == api.CurrentnessCurrentVolatile && status.Work.ReconciledWatermark == 10
	return current
}

func requireCapturedBaseline(t *testing.T, status api.Status) {
	t.Helper()
	if status.Ready || len(status.RootStates) != 1 {
		t.Fatalf("captured baseline readiness/root count changed: %+v", status)
	}
	var root api.RootState = status.RootStates[0]
	if root.Indexed || !root.Stale || root.Generation != 0 || root.Records != 0 {
		t.Fatalf("captured baseline catalogue changed: %+v", status)
	}
	if status.Work.Currentness != api.CurrentnessReconciling || status.Work.ReconciledWatermark != 0 {
		t.Fatalf("captured baseline was combined with later observation state: %+v", status)
	}
	if !status.Work.BackgroundIngestion || !status.Work.BacklogKnown || status.Work.WatermarkDurable {
		t.Fatalf("captured baseline observation semantics changed: %+v", status)
	}
	if status.Configuration.IngestionMode != "native_adapter_experimental" || status.Configuration.RootPolicyPersistent {
		t.Fatalf("captured baseline configuration changed: %+v", status.Configuration)
	}
	if len(status.Configuration.RootPolicy) != 1 || status.Configuration.RootPolicy[0] != root.Root {
		t.Fatalf("captured policy does not match captured root: %+v", status)
	}
}

func requireReconciledStatus(t *testing.T, status api.Status, persistent bool) {
	t.Helper()
	if !status.Ready || len(status.RootStates) != 1 {
		t.Fatalf("reconciled status is not ready: %+v", status)
	}
	var root api.RootState = status.RootStates[0]
	if !root.Indexed || root.Stale || root.Records == 0 || root.Generation == 0 || root.Generation != status.Generation {
		t.Fatalf("reconciled catalogue is inconsistent: %+v", status)
	}
	if !statusConsistencyCurrent(status) || status.Work.ObservedWatermark != 10 || status.Work.WatermarkDurable {
		t.Fatalf("reconciled currentness is inconsistent: %+v", status)
	}
	if status.Configuration.RootPolicyPersistent != persistent || status.Configuration.IngestionMode != "native_adapter_experimental" {
		t.Fatalf("reconciled configuration is inconsistent: %+v", status.Configuration)
	}
}

func testStatusCaptureSurvivesReconciliation(t *testing.T, persistent bool) {
	t.Helper()
	var fixture *statusConsistencyFixture = newStatusConsistencyFixture(t, persistent)
	var captured statusSnapshot = fixture.engine.captureStatus()
	fixture.releaseScan()
	// The real background worker publishes the catalogue and finishes its
	// coalescer work before the old capture is projected. No timing interleaving
	// is needed to exercise the formerly independent decoration read.
	var fresh api.Status = waitForBackgroundState(t, fixture.engine, statusConsistencyCurrent)
	requireReconciledStatus(t, fresh, persistent)
	var prior api.Status = fixture.engine.projectStatus(captured)
	requireCapturedBaseline(t, prior)

	// Stopping the actual adapter also must not change the captured mode/digest.
	var ctx context.Context = context.Background()
	var err error = fixture.engine.StopBackgroundObservation(ctx)
	if err != nil {
		t.Fatal(err)
	}
	var afterStop api.Status = fixture.engine.projectStatus(captured)
	requireCapturedBaseline(t, afterStop)
	if afterStop.Configuration.Digest != prior.Configuration.Digest {
		t.Fatal("captured configuration digest changed after adapter stop")
	}
	var stopped api.Status = api.Status{}
	stopped, err = fixture.engine.Status(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if stopped.Ready || stopped.Work.Currentness != api.CurrentnessObservationUnavailable || stopped.Configuration.IngestionMode != "manual_reconcile" {
		t.Fatalf("fresh stopped status did not observe adapter stop: %+v", stopped)
	}
}

func TestStatusCaptureSurvivesVolatileReconciliation(t *testing.T) {
	testStatusCaptureSurvivesReconciliation(t, false)
}

func TestStatusCaptureSurvivesPersistentReconciliation(t *testing.T) {
	testStatusCaptureSurvivesReconciliation(t, true)
}

type statusConsistencyCall struct {
	engine *Service
	done   chan struct{}
	status api.Status
	err    error
}

func (call *statusConsistencyCall) run() {
	var ctx context.Context = context.Background()
	call.status, call.err = call.engine.Status(ctx)
	close(call.done)
}

func testStatusReturnsDuringScan(t *testing.T, persistent bool) {
	t.Helper()
	var fixture *statusConsistencyFixture = newStatusConsistencyFixture(t, persistent)
	var call *statusConsistencyCall = &statusConsistencyCall{
		engine: fixture.engine, done: make(chan struct{}), status: api.Status{}, err: nil,
	}
	go call.run()
	var deadline *time.Timer = time.NewTimer(5 * time.Second)
	defer deadline.Stop()
	select {
	case <-call.done:
		if call.err != nil {
			t.Fatal(call.err)
		}
		requireCapturedBaseline(t, call.status)
	case <-deadline.C:
		// Release a wrongly acquired admin lock before cleanup drains the service.
		fixture.releaseScan()
		deadline.Reset(5 * time.Second)
		select {
		case <-call.done:
		case <-deadline.C:
			t.Fatal("Status did not finish after the scan gate was released")
		}
		t.Fatal("Status waited for a blocked metadata scan")
	}
}

func TestStatusReturnsDuringVolatileScan(t *testing.T) {
	testStatusReturnsDuringScan(t, false)
}

func TestStatusReturnsDuringPersistentScan(t *testing.T) {
	testStatusReturnsDuringScan(t, true)
}
