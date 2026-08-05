package benchmarks

import (
	"context"
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"sort"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/exact"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
	"filemanager/engine/internal/workload"
)

const measurementEnvironment = "FILEMAN_ENGINE_MEASURE"

func TestReferenceExactDistribution(t *testing.T) {
	requireMeasurement(t)
	const recordCount = 100_000
	runtime.GC()
	var before runtime.MemStats
	runtime.ReadMemStats(&before)
	started := time.Now()
	snapshot, unique, repeated := virtualSnapshot(t, recordCount)
	buildElapsed := time.Since(started)
	runtime.GC()
	var after runtime.MemStats
	runtime.ReadMemStats(&after)
	heapBytes := after.HeapAlloc - before.HeapAlloc
	projection, _ := snapshot.Projection("fixture")
	manifest := projection.Manifest()

	uniqueSamples, uniqueDigest := sampleExact(t, snapshot, unique, 20_000)
	repeatedSamples, repeatedDigest := sampleExact(t, snapshot, repeated, 2_000)
	t.Logf("environment os=%s arch=%s go=%s", runtime.GOOS, runtime.GOARCH, runtime.Version())
	t.Logf("reference-build file_objects=%d total_objects=%d bindings=%d elapsed=%s retained_heap=%d bytes_per_object=%.2f", recordCount, manifest.ObjectCount, manifest.BindingCount, buildElapsed, heapBytes, float64(heapBytes)/float64(manifest.ObjectCount))
	logDistribution(t, "unique-name", uniqueSamples, uniqueDigest)
	logDistribution(t, "10k-repeated-name", repeatedSamples, repeatedDigest)
	runtime.KeepAlive(snapshot)
}

func TestSandboxScanAndServiceDistribution(t *testing.T) {
	requireMeasurement(t)
	const recordCount = 10_000
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	if err := os.Mkdir(source, 0o700); err != nil {
		t.Fatal(err)
	}
	for index := 0; index < recordCount; index++ {
		name := filepath.Join(source, fmt.Sprintf("file-%05d", index))
		if err := os.WriteFile(name, nil, 0o600); err != nil {
			t.Fatal(err)
		}
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := service.New(guard)
	if err != nil {
		t.Fatal(err)
	}
	ctx := context.Background()
	if _, err := engine.ApplyRoots(ctx, []api.RootSpec{{ID: "fixture", Path: source}}); err != nil {
		t.Fatal(err)
	}
	started := time.Now()
	report, err := engine.Reconcile(ctx, "fixture")
	if err != nil {
		t.Fatal(err)
	}
	reconcileElapsed := time.Since(started)
	request := api.Query{Scope: api.Scope{Root: "fixture", Descendants: true}, Filters: map[string]string{"name": "file-05000"}}
	samples := make([]time.Duration, 5_000)
	digest := sha256.New()
	for index := range samples {
		started = time.Now()
		response, err := engine.Query(ctx, request)
		samples[index] = time.Since(started)
		if err != nil || len(response.Results) != 1 {
			t.Fatalf("service query result=%d err=%v", len(response.Results), err)
		}
		_, _ = digest.Write([]byte(response.Results[0].Object.ID))
	}
	t.Logf("sandbox-scan records=%d committed=%d elapsed=%s records_per_second=%.0f", recordCount, report.Records, reconcileElapsed, recordCount/reconcileElapsed.Seconds())
	logDistribution(t, "service-exact-name", samples, fmt.Sprintf("%x", digest.Sum(nil)[:8]))
}

func virtualSnapshot(t *testing.T, count int) (*catalog.Snapshot, api.Query, api.Query) {
	t.Helper()
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "virtual")}
	corpus, err := workload.ScaleV1(count, 10, 10)
	if err != nil {
		t.Fatal(err)
	}
	shard, err := corpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	store := catalog.NewStore()
	if _, _, err := store.ApplyRoots([]api.RootSpec{root}); err != nil {
		t.Fatal(err)
	}
	if _, err := store.Publish(root.ID, shard); err != nil {
		t.Fatal(err)
	}
	base := api.Query{Scope: api.Scope{Root: root.ID, Descendants: true}, Limit: 100, Filters: map[string]string{"name": "repeated"}}
	unique := base
	unique.Filters = map[string]string{"name": "file-000050001"}
	return store.Snapshot(), unique, base
}

func sampleExact(t *testing.T, snapshot *catalog.Snapshot, request api.Query, count int) ([]time.Duration, string) {
	t.Helper()
	samples := make([]time.Duration, count)
	digest := sha256.New()
	ctx := context.Background()
	for index := range samples {
		started := time.Now()
		matches, _, err := exact.Query(ctx, snapshot, request)
		samples[index] = time.Since(started)
		if err != nil || len(matches) == 0 {
			t.Fatalf("exact query matches=%d err=%v", len(matches), err)
		}
		_, _ = digest.Write([]byte(matches[0].Record.ObjectID()))
	}
	return samples, fmt.Sprintf("%x", digest.Sum(nil)[:8])
}

func logDistribution(t *testing.T, name string, samples []time.Duration, digest string) {
	t.Helper()
	sort.Slice(samples, func(i, j int) bool { return samples[i] < samples[j] })
	percentile := func(numerator int) time.Duration {
		index := (len(samples)*numerator + 99) / 100
		if index == 0 {
			return samples[0]
		}
		return samples[index-1]
	}
	t.Logf("distribution=%s samples=%d p50=%s p95=%s p99=%s max=%s digest=%s", name, len(samples), percentile(50), percentile(95), percentile(99), samples[len(samples)-1], digest)
}

func requireMeasurement(t *testing.T) {
	t.Helper()
	if os.Getenv(measurementEnvironment) != "1" {
		t.Skipf("set %s=1 to run resource measurements", measurementEnvironment)
	}
}
