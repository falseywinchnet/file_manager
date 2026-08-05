package benchmarks

import (
	"context"
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"sort"
	"strconv"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/generation"
	"filemanager/engine/internal/workload"
)

type ownedIndexedRun struct {
	delta      *generation.DeltaReader
	index      *generation.DeltaExactIndex
	deltaBytes uint64
	indexBytes uint64
}

func (r *ownedIndexedRun) close() {
	_ = r.index.Close()
	_ = r.delta.Close()
}

// TestTieredIndexedCompactionDistribution exercises the retained schedule with
// checked disk indexes and the O(run-count) streaming cohort compactor. No
// oracle checkpoint or changed-path heap overlay participates.
func TestTieredIndexedCompactionDistribution(t *testing.T) {
	if os.Getenv(m2MeasurementEnvironment) != "1" {
		t.Skipf("set %s=1 to run tiered indexed measurements", m2MeasurementEnvironment)
	}
	recordCount := 250_000
	if encoded := os.Getenv("FILEMAN_ENGINE_RECORDS"); encoded != "" {
		parsed, err := strconv.Atoi(encoded)
		if err != nil || parsed < 200_000 {
			t.Fatalf("FILEMAN_ENGINE_RECORDS=%q is not an integer at least 200000", encoded)
		}
		recordCount = parsed
	}
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	baseCorpus, err := workload.ScaleV1(recordCount, 10, 10)
	if err != nil {
		t.Fatal(err)
	}
	baseShard, err := baseCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	basePath := filepath.Join(t.TempDir(), "base.seg")
	if _, err := generation.Write(basePath, 1, baseShard); err != nil {
		t.Fatal(err)
	}
	baseCorpus = workload.Corpus{}
	baseShard = nil
	runtime.GC()

	for _, batch := range []int{4_096, 10_000} {
		t.Run(fmt.Sprintf("mixed-%d", batch), func(t *testing.T) {
			const finalEpoch = 22
			if (3*batch/4)*finalEpoch >= recordCount {
				t.Skipf("record count %d is too small for %d disjoint %d-change epochs", recordCount, finalEpoch, batch)
			}
			base, err := generation.Open(basePath, root)
			if err != nil {
				t.Fatal(err)
			}
			defer base.Close()
			runtime.GC()
			var heapBefore runtime.MemStats
			runtime.ReadMemStats(&heapBefore)
			var storage []*ownedIndexedRun
			var fresh []*ownedIndexedRun
			var current *generation.TieredIndexCandidate
			var cohortBase *generation.TieredIndexCandidate
			defer func() {
				for _, run := range storage {
					run.close()
				}
			}()
			var freshDeltaBytes, freshIndexBytes, cohortDeltaBytes, cohortIndexBytes, logicalBytes uint64
			var maximumIndexBuildAllocated, maximumCompactionAllocated uint64
			cohorts := 0
			var finalSamples multiRunSamples

			for epoch := 1; epoch <= finalEpoch; epoch++ {
				corpus, samples, err := mixedScaleCorpusEpochs(recordCount, batch, epoch)
				if err != nil {
					t.Fatal(err)
				}
				updated, err := corpus.ReferenceShard(root)
				if err != nil {
					t.Fatal(err)
				}
				corpus = workload.Corpus{}
				if epoch == finalEpoch {
					finalSamples = samples
				}
				freshPath := filepath.Join(t.TempDir(), fmt.Sprintf("fresh-%02d.delta", epoch))
				var deltaMetadata generation.DeltaMetadata
				if current == nil {
					deltaMetadata, err = generation.WriteDeltaCandidate(context.Background(), freshPath, api.Generation(epoch+1), base, updated)
				} else {
					deltaMetadata, err = generation.WriteDeltaFromTieredCandidate(context.Background(), freshPath, api.Generation(epoch+1), current, updated)
				}
				if err != nil {
					t.Fatal(err)
				}
				if deltaMetadata.Changes() != uint64(batch) {
					t.Fatalf("epoch %d changes=%d want=%d", epoch, deltaMetadata.Changes(), batch)
				}
				delta, err := generation.OpenDeltaCandidate(freshPath, root)
				if err != nil {
					t.Fatal(err)
				}
				indexPath := filepath.Join(t.TempDir(), fmt.Sprintf("fresh-%02d.dxi", epoch))
				var indexBefore runtime.MemStats
				runtime.ReadMemStats(&indexBefore)
				indexMetadata, err := generation.WriteDeltaExactIndex(context.Background(), indexPath, delta, batch)
				if err != nil {
					delta.Close()
					t.Fatal(err)
				}
				var indexAfter runtime.MemStats
				runtime.ReadMemStats(&indexAfter)
				if allocated := indexAfter.TotalAlloc - indexBefore.TotalAlloc; allocated > maximumIndexBuildAllocated {
					maximumIndexBuildAllocated = allocated
				}
				index, err := generation.OpenDeltaExactIndex(indexPath, delta)
				if err != nil {
					delta.Close()
					t.Fatal(err)
				}
				run := &ownedIndexedRun{
					delta: delta, index: index,
					deltaBytes: deltaMetadata.Size + deltaCandidateHeaderRewriteBytes,
					indexBytes: indexMetadata.Size + 256,
				}
				storage = append(storage, run)
				fresh = append(fresh, run)
				freshDeltaBytes += run.deltaBytes
				freshIndexBytes += run.indexBytes
				if err := delta.Iterate(context.Background(), func(_ generation.ChangeKind, row catalog.Row) error {
					logicalBytes += canonicalDeltaOperationBytes(row)
					return nil
				}); err != nil {
					t.Fatal(err)
				}

				indexes := tieredRunIndexes(storage)
				current, err = generation.NewTieredIndexCandidate(base, indexes, 16, finalEpoch*batch)
				if err != nil || current.Generation() != api.Generation(epoch+1) || current.Digest() != updated.Digest() {
					t.Fatalf("epoch %d tiered generation=%v err=%v", epoch, current, err)
				}
				if _, err := current.PrimeIndexCache(8 << 20); err != nil {
					t.Fatalf("epoch %d prime exact indexes: %v", epoch, err)
				}
				if len(fresh) == 8 {
					cohorts++
					cohortPath := filepath.Join(t.TempDir(), fmt.Sprintf("cohort-%02d.delta", cohorts))
					var compactBefore runtime.MemStats
					runtime.ReadMemStats(&compactBefore)
					started := time.Now()
					var cohortMetadata generation.DeltaMetadata
					if cohortBase == nil {
						cohortMetadata, err = generation.WriteTieredCohortFromBase(context.Background(), cohortPath, base, current)
					} else {
						cohortMetadata, err = generation.WriteTieredCohort(context.Background(), cohortPath, cohortBase, current)
					}
					if err != nil {
						t.Fatal(err)
					}
					compactionElapsed := time.Since(started)
					var compactAfter runtime.MemStats
					runtime.ReadMemStats(&compactAfter)
					if allocated := compactAfter.TotalAlloc - compactBefore.TotalAlloc; allocated > maximumCompactionAllocated {
						maximumCompactionAllocated = allocated
					}
					cohortDelta, err := generation.OpenDeltaCandidate(cohortPath, root)
					if err != nil {
						t.Fatal(err)
					}
					cohortIndexPath := filepath.Join(t.TempDir(), fmt.Sprintf("cohort-%02d.dxi", cohorts))
					indexMetadata, err := generation.WriteDeltaExactIndex(context.Background(), cohortIndexPath, cohortDelta, int(cohortMetadata.Changes()))
					if err != nil {
						cohortDelta.Close()
						t.Fatal(err)
					}
					cohortIndex, err := generation.OpenDeltaExactIndex(cohortIndexPath, cohortDelta)
					if err != nil {
						cohortDelta.Close()
						t.Fatal(err)
					}
					cohortRun := &ownedIndexedRun{
						delta: cohortDelta, index: cohortIndex,
						deltaBytes: cohortMetadata.Size + deltaCandidateHeaderRewriteBytes,
						indexBytes: indexMetadata.Size + 256,
					}
					for _, stale := range fresh {
						stale.close()
					}
					storage = append(storage[:len(storage)-len(fresh)], cohortRun)
					fresh = nil
					cohortDeltaBytes += cohortRun.deltaBytes
					cohortIndexBytes += cohortRun.indexBytes
					current, err = generation.NewTieredIndexCandidate(base, tieredRunIndexes(storage), 8, finalEpoch*batch)
					if err != nil || current.Digest() != updated.Digest() {
						t.Fatalf("cohort %d tiered err=%v", cohorts, err)
					}
					if _, err := current.PrimeIndexCache(8 << 20); err != nil {
						t.Fatalf("cohort %d prime exact indexes: %v", cohorts, err)
					}
					cohortBase = current
					t.Logf("tiered-indexed-cohort records=%d batch=%d cohort=%d epochs=%d compaction_elapsed=%s visible_runs=%d cumulative_delta_bytes=%d cumulative_index_bytes=%d logical_bytes=%d indexed_wa=%.4f compaction_allocated_bytes=%d",
						recordCount, batch, cohorts, epoch, compactionElapsed, len(storage),
						freshDeltaBytes+cohortDeltaBytes, freshIndexBytes+cohortIndexBytes, logicalBytes,
						float64(freshDeltaBytes+freshIndexBytes+cohortDeltaBytes+cohortIndexBytes)/float64(logicalBytes),
						compactAfter.TotalAlloc-compactBefore.TotalAlloc,
					)
				}
				if len(storage) > 8 {
					t.Fatalf("epoch %d published %d tiered runs", epoch, len(storage))
				}
				updated = nil
			}

			pathSample := filepath.Join(root.Path, finalSamples.unchanged)
			coldPathStarted := time.Now()
			coldOrdinal, coldExists, coldErr := current.PathIndex(pathSample)
			if coldErr != nil || !coldExists {
				t.Fatalf("cold path sample exists=%v err=%v", coldExists, coldErr)
			}
			if _, exists, err := current.Row(coldOrdinal); err != nil || !exists {
				t.Fatalf("cold path row exists=%v err=%v", exists, err)
			}
			coldPathElapsed := time.Since(coldPathStarted)
			pathBase, pathTiered := pairedPathIndexSamples(t, base, current, pathSample, 20_000)
			coldNameStarted := time.Now()
			if _, _, exceeded, err := current.CandidateNamePage("repeated", 0, 100, 100_000); err != nil || exceeded {
				t.Fatalf("cold name sample exceeded=%v err=%v", exceeded, err)
			}
			coldNameElapsed := time.Since(coldNameStarted)
			runtime.GC()
			var retained runtime.MemStats
			runtime.ReadMemStats(&retained)
			nameBase, nameTiered := pairedNameIndexSamples(t, base, current, "repeated", 500)
			var liveBytes uint64
			for _, run := range storage {
				liveBytes += run.deltaBytes + run.indexBytes
			}
			totalWriteBytes := freshDeltaBytes + freshIndexBytes + cohortDeltaBytes + cohortIndexBytes
			t.Logf("tiered-indexed-final records=%d batch=%d epochs=%d cohorts=%d fresh_tail=%d visible_runs=%d live_bytes=%d delta_write_bytes=%d index_write_bytes=%d logical_bytes=%d indexed_wa=%.4f retained_heap_growth=%d maximum_index_build_allocated=%d maximum_compaction_allocated=%d path_cold=%s path_base_p99=%s path_tiered_p99=%s path_p99_ratio=%.3f name_cold=%s name_base_p99=%s name_tiered_p99=%s name_p99_ratio=%.3f",
				recordCount, batch, finalEpoch, cohorts, len(fresh), len(storage), liveBytes,
				freshDeltaBytes+cohortDeltaBytes, freshIndexBytes+cohortIndexBytes, logicalBytes,
				float64(totalWriteBytes)/float64(logicalBytes), positiveDifference(retained.HeapAlloc, heapBefore.HeapAlloc),
				maximumIndexBuildAllocated, maximumCompactionAllocated,
				coldPathElapsed,
				percentileDuration(pathBase, 99), percentileDuration(pathTiered, 99), durationRatio(percentileDuration(pathTiered, 99), percentileDuration(pathBase, 99)),
				coldNameElapsed,
				percentileDuration(nameBase, 99), percentileDuration(nameTiered, 99), durationRatio(percentileDuration(nameTiered, 99), percentileDuration(nameBase, 99)),
			)
			if cohorts != 2 || len(fresh) != 6 || len(storage) != 8 || current.Digest() == [sha256.Size]byte{} {
				t.Fatalf("unexpected final indexed tiered state")
			}
		})
	}
}

func tieredRunIndexes(runs []*ownedIndexedRun) []*generation.DeltaExactIndex {
	indexes := make([]*generation.DeltaExactIndex, len(runs))
	for index, run := range runs {
		indexes[index] = run.index
	}
	return indexes
}

type pathIndexReader interface {
	PathIndex(string) (uint32, bool, error)
	Row(uint32) (catalog.Row, bool, error)
}

func pairedPathIndexSamples(t *testing.T, base, tiered pathIndexReader, path string, count int) ([]time.Duration, []time.Duration) {
	t.Helper()
	baseSamples := make([]time.Duration, 0, count)
	tieredSamples := make([]time.Duration, 0, count)
	measure := func(index pathIndexReader) time.Duration {
		started := time.Now()
		ordinal, exists, err := index.PathIndex(path)
		if err != nil || !exists {
			t.Fatalf("path sample %q exists=%v err=%v", path, exists, err)
		}
		if _, exists, err := index.Row(ordinal); err != nil || !exists {
			t.Fatalf("path row %q exists=%v err=%v", path, exists, err)
		}
		return time.Since(started)
	}
	for index := 0; index < count; index++ {
		if index&1 == 0 {
			baseSamples = append(baseSamples, measure(base))
			tieredSamples = append(tieredSamples, measure(tiered))
		} else {
			tieredSamples = append(tieredSamples, measure(tiered))
			baseSamples = append(baseSamples, measure(base))
		}
	}
	sort.Slice(baseSamples, func(i, j int) bool { return baseSamples[i] < baseSamples[j] })
	sort.Slice(tieredSamples, func(i, j int) bool { return tieredSamples[i] < tieredSamples[j] })
	return baseSamples, tieredSamples
}

type nameIndexReader interface {
	CandidateNamePage(string, int, int, int) ([]uint32, int, bool, error)
	Row(uint32) (catalog.Row, bool, error)
}

func pairedNameIndexSamples(t *testing.T, base, tiered nameIndexReader, name string, count int) ([]time.Duration, []time.Duration) {
	t.Helper()
	baseSamples := make([]time.Duration, 0, count)
	tieredSamples := make([]time.Duration, 0, count)
	measure := func(index nameIndexReader) time.Duration {
		started := time.Now()
		ordinals, _, exceeded, err := index.CandidateNamePage(name, 0, 100, 100_000)
		if err != nil || exceeded {
			t.Fatalf("name sample %q exceeded=%v err=%v", name, exceeded, err)
		}
		for _, ordinal := range ordinals {
			if _, exists, err := index.Row(ordinal); err != nil || !exists {
				t.Fatalf("name row %q exists=%v err=%v", name, exists, err)
			}
		}
		return time.Since(started)
	}
	for index := 0; index < count; index++ {
		if index&1 == 0 {
			baseSamples = append(baseSamples, measure(base))
			tieredSamples = append(tieredSamples, measure(tiered))
		} else {
			tieredSamples = append(tieredSamples, measure(tiered))
			baseSamples = append(baseSamples, measure(base))
		}
	}
	sort.Slice(baseSamples, func(i, j int) bool { return baseSamples[i] < baseSamples[j] })
	sort.Slice(tieredSamples, func(i, j int) bool { return tieredSamples[i] < tieredSamples[j] })
	return baseSamples, tieredSamples
}
