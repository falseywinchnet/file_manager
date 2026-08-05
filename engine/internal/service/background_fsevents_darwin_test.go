//go:build darwin && cgo

package service

import (
	"context"
	"os"
	"path/filepath"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/observation/fsevents"
)

func TestFSEventsServiceDogfoodReconcilesDisposableRoot(t *testing.T) {
	engine, sandboxPath := testService(t)
	defer engine.Close()
	source := filepath.Join(sandboxPath, "native-dogfood")
	if err := os.Mkdir(source, 0o700); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	config := fsevents.DefaultConfig()
	config.Latency = 50 * time.Millisecond
	adapter, err := fsevents.New([]api.RootSpec{{ID: "docs", Path: source}}, config)
	if err != nil {
		t.Fatal(err)
	}
	policy := backgroundTestPolicy()
	policy.Coalescer.MaxAge = 50 * time.Millisecond
	if err := engine.StartBackgroundObservation(context.Background(), adapter, policy); err != nil {
		t.Fatal(err)
	}
	baseline := waitForBackgroundState(t, engine, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCoverageIncomplete
	})
	if err := os.WriteFile(filepath.Join(source, "dogfood.txt"), []byte("dogfood"), 0o600); err != nil {
		t.Fatal(err)
	}
	updated := waitForBackgroundStateWithDeadline(t, engine, 10*time.Second, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCoverageIncomplete && status.Work.ReconciledWatermark > baseline.Work.ReconciledWatermark && status.Generation > baseline.Generation
	})
	query, err := engine.Query(context.Background(), api.Query{
		Scope: api.Scope{Root: "docs", Descendants: true}, Filters: map[string]string{"name": "dogfood.txt"},
	})
	if err != nil || len(query.Results) != 1 || len(query.Plan.StaleRoot) != 1 {
		t.Fatalf("native dogfood status=%+v query=%+v err=%v", updated, query, err)
	}
}

func TestFSEventsBackgroundPreservesIdentityAcrossMutationSequence(t *testing.T) {
	engine, sandboxPath := testService(t)
	defer engine.Close()
	rootPath := filepath.Join(sandboxPath, "native-identity")
	directoryA := filepath.Join(rootPath, "a")
	directoryB := filepath.Join(rootPath, "b")
	if err := os.MkdirAll(directoryA, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(directoryB, 0o700); err != nil {
		t.Fatal(err)
	}
	originalPath := filepath.Join(directoryA, "item")
	if err := os.WriteFile(originalPath, []byte("original"), 0o600); err != nil {
		t.Fatal(err)
	}
	plan, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "oracle", Path: rootPath}})
	if err != nil {
		t.Fatal(err)
	}
	canonicalRoot := plan.Roots[0].Path
	config := fsevents.DefaultConfig()
	config.Latency = 20 * time.Millisecond
	adapter, err := fsevents.New(plan.Roots, config)
	if err != nil {
		t.Fatal(err)
	}
	policy := backgroundTestPolicy()
	policy.Coalescer.MaxAge = 20 * time.Millisecond
	if err := engine.StartBackgroundObservation(context.Background(), adapter, policy); err != nil {
		t.Fatal(err)
	}
	status := waitForBackgroundStateWithDeadline(t, engine, 10*time.Second, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCoverageIncomplete
	})
	canonicalOriginal := filepath.Join(canonicalRoot, "a", "item")
	originalID := exactPathID(t, engine, canonicalOriginal)

	renamedPath := filepath.Join(directoryA, "renamed")
	status = mutateAndWaitForFSEvents(t, engine, status, func() error {
		return os.Rename(originalPath, renamedPath)
	})
	canonicalRenamed := filepath.Join(canonicalRoot, "a", "renamed")
	if got := exactPathID(t, engine, canonicalRenamed); got != originalID {
		t.Fatalf("rename changed identity: %q != %q", got, originalID)
	}
	assertPathAbsent(t, engine, canonicalOriginal)

	movedPath := filepath.Join(directoryB, "moved")
	status = mutateAndWaitForFSEvents(t, engine, status, func() error {
		return os.Rename(renamedPath, movedPath)
	})
	canonicalMoved := filepath.Join(canonicalRoot, "b", "moved")
	if got := exactPathID(t, engine, canonicalMoved); got != originalID {
		t.Fatalf("move changed identity: %q != %q", got, originalID)
	}
	assertPathAbsent(t, engine, canonicalRenamed)

	oldPath := filepath.Join(directoryB, "old-object")
	status = mutateAndWaitForFSEvents(t, engine, status, func() error {
		if err := os.Rename(movedPath, oldPath); err != nil {
			return err
		}
		return os.WriteFile(movedPath, []byte("replacement"), 0o600)
	})
	replacementID := exactPathID(t, engine, canonicalMoved)
	if replacementID == originalID {
		t.Fatal("replace-at-path retained old identity")
	}
	if got := exactPathID(t, engine, filepath.Join(canonicalRoot, "b", "old-object")); got != originalID {
		t.Fatalf("old object identity=%q, want %q", got, originalID)
	}

	hardLinkPath := filepath.Join(directoryB, "hard-link")
	status = mutateAndWaitForFSEvents(t, engine, status, func() error {
		return os.Link(movedPath, hardLinkPath)
	})
	canonicalHardLink := filepath.Join(canonicalRoot, "b", "hard-link")
	if got := exactPathID(t, engine, canonicalHardLink); got != replacementID {
		t.Fatalf("hard-link identity=%q, want %q", got, replacementID)
	}

	status = mutateAndWaitForFSEvents(t, engine, status, func() error {
		if err := os.Remove(movedPath); err != nil {
			return err
		}
		return os.WriteFile(movedPath, []byte("recreated"), 0o600)
	})
	recreatedID := exactPathID(t, engine, canonicalMoved)
	if recreatedID == replacementID {
		t.Fatal("delete/recreate retained prior incarnation identity")
	}
	if got := exactPathID(t, engine, canonicalHardLink); got != replacementID {
		t.Fatalf("recreate changed surviving hard-link identity=%q, want %q", got, replacementID)
	}

	if status.Work.Currentness != api.CurrentnessCoverageIncomplete || status.Ready {
		t.Fatalf("hard-link root did not fail closed: %+v", status)
	}
	beforeSilentUnlink := status
	if err := os.Remove(hardLinkPath); err != nil {
		t.Fatal(err)
	}
	time.Sleep(250 * time.Millisecond)
	status, err = engine.Status(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	if status.Work.Currentness != api.CurrentnessCoverageIncomplete || status.Ready {
		t.Fatalf("last hard-link removal was falsely reported current: %+v", status)
	}
	stale, err := engine.Query(context.Background(), api.Query{
		Scope: api.Scope{Root: "oracle", Descendants: true}, Filters: map[string]string{"path": canonicalHardLink},
	})
	if err != nil || len(stale.Plan.StaleRoot) != 1 {
		t.Fatalf("coverage-incomplete query did not report staleness: response=%+v err=%v", stale, err)
	}
	status = mutateAndWaitForFSEvents(t, engine, beforeSilentUnlink, func() error {
		return os.WriteFile(movedPath, []byte("recreated-and-observed"), 0o600)
	})
	if status.Work.Currentness != api.CurrentnessCoverageIncomplete || status.Ready {
		t.Fatalf("hard-link coverage limitation was cleared without a stronger adapter: %+v", status)
	}
	assertPathAbsent(t, engine, canonicalHardLink)
}

func mutateAndWaitForFSEvents(t *testing.T, engine *Service, before api.Status, mutate func() error) api.Status {
	t.Helper()
	if err := mutate(); err != nil {
		t.Fatal(err)
	}
	// The adapter deliberately uses FSEvents latency/coalescing rather than a
	// synchronous syscall acknowledgement. Let the native delivery window
	// close so this helper attributes the next cursor to this mutation instead
	// of an older in-flight directory event.
	time.Sleep(75 * time.Millisecond)
	return waitForBackgroundStateWithDeadline(t, engine, 10*time.Second, func(status api.Status) bool {
		return (status.Work.Currentness == api.CurrentnessCurrentVolatile || status.Work.Currentness == api.CurrentnessCoverageIncomplete) &&
			status.Work.ReconciledWatermark > before.Work.ReconciledWatermark && status.Generation > before.Generation
	})
}

func waitForBackgroundStateWithDeadline(t *testing.T, engine *Service, duration time.Duration, ready func(api.Status) bool) api.Status {
	t.Helper()
	deadline := time.Now().Add(duration)
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
		time.Sleep(5 * time.Millisecond)
	}
	t.Fatalf("background state timeout; last=%+v", last)
	return api.Status{}
}
