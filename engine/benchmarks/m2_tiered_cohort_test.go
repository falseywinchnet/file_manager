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
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/generation"
	"filemanager/engine/internal/workload"
)

// TestTieredCohortCompactionDistribution compacts only each completed cohort
// of eight fresh runs. Full checkpoint segments are used solely as exact diff
// oracles and excluded from candidate write accounting; a production cohort
// compactor must stream from the preceding checked composite instead.
func TestTieredCohortCompactionDistribution(t *testing.T) {
	if os.Getenv(m2MeasurementEnvironment) != "1" {
		t.Skipf("set %s=1 to run tiered-cohort measurements", m2MeasurementEnvironment)
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
			cohortBase := base
			var cohortCheckpoint *generation.Reader
			defer func() {
				if cohortCheckpoint != nil {
					_ = cohortCheckpoint.Close()
				}
			}()
			var storageReaders []*generation.DeltaReader
			var storageSizes []uint64
			var freshReaders []*generation.DeltaReader
			var current *generation.OverlayCandidate
			defer func() {
				for _, reader := range storageReaders {
					_ = reader.Close()
				}
			}()
			var freshWriteBytes, cohortWriteBytes, canonicalLogicalBytes uint64
			cohorts := 0

			for epoch := 1; epoch <= finalEpoch; epoch++ {
				corpus, _, err := mixedScaleCorpusEpochs(recordCount, batch, epoch)
				if err != nil {
					t.Fatal(err)
				}
				updated, err := corpus.ReferenceShard(root)
				if err != nil {
					t.Fatal(err)
				}
				corpus = workload.Corpus{}
				freshPath := filepath.Join(t.TempDir(), fmt.Sprintf("fresh-%02d.delta", epoch))
				var freshMetadata generation.DeltaMetadata
				if current == nil {
					freshMetadata, err = generation.WriteDeltaCandidate(context.Background(), freshPath, api.Generation(epoch+1), base, updated)
				} else {
					freshMetadata, err = generation.WriteDeltaFromOverlayCandidate(context.Background(), freshPath, api.Generation(epoch+1), current, updated)
				}
				if err != nil {
					t.Fatal(err)
				}
				if freshMetadata.Changes() != uint64(batch) {
					t.Fatalf("epoch %d changes=%d want=%d", epoch, freshMetadata.Changes(), batch)
				}
				fresh, err := generation.OpenDeltaCandidate(freshPath, root)
				if err != nil {
					t.Fatal(err)
				}
				storageReaders = append(storageReaders, fresh)
				storageSizes = append(storageSizes, freshMetadata.Size+deltaCandidateHeaderRewriteBytes)
				freshReaders = append(freshReaders, fresh)
				freshWriteBytes += freshMetadata.Size + deltaCandidateHeaderRewriteBytes
				if err := fresh.Iterate(context.Background(), func(_ generation.ChangeKind, row catalog.Row) error {
					canonicalLogicalBytes += canonicalDeltaOperationBytes(row)
					return nil
				}); err != nil {
					t.Fatal(err)
				}

				if len(freshReaders) == 8 {
					cohorts++
					cohortPath := filepath.Join(t.TempDir(), fmt.Sprintf("cohort-%02d.delta", cohorts))
					started := time.Now()
					cohortMetadata, err := generation.WriteDeltaCandidate(
						context.Background(), cohortPath, api.Generation(epoch+1), cohortBase, updated,
					)
					if err != nil {
						t.Fatal(err)
					}
					cohortElapsed := time.Since(started)
					cohort, err := generation.OpenDeltaCandidate(cohortPath, root)
					if err != nil {
						t.Fatal(err)
					}
					for _, reader := range freshReaders {
						if err := reader.Close(); err != nil {
							cohort.Close()
							t.Fatal(err)
						}
					}
					storageReaders = append(storageReaders[:len(storageReaders)-len(freshReaders)], cohort)
					storageSizes = append(storageSizes[:len(storageSizes)-len(freshReaders)], cohortMetadata.Size+deltaCandidateHeaderRewriteBytes)
					freshReaders = nil
					cohortWriteBytes += cohortMetadata.Size + deltaCandidateHeaderRewriteBytes

					checkpointPath := filepath.Join(t.TempDir(), fmt.Sprintf("oracle-checkpoint-%02d.seg", cohorts))
					if _, err := generation.Write(checkpointPath, api.Generation(epoch+1), updated); err != nil {
						cohort.Close()
						t.Fatal(err)
					}
					nextCohortBase, err := generation.Open(checkpointPath, root)
					if err != nil {
						cohort.Close()
						t.Fatal(err)
					}
					if cohortCheckpoint != nil {
						if err := cohortCheckpoint.Close(); err != nil {
							nextCohortBase.Close()
							t.Fatal(err)
						}
					}
					cohortCheckpoint = nextCohortBase
					cohortBase = cohortCheckpoint
					t.Logf("tiered-cohort records=%d batch=%d cohort=%d epochs=%d cohort_changes=%d cohort_elapsed=%s fresh_write_bytes=%d cohort_write_bytes=%d canonical_logical_bytes=%d cumulative_wa=%.4f visible_runs=%d",
						recordCount, batch, cohorts, epoch, cohortMetadata.Changes(), cohortElapsed,
						freshWriteBytes, cohortWriteBytes, canonicalLogicalBytes,
						float64(freshWriteBytes+cohortWriteBytes)/float64(canonicalLogicalBytes), len(storageReaders),
					)
				}

				if len(storageReaders) > 8 {
					t.Fatalf("epoch %d tiered chain exposed %d runs", epoch, len(storageReaders))
				}
				current, err = generation.NewMultiRunOverlayCandidate(context.Background(), base, storageReaders, 8, finalEpoch*batch)
				if err != nil {
					t.Fatal(err)
				}
				if current.Generation() != api.Generation(epoch+1) || current.Digest() != updated.Digest() {
					t.Fatalf("epoch %d tiered overlay differs from exact target", epoch)
				}
				updated = nil
			}

			runtime.GC()
			var retained runtime.MemStats
			runtime.ReadMemStats(&retained)
			var liveRunBytes uint64
			for _, size := range storageSizes {
				liveRunBytes += size
			}
			t.Logf("tiered-cohort-final records=%d batch=%d epochs=%d cohorts=%d fresh_tail=%d visible_runs=%d changed_paths=%d retained_rows=%d live_run_bytes=%d fresh_write_bytes=%d cohort_write_bytes=%d canonical_logical_bytes=%d cumulative_wa=%.4f retained_heap_growth=%d process_heap_alloc=%d",
				recordCount, batch, finalEpoch, cohorts, len(freshReaders), len(storageReaders), current.ChangedPathCount(), current.RetainedRowCount(),
				liveRunBytes, freshWriteBytes, cohortWriteBytes, canonicalLogicalBytes,
				float64(freshWriteBytes+cohortWriteBytes)/float64(canonicalLogicalBytes),
				positiveDifference(retained.HeapAlloc, heapBefore.HeapAlloc), retained.HeapAlloc,
			)
			if cohorts != 2 || len(freshReaders) != 6 || len(storageReaders) != 8 || current.Generation() != finalEpoch+1 {
				t.Fatalf("unexpected final tiered state: cohorts=%d fresh=%d runs=%d generation=%d", cohorts, len(freshReaders), len(storageReaders), current.Generation())
			}
		})
	}
}
