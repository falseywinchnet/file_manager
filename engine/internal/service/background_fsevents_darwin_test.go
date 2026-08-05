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
		return status.Work.Currentness == api.CurrentnessCurrentVolatile
	})
	if err := os.WriteFile(filepath.Join(source, "dogfood.txt"), []byte("dogfood"), 0o600); err != nil {
		t.Fatal(err)
	}
	updated := waitForBackgroundStateWithDeadline(t, engine, 10*time.Second, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCurrentVolatile && status.Work.ReconciledWatermark > baseline.Work.ReconciledWatermark && status.Generation > baseline.Generation
	})
	query, err := engine.Query(context.Background(), api.Query{
		Scope: api.Scope{Root: "docs", Descendants: true}, Filters: map[string]string{"name": "dogfood.txt"},
	})
	if err != nil || len(query.Results) != 1 || len(query.Plan.StaleRoot) != 0 {
		t.Fatalf("native dogfood status=%+v query=%+v err=%v", updated, query, err)
	}
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
