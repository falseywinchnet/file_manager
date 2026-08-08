package service

import (
	"context"
	"errors"
	"os"
	"path/filepath"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/sandbox"
)

func TestShutdownCancelsAndDrainsActiveWork(t *testing.T) {
	engine, _ := testService(t)
	operationContext, finish, err := engine.beginOperation(context.Background(), operationQuery, "")
	if err != nil {
		t.Fatal(err)
	}
	status, err := engine.Status(context.Background())
	if err != nil || status.Lifecycle.State != api.LifecycleReady || status.Work.ActiveRequests != 1 || status.Work.ActiveQueries != 1 {
		t.Fatalf("active status=%+v err=%v", status, err)
	}
	readyGeneration := status.Lifecycle.Generation

	type shutdownResult struct {
		status api.LifecycleStatus
		err    error
	}
	result := make(chan shutdownResult, 1)
	go func() {
		shutdownStatus, shutdownErr := engine.Shutdown(context.Background())
		result <- shutdownResult{status: shutdownStatus, err: shutdownErr}
	}()
	select {
	case <-operationContext.Done():
	case <-time.After(5 * time.Second):
		t.Fatal("shutdown did not cancel service-owned operation context")
	}
	draining, err := engine.Status(context.Background())
	if err != nil || draining.Lifecycle.State != api.LifecycleDraining || draining.Ready || draining.Work.Phase != api.WorkDraining {
		t.Fatalf("draining status=%+v err=%v", draining, err)
	}
	if draining.Lifecycle.Generation <= readyGeneration {
		t.Fatalf("draining lifecycle generation=%d, ready=%d", draining.Lifecycle.Generation, readyGeneration)
	}
	if _, err := engine.Query(context.Background(), api.Query{}); lifecycleFaultCode(err) != api.ErrorMethodUnavailable {
		t.Fatalf("query admitted while draining: %v", err)
	}
	finish()
	select {
	case stopped := <-result:
		if stopped.err != nil || stopped.status.State != api.LifecycleStopped {
			t.Fatalf("shutdown result=%+v err=%v", stopped.status, stopped.err)
		}
		if stopped.status.Generation <= draining.Lifecycle.Generation {
			t.Fatalf("stopped lifecycle generation=%d, draining=%d", stopped.status.Generation, draining.Lifecycle.Generation)
		}
	case <-time.After(5 * time.Second):
		t.Fatal("shutdown did not finish after active operation drained")
	}
	second, err := engine.Shutdown(context.Background())
	if err != nil || second.State != api.LifecycleStopped {
		t.Fatalf("idempotent shutdown=%+v err=%v", second, err)
	}
}

func TestPersistentRestartChangesInstanceAndPreservesEffectiveConfiguration(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "store")
	for _, path := range []string{source, storePath} {
		if err := os.Mkdir(path, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	if err := os.WriteFile(filepath.Join(source, "record.txt"), []byte("record"), 0o600); err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	first, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	emptyDigest := first.Configuration().Digest
	if _, err := first.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	planned := first.Configuration()
	if planned.Digest == emptyDigest || planned.RootPolicyPersistent {
		t.Fatalf("unreconciled configuration=%+v", planned)
	}
	if _, err := first.Reconcile(context.Background(), "docs"); err != nil {
		t.Fatal(err)
	}
	committed := first.Configuration()
	if committed.Digest != planned.Digest || !committed.RootPolicyPersistent {
		t.Fatalf("committed configuration=%+v planned=%+v", committed, planned)
	}
	firstVersion := first.Version()
	storeBefore := lifecycleStoreState(t, storePath)
	stopped, err := first.Shutdown(context.Background())
	if err != nil || stopped.State != api.LifecycleStopped {
		t.Fatalf("first shutdown=%+v err=%v", stopped, err)
	}
	if storeAfter := lifecycleStoreState(t, storePath); storeAfter != storeBefore {
		t.Fatalf("clean shutdown changed durable files: before=%+v after=%+v", storeBefore, storeAfter)
	}

	restarted, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	defer restarted.Close()
	restartedVersion := restarted.Version()
	restartedConfiguration := restarted.Configuration()
	if restartedVersion.InstanceID == firstVersion.InstanceID {
		t.Fatal("supervisor-style restart reused process instance identity")
	}
	if restartedConfiguration.Digest != committed.Digest || !restartedConfiguration.RootPolicyPersistent {
		t.Fatalf("restart configuration=%+v committed=%+v", restartedConfiguration, committed)
	}
	status, err := restarted.Status(context.Background())
	if err != nil || status.Lifecycle.State != api.LifecycleReady || !status.Ready || status.Work.BackgroundIngestion || status.Work.BacklogKnown {
		t.Fatalf("restart status=%+v err=%v", status, err)
	}
}

func TestPersistentUnchangedReconcileProducesNoDurableWrites(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "store")
	for _, path := range []string{source, storePath} {
		if err := os.Mkdir(path, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	if err := os.WriteFile(filepath.Join(source, "unchanged.txt"), []byte("stable"), 0o600); err != nil {
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
	first, err := engine.Reconcile(context.Background(), "docs")
	if err != nil || !first.Published {
		t.Fatalf("initial reconcile=%+v err=%v", first, err)
	}
	before := lifecycleStoreState(t, storePath)
	second, err := engine.Reconcile(context.Background(), "docs")
	if err != nil {
		t.Fatal(err)
	}
	after := lifecycleStoreState(t, storePath)
	if second.Published || second.Generation != first.Generation || second.Records != first.Records {
		t.Fatalf("unchanged reconcile=%+v first=%+v", second, first)
	}
	if after != before {
		t.Fatalf("unchanged reconcile wrote durable state: before=%+v after=%+v", before, after)
	}
	rebuilt, err := engine.Rebuild(context.Background(), "docs")
	if err != nil || !rebuilt.Published || rebuilt.Generation != first.Generation+1 {
		t.Fatalf("forced rebuild=%+v err=%v", rebuilt, err)
	}
}

func TestCapabilitiesDeclareMissingEnginePieces(t *testing.T) {
	engine, _ := testService(t)
	defer engine.Close()
	states := make(map[string]api.CapabilityState)
	for _, capability := range engine.Version().Capabilities {
		states[capability.ID] = capability.State
		if capability.State != api.CapabilityAvailable && capability.Reason == "" {
			t.Fatalf("non-available capability lacks reason: %+v", capability)
		}
	}
	for id, want := range map[string]api.CapabilityState{
		"engine.exact.query":            api.CapabilityAvailable,
		"engine.live.query":             api.CapabilityAvailable,
		"engine.lexical.index":          api.CapabilityUnavailable,
		"engine.content_feature.intake": api.CapabilityDeferred,
		"engine.similarity.fixed_width": api.CapabilityExperimental,
		"engine.background.observation": api.CapabilityExperimental,
		"engine.transport.framed_local": api.CapabilityDeferred,
		"contract.ORC-LIF-001":          api.CapabilityAvailable,
		"contract.ORC-ENG-001":          api.CapabilityAvailable,
		"contract.ORC-ENG-002":          api.CapabilityAvailable,
		"contract.ORC-ENG-003":          api.CapabilityAvailable,
		"contract.ORC-ENG-004":          api.CapabilityAvailable,
	} {
		if states[id] != want {
			t.Fatalf("capability %s=%s, want %s", id, states[id], want)
		}
	}
}

func TestExpectedConfigurationDigestRejectsStaleRootApply(t *testing.T) {
	engine, sandboxPath := testService(t)
	defer engine.Close()
	firstPath := filepath.Join(sandboxPath, "first")
	secondPath := filepath.Join(sandboxPath, "second")
	for _, path := range []string{firstPath, secondPath} {
		if err := os.Mkdir(path, 0o700); err != nil {
			t.Fatal(err)
		}
	}

	firstRoots := []api.RootSpec{{ID: "first", Path: firstPath}}
	secondRoots := []api.RootSpec{{ID: "second", Path: secondPath}}
	firstPlan, err := engine.PlanRoots(context.Background(), firstRoots)
	if err != nil {
		t.Fatal(err)
	}
	secondPlan, err := engine.PlanRoots(context.Background(), secondRoots)
	if err != nil {
		t.Fatal(err)
	}
	if firstPlan.CurrentConfigurationDigest == "" || firstPlan.CurrentConfigurationDigest != secondPlan.CurrentConfigurationDigest {
		t.Fatalf("plans did not share an initial configuration: first=%+v second=%+v", firstPlan, secondPlan)
	}
	if !firstPlan.Changed || !secondPlan.Changed || firstPlan.ProposedConfigurationDigest == secondPlan.ProposedConfigurationDigest {
		t.Fatalf("plans did not describe distinct changes: first=%+v second=%+v", firstPlan, secondPlan)
	}

	if _, err := engine.ApplyRootsExpected(context.Background(), firstRoots, firstPlan.CurrentConfigurationDigest); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.ApplyRootsExpected(context.Background(), secondRoots, secondPlan.CurrentConfigurationDigest); lifecycleFaultCode(err) != api.ErrorStaleConfiguration {
		t.Fatalf("stale apply error=%v, want %s", err, api.ErrorStaleConfiguration)
	}
	configuration := engine.Configuration()
	if len(configuration.RootPolicy) != 1 || configuration.RootPolicy[0].ID != "first" || configuration.Digest != firstPlan.ProposedConfigurationDigest {
		t.Fatalf("stale apply changed configuration: %+v", configuration)
	}

	replanned, err := engine.PlanRoots(context.Background(), secondRoots)
	if err != nil {
		t.Fatal(err)
	}
	if replanned.CurrentConfigurationDigest != configuration.Digest {
		t.Fatalf("replanned against %q, want %q", replanned.CurrentConfigurationDigest, configuration.Digest)
	}
	if _, err := engine.ApplyRootsExpected(context.Background(), secondRoots, replanned.CurrentConfigurationDigest); err != nil {
		t.Fatal(err)
	}
}

func lifecycleFaultCode(err error) api.ErrorCode {
	var fault *api.Fault
	if errors.As(err, &fault) {
		return fault.Code
	}
	return ""
}

type lifecycleFileState struct {
	Files int
	Bytes int64
	Time  int64
}

func lifecycleStoreState(t *testing.T, root string) lifecycleFileState {
	t.Helper()
	state := lifecycleFileState{}
	err := filepath.WalkDir(root, func(path string, entry os.DirEntry, err error) error {
		if err != nil {
			return err
		}
		if entry.IsDir() {
			return nil
		}
		info, err := entry.Info()
		if err != nil {
			return err
		}
		state.Files++
		state.Bytes += info.Size()
		if modified := info.ModTime().UnixNano(); modified > state.Time {
			state.Time = modified
		}
		return nil
	})
	if err != nil {
		t.Fatal(err)
	}
	return state
}
