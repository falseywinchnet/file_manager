//go:build windows

package service

import (
	"context"
	"os"
	"path/filepath"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/observation/rdcw"
)

func TestReadDirectoryChangesWServiceReconcilesButFailsClosed(t *testing.T) {
	engine, sandboxPath := testService(t)
	defer engine.Close()
	source := filepath.Join(sandboxPath, "rdcw-dogfood")
	if err := os.Mkdir(source, 0o700); err != nil {
		t.Fatal(err)
	}
	plan, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}})
	if err != nil {
		t.Fatal(err)
	}
	adapter, err := rdcw.New(plan.Roots, rdcw.DefaultConfig())
	if err != nil {
		t.Fatal(err)
	}
	policy := backgroundTestPolicy()
	if err := engine.StartBackgroundObservation(context.Background(), adapter, policy); err != nil {
		t.Fatal(err)
	}
	baseline := waitForBackgroundState(t, engine, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCoverageIncomplete && status.Generation != 0
	})
	if baseline.Ready || !baseline.Work.CoverageIncomplete {
		t.Fatalf("ReadDirectoryChangesW baseline overstated coverage: %+v", baseline)
	}
	if err := os.WriteFile(filepath.Join(source, "native.txt"), []byte("native"), 0o600); err != nil {
		t.Fatal(err)
	}
	updated := waitForBackgroundStateWithWindowsDeadline(t, engine, 10*time.Second, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCoverageIncomplete &&
			status.Work.ReconciledWatermark > baseline.Work.ReconciledWatermark && status.Generation > baseline.Generation
	})
	query, err := engine.Query(context.Background(), api.Query{
		Scope: api.Scope{Root: "docs", Descendants: true}, Filters: map[string]string{"name": "native.txt"},
	})
	if err != nil || len(query.Results) != 1 || len(query.Plan.StaleRoot) != 1 {
		t.Fatalf("ReadDirectoryChangesW status=%+v query=%+v err=%v", updated, query, err)
	}
}

func waitForBackgroundStateWithWindowsDeadline(t *testing.T, engine *Service, duration time.Duration, ready func(api.Status) bool) api.Status {
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
	t.Fatalf("ReadDirectoryChangesW background state timeout; last=%+v", last)
	return api.Status{}
}
