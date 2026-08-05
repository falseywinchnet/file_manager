//go:build darwin && cgo

package benchmarks

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"sort"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/observation/fsevents"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
)

func TestM4FSEventsBackgroundCurrentnessDogfood(t *testing.T) {
	requireMeasurement(t)
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "store")
	for _, directory := range []string{source, storePath} {
		if err := os.Mkdir(directory, 0o700); err != nil {
			t.Fatal(err)
		}
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
	config := fsevents.DefaultConfig()
	config.Latency = 50 * time.Millisecond
	adapter, err := fsevents.New(engine.Configuration().RootPolicy, config)
	if err != nil {
		t.Fatal(err)
	}
	policy := service.DefaultBackgroundPolicy()
	policy.Coalescer.MaxAge = 100 * time.Millisecond
	policy.Coalescer.MaxOperations = 4096
	if err := engine.StartBackgroundObservation(context.Background(), adapter, policy); err != nil {
		t.Fatal(err)
	}
	status := waitM4Status(t, engine, 10*time.Second, func(status api.Status) bool {
		return status.Work.Currentness == api.CurrentnessCurrentVolatile && status.Generation != 0
	})

	const serialMutations = 32
	latencies := make([]time.Duration, 0, serialMutations)
	serialPath := filepath.Join(source, "serial.txt")
	for index := range serialMutations {
		beforeWatermark := status.Work.ReconciledWatermark
		beforeGeneration := status.Generation
		started := time.Now()
		if err := os.WriteFile(serialPath, []byte(fmt.Sprintf("serial-%d-%s", index, string(make([]byte, index%13)))), 0o600); err != nil {
			t.Fatal(err)
		}
		status = waitM4Status(t, engine, 10*time.Second, func(candidate api.Status) bool {
			return candidate.Work.Currentness == api.CurrentnessCurrentVolatile &&
				candidate.Work.ReconciledWatermark > beforeWatermark && candidate.Generation > beforeGeneration
		})
		latencies = append(latencies, time.Since(started))
	}
	sort.Slice(latencies, func(i, j int) bool { return latencies[i] < latencies[j] })

	var memoryBefore runtime.MemStats
	runtime.GC()
	runtime.ReadMemStats(&memoryBefore)
	stormGeneration := status.Generation
	stormWatermark := status.Work.ReconciledWatermark
	stormStarted := time.Now()
	for operation := range 4096 {
		name := filepath.Join(source, fmt.Sprintf("storm-%03d.tmp", operation%256))
		if err := os.WriteFile(name, []byte(fmt.Sprintf("operation-%d", operation)), 0o600); err != nil {
			t.Fatal(err)
		}
	}
	status = waitM4Status(t, engine, 20*time.Second, func(candidate api.Status) bool {
		return candidate.Work.Currentness == api.CurrentnessCurrentVolatile &&
			candidate.Work.ReconciledWatermark > stormWatermark && candidate.Generation > stormGeneration
	})
	stormElapsed := time.Since(stormStarted)
	var memoryAfter runtime.MemStats
	runtime.ReadMemStats(&memoryAfter)

	quietBefore := lifecycleBenchmarkStoreState(t, storePath)
	quietStarted := time.Now()
	time.Sleep(2 * time.Second)
	quietAfter := lifecycleBenchmarkStoreState(t, storePath)
	quietStatus, err := engine.Status(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	if quietAfter != quietBefore || quietStatus.Generation != status.Generation || quietStatus.Work.Currentness != api.CurrentnessCurrentVolatile {
		t.Fatalf("quiet currentness changed durable state: before=%+v after=%+v status=%+v", quietBefore, quietAfter, quietStatus)
	}

	pick := func(percentile float64) time.Duration {
		index := int(float64(len(latencies)-1) * percentile)
		return latencies[index]
	}
	t.Logf("m4-fsevents serial=%d p50=%s p95=%s max=%s", serialMutations, pick(.50), pick(.95), latencies[len(latencies)-1])
	t.Logf("m4-fsevents storm_operations=4096 distinct_paths=256 elapsed=%s generation_publications=%d observed_watermark=%d reconciled_watermark=%d heap_alloc_delta=%d",
		stormElapsed, status.Generation-stormGeneration, status.Work.ObservedWatermark,
		status.Work.ReconciledWatermark, int64(memoryAfter.HeapAlloc)-int64(memoryBefore.HeapAlloc))
	t.Logf("m4-fsevents quiet=%s durable_writes=0 generation=%d store_files=%d store_bytes=%d watermark_durable=%v",
		time.Since(quietStarted), quietStatus.Generation, quietAfter.files, quietAfter.bytes, quietStatus.Work.WatermarkDurable)
}

func waitM4Status(t *testing.T, engine *service.Service, duration time.Duration, ready func(api.Status) bool) api.Status {
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
		time.Sleep(2 * time.Millisecond)
	}
	t.Fatalf("M4 currentness timeout; last=%+v", last)
	return api.Status{}
}
