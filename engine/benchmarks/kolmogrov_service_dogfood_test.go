package benchmarks

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strconv"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
	"filemanager/engine/internal/similarity"
)

const kolmogrovServiceMeasurementEnvironment = "FILEMAN_ENGINE_KOLMOGROV_SERVICE_MEASURE"

// TestKolmogrovServiceGenerationDogfood measures the exact durable service and
// experimental projection over one real, disposable filesystem corpus. It is
// opt-in because populating the fixture creates one file per exact record.
func TestKolmogrovServiceGenerationDogfood(t *testing.T) {
	if os.Getenv(kolmogrovServiceMeasurementEnvironment) != "1" {
		t.Skipf("set %s=1 to run persistent-service Kolmogrov dogfood", kolmogrovServiceMeasurementEnvironment)
	}
	recordCount := 10_000
	if encoded := os.Getenv("FILEMAN_ENGINE_RECORDS"); encoded != "" {
		parsed, err := strconv.Atoi(encoded)
		if err != nil || parsed < 1_000 || parsed > 65_000 {
			t.Fatalf("FILEMAN_ENGINE_RECORDS=%q is not an integer from 1000 through 65000", encoded)
		}
		recordCount = parsed
	}

	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "exact-store")
	projectionDirectory := filepath.Join(sandboxPath, "similarity-store")
	for _, path := range []string{source, storePath, projectionDirectory} {
		if err := os.Mkdir(path, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	for ordinal := 0; ordinal < recordCount; ordinal++ {
		path := filepath.Join(source, fmt.Sprintf("file-%012x.dat", ordinal))
		file, err := os.OpenFile(path, os.O_CREATE|os.O_EXCL|os.O_WRONLY, 0o600)
		if err != nil {
			t.Fatal(err)
		}
		if err := file.Close(); err != nil {
			t.Fatal(err)
		}
	}
	sourceBefore := measurementTreeState(t, source)

	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	runtime.GC()
	var baseline runtime.MemStats
	runtime.ReadMemStats(&baseline)
	engine, err := service.NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	ctx := context.Background()
	if _, err := engine.ApplyRoots(ctx, []api.RootSpec{{ID: "scale", Path: source}}); err != nil {
		t.Fatal(err)
	}
	reconcileStarted := time.Now()
	report, err := engine.Reconcile(ctx, "scale")
	if err != nil {
		t.Fatal(err)
	}
	reconcileElapsed := time.Since(reconcileStarted)
	runtime.GC()
	var exactMemory runtime.MemStats
	runtime.ReadMemStats(&exactMemory)
	exactRetained := positiveDifference(exactMemory.HeapAlloc, baseline.HeapAlloc)
	exactBytes := measurementDirectoryBytes(t, storePath)

	leaseStarted := time.Now()
	lease, err := engine.PinSimilarityGeneration(ctx, "scale")
	if err != nil {
		t.Fatal(err)
	}
	leaseElapsed := time.Since(leaseStarted)
	configuration, err := similarity.DefaultHistoryTupleConfiguration()
	if err != nil {
		t.Fatal(err)
	}
	projectionPath := filepath.Join(projectionDirectory, "generation-1.kht")
	var buildBefore runtime.MemStats
	runtime.ReadMemStats(&buildBefore)
	buildStarted := time.Now()
	metadata, err := similarity.BuildHistoryTuplePostingFileFromResolver(ctx, projectionPath, configuration, lease)
	if err != nil {
		t.Fatal(err)
	}
	buildElapsed := time.Since(buildStarted)
	var buildAfter runtime.MemStats
	runtime.ReadMemStats(&buildAfter)
	buildAllocated := buildAfter.TotalAlloc - buildBefore.TotalAlloc
	openStarted := time.Now()
	index, _, err := similarity.OpenHistoryTuplePostingFile(projectionPath, lease)
	if err != nil {
		t.Fatal(err)
	}
	openElapsed := time.Since(openStarted)
	runtime.GC()
	var combinedMemory runtime.MemStats
	runtime.ReadMemStats(&combinedMemory)
	combinedRetained := positiveDifference(combinedMemory.HeapAlloc, baseline.HeapAlloc)

	const samples = 1_000
	exactDurations := make([]time.Duration, samples)
	fuzzyDurations := make([]time.Duration, samples)
	for sample := 0; sample < samples; sample++ {
		ordinal := sample * 7_919 % recordCount
		name := fmt.Sprintf("file-%012x.dat", ordinal)
		started := time.Now()
		response, err := engine.Query(ctx, api.Query{
			Scope: api.Scope{Root: "scale", Descendants: true}, Filters: map[string]string{"name": name}, Limit: 4,
		})
		exactDurations[sample] = time.Since(started)
		if err != nil || len(response.Results) != 1 {
			t.Fatalf("exact query %q results=%d err=%v", name, len(response.Results), err)
		}
		started = time.Now()
		batch, err := index.SearchVerified(ctx, mutateFilename(name, sample%5), similarity.Budget{})
		fuzzyDurations[sample] = time.Since(started)
		if err != nil {
			t.Fatalf("fuzzy query %q: %v", name, err)
		}
		found := false
		for _, candidate := range batch.Candidates {
			if candidate.Candidate.Ordinal == uint32(ordinal) {
				found = true
				break
			}
		}
		if !found {
			t.Fatalf("fuzzy query missed source ordinal %d", ordinal)
		}
	}
	if after := measurementTreeState(t, source); sourceBefore != after {
		t.Fatal("engine changed indexed source metadata during dogfood")
	}

	if err := index.Close(); err != nil {
		t.Fatal(err)
	}
	if err := lease.Close(); err != nil {
		t.Fatal(err)
	}
	if err := engine.Close(); err != nil {
		t.Fatal(err)
	}
	restartStarted := time.Now()
	restarted, err := service.NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	restartElapsed := time.Since(restartStarted)
	defer restarted.Close()
	status, err := restarted.Status(ctx)
	if err != nil || !status.Ready || status.Generation != report.Generation {
		t.Fatalf("restarted status=%+v err=%v", status, err)
	}

	t.Logf("environment os=%s arch=%s go=%s filesystem_corpus_records=%d", runtime.GOOS, runtime.GOARCH, runtime.Version(), recordCount)
	t.Logf("service-exact reconcile=%s lease_open=%s store_bytes=%d store_bytes_per_record=%.2f retained_heap_delta=%d", reconcileElapsed, leaseElapsed, exactBytes, float64(exactBytes)/float64(recordCount), exactRetained)
	t.Logf("service-kolmogrov build=%s build_allocated_bytes=%d build_scratch_bytes=%d checked_open=%s file_bytes=%d file_bytes_per_record=%.2f combined_retained_heap_delta=%d", buildElapsed, buildAllocated, metadata.BuildScratchBytes, openElapsed, metadata.FileBytes, float64(metadata.FileBytes)/float64(recordCount), combinedRetained)
	t.Logf("service-restart checked_recovery=%s total_index_bytes_per_record=%.2f", restartElapsed, float64(exactBytes+metadata.FileBytes)/float64(recordCount))
	logDistribution(t, "service-exact-name", exactDurations, "warm")
	logDistribution(t, "service-kolmogrov-verified", fuzzyDurations, "warm")
}

type measurementTree struct {
	Files         int
	Bytes         int64
	NewestModTime int64
}

func measurementTreeState(t *testing.T, root string) measurementTree {
	t.Helper()
	state := measurementTree{}
	err := filepath.WalkDir(root, func(path string, entry os.DirEntry, err error) error {
		if err != nil {
			return err
		}
		info, err := entry.Info()
		if err != nil {
			return err
		}
		if !entry.IsDir() {
			state.Files++
			state.Bytes += info.Size()
		}
		if modified := info.ModTime().UnixNano(); modified > state.NewestModTime {
			state.NewestModTime = modified
		}
		return nil
	})
	if err != nil {
		t.Fatal(err)
	}
	return state
}

func measurementDirectoryBytes(t *testing.T, root string) uint64 {
	t.Helper()
	var total uint64
	err := filepath.WalkDir(root, func(path string, entry os.DirEntry, err error) error {
		if err != nil {
			return err
		}
		if entry.Type().IsRegular() {
			info, err := entry.Info()
			if err != nil {
				return err
			}
			total += uint64(info.Size())
		}
		return nil
	})
	if err != nil {
		t.Fatal(err)
	}
	return total
}
