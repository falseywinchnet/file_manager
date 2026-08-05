package benchmarks

import (
	"context"
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strconv"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/exact"
	"filemanager/engine/internal/generation"
	"filemanager/engine/internal/workload"
)

const m2MeasurementEnvironment = "FILEMAN_ENGINE_M2_MEASURE"

// TestImmutableGenerationDistribution is a component measurement, not a
// service-promotion result. It intentionally drops the reference shard before
// measuring reopened-reader heap so a complete heap mirror cannot hide in the
// number.
func TestImmutableGenerationDistribution(t *testing.T) {
	if os.Getenv(m2MeasurementEnvironment) != "1" {
		t.Skipf("set %s=1 to run durable-generation measurements", m2MeasurementEnvironment)
	}
	recordCount := 1_000_000
	if encoded := os.Getenv("FILEMAN_ENGINE_RECORDS"); encoded != "" {
		parsed, err := strconv.Atoi(encoded)
		if err != nil || parsed < 10_000 {
			t.Fatalf("FILEMAN_ENGINE_RECORDS=%q is not an integer at least 10000", encoded)
		}
		recordCount = parsed
	}
	directory := t.TempDir()
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}

	runtime.GC()
	var baseline runtime.MemStats
	runtime.ReadMemStats(&baseline)
	corpus, err := workload.ScaleV1(recordCount, 10, 10)
	if err != nil {
		t.Fatal(err)
	}
	buildStarted := time.Now()
	shard, err := corpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	buildElapsed := time.Since(buildStarted)
	referenceDigest := shard.Digest()
	uniqueIndex := recordCount/2 + 1
	uniqueName := fmt.Sprintf("file-%09d", uniqueIndex)
	uniquePath := filepath.Join(root.Path, fmt.Sprintf("dir-%07d", uniqueIndex/10), uniqueName)
	wantPath, exists := shard.Path(uniquePath)
	if !exists {
		t.Fatalf("reference unique path %q is missing", uniquePath)
	}
	wantRepeated := referenceRecords(shard, shard.NameRange("repeated"), 100)

	store, err := generation.OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	publishStarted := time.Now()
	reader, err := store.Publish(1, shard)
	if err != nil {
		t.Fatal(err)
	}
	publishElapsed := time.Since(publishStarted)
	metadata := reader.Metadata()
	if metadata.CatalogDigest != referenceDigest {
		t.Fatal("durable catalogue digest differs from the reference")
	}
	if err := reader.Close(); err != nil {
		t.Fatal(err)
	}

	corpus = workload.Corpus{}
	shard = nil
	runtime.GC()
	var reopenedBaseline runtime.MemStats
	runtime.ReadMemStats(&reopenedBaseline)
	probeStarted := time.Now()
	head, err := store.Probe()
	if err != nil {
		t.Fatal(err)
	}
	probeElapsed := time.Since(probeStarted)
	if !head.IntegrityPending || head.Metadata.CatalogDigest != referenceDigest {
		t.Fatal("manifest probe lost its pending-integrity or reference-digest evidence")
	}
	recoverStarted := time.Now()
	reopened, err := store.Recover()
	if err != nil {
		t.Fatal(err)
	}
	recoverElapsed := time.Since(recoverStarted)
	runtime.GC()
	var reopenedMemory runtime.MemStats
	runtime.ReadMemStats(&reopenedMemory)
	reopenedHeap := uint64(0)
	if reopenedMemory.HeapAlloc > reopenedBaseline.HeapAlloc {
		reopenedHeap = reopenedMemory.HeapAlloc - reopenedBaseline.HeapAlloc
	}

	gotPath, exists, err := reopened.Path(uniquePath)
	if err != nil || !exists || gotPath != wantPath {
		t.Fatalf("durable path mismatch: exists=%v err=%v\n got  %+v\n want %+v", exists, err, gotPath, wantPath)
	}
	gotRepeated, err := reopened.Name("repeated", 100)
	if err != nil {
		t.Fatal(err)
	}
	if !sameRecords(gotRepeated, wantRepeated) {
		t.Fatal("durable repeated-name results differ from reference order")
	}

	uniqueSamples, uniqueResultDigest := sampleGenerationPath(t, reopened, uniquePath, 20_000)
	repeatedSamples, repeatedResultDigest := sampleGenerationName(t, reopened, "repeated", 100, 2_000)
	policy := catalog.NewStore()
	if _, _, err := policy.ApplyRoots([]api.RootSpec{root}); err != nil {
		t.Fatal(err)
	}
	pipelineSamples, pipelineDigest := sampleGenerationPipeline(t, policy.Snapshot(), reopened, api.Query{
		Scope: api.Scope{Root: root.ID, Descendants: true}, Filters: map[string]string{"name": "repeated"}, Limit: 100,
	}, 2_000)
	runtime.GC()
	var warmedMemory runtime.MemStats
	runtime.ReadMemStats(&warmedMemory)
	warmedHeap := uint64(0)
	if warmedMemory.HeapAlloc > reopenedBaseline.HeapAlloc {
		warmedHeap = warmedMemory.HeapAlloc - reopenedBaseline.HeapAlloc
	}
	t.Logf("environment os=%s arch=%s go=%s", runtime.GOOS, runtime.GOARCH, runtime.Version())
	t.Logf("generation-build file_objects=%d bindings=%d reference_build=%s publish=%s manifest_probe=%s recover_checked=%s", recordCount, metadata.BindingCount, buildElapsed, publishElapsed, probeElapsed, recoverElapsed)
	t.Logf("generation-storage bytes=%d bytes_per_binding=%.2f reopened_heap_delta=%d warmed_heap_delta=%d cache_bytes=%d cache_limit=%d", metadata.Size, float64(metadata.Size)/float64(metadata.BindingCount), reopenedHeap, warmedHeap, reopened.CacheBytes(), generation.ReadCacheLimit)
	logDistribution(t, "generation-exact-path", uniqueSamples, uniqueResultDigest)
	logDistribution(t, "generation-name-first-100", repeatedSamples, repeatedResultDigest)
	logDistribution(t, "generation-pipeline-name-first-100", pipelineSamples, pipelineDigest)
	runtime.KeepAlive(reopened)
	if err := reopened.Close(); err != nil {
		t.Fatal(err)
	}
}

func sampleGenerationPipeline(t *testing.T, snapshot *catalog.Snapshot, reader *generation.Reader, query api.Query, count int) ([]time.Duration, string) {
	t.Helper()
	samples := make([]time.Duration, count)
	digest := sha256.New()
	for index := range samples {
		started := time.Now()
		matches, _, err := exact.QueryIndex(context.Background(), snapshot, reader.Metadata().Generation, reader.Root().ID, reader, query)
		samples[index] = time.Since(started)
		if err != nil || len(matches) != query.Limit {
			t.Fatalf("generation pipeline matches=%d err=%v", len(matches), err)
		}
		_, _ = digest.Write([]byte(matches[0].Record.ObjectID()))
	}
	return samples, fmt.Sprintf("%x", digest.Sum(nil)[:8])
}

func referenceRecords(shard *catalog.Shard, ordinals []uint32, limit int) []catalog.Record {
	if len(ordinals) > limit {
		ordinals = ordinals[:limit]
	}
	result := make([]catalog.Record, 0, len(ordinals))
	for _, ordinal := range ordinals {
		record, _ := shard.Record(ordinal)
		result = append(result, record)
	}
	return result
}

func sameRecords(left, right []catalog.Record) bool {
	if len(left) != len(right) {
		return false
	}
	for index := range left {
		if left[index] != right[index] {
			return false
		}
	}
	return true
}

func sampleGenerationPath(t *testing.T, reader *generation.Reader, path string, count int) ([]time.Duration, string) {
	t.Helper()
	samples := make([]time.Duration, count)
	digest := sha256.New()
	for index := range samples {
		started := time.Now()
		record, exists, err := reader.Path(path)
		samples[index] = time.Since(started)
		if err != nil || !exists {
			t.Fatalf("generation path exists=%v err=%v", exists, err)
		}
		_, _ = digest.Write([]byte(record.ObjectID()))
	}
	return samples, fmt.Sprintf("%x", digest.Sum(nil)[:8])
}

func sampleGenerationName(t *testing.T, reader *generation.Reader, name string, limit uint32, count int) ([]time.Duration, string) {
	t.Helper()
	samples := make([]time.Duration, count)
	digest := sha256.New()
	for index := range samples {
		started := time.Now()
		records, err := reader.Name(name, limit)
		samples[index] = time.Since(started)
		if err != nil || len(records) != int(limit) {
			t.Fatalf("generation name results=%d err=%v", len(records), err)
		}
		_, _ = digest.Write([]byte(records[0].ObjectID()))
	}
	return samples, fmt.Sprintf("%x", digest.Sum(nil)[:8])
}
