//go:build darwin && cgo

package benchmarks

import (
	"context"
	"os"
	"path/filepath"
	"runtime"
	"syscall"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/observation/fsevents"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
)

func TestM4FSEventsTenMinuteIdle(t *testing.T) {
	if os.Getenv("FILEMAN_ENGINE_LONG_MEASURE") != "1" {
		t.Skip("set FILEMAN_ENGINE_LONG_MEASURE=1 for the ten-minute native idle gate")
	}
	const idleDuration = 10 * time.Minute
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "store")
	for _, directory := range []string{source, storePath} {
		if err := os.Mkdir(directory, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	if err := os.WriteFile(filepath.Join(source, "stable.txt"), []byte("stable"), 0o600); err != nil {
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
	defer engine.Close()
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	adapter, err := fsevents.New(engine.Configuration().RootPolicy, fsevents.DefaultConfig())
	if err != nil {
		t.Fatal(err)
	}
	if err := engine.StartBackgroundObservation(context.Background(), adapter, service.DefaultBackgroundPolicy()); err != nil {
		t.Fatal(err)
	}
	baseline := waitM4Status(t, engine, 10*time.Second, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCoverageIncomplete && status.Generation != 0
	})
	runtime.GC()
	var memoryBefore runtime.MemStats
	runtime.ReadMemStats(&memoryBefore)
	usageBefore := readRusage(t)
	storeBefore := lifecycleBenchmarkStoreState(t, storePath)
	started := time.Now()
	time.Sleep(idleDuration)
	wall := time.Since(started)
	usageAfter := readRusage(t)
	var memoryAfter runtime.MemStats
	runtime.ReadMemStats(&memoryAfter)
	storeAfter := lifecycleBenchmarkStoreState(t, storePath)
	status, err := engine.Status(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	cpu := rusageCPU(usageAfter) - rusageCPU(usageBefore)
	cpuPercent := 100 * cpu.Seconds() / wall.Seconds()
	if storeAfter != storeBefore || status.Generation != baseline.Generation {
		t.Fatalf("idle service wrote durable state: before=%+v after=%+v baseline=%+v status=%+v", storeBefore, storeAfter, baseline, status)
	}
	if status.Work.Currentness != api.CurrentnessCoverageIncomplete || status.Work.PendingObservations != 0 || status.Work.ObservationGap {
		t.Fatalf("idle service lost currentness: %+v", status)
	}
	if cpuPercent >= 0.1 {
		t.Fatalf("idle CPU %.6f%% exceeds 0.1%% gate", cpuPercent)
	}
	t.Logf("m4-idle wall=%s process_cpu=%s one_core_cpu_percent=%.6f durable_writes=0 generation=%d heap_alloc_delta=%d heap_inuse_delta=%d max_rss_bytes=%d",
		wall, cpu, cpuPercent, status.Generation,
		int64(memoryAfter.HeapAlloc)-int64(memoryBefore.HeapAlloc),
		int64(memoryAfter.HeapInuse)-int64(memoryBefore.HeapInuse),
		usageAfter.Maxrss)
}

func readRusage(t *testing.T) syscall.Rusage {
	t.Helper()
	var usage syscall.Rusage
	if err := syscall.Getrusage(syscall.RUSAGE_SELF, &usage); err != nil {
		t.Fatal(err)
	}
	return usage
}

func rusageCPU(usage syscall.Rusage) time.Duration {
	seconds := usage.Utime.Sec + usage.Stime.Sec
	microseconds := usage.Utime.Usec + usage.Stime.Usec
	return time.Duration(seconds)*time.Second + time.Duration(microseconds)*time.Microsecond
}
