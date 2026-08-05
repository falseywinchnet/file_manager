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

const repeatedCycleSegmentHeaderRewriteBytes = 512

// TestRepeatedConsolidationAndReplacementBaseDistribution measures the write
// cost of repeatedly enforcing an eight-run bound. It is a component control:
// no manifest schema, live service path, or compaction trigger is admitted.
func TestRepeatedConsolidationAndReplacementBaseDistribution(t *testing.T) {
	if os.Getenv(m2MeasurementEnvironment) != "1" {
		t.Skipf("set %s=1 to run repeated-cycle measurements", m2MeasurementEnvironment)
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
			var activeReaders []*generation.DeltaReader
			var current *generation.OverlayCandidate
			defer func() {
				for _, reader := range activeReaders {
					_ = reader.Close()
				}
			}()
			var applicationWriteBytes, canonicalLogicalBytes uint64
			cycle := 0

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
				runPath := filepath.Join(t.TempDir(), fmt.Sprintf("fresh-%02d.delta", epoch))
				var metadata generation.DeltaMetadata
				if current == nil {
					metadata, err = generation.WriteDeltaCandidate(context.Background(), runPath, api.Generation(epoch+1), base, updated)
				} else {
					metadata, err = generation.WriteDeltaFromOverlayCandidate(context.Background(), runPath, api.Generation(epoch+1), current, updated)
				}
				if err != nil {
					t.Fatal(err)
				}
				if metadata.Changes() != uint64(batch) {
					t.Fatalf("epoch %d changes=%d want=%d", epoch, metadata.Changes(), batch)
				}
				fresh, err := generation.OpenDeltaCandidate(runPath, root)
				if err != nil {
					t.Fatal(err)
				}
				activeReaders = append(activeReaders, fresh)
				applicationWriteBytes += metadata.Size + deltaCandidateHeaderRewriteBytes
				if err := fresh.Iterate(context.Background(), func(_ generation.ChangeKind, row catalog.Row) error {
					canonicalLogicalBytes += canonicalDeltaOperationBytes(row)
					return nil
				}); err != nil {
					t.Fatal(err)
				}
				current, err = generation.NewMultiRunOverlayCandidate(context.Background(), base, activeReaders, 8, finalEpoch*batch)
				if err != nil {
					t.Fatal(err)
				}
				if current.Generation() != api.Generation(epoch+1) || current.Digest() != updated.Digest() {
					t.Fatalf("epoch %d overlay differs from exact target", epoch)
				}
				if len(activeReaders) < 8 {
					updated = nil
					continue
				}

				cycle++
				beforeConsolidationBytes := applicationWriteBytes
				consolidatedPath := filepath.Join(t.TempDir(), fmt.Sprintf("cycle-%02d.delta", cycle))
				started := time.Now()
				consolidatedMetadata, err := generation.WritePacedConsolidatedDeltaCandidate(
					context.Background(), consolidatedPath, current,
					generation.ConsolidationPacingCandidate{OperationsPerPause: multiRunPacingOperations, Pause: multiRunPacingPause},
				)
				if err != nil {
					t.Fatal(err)
				}
				consolidationElapsed := time.Since(started)
				applicationWriteBytes += consolidatedMetadata.Size + deltaCandidateHeaderRewriteBytes
				consolidated, err := generation.OpenDeltaCandidate(consolidatedPath, root)
				if err != nil {
					t.Fatal(err)
				}
				consolidatedView, err := generation.NewOverlayCandidate(context.Background(), base, consolidated, finalEpoch*batch)
				if err != nil {
					consolidated.Close()
					t.Fatal(err)
				}
				if consolidatedView.Generation() != api.Generation(epoch+1) || consolidatedView.Digest() != updated.Digest() {
					consolidated.Close()
					t.Fatalf("cycle %d consolidated view differs from exact target", cycle)
				}

				replacementPath := filepath.Join(t.TempDir(), fmt.Sprintf("replacement-%02d.seg", cycle))
				replacementStarted := time.Now()
				replacementMetadata, err := generation.Write(replacementPath, api.Generation(epoch+1), updated)
				if err != nil {
					consolidated.Close()
					t.Fatal(err)
				}
				replacementElapsed := time.Since(replacementStarted)
				replacementBytes := replacementMetadata.Size + repeatedCycleSegmentHeaderRewriteBytes
				t.Logf("repeated-cycle records=%d batch=%d cycle=%d epochs=%d active_runs_before=%d net_changed_paths=%d canonical_logical_bytes=%d cumulative_run_and_consolidation_bytes=%d cumulative_consolidation_wa=%.4f replacement_base_bytes=%d replace_instead_of_cycle_bytes=%d replacement_schedule_wa=%.4f consolidation_elapsed=%s replacement_elapsed=%s",
					recordCount, batch, cycle, epoch, len(activeReaders), current.ChangedPathCount(), canonicalLogicalBytes,
					applicationWriteBytes, float64(applicationWriteBytes)/float64(canonicalLogicalBytes), replacementBytes,
					beforeConsolidationBytes+replacementBytes, float64(beforeConsolidationBytes+replacementBytes)/float64(canonicalLogicalBytes),
					consolidationElapsed, replacementElapsed,
				)

				for _, reader := range activeReaders {
					if err := reader.Close(); err != nil {
						consolidated.Close()
						t.Fatal(err)
					}
				}
				activeReaders = []*generation.DeltaReader{consolidated}
				current = consolidatedView
				updated = nil
				runtime.GC()
			}
			if cycle != 3 {
				t.Fatalf("completed %d consolidation cycles, want 3", cycle)
			}
		})
	}
}
