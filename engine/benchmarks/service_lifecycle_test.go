package benchmarks

import (
	"context"
	"os"
	"path/filepath"
	"runtime"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
)

func TestServiceLifecycleResourceDogfood(t *testing.T) {
	requireMeasurement(t)
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "store")
	for _, path := range []string{source, storePath} {
		if err := os.Mkdir(path, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	if err := os.WriteFile(filepath.Join(source, "record.txt"), []byte("stable"), 0o600); err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := service.NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	if report, err := engine.Reconcile(context.Background(), "docs"); err != nil || !report.Published {
		t.Fatalf("initial reconcile=%+v err=%v", report, err)
	}

	const calls = 10_000
	measure := func(name string, call func()) {
		t.Helper()
		runtime.GC()
		var before runtime.MemStats
		runtime.ReadMemStats(&before)
		started := time.Now()
		for range calls {
			call()
		}
		elapsed := time.Since(started)
		var after runtime.MemStats
		runtime.ReadMemStats(&after)
		t.Logf("service-lifecycle operation=%s calls=%d elapsed=%s ns_per_call=%d allocated_bytes=%d bytes_per_call=%.2f mallocs=%d mallocs_per_call=%.3f",
			name, calls, elapsed, elapsed.Nanoseconds()/calls,
			after.TotalAlloc-before.TotalAlloc, float64(after.TotalAlloc-before.TotalAlloc)/calls,
			after.Mallocs-before.Mallocs, float64(after.Mallocs-before.Mallocs)/calls)
	}
	measure("configuration", func() { _ = engine.Configuration() })
	measure("version", func() { _ = engine.Version() })
	measure("status", func() {
		if _, err := engine.Status(context.Background()); err != nil {
			t.Fatal(err)
		}
	})

	before := lifecycleBenchmarkStoreState(t, storePath)
	started := time.Now()
	unchanged, err := engine.Reconcile(context.Background(), "docs")
	if err != nil {
		t.Fatal(err)
	}
	after := lifecycleBenchmarkStoreState(t, storePath)
	if unchanged.Published || before != after {
		t.Fatalf("unchanged reconcile wrote state: report=%+v before=%+v after=%+v", unchanged, before, after)
	}
	shutdownStarted := time.Now()
	stopped, err := engine.Shutdown(context.Background())
	if err != nil || stopped.State != api.LifecycleStopped {
		t.Fatalf("shutdown=%+v err=%v", stopped, err)
	}
	shutdownStore := lifecycleBenchmarkStoreState(t, storePath)
	if shutdownStore != after {
		t.Fatalf("shutdown wrote state: before=%+v after=%+v", after, shutdownStore)
	}
	t.Logf("service-lifecycle unchanged_reconcile=%s unchanged_generation=%d durable_files=%d durable_bytes=%d shutdown=%s shutdown_durable_bytes_written=0",
		time.Since(started), unchanged.Generation, after.files, after.bytes, time.Since(shutdownStarted))
}

type lifecycleBenchmarkState struct {
	files int
	bytes int64
	mtime int64
}

func lifecycleBenchmarkStoreState(t *testing.T, root string) lifecycleBenchmarkState {
	t.Helper()
	state := lifecycleBenchmarkState{}
	if err := filepath.WalkDir(root, func(path string, entry os.DirEntry, err error) error {
		if err != nil || entry.IsDir() {
			return err
		}
		info, err := entry.Info()
		if err != nil {
			return err
		}
		state.files++
		state.bytes += info.Size()
		if modified := info.ModTime().UnixNano(); modified > state.mtime {
			state.mtime = modified
		}
		return nil
	}); err != nil {
		t.Fatal(err)
	}
	return state
}
